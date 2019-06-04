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
//�ں˴����ȡ��ɺ�ĵ�һ�����е�ַ
extern char end[]; // first address after kernel loaded from ELF file

// Bootstrap processor starts running C code here.
// Allocate a real stack and switch to it, first
// doing some setup required for memory allocator to work.
//�ں�������boot��ɺ������￪ʼ��ʼ������ϵͳ
int
main(void)
{
	//��ʼ��������ҳ����ʱҳ��������
	//������������freerange�ͷ��ں��Ժ�ֱ�������ڴ�ĩβ���ڴ�
	//���ûὫ���Ƕ��ָ�Ϊҳ�������ҳ����
	//P2V�������ڴ�ĩβ��ַת��Ϊ�����ڴ��ַ�Ӷ�������
	kinit1(end, P2V(4 * 1024 * 1024)); // phys page allocator
	//��ʼ�������ַӳ��
	kvmalloc();      // kernel page table
	//����������������������
	mpinit();        // detect other processors
	lapicinit();     // interrupt controller
	seginit();       // segment descriptors
	picinit();       // disable pic
	ioapicinit();    // another interrupt controller
	consoleinit();   // console hardware
	uartinit();      // serial port
	//��ʼ�����̱�
	pinit();         // process table
	//��ʼ���ж�����
	tvinit();        // trap vectors
	binit();         // buffer cache
	//�ļ���
	fileinit();      // file table
	//Ӳ�̳�ʼ��
	ideinit();       // disk 
	startothers();   // start other processors
	//��һ����ʼ���ں��ڴ�ӳ��
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
//xv6��ҳ���1024������ҳ������1024��ҳ��ÿ��ҳ4K�ڴ���ɵ�
//ÿ������ҳ����Դ���4M�����ݣ�Ҳ����Ŀǰ��ʼҳ���ҳ���С
__attribute__((__aligned__(PGSIZE)))
//�����ǳ�ʼ����ʱ��ҳĿ¼��ֻ��һ����������1024��Ԫ��
//��ʱ����û�з�ҳ��ֻ��4M�������ڴ棬�Ժ�����Ϊ����ҳ��
pde_t entrypgdir[NPDENTRIES] = {
	// Map VA's [0, 4MB) to PA's [0, 4MB)
	//ͨ������PTE_PSʹ�ÿ��Է���һ��������2��ҳ���С���ڴ棬Ҳ����������4M
	//��ʵ�ʵ�ַ[0, 4MB)ӳ�䵽�����ַ��[0, 4MB)�������ַ[KERNBASE, KERNBASE+4MB)
	//[0, 4MB)Ŀǰ������ں˴���

	//�����ʼ������0����
	[0] = (0) | PTE_P | PTE_W | PTE_PS,

	// Map VA's [KERNBASE, KERNBASE+4MB) to PA's [0, 4MB)
	//KERNBASE����22λ����Ŀ¼��һ��λ�ã�512��
	[KERNBASE >> PDXSHIFT] = (0) | PTE_P | PTE_W | PTE_PS,
};

//PAGEBREAK!
// Blank page.
//PAGEBREAK!
// Blank page.
//PAGEBREAK!
// Blank page.

