// Physical memory allocator, intended to allocate
// memory for user processes, kernel stacks, page table pages,
// and pipe buffers. Allocates 4096-byte pages.
//物理内存分配器，内存管理是以页为单位的

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
				   //内核代码读取完成后的第一个空闲地址

struct run {
	//链，就是一个不断指向下一页的指针(但是为什么是虚拟地址？)
	struct run *next;
};

//内核内存结构，包含两个锁和一个空闲页链表，用来计算内核管理着的可用内存页
struct {
	//内核内存空闲页锁
	struct spinlock lock;
	//是否需要使用锁
	int use_lock;
	//空闲页链表，用run串在一起，保存了各个空闲页的起点
	struct run *freelist;
} kmem;

// Initialization happens in two phases.
//内核初始化是两步走的，先是在页表还未完全初始化前，然后是初始化完成后
//目前是未初始化完页表，也就是还在使用只有一级页表的entrypgdir状态
// 1. main() calls kinit1() while still using entrypgdir to place just
// the pages mapped by entrypgdir on free list.
void
kinit1(void *vstart, void *vend)
{
	//先初始化一个锁
	initlock(&kmem.lock, "kmem");
	//但是由于运行到这里时还是单线程状态，所以内存还无需使用锁保护
	kmem.use_lock = 0;
	//释放start到end的页
	freerange(vstart, vend);
}

// 2. main() calls kinit2() with the rest of the physical pages
// after installing a full page table that maps them on all cores.
//过程与kinit1雷西斯，不需要初始化锁了
void
kinit2(void *vstart, void *vend)
{
	freerange(vstart, vend);
	//走到这一步时系统已经即将构建完成，开启页表锁的使用
	kmem.use_lock = 1;
}

//释放从start到end的虚拟地址内存，单位是页
void
freerange(void *vstart, void *vend)
{
	//因为页内部是以字节为单位，所以使用char类型作为该页的起点标记
	char *p;
	//将p置于起点地址后的第一个页的开始位置
	p = (char*)PGROUNDUP((uint)vstart);
	for (; p + PGSIZE <= (char*)vend; p += PGSIZE)
		//以页为单位依次释放直到end位置的页
		kfree(p);
}
//PAGEBREAK: 21
// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
//释放掉选定的页的内存，把页重新放回kmem的空闲页链表中
void
kfree(char *v)
{
	//先初始化一个子链
	struct run *r;

	if ((uint)v % PGSIZE || v < end || V2P(v) >= PHYSTOP)
		//异常检测(是那个页的起点，位置在内核后，位置在物理页的设备部分前)
		panic("kfree");

	// Fill with junk to catch dangling refs.
	//抹去原有内容
	memset(v, 1, PGSIZE);

	if (kmem.use_lock)
		//锁功能开启时需要申请锁再使用
		acquire(&kmem.lock);
	//子链的起点为这个页
	r = (struct run*)v;
	//然后将其下一页链向空闲页链
	r->next = kmem.freelist;
	//改为新的空闲页链，也就是将此页加到空闲页链的最前端的意思
	kmem.freelist = r;
	if (kmem.use_lock)
		release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
//向内核申请一个4K的内存页，返回一个空闲页的指针，无法申请时返回0
char*
kalloc(void)
{
	//r是页的起点指针，会默认初始化为0
	struct run *r;

	if (kmem.use_lock)
		//若内存此时需要锁测试则请求
		acquire(&kmem.lock);
	//取空闲页链的头部
	r = kmem.freelist;
	if (r)
		//当存在时，将空闲链表指向下一页，这样r就是一个已经被拿出空闲页链了
		//r的原本内容是对下一页的指针，但是r到下一页间至少隔有4K
		//所以可以以r为起点对其后4K的内存进行修改，r后面的内存就是一个页
		kmem.freelist = r->next;
	//不存在时代表申请失败，此时r按照默认初始化就是0，可以安心返回
	if (kmem.use_lock)
		//对称地在此释放掉锁
		release(&kmem.lock);
	//将r进行类型转换，表示它不再是空闲页链的节点了

	kallocNum++;

	return (char*)r;
}

