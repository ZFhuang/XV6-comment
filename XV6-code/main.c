#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"

static void startothers(void);
static void mpmain(void)  __attribute__((noreturn));
extern pde_t *kpgdir;
//内核代码读取完成后的第一个空闲地址
extern char end[]; // first address after kernel loaded from ELF file

// Bootstrap processor starts running C code here.
// Allocate a real stack and switch to it, first
// doing some setup required for memory allocator to work.
//内核引导器boot完成后在这里开始初始化操作系统
int
main(void)
{
	//初始分配物理页，此时页表还不完整
	//在里面巧用了freerange释放内核以后直到物理内存末尾的内存
	//正好会将它们都分割为页存入空闲页链中
	//P2V将物理内存末尾地址转换为虚拟内存地址从而来操作
	kinit1(end, P2V(4 * 1024 * 1024)); // phys page allocator
	//初始化虚拟地址映射
	kvmalloc();      // kernel page table
	//检测其他处理器，启动多核
	mpinit();        // detect other processors
	lapicinit();     // interrupt controller
	seginit();       // segment descriptors
	picinit();       // disable pic
	ioapicinit();    // another interrupt controller
	consoleinit();   // console hardware
	uartinit();      // serial port
	//初始化进程表
	pinit();         // process table
	//初始化中断向量
	tvinit();        // trap vectors
	binit();         // buffer cache
	//文件表
	fileinit();      // file table
	//硬盘初始化
	ideinit();       // disk 
	startothers();   // start other processors
	//进一步初始化内核内存映射
	kinit2(P2V(4 * 1024 * 1024), P2V(PHYSTOP)); // must come after startothers()
	userinit();      // first user process
	mpmain();        // finish this processor's setup
}

// Other CPUs jump here from entryother.S.
static void
mpenter(void)
{
	switchkvm();
	seginit();
	lapicinit();
	mpmain();
}

// Common CPU setup code.
static void
mpmain(void)
{
	cprintf("cpu%d: starting %d\n", cpuid(), cpuid());
	idtinit();       // load idt register
	xchg(&(mycpu()->started), 1); // tell startothers() we're up
	scheduler();     // start running processes
}

pde_t entrypgdir[];  // For entry.S

// Start the non-boot (AP) processors.
static void
startothers(void)
{
	extern uchar _binary_entryother_start[], _binary_entryother_size[];
	uchar *code;
	struct cpu *c;
	char *stack;

	// Write entry code to unused memory at 0x7000.
	// The linker has placed the image of entryother.S in
	// _binary_entryother_start.
	code = P2V(0x7000);
	memmove(code, _binary_entryother_start, (uint)_binary_entryother_size);

	for (c = cpus; c < cpus + ncpu; c++) {
		if (c == mycpu())  // We've started already.
			continue;

		// Tell entryother.S what stack to use, where to enter, and what
		// pgdir to use. We cannot use kpgdir yet, because the AP processor
		// is running in low  memory, so we use entrypgdir for the APs too.
		stack = kalloc();
		*(void**)(code - 4) = stack + KSTACKSIZE;
		*(void(**)(void))(code - 8) = mpenter;
		*(int**)(code - 12) = (void *)V2P(entrypgdir);

		lapicstartap(c->apicid, V2P(code));

		// wait for cpu to finish mpmain()
		while (c->started == 0)
			;
	}
}

// The boot page table used in entry.S and entryother.S.
// Page directories (and page tables) must start on page boundaries,
// hence the __aligned__ attribute.
// PTE_PS in a page directory entry enables 4Mbyte pages.
//xv6的页表分1024个二级页表，是由1024个页，每个页4K内存组成的
//每个二级页表可以储存4M的内容，也就是目前初始页表的页面大小
__attribute__((__aligned__(PGSIZE)))
//这里是初始进入时的页目录表，只有一级，包含了1024个元素
//此时里面没有分页表，只有4M的连续内存，稍后会分配为正常页表：
pde_t entrypgdir[NPDENTRIES] = {
	// Map VA's [0, 4MB) to PA's [0, 4MB)
	//通过设置PTE_PS使得可以分配一个连续的2级页表大小的内存，也就是连续的4M
	//将实际地址[0, 4MB)映射到虚拟地址的[0, 4MB)和虚拟地址[KERNBASE, KERNBASE+4MB)
	//[0, 4MB)目前存放了内核代码

	//这里初始化了其0号项
	[0] = (0) | PTE_P | PTE_W | PTE_PS,

	// Map VA's [KERNBASE, KERNBASE+4MB) to PA's [0, 4MB)
	//KERNBASE右移22位，即目录的一半位置，512项
	[KERNBASE >> PDXSHIFT] = (0) | PTE_P | PTE_W | PTE_PS,
};

//PAGEBREAK!
// Blank page.
//PAGEBREAK!
// Blank page.
//PAGEBREAK!
// Blank page.

