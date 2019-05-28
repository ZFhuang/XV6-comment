#include "syscall.h"
#include "traps.h"

# user.h中的系统调用用户态函数的实现
# 下面的这段define了SYSCALL的操作，与initcode.S中的对exec的调用用法相同
#define SYSCALL(name) \
  .globl name; \
  name: \
    # 在eax存入系统调用号，例如SYS_sleep
    movl $SYS_ ## name, %eax; \
    # IDT是中断描述符表 interrupt descriptor table
    # int指令负责压栈必要的寄存器，进行特权级转换并将CPU切换为内核态，参数是IDT的索引
    # idt的初始化在tvinit中，也就是trap.c里
    # int可产生一个系统调用的中断(陷入)，不能再调用其它中断(权限低)
    # int会由于修改了%eip的原因让程序跳转到中断描述符指定的位置，即IDT[T_SYSCALL]
    # 可以使用iret指令回到int压栈的地方，从而继续之前的代码执行
    int $T_SYSCALL; \
    ret

SYSCALL(fork)
SYSCALL(exit)
SYSCALL(wait)
SYSCALL(pipe)
SYSCALL(read)
SYSCALL(write)
SYSCALL(close)
SYSCALL(kill)
SYSCALL(exec)
SYSCALL(open)
SYSCALL(mknod)
SYSCALL(unlink)
SYSCALL(fstat)
SYSCALL(link)
SYSCALL(mkdir)
SYSCALL(chdir)
SYSCALL(dup)
SYSCALL(getpid)
SYSCALL(sbrk)
# sleep函数
SYSCALL(sleep)
SYSCALL(uptime)
