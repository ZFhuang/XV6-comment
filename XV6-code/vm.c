#include "param.h"
#include "types.h"
#include "defs.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "elf.h"

//为了处理进程的地址翻译问题，每个进程需要有一个页表来实现其虚拟内存地址到物理地址的转换
//内核也不例外，需要内核页表，此文件用于处理虚拟内存
//vm：虚拟内存virtual memory
//kvm：内核虚拟内存kernel ~

extern char data[];  // defined by kernel.ld
//pde_t是uint格式的，kpgdir是页目录表，利用页目录索引了各个页表的物理地址
pde_t *kpgdir;  // for use in scheduler()
int kallocNum;

//设置CPU的内核段描述符
// Set up CPU's kernel segment descriptors.
//每次启动一个CPU时要进行设置
// Run once on entry on each CPU.
void
seginit(void)
{
	struct cpu *c;

	//CPU的虚拟内存分为四段
	// Map "logical" addresses to virtual addresses using identity map.
	// Cannot share a CODE descriptor for both kernel and user
	// because it would have to have DPL_USR, but the CPU forbids
	// an interrupt from CPL=0 to DPL=3.
	c = &cpus[cpuid()];
	c->gdt[SEG_KCODE] = SEG(STA_X | STA_R, 0, 0xffffffff, 0);
	c->gdt[SEG_KDATA] = SEG(STA_W, 0, 0xffffffff, 0);
	c->gdt[SEG_UCODE] = SEG(STA_X | STA_R, 0, 0xffffffff, DPL_USER);
	c->gdt[SEG_UDATA] = SEG(STA_W, 0, 0xffffffff, DPL_USER);
	lgdt(c->gdt, sizeof(c->gdt));
}

// Return the address of the PTE in page table pgdir
// that corresponds to virtual address va.  If alloc!=0,
// create any required page table pages.
//PTE即page table entries，页表项，想要指到具体字节还需要利用其内容中的物理地址进行偏移
//此函数从页目录表中利用虚拟地址va跳转到对应的页表中返回这个地址对应的页表项的地址
//当alloc不等于0时允许创建未在使用的页表项
static pte_t *
walkpgdir(pde_t *pgdir, const void *va, int alloc)
{
	//pde是页目录表中的一项的物理地址，只是个uint而已，里面包含了页表的物理地址和标识
	pde_t *pde;
	//这是对应的页表的地址，也是个数组
	pte_t *pgtab;

	//利用虚地址中的目录索引先取到对应页表在页目录中的项，项的内容包含页表的地址
	pde = &pgdir[PDX(va)];
	if (*pde & PTE_P) {
		//当得到的项最低1位即正在使用标志位不为0时，表示这个页表是正在使用的
		//用PTE_ADDR将这个项提取出其索引的页表的物理地址
		//再利用P2V将这个物理地址转换回虚拟地址(?)
		pgtab = (pte_t*)P2V(PTE_ADDR(*pde));
	}
	else {
		//当得到的页表没有在使用时，由于页表也是存在页中的，如果不允许创建新页表
		//或无法申请到新页面存放新页表时返回0
		//申请到的pgtab是虚拟地址
		if (!alloc || (pgtab = (pte_t*)kalloc()) == 0)
			return 0;
		// Make sure all those PTE_P bits are zero.
		//否则利用刚才得到的新页面作为新的页表，将这个页表的内容清空为0
		memset(pgtab, 0, PGSIZE);
		// The permissions here are overly generous, but they can
		// be further restricted by the permissions in the page table
		// entries, if necessary.
		//然后将页表地址转为物理地址并赋上标记位放入页目录中
		//也就是将目录中的页表替换成了新的一个空页表
		*pde = V2P(pgtab) | PTE_P | PTE_W | PTE_U;
	}
	//上面找到了此虚拟地址对应的页表数组，这里通过提取其页表索引取地址得到此项的地址
	return &pgtab[PTX(va)];
}

// Create PTEs for virtual addresses starting at va that refer to
// physical addresses starting at pa. va and size might not
// be page-aligned.
//建立整个页虚拟地址到物理地址映射的过程，不只是完成了页目录的映射还要完成页表的映射
static int
mappages(pde_t *pgdir, void *va, uint size, uint pa, int perm)
{
	char *a, *last;
	pte_t *pte;

	//先得到此虚存地址的最低虚拟页开始地址
	a = (char*)PGROUNDDOWN((uint)va);
	//再计算结束页虚拟地址
	last = (char*)PGROUNDDOWN(((uint)va) + size - 1);
	for (;;) {
		//此函数返回当前遍历着的这个页对应页表中的页表项的地址，自动深入两层
		if ((pte = walkpgdir(pgdir, a, 1)) == 0)
			return -1;
		if (*pte & PTE_P)
			//当这个页目前不在使用时报错
			panic("remap");
		//向这个页表项映射入此物理页的物理地址和相应标记
		*pte = pa | perm | PTE_P;
		//循环整个块直到到达最后一页
		if (a == last)
			break;
		a += PGSIZE;
		pa += PGSIZE;
	}
	return 0;
}

//每个进程都有自己的一个页表，然后多余一个页表是当内核没有进程在运行时映射内核用
// There is one page table per process, plus one that's used when
//内核平时是使用当前正在运行的进程的页表作为自己的页表的
// a CPU is not running any process (kpgdir). The kernel uses the
// current process's page table during system calls and interrupts;
//页表保护位防止用户代码访问到内核映射
// page protection bits prevent user code from using the kernel's
// mappings.
//
//每一份页表的初始化都大概是0..KERNBASE映射给用户内存
//KERNBASE..KERNBASE+EXTMEM给IO
//KERNBASE+EXTMEM..data给内核指令等
//data..KERNBASE+PHYSTOP空闲
//0xfe000000..0映射给其他设备
//
// setupkvm() and exec() set up every page table like this:
//
//   0..KERNBASE: user memory (text+data+stack+heap), mapped to
//                phys memory allocated by the kernel
//   KERNBASE..KERNBASE+EXTMEM: mapped to 0..EXTMEM (for I/O space)
//   KERNBASE+EXTMEM..data: mapped to EXTMEM..V2P(data)
//                for the kernel's instructions and r/o data
//   data..KERNBASE+PHYSTOP: mapped to V2P(data)..PHYSTOP,
//                                  rw data + free physical memory
//   0xfe000000..0: mapped direct (devices such as ioapic)
//
//内核在其自己的堆区和用户
//
// The kernel allocates physical memory for its heap and for user memory
// between V2P(end) and the end of physical memory (PHYSTOP)
// (directly addressable from end..P2V(PHYSTOP)).

// This table defines the kernel's mappings, which are present in
// every process's page table.
//这个表是内核在页表中的映射
static struct kmap {
	void *virt;//虚存起始地址
	uint phys_start;//物理内存部分的开始地址
	uint phys_end;//物理内存部分的结束地址
	int perm;//permission权限位
}
//这是kmap表数组kmap的定义，包含了四个子部分，与文档中的图对应，每项都初始化上面的参数
//这里一共初始化了的虚拟地址空间是256M
kmap[] = {
   // I/O space IO空间，1M
 { (void*)KERNBASE, 0,             EXTMEM,    PTE_W}, 
   // kern text+rodata，此项不可写，是内核代码部分
 { (void*)KERNLINK, V2P(KERNLINK), V2P(data), 0},  
   // kern data内核数据，此两项共223M
 { (void*)data,     V2P(data),     PHYSTOP,   PTE_W}, 
   // more devices其余设备，32M
 { (void*)DEVSPACE, DEVSPACE,      0,         PTE_W}, 
};

// Set up kernel part of a page table.
//映射内核虚拟内存，返回页目录表
pde_t*
setupkvm(void)
{
	//页目录表也是储存在页中的，这个页用来储存页表
	pde_t *pgdir;
	//用来决定每个页属于哪一部分各自拥有什么权限
	struct kmap *k;

	kallocNum = 0;

	//申请一个空闲页，即从空闲页链的头部取一个
	if ((pgdir = (pde_t*)kalloc()) == 0)
		return 0;
	//将得到的这个页置0，这个置0正好置0了最后的标记位也就方便了后面检测此项有没有被使用
	memset(pgdir, 0, PGSIZE);
	if (P2V(PHYSTOP) > (void*)DEVSPACE)
		//防止PHYSTOP设置出错，主要是会被KERNBASE影响，但是这个检查有必要么？
		panic("PHYSTOP too high");
	//下面这个for循环将自动分析这个kmap中的元素个数并利用其地址作为边界限制来遍历
	for (k = kmap; k < &kmap[NELEM(kmap)]; k++)
		//初始化页目录表中对应每个页表的对应页的物理地址并按照kmap进行标记
		if (mappages(pgdir, k->virt, k->phys_end - k->phys_start,
			(uint)k->phys_start, k->perm) < 0) {
			//写入失败时释放当前在写的这个页目录表并跳出
			freevm(pgdir);
			return 0;
		}
	cprintf("kalloc times: %d", kallocNum);
	//返回写好的页目录表
	return pgdir;
}

// Allocate one page table for the machine for the kernel address
// space for scheduler processes.
//为内核调度器进程分配页表
void
kvmalloc(void)
{
	//得到页目录表
	kpgdir = setupkvm();
	//切换进程到内核页表
	switchkvm();
}

// Switch h/w page table register to the kernel-only page table,
// for when no process is running.
//切换当前页表，没有参数的情况就是切换到内核页表
void
switchkvm(void)
{
	lcr3(V2P(kpgdir));   // switch to the kernel page table
}

// Switch TSS and h/w page table to correspond to process p.
//有参数时切换到目标进程页表
void
switchuvm(struct proc *p)
{
	if (p == 0)
		//是否有进程的判断
		panic("switchuvm: no process");
	if (p->kstack == 0)
		//是否有
		panic("switchuvm: no kstack");
	if (p->pgdir == 0)
		//目标进程是否有页表
		panic("switchuvm: no pgdir");

	//关闭中断压栈
	pushcli();
	mycpu()->gdt[SEG_TSS] = SEG16(STS_T32A, &mycpu()->ts,
		sizeof(mycpu()->ts) - 1, 0);
	mycpu()->gdt[SEG_TSS].s = 0;
	mycpu()->ts.ss0 = SEG_KDATA << 3;
	mycpu()->ts.esp0 = (uint)p->kstack + KSTACKSIZE;
	// setting IOPL=0 in eflags *and* iomb beyond the tss segment limit
	// forbids I/O instructions (e.g., inb and outb) from user space
	mycpu()->ts.iomb = (ushort)0xFFFF;
	ltr(SEG_TSS << 3);
	lcr3(V2P(p->pgdir));  // switch to process's address space
	//恢复中断出栈
	popcli();
}

// Load the initcode into address 0 of pgdir.
// sz must be less than a page.
//第一个进程的页表
void
inituvm(pde_t *pgdir, char *init, uint sz)
{
	char *mem;

	if (sz >= PGSIZE)
		panic("inituvm: more than a page");
	mem = kalloc();
	memset(mem, 0, PGSIZE);
	mappages(pgdir, 0, PGSIZE, V2P(mem), PTE_W | PTE_U);
	memmove(mem, init, sz);
}

// Load a program segment into pgdir.  addr must be page-aligned
// and the pages from addr to addr+sz must already be mapped.
//载入用户页表
int
loaduvm(pde_t *pgdir, char *addr, struct inode *ip, uint offset, uint sz)
{
	uint i, pa, n;
	pte_t *pte;

	if ((uint)addr % PGSIZE != 0)
		panic("loaduvm: addr must be page aligned");
	for (i = 0; i < sz; i += PGSIZE) {
		if ((pte = walkpgdir(pgdir, addr + i, 0)) == 0)
			panic("loaduvm: address should exist");
		pa = PTE_ADDR(*pte);
		if (sz - i < PGSIZE)
			n = sz - i;
		else
			n = PGSIZE;
		if (readi(ip, P2V(pa), offset + i, n) != n)
			return -1;
	}
	return 0;
}

// Allocate page tables and physical memory to grow process from oldsz to
// newsz, which need not be page aligned.  Returns new size or 0 on error.
//为用户分配页表
int
allocuvm(pde_t *pgdir, uint oldsz, uint newsz)
{
	char *mem;
	uint a;

	if (newsz >= KERNBASE)
		return 0;
	if (newsz < oldsz)
		return oldsz;

	a = PGROUNDUP(oldsz);
	for (; a < newsz; a += PGSIZE) {
		mem = kalloc();
		if (mem == 0) {
			cprintf("allocuvm out of memory\n");
			deallocuvm(pgdir, newsz, oldsz);
			return 0;
		}
		memset(mem, 0, PGSIZE);
		if (mappages(pgdir, (char*)a, PGSIZE, V2P(mem), PTE_W | PTE_U) < 0) {
			cprintf("allocuvm out of memory (2)\n");
			deallocuvm(pgdir, newsz, oldsz);
			kfree(mem);
			return 0;
		}
	}
	return newsz;
}

// Deallocate user pages to bring the process size from oldsz to
// newsz.  oldsz and newsz need not be page-aligned, nor does newsz
// need to be less than oldsz.  oldsz can be larger than the actual
// process size.  Returns the new process size.
//回收页表分配出去的内存空间
int
deallocuvm(pde_t *pgdir, uint oldsz, uint newsz)
{
	pte_t *pte;
	uint a, pa;

	if (newsz >= oldsz)
		return oldsz;

	a = PGROUNDUP(newsz);
	for (; a < oldsz; a += PGSIZE) {
		pte = walkpgdir(pgdir, (char*)a, 0);
		if (!pte)
			a = PGADDR(PDX(a) + 1, 0, 0) - PGSIZE;
		else if ((*pte & PTE_P) != 0) {
			pa = PTE_ADDR(*pte);
			if (pa == 0)
				panic("kfree");
			char *v = P2V(pa);
			kfree(v);
			*pte = 0;
		}
	}
	return newsz;
}

// Free a page table and all the physical memory pages
// in the user part.
//释放此页目录表(二级页表)
void
freevm(pde_t *pgdir)
{
	uint i;

	//没有找到时报错
	if (pgdir == 0)
		panic("freevm: no pgdir");
	//撤销此页目录表的虚拟地址映射
	deallocuvm(pgdir, KERNBASE, 0);
	for (i = 0; i < NPDENTRIES; i++) {
		if (pgdir[i] & PTE_P) {
			char * v = P2V(PTE_ADDR(pgdir[i]));
			kfree(v);
		}
	}
	kfree((char*)pgdir);
}

// Clear PTE_U on a page. Used to create an inaccessible
// page beneath the user stack.
void
clearpteu(pde_t *pgdir, char *uva)
{
	pte_t *pte;

	pte = walkpgdir(pgdir, uva, 0);
	if (pte == 0)
		panic("clearpteu");
	*pte &= ~PTE_U;
}

// Given a parent process's page table, create a copy
// of it for a child.
//复制一份页表，fork中要用
pde_t*
copyuvm(pde_t *pgdir, uint sz)
{
	pde_t *d;
	pte_t *pte;
	uint pa, i, flags;
	char *mem;

	if ((d = setupkvm()) == 0)
		return 0;
	for (i = 0; i < sz; i += PGSIZE) {
		if ((pte = walkpgdir(pgdir, (void *)i, 0)) == 0)
			panic("copyuvm: pte should exist");
		if (!(*pte & PTE_P))
			panic("copyuvm: page not present");
		pa = PTE_ADDR(*pte);
		flags = PTE_FLAGS(*pte);
		if ((mem = kalloc()) == 0)
			goto bad;
		memmove(mem, (char*)P2V(pa), PGSIZE);
		if (mappages(d, (void*)i, PGSIZE, V2P(mem), flags) < 0) {
			kfree(mem);
			goto bad;
		}
	}
	return d;

bad:
	freevm(d);
	return 0;
}

//PAGEBREAK!
// Map user virtual address to kernel address.
char*
uva2ka(pde_t *pgdir, char *uva)
{
	pte_t *pte;

	pte = walkpgdir(pgdir, uva, 0);
	if ((*pte & PTE_P) == 0)
		return 0;
	if ((*pte & PTE_U) == 0)
		return 0;
	return (char*)P2V(PTE_ADDR(*pte));
}

// Copy len bytes from p to user address va in page table pgdir.
// Most useful when pgdir is not the current page table.
// uva2ka ensures this only works for PTE_U pages.
int
copyout(pde_t *pgdir, uint va, void *p, uint len)
{
	char *buf, *pa0;
	uint n, va0;

	buf = (char*)p;
	while (len > 0) {
		va0 = (uint)PGROUNDDOWN(va);
		pa0 = uva2ka(pgdir, (char*)va0);
		if (pa0 == 0)
			return -1;
		n = PGSIZE - (va - va0);
		if (n > len)
			n = len;
		memmove(pa0 + (va - va0), buf, n);
		len -= n;
		buf += n;
		va = va0 + PGSIZE;
	}
	return 0;
}

//PAGEBREAK!
// Blank page.
//PAGEBREAK!
// Blank page.
//PAGEBREAK!
// Blank page.

