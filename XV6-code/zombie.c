// Create a zombie process that
// must be reparented at exit.
//创建一个zombie进程

#include "types.h"
#include "stat.h"
#include "user.h"

int
main(void)
{
	//fork一个子进程
	if (fork() > 0)
		//子进程sleep5个单位，实际上是指睡眠5个ticks，也就是50ms
		sleep(5);  // Let child exit before parent.
	//调用exit函数回到父进程
	exit();
}
