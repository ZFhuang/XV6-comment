#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "traps.h"
#include "spinlock.h"

// Interrupt descriptor table (shared by all CPUs).
struct gatedesc idt[256];
extern uint vectors[];  // in vectors.S: array of 256 entry pointers
//ʱ�������Ǹ�������
struct spinlock tickslock;
//��ǰ�ļ�ʱ��ticks
uint ticks;

//��main���ã�������ʼ����256��IDT���Ԫ��
//x86 ���� 256 ����ͬ���жϡ��ж� 0-31 ������Ϊ����쳣
//����� 0 ����ͷ��ʷǷ����ڴ�ҳ��xv6 ���жϺ� 32-63 ӳ���Ӳ���ж�
//64Ϊϵͳ���õ��жϺš�
void
tvinit(void)
{
	int i;

	//��ѭ��������idt��ÿһ���Щ���жϣ���Ҫ���Ȩ��
	for (i = 0; i < 256; i++)
		SETGATE(idt[i], 0, SEG_KCODE << 3, vectors[i], 0);
	//�ر���ϵͳ���õ������һ�����룬�û�Ȩ��3����
	//���ϵͳ����������int $T_SYSCALL����
	SETGATE(idt[T_SYSCALL], 1, SEG_KCODE << 3, vectors[T_SYSCALL], DPL_USER);

	//��ʼ��ʱ����
	initlock(&tickslock, "time");
}

void
idtinit(void)
{
	lidt(idt, sizeof(idt));
}

//PAGEBREAK: 41
//�жϴ��������ǵ�ǰ���ж�ָ֡�룬�Ǳ�alltraps���ö���
void
trap(struct trapframe *tf)
{
	//�Ӵ�������alltraps�õ�trapno
	if (tf->trapno == T_SYSCALL) {
		//ϵͳ�����ж�
		if (myproc()->killed)
			exit();
		//�л��˽��̵ĵ�ǰ�ж�֡
		myproc()->tf = tf;
		//����syscall����ϵͳ����
		syscall();
		if (myproc()->killed)
			exit();
		//ֱ�ӷ���
		return;
	}

	//���������ж�ʱ
	switch (tf->trapno) {
	case T_IRQ0 + IRQ_TIMER:
		//��ʱӲ����ʱ���жϣ�ÿ�뱻����100�Σ�Ҳ����ÿ��ticks����10ms
		if (cpuid() == 0) {
			acquire(&tickslock);
			ticks++;
			//���Ѷ�Ӧʱ���˯�߶��е���
			wakeup(&ticks);
			release(&tickslock);
		}
		lapiceoi();
		break;
	case T_IRQ0 + IRQ_IDE:
		ideintr();
		lapiceoi();
		break;
	case T_IRQ0 + IRQ_IDE + 1:
		// Bochs generates spurious IDE1 interrupts.
		break;
	case T_IRQ0 + IRQ_KBD:
		kbdintr();
		lapiceoi();
		break;
	case T_IRQ0 + IRQ_COM1:
		uartintr();
		lapiceoi();
		break;
	case T_IRQ0 + 7:
	case T_IRQ0 + IRQ_SPURIOUS:
		cprintf("cpu%d: spurious interrupt at %x:%x\n",
			cpuid(), tf->cs, tf->eip);
		lapiceoi();
		break;

		//PAGEBREAK: 13
	default:
		if (myproc() == 0 || (tf->cs & 3) == 0) {
			// In kernel, it must be our mistake.
			cprintf("unexpected trap %d from cpu %d eip %x (cr2=0x%x)\n",
				tf->trapno, cpuid(), tf->eip, rcr2());
			panic("trap");
		}
		// In user space, assume process misbehaved.
		cprintf("pid %d %s: trap %d err %d on cpu %d "
			"eip 0x%x addr 0x%x--kill proc\n",
			myproc()->pid, myproc()->name, tf->trapno,
			tf->err, cpuid(), tf->eip, rcr2());
		myproc()->killed = 1;
	}

	// Force process exit if it has been killed and is in user space.
	// (If it is still executing in the kernel, let it keep running
	// until it gets to the regular system call return.)
	if (myproc() && myproc()->killed && (tf->cs & 3) == DPL_USER)
		exit();

	// Force process to give up CPU on clock tick.
	// If interrupts were on while locks held, would need to check nlock.
	if (myproc() && myproc()->state == RUNNING &&
		tf->trapno == T_IRQ0 + IRQ_TIMER)
		yield();

	// Check if the process has been killed since we yielded
	if (myproc() && myproc()->killed && (tf->cs & 3) == DPL_USER)
		exit();
}
