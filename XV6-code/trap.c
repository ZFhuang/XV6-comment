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
//时钟锁，是个自旋锁
struct spinlock tickslock;
//当前的计时器ticks
uint ticks;

//被main调用，用来初始化了256个IDT表的元素
//x86 允许 256 个不同的中断。中断 0-31 被定义为软件异常
//比如除 0 错误和访问非法的内存页。xv6 将中断号 32-63 映射给硬件中断
//64为系统调用的中断号。
void
tvinit(void)
{
	int i;

	//在循环中设置idt的每一项，这些是中断，需要最高权限
	for (i = 0; i < 256; i++)
		SETGATE(idt[i], 0, SEG_KCODE << 3, vectors[i], 0);
	//特别处理系统调用的项，这是一个陷入，用户权限3即可
	//这个系统调用陷入由int $T_SYSCALL产生
	SETGATE(idt[T_SYSCALL], 1, SEG_KCODE << 3, vectors[T_SYSCALL], DPL_USER);

	//初始化时间锁
	initlock(&tickslock, "time");
}

void
idtinit(void)
{
	lidt(idt, sizeof(idt));
}

//PAGEBREAK: 41
//中断处理，参数是当前的中断帧指针，是被alltraps调用而来
void
trap(struct trapframe *tf)
{
	//从传递来的alltraps得到trapno
	if (tf->trapno == T_SYSCALL) {
		//系统调用中断
		if (myproc()->killed)
			exit();
		//切换此进程的当前中断帧
		myproc()->tf = tf;
		//调用syscall进行系统调用
		syscall();
		if (myproc()->killed)
			exit();
		//直接返回
		return;
	}

	//是其他的中断时
	switch (tf->trapno) {
	case T_IRQ0 + IRQ_TIMER:
		//分时硬件和时钟中断，每秒被调用100次，也就是每个ticks代表10ms
		if (cpuid() == 0) {
			acquire(&tickslock);
			ticks++;
			//唤醒对应时间的睡眠队列的项
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
