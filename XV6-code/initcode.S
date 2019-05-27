# Initial process execs /init.
# This code runs in user space.

#include "syscall.h"
#include "traps.h"


# exec(init, argv)
.globl start
start:
  # 进行了一次系统调用
  # 先将所需的参数压栈
  pushl $argv
  pushl $init
  pushl $0  // where caller pc would be
  # 在eax存入系统调用号，这里是sys_exec
  movl $SYS_exec, %eax
  # int指令负责压栈必要的寄存器，进行特权级转换并将CPU切换为内核态，参数是IDT的索引
  # int可产生一个系统调用的中断(陷入)，不能再调用其它中断(权限低)
    # int会由于修改了%eip的原因让程序跳转到中断描述符指定的位置，即vectors[T_SYSCALL]
  # vectors在vectors.S里
  # 可以使用iret指令回到int压栈的地方，从而继续之前的代码执行
  # IDT是中断描述符表 interrupt descriptor table
  # 具体对应的中断表在syscall.c里，T_SYSCALL=64，而IDT的初始化在trap.c的tvinit里
  int $T_SYSCALL

# for(;;) exit();
exit:
  movl $SYS_exit, %eax
  int $T_SYSCALL
  jmp exit

# char init[] = "/init\0";
init:
  .string "/init\0"

# char *argv[] = { init, 0 };
.p2align 2
argv:
  .long init
  .long 0

