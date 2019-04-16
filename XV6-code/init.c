// init: The initial user-level program

#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

char *argv[] = { "sh", 0 };

int
main(void)
{
	int pid, wpid;

	//console->�ն�
	if (open("console", O_RDWR) < 0) {
		//û���ļ�ʱ�����������ڸ�Ŀ¼�ҵ�
		mknod("console", 1, 1);
		open("console", O_RDWR);
	}
	//dup�����ļ����������µ�λ�ã�
	dup(0);  // stdout
	dup(0);  // stderr

	for (;;) {
		printf(1, "init: starting sh\n");
		//�½��ӽ���
		pid = fork();
		if (pid < 0) {
			//ʧ�����
			printf(1, "init: fork failed\n");
			exit();
		}
		if (pid == 0) {
			//�ӽ����п���shell
			exec("sh", argv);
			printf(1, "init: exec sh failed\n");
			exit();
		}
		//���ӽ����е�shell���ˣ�����β��������һ��
		while ((wpid = wait()) >= 0 && wpid != pid)
			printf(1, "zombie!\n");
	}
}
