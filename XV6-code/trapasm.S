#include "mmu.h"

  # vectors.S sends all traps here.
  # 所有的trap都被送往这里，里面有压栈好的trap索引
.globl alltraps
alltraps:
  # Build trap frame.
  # 开始压栈构造中断帧，结构在x86.h的结尾
  # 这里压入的信息都是处理器从内核态恢复到用户态需要的信息
  pushl %ds
  pushl %es
  pushl %fs
  pushl %gs
  pushal
  
  # Set up data segments.
  movw $(SEG_KDATA<<3), %ax
  movw %ax, %ds
  movw %ax, %es

  # Call trap(tf), where tf=%esp
  # 完成了中断帧的构建后，压入堆栈指针%esp作为参数，也就是中断帧tf
  # 然后调用trap函数，也就是跳转到trap.c里
  pushl %esp
  call trap
  addl $4, %esp

  # Return falls through to trapret...
  # trapret在此，例如fork就会来到这里
.globl trapret
trapret:
  # 弹出上面alltraps压入的一些寄存器来恢复用户模式
  popal
  popl %gs
  popl %fs
  popl %es
  popl %ds
  addl $0x8, %esp  # trapno and errcode
  # iret会回到正常执行的用户代码，也就是出栈恢复int中压入的寄存器
  iret
