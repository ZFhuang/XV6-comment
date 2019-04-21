#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"

struct {
	//���̱�ṹ��
	struct spinlock lock;
	struct proc proc[NPROC];
} ptable;

static struct proc *initproc;

//�¸�pid�ķ���ֵ���趨Ϊ1�����ͻ
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
//�����������õ�CPU
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
			//�ҵ�apicid==��ǰID��cpu����
			return &cpus[i];
	}
	panic("unknown apicid\n");
}

// Disable interrupts so that we are not rescheduled
// while reading proc from the cpu structure
//�������ڵ�proc
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

	//����PCB��ķ���
	acquire(&ptable.lock);

	for (p = ptable.proc; p < &ptable.proc[NPROC]; p++)
		if (p->state == UNUSED)
			//�ҵ���ʹ�õ�unusedPCB����ת
			goto found;

	//�����ͷ���������NULL
	release(&ptable.lock);
	return 0;

found:
	//��ѿ��
	p->state = EMBRYO;
	//�����µ�pid��nextpid�ĳ�ʼֵΪ1���������ӽ��̳�ͻ
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
//��֧��һ���µĽ���
int
fork(void)
{
	int i, pid;
	struct proc *np;
	struct proc *curproc = myproc();

	// Allocate process.�������
	if ((np = allocproc()) == 0) {
		//ʧ��ʱ����-1
		return -1;
	}

	// Copy process state from proc.����״̬
	if ((np->pgdir = copyuvm(curproc->pgdir, curproc->sz)) == 0) {
		kfree(np->kstack);
		np->kstack = 0;
		np->state = UNUSED;
		return -1;
	}
	np->sz = curproc->sz;
	np->parent = curproc;
	//����PCBһ����ջ֡�������½��̴��������
	*np->tf = *curproc->tf;

	// Clear %eax so that fork returns 0 in the child.
	//�����½��̵�eax��ʹ���½��̵�fork���Է���0��ʾ�Լ����ӽ���
	np->tf->eax = 0;

	for (i = 0; i < NOFILE; i++)
		if (curproc->ofile[i])
			np->ofile[i] = filedup(curproc->ofile[i]);
	np->cwd = idup(curproc->cwd);

	safestrcpy(np->name, curproc->name, sizeof(curproc->name));

	//�½��̵�pid
	pid = np->pid;

	//��
	acquire(&ptable.lock);

	//�����½���PCBΪrunnable
	np->state = RUNNABLE;

	//�ͷ���
	release(&ptable.lock);

	//�����½��̵�pid��ֵ
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
//��������һ����ʱ��������̣߳�ͨ���л������Ľ�����
//���ȱ����Ͼ���ѡ��Ҫ��CPU���еĺ��ʵ�������
//�������߳����������صģ����������ѡ���½��̣�ͨ�����������������ڲ���Ϣ
void
scheduler(void)
{
	//�ȵõ�Ŀǰ��cpu������ֻ�����һ��
	struct proc *p;
	struct cpu *c = mycpu();
	//cpu�ĵ�ǰ������0��ʼ����
	c->proc = 0;

	for (;;) {
		// Enable interrupts on this processor.
		//Ҫ�ڵ���ʱ���жϣ�����Ϊ�˷�ֹ�����н��̶��ڵȴ�IOʱ�����ڹر��ж϶���������������
		sti();

		// Loop over process table looking for process to run.
		//��һ��������ʱ�ǵ������̵߳�����������Ĳ�һ�����Բ���panic
		acquire(&ptable.lock);
		for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
			//�е���fork���ҽ���PCB��ֻ���������Ҫ�ҵ���һ��RUNNABLE�Ľ���
			//����Ҳ˵����xv6�ĵ����㷨��ʵ���Ǽ򵥵���ת��
			if (p->state != RUNNABLE)
				continue;

			// Switch to chosen process.  It is the process's job
			// to release ptable.lock and then reacquire it
			// before jumping back to us.
			//����ǰ���̱�ʶѡΪ�ҵ��ĵ�һ��RUNNABLE����
			c->proc = p;
			//�л�������ҳ��
			switchuvm(p);
			//���ý�����Ҫ���е����������RUNNING
			p->state = RUNNING;

			//������������ٽ������ĸĵ����ѡ�еĽ��̵���������ͬʱ�����˵�������������
			//������ͨ���л��������ڱ���������ѭ����ͣ������°�CPU�������½���
			swtch(&(c->scheduler), p->context);
			//�ص������ʱ����Ϊ���������������ں˵ģ��л����ں�ҳ��
			switchkvm();

			// Process is done running for now.
			// It should have changed its p->state before coming back.
			//cpu�ĵ�ǰ��������0�ˣ������ͻָ�����ѭ����ʼ��ʱ�򣬼����¸�����
			c->proc = 0;
		}
		//�����������һȦ���̱�ı������ͷ���Ȼ�����ѭ��
		//���ѭ����û�з���ֵ��������ֹ��
		//�ͷ�������Ϊ��Ӧ����һ������״̬�ĵ�����һֱ�ѳ��Ž�����
		release(&ptable.lock);

	}
}

// Enter scheduler.  Must hold only ptable.lock
//ֻ�е�����˽�����ʱ���ܽ��������
// and have changed proc->state. Saves and restores
//��Ҫ�ı���RUNNING�Ľ���
// intena because intena is a property of this
//��Ҫ����ͻָ��жϱ�ʶ��
// kernel thread, not this CPU. It should
//��Ϊ����жϱ�ʶ�Ŀ���������ں˵������̶߳���ԭ�Ƚ��̵�
// be proc->intena and proc->ncli, but that would
// break in the few places where a lock is held but
// there's no process.
//����Щ����û����ʱ�������
void
sched(void)
{
	int intena;
	struct proc *p = myproc();

	//�ټ�����Ƿ񱣳��������
	if (!holding(&ptable.lock))
		panic("sched ptable.lock");
	//ֻ�е�һ��cli���ǵõ������ĵ��ã������Ķ��ڶ����ﻹû�ֵ�
	if (mycpu()->ncli != 1)
		panic("sched locks");
	//������ֹͣ���еĽ��̣�����RUNNABLE
	if (p->state == RUNNING)
		panic("sched running");
	//�жϹر�ʱ����仹���Ǻ�����(?)
	if (readeflags()&FL_IF)
		panic("sched interruptible");
	//����ԭ�ȵ��жϱ�ʶ
	intena = mycpu()->intena;
	//�������д˽��̵ļĴ��������ĺ����CPU�ĵ�����������
	//����Ҫ���ı�������̵������ģ��������ĸ�Ϊscheduler��
	//Ҳ����ͨ����ʱ�����˵������߳�
	swtch(&p->context, mycpu()->scheduler);
	//�ָ��жϱ�ʶ
	mycpu()->intena = intena;
}

// Give up the CPU for one scheduling round.
//����������ò�CPU�ĵ�������
void
yield(void)
{
	//�������������ΪҪ���޸Ľ�����Ϣ�ˣ��Ѿ������������panic
	//����������Ϊ������Ҫ�޸ĵ�proc��
	acquire(&ptable.lock);  //DOC: yieldlock
	//��״̬�ó�����ǰ�Ľ�������ΪRUNNABLE
	myproc()->state = RUNNABLE;
	//�������ô�������
	sched();
	//�ͷ���
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
//�Զ����ͷ����������̷���˯�ߵȴ�������
//Ҫ��˽��̱���ӵ�н�������ֹwakeup
void
sleep(void *chan, struct spinlock *lk)
{
	struct proc *p = myproc();

	if (p == 0)
		panic("sleep");

	//û����ʱ����
	if (lk == 0)
		panic("sleep without lk");

	// Must acquire ptable.lock in order to
	// change p->state and then call sched.
	// Once we hold ptable.lock, we can be
	// guaranteed that we won't miss any wakeup
	// (wakeup runs with ptable.lock locked),
	// so it's okay to release lk.
	//ǿ��Ҫ����̴���������Ϊ�������Է�ֹ���Ǵ��releaseǰ��wakeup
	//�������������wakeup�޷���������
	if (lk != &ptable.lock) {  //DOC: sleeplock0
		//������������������ǽ��̱�������ô��������ͷ���
		//�����ǽ��̱���ʱ�����Է���Ҫ��һ�����̱�������ס�޷�wakeup��״̬
		acquire(&ptable.lock);  //DOC: sleeplock1
		//��Ϊ���˽��̱��˾Ϳ��Է����ͷ�ԭ������Ȼ������޸��������
		release(lk);
	}
	// Go to sleep.
	//�������˯�߶���
	p->chan = chan;
	//ת��״̬
	p->state = SLEEPING;

	//����yield�������õ���
	sched();

	// Tidy up. ��ʰ��������˯�߶����Ƴ�
	p->chan = 0;

	// Reacquire original lock.
	if (lk != &ptable.lock) {  //DOC: sleeplock2
		//�ԳƵ��ͷŽ��̱���Ҫ�������
		release(&ptable.lock);
		acquire(lk);
	}
	//�������ǽ��̱���ʱ����(ʲô����´�����ǽ��̱���?)
}

//PAGEBREAK!
// Wake up all processes sleeping on chan.
// The ptable lock must be held.
//���Ѳ����������ط�������Ҫ����
//���Ѵ�˯�߶����е����н���
static void
wakeup1(void *chan)
{
	struct proc *p;

	for (p = ptable.proc; p < &ptable.proc[NPROC]; p++)
		if (p->state == SLEEPING && p->chan == chan)
			//�ҵ����̱��д���˯���Ҵ��ڶ����еĽ��̣����޸�ΪRUNNABLE
			p->state = RUNNABLE;
}

// Wake up all processes sleeping on chan.
//����˯�߶���
void
wakeup(void *chan)
{
	//����Ҫ�޸Ľ���״̬��Ȼ��Ҫ���̱���
	acquire(&ptable.lock);
	//���������wakeup1���������Ĵ���
	wakeup1(chan);
	//�ͷ�
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
//Ctrl+P����ӡ�����еĽ��̣�û�������������ֹ��ס����
void
procdump(void)
{
	//������״̬��תΪ�ַ���
	//����﷨����
	static char *states[] = {
	[UNUSED]    "unused",
	[EMBRYO]    "embryo",
	[SLEEPING]  "sleep ",
	[RUNNABLE]  "runble",
	[RUNNING]   "run   ",
	[ZOMBIE]    "zombie"
	};
	//�����ݴ�
	int i;
	struct proc *p;
	char *state;
	uint pc[10];

	for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
		//ѭ���������̱�
		if (p->state == UNUSED)
			//δʹ�õĽ��������ӡ
			continue;
		//state>0����UNUSED��< NELEM(states)��Χ�ڣ�states[p->state]״̬����
		if (p->state >= 0 && p->state < NELEM(states) && states[p->state])
			//�õ���Ӧ���ַ���
			state = states[p->state];
		else
			//�������״̬����Ԥ���⣬û�ж�Ӧ���ַ���
			state = "???";
		//������pid��״̬�ͽ�������ӡ
		cprintf("%d %s %s", p->pid, state, p->name);
		if (p->state == SLEEPING) {
			//�õ�SLEEPING���̵ĵ�����(Ϊɶ�Ҳ�������)
			getcallerpcs((uint*)p->context->ebp + 2, pc);
			for (i = 0; i < 10 && pc[i] != 0; i++)
				//��ӡ��ÿ�����õ�pc
				cprintf(" %p", pc[i]);
		}
		cprintf("\n");
	}
}
