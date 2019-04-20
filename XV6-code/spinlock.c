// Mutual exclusion spin locks.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"

//初始化这个锁
void
initlock(struct spinlock *lk, char *name)
{
	//一些初始信息，写入带进来的名字
	lk->name = name;
	lk->locked = 0;
	lk->cpu = 0;
}

// Acquire the lock.
// Loops (spins) until the lock is acquired.
// Holding a lock for a long time may cause
// other CPUs to waste time spinning to acquire it.
//当要修改内核的临界区时，需要设置进程锁并停止中断操作
//spinlock自旋锁:当进程拿不到锁时会进入无限循环，直到拿到锁退出循环
//通过acquire可以原语地修改lock
void
acquire(struct spinlock *lk)
{
	// disable interrupts to avoid deadlock.
	//一上来就禁止中断发生来防止死锁，且这是可以多层嵌套的pushcli
	//这是因为如果一个进程得到锁后，触发中断，另一个进程请求锁就会发生死锁
	pushcli(); 
	//检验是否被锁
	if (holding(lk))
		//当发现有锁了，请求失败，已被占用
		panic("acquire");

	// The xchg is atomic.
	while (xchg(&lk->locked, 1) != 0)
		;

	// Tell the C compiler and the processor to not move loads or stores
	// past this point, to ensure that the critical section's memory
	// references happen after the lock is acquired.
	__sync_synchronize();

	// Record info about lock acquisition for debugging.记录下这个锁的归属
	lk->cpu = mycpu();
	//记录下这个锁住的CPU的程序计数器PC
	getcallerpcs(&lk, lk->pcs);
}

// Release the lock.
//释放锁，一个aquire对应一个release，这是因为有pushcli的存在
void
release(struct spinlock *lk)
{
	if (!holding(lk))
		panic("release");

	lk->pcs[0] = 0;
	lk->cpu = 0;

	// Tell the C compiler and the processor to not move loads or stores
	// past this point, to ensure that all the stores in the critical
	// section are visible to other cores before the lock is released.
	// Both the C compiler and the hardware may re-order loads and
	// stores; __sync_synchronize() tells them both not to.
	__sync_synchronize();

	// Release the lock, equivalent to lk->locked = 0.
	// This code can't use a C assignment, since it might
	// not be atomic. A real OS would use C atomics here.
	asm volatile("movl $0, %0" : "+m" (lk->locked) : );

	//通过popcli来减少锁的计数，需要和push成对
	popcli();
}

// Record the current call stack in pcs[] by following the %ebp chain.
//记录下这个锁住的CPU的程序计数器PC
void
getcallerpcs(void *v, uint pcs[])
{
	uint *ebp;
	int i;

	ebp = (uint*)v - 2;
	for (i = 0; i < 10; i++) {
		if (ebp == 0 || ebp < (uint*)KERNBASE || ebp == (uint*)0xffffffff)
			break;
		pcs[i] = ebp[1];     // saved %eip
		ebp = (uint*)ebp[0]; // saved %ebp
	}
	for (; i < 10; i++)
		pcs[i] = 0;
}

// Check whether this cpu is holding the lock.
//查看cpu是否有锁，有锁返回true
int
holding(struct spinlock *lock)
{
	int r;
	pushcli();
	//返回cpu是否有锁，锁锁住&&锁的cpu是当前进程的cpu
	r = lock->locked && lock->cpu == mycpu();
	popcli();
	return r;
}


// Pushcli/popcli are like cli/sti except that they are matched:
// it takes two popcli to undo two pushcli.  Also, if interrupts
// are off, then pushcli, popcli leaves them off.

//关闭中断，会增加锁的计数
void
pushcli(void)
{
	int eflags;

	//得到个读取标识
	eflags = readeflags();
	//？
	cli();
	if (mycpu()->ncli == 0)
		//当这是此CPU的第一个锁时，关闭中断
		mycpu()->intena = eflags & FL_IF;
	mycpu()->ncli += 1;	//锁加一层
}

//恢复中断，会减少(弹出)锁的计数
void
popcli(void)
{
	if (readeflags()&FL_IF)
		panic("popcli - interruptible");
	//将计数减1
	if (--mycpu()->ncli < 0)
		panic("popcli");
	//当已经没有计数了，就恢复中断，关闭锁
	if (mycpu()->ncli == 0 && mycpu()->intena)
		//恢复中断
		sti();
}

