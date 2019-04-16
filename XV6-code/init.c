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

	//console->终端
	if (open("console", O_RDWR) < 0) {
		//没有文件时建立，可以在根目录找到
		mknod("console", 1, 1);
		open("console", O_RDWR);
	}
	//dup复制文件描述符在新的位置？
	dup(0);  // stdout
	dup(0);  // stderr

	for (;;) {
		printf(1, "init: starting sh\n");
		//新建子进程
		pid = fork();
		if (pid < 0) {
			//失败情况
			printf(1, "init: fork failed\n");
			exit();
		}
		if (pid == 0) {
			//子进程中开启shell
			exec("sh", argv);
			printf(1, "init: exec sh failed\n");
			exit();
		}
		//若子进程中的shell崩了，则收尾并再启动一个
		while ((wpid = wait()) >= 0 && wpid != pid)
			printf(1, "zombie!\n");
	}
}
