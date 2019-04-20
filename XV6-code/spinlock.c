// Mutual exclusion spin locks.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"

//��ʼ�������
void
initlock(struct spinlock *lk, char *name)
{
	//һЩ��ʼ��Ϣ��д�������������
	lk->name = name;
	lk->locked = 0;
	lk->cpu = 0;
}

// Acquire the lock.
// Loops (spins) until the lock is acquired.
// Holding a lock for a long time may cause
// other CPUs to waste time spinning to acquire it.
//��Ҫ�޸��ں˵��ٽ���ʱ����Ҫ���ý�������ֹͣ�жϲ���
//spinlock������:�������ò�����ʱ���������ѭ����ֱ���õ����˳�ѭ��
//ͨ��acquire����ԭ����޸�lock
void
acquire(struct spinlock *lk)
{
	// disable interrupts to avoid deadlock.
	//һ�����ͽ�ֹ�жϷ�������ֹ�����������ǿ��Զ��Ƕ�׵�pushcli
	//������Ϊ���һ�����̵õ����󣬴����жϣ���һ�������������ͻᷢ������
	pushcli(); 
	//�����Ƿ���
	if (holding(lk))
		//�����������ˣ�����ʧ�ܣ��ѱ�ռ��
		panic("acquire");

	// The xchg is atomic.
	while (xchg(&lk->locked, 1) != 0)
		;

	// Tell the C compiler and the processor to not move loads or stores
	// past this point, to ensure that the critical section's memory
	// references happen after the lock is acquired.
	__sync_synchronize();

	// Record info about lock acquisition for debugging.��¼��������Ĺ���
	lk->cpu = mycpu();
	//��¼�������ס��CPU�ĳ��������PC
	getcallerpcs(&lk, lk->pcs);
}

// Release the lock.
//�ͷ�����һ��aquire��Ӧһ��release��������Ϊ��pushcli�Ĵ���
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

	//ͨ��popcli���������ļ�������Ҫ��push�ɶ�
	popcli();
}

// Record the current call stack in pcs[] by following the %ebp chain.
//��¼�������ס��CPU�ĳ��������PC
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
//�鿴cpu�Ƿ���������������true
int
holding(struct spinlock *lock)
{
	int r;
	pushcli();
	//����cpu�Ƿ�����������ס&&����cpu�ǵ�ǰ���̵�cpu
	r = lock->locked && lock->cpu == mycpu();
	popcli();
	return r;
}


// Pushcli/popcli are like cli/sti except that they are matched:
// it takes two popcli to undo two pushcli.  Also, if interrupts
// are off, then pushcli, popcli leaves them off.

//�ر��жϣ����������ļ���
void
pushcli(void)
{
	int eflags;

	//�õ�����ȡ��ʶ
	eflags = readeflags();
	//��
	cli();
	if (mycpu()->ncli == 0)
		//�����Ǵ�CPU�ĵ�һ����ʱ���ر��ж�
		mycpu()->intena = eflags & FL_IF;
	mycpu()->ncli += 1;	//����һ��
}

//�ָ��жϣ������(����)���ļ���
void
popcli(void)
{
	if (readeflags()&FL_IF)
		panic("popcli - interruptible");
	//��������1
	if (--mycpu()->ncli < 0)
		panic("popcli");
	//���Ѿ�û�м����ˣ��ͻָ��жϣ��ر���
	if (mycpu()->ncli == 0 && mycpu()->intena)
		//�ָ��ж�
		sti();
}

