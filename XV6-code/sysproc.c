#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int
sys_fork(void)
{
	return fork();
}

int
sys_exit(void)
{
	exit();
	return 0;  // not reached
}

int
sys_wait(void)
{
	return wait();
}

int
sys_kill(void)
{
	int pid;

	if (argint(0, &pid) < 0)
		return -1;
	return kill(pid);
}

int
sys_getpid(void)
{
	return myproc()->pid;
}

int
sys_sbrk(void)
{
	int addr;
	int n;

	if (argint(0, &n) < 0)
		return -1;
	addr = myproc()->sz;
	if (growproc(n) < 0)
		return -1;
	return addr;
}

//sys_sleep的调用，没有参数是因为参数是在下面通过工具函数取得的
//工具函数  argint、argptr 和 argstr 获得第 n 个系统调用参数
//他们获取  整数、  指针    和 字符串起始地址
int
sys_sleep(void)
{
	int n;
	uint ticks0;

	//查看第0个参数的值并赋值给n，返回-1代表出错
	//n就是此进程需要sleep的时间
	if (argint(0, &n) < 0)
		return -1;
	//请求时钟锁
	acquire(&tickslock);
	//赋值初始的时间
	ticks0 = ticks;
	//只要当前时间减初始时间ticks0还在n的范围内
	while (ticks - ticks0 < n) {
		if (myproc()->killed) {
			//如果进程已经被杀死代表sleep出错，释放锁并返回-1
			release(&tickslock);
			return -1;
		}
		//就进行sleep，sleep会等待被时钟中断wakeup
		//然后用新的ticks检查是否应该离开循环
		sleep(&ticks, &tickslock);
	}
	//系统调用结束，睡眠结束，释放时钟锁
	release(&tickslock);
	return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
	uint xticks;

	acquire(&tickslock);
	xticks = ticks;
	release(&tickslock);
	return xticks;
}
