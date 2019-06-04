#include "param.h"
#include "types.h"
#include "defs.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "elf.h"

//Ϊ�˴�����̵ĵ�ַ�������⣬ÿ��������Ҫ��һ��ҳ����ʵ���������ڴ��ַ�������ַ��ת��
//�ں�Ҳ�����⣬��Ҫ�ں�ҳ�����ļ����ڴ��������ڴ�
//vm�������ڴ�virtual memory
//kvm���ں������ڴ�kernel ~

extern char data[];  // defined by kernel.ld
//pde_t��uint��ʽ�ģ�kpgdir��ҳĿ¼������ҳĿ¼�����˸���ҳ��������ַ
pde_t *kpgdir;  // for use in scheduler()
int kallocNum;

//����CPU���ں˶�������
// Set up CPU's kernel segment descriptors.
//ÿ������һ��CPUʱҪ��������
// Run once on entry on each CPU.
void
seginit(void)
{
	struct cpu *c;

	//CPU�������ڴ��Ϊ�Ķ�
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
//PTE��page table entries��ҳ�����Ҫָ�������ֽڻ���Ҫ�����������е������ַ����ƫ��
//�˺�����ҳĿ¼�������������ַva��ת����Ӧ��ҳ���з��������ַ��Ӧ��ҳ����ĵ�ַ
//��alloc������0ʱ������δ��ʹ�õ�ҳ����
static pte_t *
walkpgdir(pde_t *pgdir, const void *va, int alloc)
{
	//pde��ҳĿ¼���е�һ��������ַ��ֻ�Ǹ�uint���ѣ����������ҳ��������ַ�ͱ�ʶ
	pde_t *pde;
	//���Ƕ�Ӧ��ҳ��ĵ�ַ��Ҳ�Ǹ�����
	pte_t *pgtab;

	//�������ַ�е�Ŀ¼������ȡ����Ӧҳ����ҳĿ¼�е��������ݰ���ҳ��ĵ�ַ
	pde = &pgdir[PDX(va)];
	if (*pde & PTE_P) {
		//���õ��������1λ������ʹ�ñ�־λ��Ϊ0ʱ����ʾ���ҳ��������ʹ�õ�
		//��PTE_ADDR���������ȡ����������ҳ��������ַ
		//������P2V����������ַת���������ַ(?)
		pgtab = (pte_t*)P2V(PTE_ADDR(*pde));
	}
	else {
		//���õ���ҳ��û����ʹ��ʱ������ҳ��Ҳ�Ǵ���ҳ�еģ��������������ҳ��
		//���޷����뵽��ҳ������ҳ��ʱ����0
		//���뵽��pgtab�������ַ
		if (!alloc || (pgtab = (pte_t*)kalloc()) == 0)
			return 0;
		// Make sure all those PTE_P bits are zero.
		//�������øղŵõ�����ҳ����Ϊ�µ�ҳ�������ҳ����������Ϊ0
		memset(pgtab, 0, PGSIZE);
		// The permissions here are overly generous, but they can
		// be further restricted by the permissions in the page table
		// entries, if necessary.
		//Ȼ��ҳ���ַתΪ�����ַ�����ϱ��λ����ҳĿ¼��
		//Ҳ���ǽ�Ŀ¼�е�ҳ���滻�����µ�һ����ҳ��
		*pde = V2P(pgtab) | PTE_P | PTE_W | PTE_U;
	}
	//�����ҵ��˴������ַ��Ӧ��ҳ�����飬����ͨ����ȡ��ҳ������ȡ��ַ�õ�����ĵ�ַ
	return &pgtab[PTX(va)];
}

// Create PTEs for virtual addresses starting at va that refer to
// physical addresses starting at pa. va and size might not
// be page-aligned.
//��������ҳ�����ַ�������ַӳ��Ĺ��̣���ֻ�������ҳĿ¼��ӳ�仹Ҫ���ҳ���ӳ��
static int
mappages(pde_t *pgdir, void *va, uint size, uint pa, int perm)
{
	char *a, *last;
	pte_t *pte;

	//�ȵõ�������ַ���������ҳ��ʼ��ַ
	a = (char*)PGROUNDDOWN((uint)va);
	//�ټ������ҳ�����ַ
	last = (char*)PGROUNDDOWN(((uint)va) + size - 1);
	for (;;) {
		//�˺������ص�ǰ�����ŵ����ҳ��Ӧҳ���е�ҳ����ĵ�ַ���Զ���������
		if ((pte = walkpgdir(pgdir, a, 1)) == 0)
			return -1;
		if (*pte & PTE_P)
			//�����ҳĿǰ����ʹ��ʱ����
			panic("remap");
		//�����ҳ����ӳ���������ҳ�������ַ����Ӧ���
		*pte = pa | perm | PTE_P;
		//ѭ��������ֱ���������һҳ
		if (a == last)
			break;
		a += PGSIZE;
		pa += PGSIZE;
	}
	return 0;
}

//ÿ�����̶����Լ���һ��ҳ��Ȼ�����һ��ҳ���ǵ��ں�û�н���������ʱӳ���ں���
// There is one page table per process, plus one that's used when
//�ں�ƽʱ��ʹ�õ�ǰ�������еĽ��̵�ҳ����Ϊ�Լ���ҳ���
// a CPU is not running any process (kpgdir). The kernel uses the
// current process's page table during system calls and interrupts;
//ҳ����λ��ֹ�û�������ʵ��ں�ӳ��
// page protection bits prevent user code from using the kernel's
// mappings.
//
//ÿһ��ҳ��ĳ�ʼ���������0..KERNBASEӳ����û��ڴ�
//KERNBASE..KERNBASE+EXTMEM��IO
//KERNBASE+EXTMEM..data���ں�ָ���
//data..KERNBASE+PHYSTOP����
//0xfe000000..0ӳ��������豸
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
//�ں������Լ��Ķ������û�
//
// The kernel allocates physical memory for its heap and for user memory
// between V2P(end) and the end of physical memory (PHYSTOP)
// (directly addressable from end..P2V(PHYSTOP)).

// This table defines the kernel's mappings, which are present in
// every process's page table.
//��������ں���ҳ���е�ӳ��
static struct kmap {
	void *virt;//�����ʼ��ַ
	uint phys_start;//�����ڴ沿�ֵĿ�ʼ��ַ
	uint phys_end;//�����ڴ沿�ֵĽ�����ַ
	int perm;//permissionȨ��λ
}
//����kmap������kmap�Ķ��壬�������ĸ��Ӳ��֣����ĵ��е�ͼ��Ӧ��ÿ���ʼ������Ĳ���
//����һ����ʼ���˵������ַ�ռ���256M
kmap[] = {
   // I/O space IO�ռ䣬1M
 { (void*)KERNBASE, 0,             EXTMEM,    PTE_W}, 
   // kern text+rodata�������д�����ں˴��벿��
 { (void*)KERNLINK, V2P(KERNLINK), V2P(data), 0},  
   // kern data�ں����ݣ������223M
 { (void*)data,     V2P(data),     PHYSTOP,   PTE_W}, 
   // more devices�����豸��32M
 { (void*)DEVSPACE, DEVSPACE,      0,         PTE_W}, 
};

// Set up kernel part of a page table.
//ӳ���ں������ڴ棬����ҳĿ¼��
pde_t*
setupkvm(void)
{
	//ҳĿ¼��Ҳ�Ǵ�����ҳ�еģ����ҳ��������ҳ��
	pde_t *pgdir;
	//��������ÿ��ҳ������һ���ָ���ӵ��ʲôȨ��
	struct kmap *k;

	kallocNum = 0;

	//����һ������ҳ�����ӿ���ҳ����ͷ��ȡһ��
	if ((pgdir = (pde_t*)kalloc()) == 0)
		return 0;
	//���õ������ҳ��0�������0������0�����ı��λҲ�ͷ����˺����������û�б�ʹ��
	memset(pgdir, 0, PGSIZE);
	if (P2V(PHYSTOP) > (void*)DEVSPACE)
		//��ֹPHYSTOP���ó�����Ҫ�ǻᱻKERNBASEӰ�죬�����������б�Ҫô��
		panic("PHYSTOP too high");
	//�������forѭ�����Զ��������kmap�е�Ԫ�ظ������������ַ��Ϊ�߽�����������
	for (k = kmap; k < &kmap[NELEM(kmap)]; k++)
		//��ʼ��ҳĿ¼���ж�Ӧÿ��ҳ��Ķ�Ӧҳ�������ַ������kmap���б��
		if (mappages(pgdir, k->virt, k->phys_end - k->phys_start,
			(uint)k->phys_start, k->perm) < 0) {
			//д��ʧ��ʱ�ͷŵ�ǰ��д�����ҳĿ¼������
			freevm(pgdir);
			return 0;
		}
	cprintf("kalloc times: %d", kallocNum);
	//����д�õ�ҳĿ¼��
	return pgdir;
}

// Allocate one page table for the machine for the kernel address
// space for scheduler processes.
//Ϊ�ں˵��������̷���ҳ��
void
kvmalloc(void)
{
	//�õ�ҳĿ¼��
	kpgdir = setupkvm();
	//�л����̵��ں�ҳ��
	switchkvm();
}

// Switch h/w page table register to the kernel-only page table,
// for when no process is running.
//�л���ǰҳ��û�в�������������л����ں�ҳ��
void
switchkvm(void)
{
	lcr3(V2P(kpgdir));   // switch to the kernel page table
}

// Switch TSS and h/w page table to correspond to process p.
//�в���ʱ�л���Ŀ�����ҳ��
void
switchuvm(struct proc *p)
{
	if (p == 0)
		//�Ƿ��н��̵��ж�
		panic("switchuvm: no process");
	if (p->kstack == 0)
		//�Ƿ���
		panic("switchuvm: no kstack");
	if (p->pgdir == 0)
		//Ŀ������Ƿ���ҳ��
		panic("switchuvm: no pgdir");

	//�ر��ж�ѹջ
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
	//�ָ��жϳ�ջ
	popcli();
}

// Load the initcode into address 0 of pgdir.
// sz must be less than a page.
//��һ�����̵�ҳ��
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
//�����û�ҳ��
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
//Ϊ�û�����ҳ��
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
//����ҳ������ȥ���ڴ�ռ�
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
//�ͷŴ�ҳĿ¼��(����ҳ��)
void
freevm(pde_t *pgdir)
{
	uint i;

	//û���ҵ�ʱ����
	if (pgdir == 0)
		panic("freevm: no pgdir");
	//������ҳĿ¼��������ַӳ��
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
//����һ��ҳ��fork��Ҫ��
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

