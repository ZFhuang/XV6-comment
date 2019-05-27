// Per-CPU state
//各个CPU的状态结构体
struct cpu {
	uchar apicid;                // Local APIC ID，CPU的本地APIC
	//此CPU调度器的上下文
	struct context *scheduler;   // swtch() here to enter scheduler 
	struct taskstate ts;         // Used by x86 to find stack for interrupt
	struct segdesc gdt[NSEGS];   // x86 global descriptor table
	volatile uint started;       // Has the CPU started?，是否启动？
	//当前cli的深度，和同步锁有关，越多个锁要求深度就越深
	int ncli;                    // Depth of pushcli nesting.
	//标识pushcli操作前中断是否开启着呢？
	int intena;                  // Were interrupts enabled before pushcli?
	struct proc *proc;           // The process running on this cpu or null
};

//NCPU=8核
extern struct cpu cpus[NCPU];
extern int ncpu;

//PAGEBREAK: 17
// Saved registers for kernel context switches.
//为内核上下文切换器保存寄存器
// Don't need to save all the segment registers (%cs, etc),
//不需要保存所有寄存器(如%cs)
// because they are constant across kernel contexts.
//因为它们在内核中是不变的
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
//上下文切换中intel规定必须保存的一些寄存器
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
	//进程的内存大小
	uint sz;                     // Size of process memory (bytes)
	//进程的页表，来自vm.c
	pde_t* pgdir;                // Page table
	//进程的内核栈
	char *kstack;                // Bottom of kernel stack for this process
	//进程的状态
	enum procstate state;        // Process state
	//进程id
	int pid;                     // Process ID
	//父进程
	struct proc *parent;         // Parent process
	//当前中断的中断帧
	struct trapframe *tf;        // Trap frame for current syscall
	//此进程的上下文寄存器，每个进程都有自己的一份保存
	struct context *context;     // swtch() here to run process 
	//不是0时代表正在睡眠队列中
	void *chan;                  // If non-zero, sleeping on chan 
	//是否被杀死
	int killed;                  // If non-zero, have been killed
	//对文件的打开状态
	struct file *ofile[NOFILE];  // Open files
	//进程的当前文件目录
	struct inode *cwd;           // Current directory
	//用于调试的进程名
	char name[16];               // Process name (debugging)
};

// Process memory is laid out contiguously, low addresses first:
//   text
//   original data and bss
//   fixed-size stack
//   expandable heap
