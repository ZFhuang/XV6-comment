// Per-CPU state
//各个CPU的状态结构体
struct cpu {
  uchar apicid;                // Local APIC ID，CPU的本地APIC
  struct context *scheduler;   // swtch() here to enter scheduler 调度器的下一个上下文
  struct taskstate ts;         // Used by x86 to find stack for interrupt
  struct segdesc gdt[NSEGS];   // x86 global descriptor table
  volatile uint started;       // Has the CPU started?，是否启动？
  int ncli;                    // Depth of pushcli nesting.当前cli的深度，和同步锁有关，越多个锁要求深度就越深
  int intena;                  // Were interrupts enabled before pushcli?pushcli操作前中断是否开启着呢？
  struct proc *proc;           // The process running on this cpu or null
};

//8核
extern struct cpu cpus[NCPU];
extern int ncpu;

//PAGEBREAK: 17
// Saved registers for kernel context switches.
//为内核上下文切换器保存寄存器
// Don't need to save all the segment registers (%cs, etc),
//不需要保存所有寄存器(如%cs)
// because they are constant across kernel contexts.
//因为它们是不变的
// Don't need to save %eax, %ecx, %edx, because the
// x86 convention is that the caller has saved them.
//不用保存%eax, %ecx, %edx因为x86规定上会保存他们
// Contexts are stored at the bottom of the stack they
// describe; the stack pointer is the address of the context.
//栈指针是上下文压栈的地址，上下文被保存在栈的底部
// The layout of the context matches the layout of the stack in swtch.S
// at the "Switch stacks" comment. Switch doesn't save eip explicitly,
//swtch并不明确保存eip
// but it is on the stack and allocproc() manipulates it.
//上下文中必须保存的一些寄存器
struct context {
  uint edi;
  uint esi;
  uint ebx;
  uint ebp;
  uint eip;
};

enum procstate { UNUSED, EMBRYO, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };

// Per-process state
//进程PCB
struct proc {
  uint sz;                     // Size of process memory (bytes)
  pde_t* pgdir;                // Page table
  char *kstack;                // Bottom of kernel stack for this process
  enum procstate state;        // Process state
  int pid;                     // Process ID
  struct proc *parent;         // Parent process
  struct trapframe *tf;        // Trap frame for current syscall
  struct context *context;     // swtch() here to run process 此进程的上下文
  void *chan;                  // If non-zero, sleeping on chan 不是0时代表正在睡眠队列中
  int killed;                  // If non-zero, have been killed
  struct file *ofile[NOFILE];  // Open files
  struct inode *cwd;           // Current directory
  char name[16];               // Process name (debugging)
};

// Process memory is laid out contiguously, low addresses first:
//   text
//   original data and bss
//   fixed-size stack
//   expandable heap
