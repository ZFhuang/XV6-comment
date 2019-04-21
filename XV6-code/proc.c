#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"

struct {
	//进程表结构体
	struct spinlock lock;
	struct proc proc[NPROC];
} ptable;

static struct proc *initproc;

//下个pid的分配值，设定为1不会冲突
int nextpid = 1;
extern void forkret(void);
extern void trapret(void);

static void wakeup1(void *chan);

void
pinit(void)
{
	initlock(&ptable.lock, "ptable");
}

// Must be called with interrupts disabled
int
cpuid() {
	return mycpu() - cpus;
}

// Must be called with interrupts disabled to avoid the caller being
// rescheduled between reading lapicid and running through the loop.
//返回现在在用的CPU
struct cpu*
	mycpu(void)
{
	int apicid, i;

	if (readeflags()&FL_IF)
		panic("mycpu called with interrupts enabled\n");

	apicid = lapicid();
	// APIC IDs are not guaranteed to be contiguous. Maybe we should have
	// a reverse map, or reserve a register to store &cpus[i].
	for (i = 0; i < ncpu; ++i) {
		if (cpus[i].apicid == apicid)
			//找到apicid==当前ID的cpu返回
			return &cpus[i];
	}
	panic("unknown apicid\n");
}

// Disable interrupts so that we are not rescheduled
// while reading proc from the cpu structure
//返回现在的proc
struct proc*
	myproc(void) {
	struct cpu *c;
	struct proc *p;
	pushcli();
	c = mycpu();
	p = c->proc;
	popcli();
	return p;
}

//PAGEBREAK: 32
// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
static struct proc*
allocproc(void)
{
	struct proc *p;
	char *sp;

	//申请PCB表的访问
	acquire(&ptable.lock);

	for (p = ptable.proc; p < &ptable.proc[NPROC]; p++)
		if (p->state == UNUSED)
			//找到可使用的unusedPCB，跳转
			goto found;

	//否则释放锁并返回NULL
	release(&ptable.lock);
	return 0;

found:
	//萌芽中
	p->state = EMBRYO;
	//分配新的pid，nextpid的初始值为1，不会与子进程冲突
	p->pid = nextpid++;

	release(&ptable.lock);

	// Allocate kernel stack.
	if ((p->kstack = kalloc()) == 0) {
		p->state = UNUSED;
		return 0;
	}
	sp = p->kstack + KSTACKSIZE;

	// Leave room for trap frame.
	sp -= sizeof *p->tf;
	p->tf = (struct trapframe*)sp;

	// Set up new context to start executing at forkret,
	// which returns to trapret.
	sp -= 4;
	*(uint*)sp = (uint)trapret;

	sp -= sizeof *p->context;
	p->context = (struct context*)sp;
	memset(p->context, 0, sizeof *p->context);
	p->context->eip = (uint)forkret;

	return p;
}

//PAGEBREAK: 32
// Set up first user process.
void
userinit(void)
{
	struct proc *p;
	extern char _binary_initcode_start[], _binary_initcode_size[];

	p = allocproc();

	initproc = p;
	if ((p->pgdir = setupkvm()) == 0)
		panic("userinit: out of memory?");
	inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
	p->sz = PGSIZE;
	memset(p->tf, 0, sizeof(*p->tf));
	p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
	p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
	p->tf->es = p->tf->ds;
	p->tf->ss = p->tf->ds;
	p->tf->eflags = FL_IF;
	p->tf->esp = PGSIZE;
	p->tf->eip = 0;  // beginning of initcode.S

	safestrcpy(p->name, "initcode", sizeof(p->name));
	p->cwd = namei("/");

	// this assignment to p->state lets other cores
	// run this process. the acquire forces the above
	// writes to be visible, and the lock is also needed
	// because the assignment might not be atomic.
	acquire(&ptable.lock);

	p->state = RUNNABLE;

	release(&ptable.lock);
}

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
int
growproc(int n)
{
	uint sz;
	struct proc *curproc = myproc();

	sz = curproc->sz;
	if (n > 0) {
		if ((sz = allocuvm(curproc->pgdir, sz, sz + n)) == 0)
			return -1;
	}
	else if (n < 0) {
		if ((sz = deallocuvm(curproc->pgdir, sz, sz + n)) == 0)
			return -1;
	}
	curproc->sz = sz;
	switchuvm(curproc);
	return 0;
}

// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
//分支出一个新的进程
int
fork(void)
{
	int i, pid;
	struct proc *np;
	struct proc *curproc = myproc();

	// Allocate process.分配进程
	if ((np = allocproc()) == 0) {
		//失败时返回-1
		return -1;
	}

	// Copy process state from proc.复制状态
	if ((np->pgdir = copyuvm(curproc->pgdir, curproc->sz)) == 0) {
		kfree(np->kstack);
		np->kstack = 0;
		np->state = UNUSED;
		return -1;
	}
	np->sz = curproc->sz;
	np->parent = curproc;
	//给新PCB一样的栈帧，所以新进程从这里继续
	*np->tf = *curproc->tf;

	// Clear %eax so that fork returns 0 in the child.
	//置零新进程的eax，使得新进程的fork可以返回0表示自己是子进程
	np->tf->eax = 0;

	for (i = 0; i < NOFILE; i++)
		if (curproc->ofile[i])
			np->ofile[i] = filedup(curproc->ofile[i]);
	np->cwd = idup(curproc->cwd);

	safestrcpy(np->name, curproc->name, sizeof(curproc->name));

	//新进程的pid
	pid = np->pid;

	//锁
	acquire(&ptable.lock);

	//设置新进程PCB为runnable
	np->state = RUNNABLE;

	//释放锁
	release(&ptable.lock);

	//返回新进程的pid的值
	return pid;
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
void
exit(void)
{
	struct proc *curproc = myproc();
	struct proc *p;
	int fd;

	if (curproc == initproc)
		panic("init exiting");

	// Close all open files.
	for (fd = 0; fd < NOFILE; fd++) {
		if (curproc->ofile[fd]) {
			fileclose(curproc->ofile[fd]);
			curproc->ofile[fd] = 0;
		}
	}

	begin_op();
	iput(curproc->cwd);
	end_op();
	curproc->cwd = 0;

	acquire(&ptable.lock);

	// Parent might be sleeping in wait().
	wakeup1(curproc->parent);

	// Pass abandoned children to init.
	for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
		if (p->parent == curproc) {
			p->parent = initproc;
			if (p->state == ZOMBIE)
				wakeup1(initproc);
		}
	}

	// Jump into the scheduler, never to return.
	curproc->state = ZOMBIE;
	sched();
	panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
wait(void)
{
	struct proc *p;
	int havekids, pid;
	struct proc *curproc = myproc();

	acquire(&ptable.lock);
	for (;;) {
		// Scan through table looking for exited children.
		havekids = 0;
		for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
			if (p->parent != curproc)
				continue;
			havekids = 1;
			if (p->state == ZOMBIE) {
				// Found one.
				pid = p->pid;
				kfree(p->kstack);
				p->kstack = 0;
				freevm(p->pgdir);
				p->pid = 0;
				p->parent = 0;
				p->name[0] = 0;
				p->killed = 0;
				p->state = UNUSED;
				release(&ptable.lock);
				return pid;
			}
		}

		// No point waiting if we don't have any children.
		if (!havekids || curproc->killed) {
			release(&ptable.lock);
			return -1;
		}

		// Wait for children to exit.  (See wakeup1 call in proc_exit.)
		sleep(curproc, &ptable.lock);  //DOC: wait-sleep
	}
}

//PAGEBREAK: 42
// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.
//调度器是一个暂时进入的新线程，通过切换上下文进来的
//调度本质上就是选择要给CPU运行的合适的上下文
//调度器线程是永不返回的，会持续进行选择新进程，通过切上下文来保持内部信息
void
scheduler(void)
{
	//先得到目前的cpu，这里只会进入一次
	struct proc *p;
	struct cpu *c = mycpu();
	//cpu的当前进程设0初始化，
	c->proc = 0;

	for (;;) {
		// Enable interrupts on this processor.
		//要在调度时打开中断，这是为了防止当所有进程都在等待IO时，由于关闭中断而产生的死锁问题
		sti();

		// Loop over process table looking for process to run.
		//求一个锁，此时是调度器线程的锁，和外面的不一样所以不会panic
		acquire(&ptable.lock);
		for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
			//有点像fork在找进程PCB表，只不过这次是要找到第一个RUNNABLE的进程
			//这里也说明了xv6的调度算法其实就是简单的轮转法
			if (p->state != RUNNABLE)
				continue;

			// Switch to chosen process.  It is the process's job
			// to release ptable.lock and then reacquire it
			// before jumping back to us.
			//将当前进程标识选为找到的第一个RUNNABLE进程
			c->proc = p;
			//切换到进程页表
			switchuvm(p);
			//设置接下来要运行的这个进程是RUNNING
			p->state = RUNNING;

			//在这里调度器再将上下文改到这个选中的进程的上下文中同时保存了调度器的上下文
			//这样就通过切换上下文在保留调度器循环暂停的情况下把CPU交给了新进程
			swtch(&(c->scheduler), p->context);
			//回到这里的时候，因为调度器是运行在内核的，切换到内核页表
			switchkvm();

			// Process is done running for now.
			// It should have changed its p->state before coming back.
			//cpu的当前进程又设0了，这样就恢复到了循环开始的时候，继续下个调度
			c->proc = 0;
		}
		//调度器完成了一圈进程表的遍历，释放锁然后继续循环
		//这个循环是没有返回值，永无休止的
		//释放锁是因为不应该让一个闲置状态的调度器一直把持着进程锁
		release(&ptable.lock);

	}
}

// Enter scheduler.  Must hold only ptable.lock
//只有当获得了进程锁时才能进入调度器
// and have changed proc->state. Saves and restores
//且要改变了RUNNING的进程
// intena because intena is a property of this
//需要保存和恢复中断标识？
// kernel thread, not this CPU. It should
//因为这个中断标识的可能是这个内核调度器线程而非原先进程的
// be proc->intena and proc->ncli, but that would
// break in the few places where a lock is held but
// there's no process.
//当有些进程没有锁时会出问题
void
sched(void)
{
	int intena;
	struct proc *p = myproc();

	//再检测下是否保持着这个锁
	if (!holding(&ptable.lock))
		panic("sched ptable.lock");
	//只有第一层cli才是得到了锁的调用，其它的都在队列里还没轮到
	if (mycpu()->ncli != 1)
		panic("sched locks");
	//必须是停止运行的进程，例如RUNNABLE
	if (p->state == RUNNING)
		panic("sched running");
	//中断关闭时，这句还不是很明白(?)
	if (readeflags()&FL_IF)
		panic("sched interruptible");
	//保留原先的中断标识
	intena = mycpu()->intena;
	//参数里有此进程的寄存器上下文和这个CPU的调度器上下文
	//在这要来改变这个进程的上下文，将上下文改为scheduler的
	//也就是通过暂时进入了调度器线程
	swtch(&p->context, mycpu()->scheduler);
	//恢复中断标识
	mycpu()->intena = intena;
}

// Give up the CPU for one scheduling round.
//让这个进程让步CPU的调度周期
void
yield(void)
{
	//请求进程锁，因为要来修改进程信息了，已经被锁的情况会panic
	//请求锁是因为下面需要修改到proc了
	acquire(&ptable.lock);  //DOC: yieldlock
	//将状态让出，当前的进程设置为RUNNABLE
	myproc()->state = RUNNABLE;
	//主动调用触发调度
	sched();
	//释放锁
	release(&ptable.lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void
forkret(void)
{
	static int first = 1;
	// Still holding ptable.lock from scheduler.
	release(&ptable.lock);

	if (first) {
		// Some initialization functions must be run in the context
		// of a regular process (e.g., they call sleep), and thus cannot
		// be run from main().
		first = 0;
		iinit(ROOTDEV);
		initlog(ROOTDEV);
	}

	// Return to "caller", actually trapret (see allocproc).
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
//自动地释放锁并将进程放入睡眠等待队列中
//要求此进程必须拥有进程锁防止wakeup
void
sleep(void *chan, struct spinlock *lk)
{
	struct proc *p = myproc();

	if (p == 0)
		panic("sleep");

	//没有锁时报错
	if (lk == 0)
		panic("sleep without lk");

	// Must acquire ptable.lock in order to
	// change p->state and then call sched.
	// Once we hold ptable.lock, we can be
	// guaranteed that we won't miss any wakeup
	// (wakeup runs with ptable.lock locked),
	// so it's okay to release lk.
	//强制要求进程带锁而入因为这样可以防止我们错过release前的wakeup
	//在有锁的情况下wakeup无法在这启动
	if (lk != &ptable.lock) {  //DOC: sleeplock0
		//由于如果带进来的锁是进程表锁，那么不能随便释放它
		//当不是进程表锁时，可以放心要求一个进程表锁保持住无法wakeup的状态
		acquire(&ptable.lock);  //DOC: sleeplock1
		//因为有了进程表了就可以放心释放原来的锁然后可以修改这个进程
		release(lk);
	}
	// Go to sleep.
	//将其放入睡眠队列
	p->chan = chan;
	//转变状态
	p->state = SLEEPING;

	//类似yield主动调用调度
	sched();

	// Tidy up. 收拾，把它从睡眠队列移出
	p->chan = 0;

	// Reacquire original lock.
	if (lk != &ptable.lock) {  //DOC: sleeplock2
		//对称地释放进程表锁要求进程锁
		release(&ptable.lock);
		acquire(lk);
	}
	//本来就是进程表锁时跳过(什么情况下带入的是进程表锁?)
}

//PAGEBREAK!
// Wake up all processes sleeping on chan.
// The ptable lock must be held.
//唤醒操作的真正地方，必须要有锁
//唤醒此睡眠队列中的所有进程
static void
wakeup1(void *chan)
{
	struct proc *p;

	for (p = ptable.proc; p < &ptable.proc[NPROC]; p++)
		if (p->state == SLEEPING && p->chan == chan)
			//找到进程表中处于睡眠且处于队列中的进程，都修改为RUNNABLE
			p->state = RUNNABLE;
}

// Wake up all processes sleeping on chan.
//唤醒睡眠队列
void
wakeup(void *chan)
{
	//由于要修改进程状态自然需要进程表锁
	acquire(&ptable.lock);
	//调用上面的wakeup1来做真正的处理
	wakeup1(chan);
	//释放
	release(&ptable.lock);
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int
kill(int pid)
{
	struct proc *p;

	acquire(&ptable.lock);
	for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
		if (p->pid == pid) {
			p->killed = 1;
			// Wake process from sleep if necessary.
			if (p->state == SLEEPING)
				p->state = RUNNABLE;
			release(&ptable.lock);
			return 0;
		}
	}
	release(&ptable.lock);
	return -1;
}

//PAGEBREAK: 36
// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
//Ctrl+P来打印运行中的进程，没有申请进程来防止卡住机器
void
procdump(void)
{
	//将进程状态符转为字符串
	//这个语法？？
	static char *states[] = {
	[UNUSED]    "unused",
	[EMBRYO]    "embryo",
	[SLEEPING]  "sleep ",
	[RUNNABLE]  "runble",
	[RUNNING]   "run   ",
	[ZOMBIE]    "zombie"
	};
	//几个暂存
	int i;
	struct proc *p;
	char *state;
	uint pc[10];

	for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
		//循环遍历进程表
		if (p->state == UNUSED)
			//未使用的进程无需打印
			continue;
		//state>0不是UNUSED，< NELEM(states)范围内，states[p->state]状态存在
		if (p->state >= 0 && p->state < NELEM(states) && states[p->state])
			//得到对应的字符串
			state = states[p->state];
		else
			//否则这个状态是在预计外，没有对应的字符串
			state = "???";
		//将进程pid，状态和进程名打印
		cprintf("%d %s %s", p->pid, state, p->name);
		if (p->state == SLEEPING) {
			//得到SLEEPING进程的调用者(为啥找不到定义)
			getcallerpcs((uint*)p->context->ebp + 2, pc);
			for (i = 0; i < 10 && pc[i] != 0; i++)
				//打印出每个调用的pc
				cprintf(" %p", pc[i]);
		}
		cprintf("\n");
	}
}
