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

//sys_sleep�ĵ��ã�û�в�������Ϊ������������ͨ�����ߺ���ȡ�õ�
//���ߺ���  argint��argptr �� argstr ��õ� n ��ϵͳ���ò���
//���ǻ�ȡ  ������  ָ��    �� �ַ�����ʼ��ַ
int
sys_sleep(void)
{
	int n;
	uint ticks0;

	//�鿴��0��������ֵ����ֵ��n������-1�������
	//n���Ǵ˽�����Ҫsleep��ʱ��
	if (argint(0, &n) < 0)
		return -1;
	//����ʱ����
	acquire(&tickslock);
	//��ֵ��ʼ��ʱ��
	ticks0 = ticks;
	//ֻҪ��ǰʱ�����ʼʱ��ticks0����n�ķ�Χ��
	while (ticks - ticks0 < n) {
		if (myproc()->killed) {
			//��������Ѿ���ɱ������sleep�����ͷ���������-1
			release(&tickslock);
			return -1;
		}
		//�ͽ���sleep��sleep��ȴ���ʱ���ж�wakeup
		//Ȼ�����µ�ticks����Ƿ�Ӧ���뿪ѭ��
		sleep(&ticks, &tickslock);
	}
	//ϵͳ���ý�����˯�߽������ͷ�ʱ����
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
