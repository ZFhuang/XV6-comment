// Physical memory allocator, intended to allocate
// memory for user processes, kernel stacks, page table pages,
// and pipe buffers. Allocates 4096-byte pages.
//�����ڴ���������ڴ��������ҳΪ��λ��

#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "spinlock.h"

extern int kallocNum;

void freerange(void *vstart, void *vend);
extern char end[]; // first address after kernel loaded from ELF file
				   // defined by the kernel linker script in kernel.ld
				   //�ں˴����ȡ��ɺ�ĵ�һ�����е�ַ

struct run {
	//��������һ������ָ����һҳ��ָ��(����Ϊʲô�������ַ��)
	struct run *next;
};

//�ں��ڴ�ṹ��������������һ������ҳ�������������ں˹����ŵĿ����ڴ�ҳ
struct {
	//�ں��ڴ����ҳ��
	struct spinlock lock;
	//�Ƿ���Ҫʹ����
	int use_lock;
	//����ҳ������run����һ�𣬱����˸�������ҳ�����
	struct run *freelist;
} kmem;

// Initialization happens in two phases.
//�ں˳�ʼ���������ߵģ�������ҳ��δ��ȫ��ʼ��ǰ��Ȼ���ǳ�ʼ����ɺ�
//Ŀǰ��δ��ʼ����ҳ��Ҳ���ǻ���ʹ��ֻ��һ��ҳ���entrypgdir״̬
// 1. main() calls kinit1() while still using entrypgdir to place just
// the pages mapped by entrypgdir on free list.
void
kinit1(void *vstart, void *vend)
{
	//�ȳ�ʼ��һ����
	initlock(&kmem.lock, "kmem");
	//�����������е�����ʱ���ǵ��߳�״̬�������ڴ滹����ʹ��������
	kmem.use_lock = 0;
	//�ͷ�start��end��ҳ
	freerange(vstart, vend);
}

// 2. main() calls kinit2() with the rest of the physical pages
// after installing a full page table that maps them on all cores.
//������kinit1����˹������Ҫ��ʼ������
void
kinit2(void *vstart, void *vend)
{
	freerange(vstart, vend);
	//�ߵ���һ��ʱϵͳ�Ѿ�����������ɣ�����ҳ������ʹ��
	kmem.use_lock = 1;
}

//�ͷŴ�start��end�������ַ�ڴ棬��λ��ҳ
void
freerange(void *vstart, void *vend)
{
	//��Ϊҳ�ڲ������ֽ�Ϊ��λ������ʹ��char������Ϊ��ҳ�������
	char *p;
	//��p��������ַ��ĵ�һ��ҳ�Ŀ�ʼλ��
	p = (char*)PGROUNDUP((uint)vstart);
	for (; p + PGSIZE <= (char*)vend; p += PGSIZE)
		//��ҳΪ��λ�����ͷ�ֱ��endλ�õ�ҳ
		kfree(p);
}
//PAGEBREAK: 21
// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
//�ͷŵ�ѡ����ҳ���ڴ棬��ҳ���·Ż�kmem�Ŀ���ҳ������
void
kfree(char *v)
{
	//�ȳ�ʼ��һ������
	struct run *r;

	if ((uint)v % PGSIZE || v < end || V2P(v) >= PHYSTOP)
		//�쳣���(���Ǹ�ҳ����㣬λ�����ں˺�λ��������ҳ���豸����ǰ)
		panic("kfree");

	// Fill with junk to catch dangling refs.
	//Ĩȥԭ������
	memset(v, 1, PGSIZE);

	if (kmem.use_lock)
		//�����ܿ���ʱ��Ҫ��������ʹ��
		acquire(&kmem.lock);
	//���������Ϊ���ҳ
	r = (struct run*)v;
	//Ȼ������һҳ�������ҳ��
	r->next = kmem.freelist;
	//��Ϊ�µĿ���ҳ����Ҳ���ǽ���ҳ�ӵ�����ҳ������ǰ�˵���˼
	kmem.freelist = r;
	if (kmem.use_lock)
		release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
//���ں�����һ��4K���ڴ�ҳ������һ������ҳ��ָ�룬�޷�����ʱ����0
char*
kalloc(void)
{
	//r��ҳ�����ָ�룬��Ĭ�ϳ�ʼ��Ϊ0
	struct run *r;

	if (kmem.use_lock)
		//���ڴ��ʱ��Ҫ������������
		acquire(&kmem.lock);
	//ȡ����ҳ����ͷ��
	r = kmem.freelist;
	if (r)
		//������ʱ������������ָ����һҳ������r����һ���Ѿ����ó�����ҳ����
		//r��ԭ�������Ƕ���һҳ��ָ�룬����r����һҳ�����ٸ���4K
		//���Կ�����rΪ�������4K���ڴ�����޸ģ�r������ڴ����һ��ҳ
		kmem.freelist = r->next;
	//������ʱ��������ʧ�ܣ���ʱr����Ĭ�ϳ�ʼ������0�����԰��ķ���
	if (kmem.use_lock)
		//�ԳƵ��ڴ��ͷŵ���
		release(&kmem.lock);
	//��r��������ת������ʾ�������ǿ���ҳ���Ľڵ���

	kallocNum++;

	return (char*)r;
}

