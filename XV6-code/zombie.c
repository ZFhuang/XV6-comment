// Create a zombie process that
// must be reparented at exit.
//����һ��zombie����

#include "types.h"
#include "stat.h"
#include "user.h"

int
main(void)
{
	//forkһ���ӽ���
	if (fork() > 0)
		//�ӽ���sleep5����λ��ʵ������ָ˯��5��ticks��Ҳ����50ms
		sleep(5);  // Let child exit before parent.
	//����exit�����ص�������
	exit();
}
