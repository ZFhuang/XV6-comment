# Context switch
#
#   void swtch(struct context **old, struct context *new);
# 
# Save the current registers on the stack, creating
# a struct context, and save its address in *old.
# Switch stacks to new and pop previously-saved registers.

# 上下文切换器 用于切换上下文 保存旧的，插入新的
# swtch（2702）一开始从栈中弹出参数
# 放入寄存器 %eax 和 %edx（2709-2710）中
# swtch 必须在改变栈指针以及无法获得 %esp 前完成这些事情
# 然后 swtch 压入寄存器，在当前栈上建立一个新的上下文结构
# 仅有被调用者保存的寄存器此时需要被保存
# 按照 x86 的惯例即 %ebp %ebx %esi %edi %esp
# swtch 显式地压入前四个寄存器（2713-2716）
# 最后一个则是在 struct context* 被写入 old（2719）时隐式地保存的
# 要注意，还有一个重要的寄存器，即程序计数器(PC) %eip
# 该寄存器在使用 call 调用 swtch 时就保存在栈中 %ebp 之上的位置上
# 保存了旧寄存器后，swtch 就准备要恢复新的寄存器了
# 它将指向新上下文的指针放入栈指针中（2720）
# 新的栈结构和旧的栈相同，因为新的上下文其实是之前某次的切换中的旧上下文
# 所以 swtch 就能颠倒一下保存旧上下文的顺序来恢复新上下文
# 它弹出 %edi %esi %ebx %ebp 然后返回（2723-2727）
# 由于 swtch 改变了栈指针，所以这时恢复的寄存器就是新上下文中的寄存器值。

.globl swtch
swtch:
  # 先将栈中带来的也就是调用这段函数的参数弹出到eax和edx中
  # 第一个参数，旧的上下文，需要保存
  movl 4(%esp), %eax
  # 第二个参数，新的上下文，切换进去
  movl 8(%esp), %edx

  # Save old callee-saved registers
  # pushl a 将a寄存器的值以32位格式压入栈中保存
  # 压栈保存目前的一些x86要求保存的寄存器，压栈是与context顺序反向的
  pushl %ebp
  pushl %ebx
  pushl %esi
  pushl %edi

  # Switch stacks
  # 接着将旧的上下文指针放入刚才读入的栈顶内存
  movl %esp, (%eax)
  # 将新上下文的指针从刚才开始时读入的edx寄存器移动到栈寄存器
  movl %edx, %esp

  # Load new callee-saved registers
  # 由于栈指针的变化，此时pop出来的寄存器值就是新的上下文的寄存器了，取出自然也要反向
  popl %edi
  popl %esi
  popl %ebx
  popl %ebp

  ret
