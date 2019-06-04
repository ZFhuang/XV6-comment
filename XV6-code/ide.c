// Simple PIO-based (non-DMA) IDE driver code.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "traps.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "buf.h"

#define SECTOR_SIZE   512
#define IDE_BSY       0x80
#define IDE_DRDY      0x40
#define IDE_DF        0x20
#define IDE_ERR       0x01

#define IDE_CMD_READ  0x20
#define IDE_CMD_WRITE 0x30
#define IDE_CMD_RDMUL 0xc4
#define IDE_CMD_WRMUL 0xc5

// idequeue points to the buf now being read/written to the disk.
// idequeue->qnext points to the next buf to be processed.
// You must hold idelock while manipulating queue.

static struct spinlock idelock;
static struct buf *idequeue;

static int havedisk1;
static void idestart(struct buf*);

// Wait for IDE disk to become ready.
//等待硬盘准备好
static int
idewait(int checkerr)
{
  int r;

  //0x1f7端口表示硬盘硬件状态位
  //等待BUSY被清除且READY被设置
  while(((r = inb(0x1f7)) & (IDE_BSY|IDE_DRDY)) != IDE_DRDY)
    ;
  //错误返回
  if(checkerr && (r & (IDE_DF|IDE_ERR)) != 0)
    return -1;
  return 0;
}

void
ideinit(void)
{
  int i;

  initlock(&idelock, "ide");
  //打开IDE_IRQ多处理器中断，但是值打开最后一个CPU专门处理中断
  ioapicenable(IRQ_IDE, ncpu - 1);
  //等待磁盘接收命令
  idewait(0);

  //检查有多少磁盘，由于系统从磁盘0加载所以默认0是存在的
  //通过端口0x1f6先尝试磁盘1，当磁盘1不就绪就代表磁盘不存在
  // Check if disk 1 is present
  outb(0x1f6, 0xe0 | (1<<4));
  for(i=0; i<1000; i++){
    if(inb(0x1f7) != 0){
      havedisk1 = 1;
      break;
    }
  }

  // Switch back to disk 0.
  outb(0x1f6, 0xe0 | (0<<4));
}

// Start the request for b.  Caller must hold idelock.
//开始对b的请求，也就是对缓冲区所需的磁盘读写
static void
idestart(struct buf *b)
{
  if(b == 0)
    panic("idestart");
  if(b->blockno >= FSSIZE)
    panic("incorrect blockno");
  int sector_per_block =  BSIZE/SECTOR_SIZE;
  int sector = b->blockno * sector_per_block;
  //通过标志位来判断是读还是写
  //脏代表需要写，无效代表需要读
  int read_cmd = (sector_per_block == 1) ? IDE_CMD_READ :  IDE_CMD_RDMUL;
  int write_cmd = (sector_per_block == 1) ? IDE_CMD_WRITE : IDE_CMD_WRMUL;

  if (sector_per_block > 7) panic("idestart");

  idewait(0);
  //初始化中断，都在利用硬件端口
  outb(0x3f6, 0);  // generate interrupt
  outb(0x1f2, sector_per_block);  // number of sectors
  outb(0x1f3, sector & 0xff);
  outb(0x1f4, (sector >> 8) & 0xff);
  outb(0x1f5, (sector >> 16) & 0xff);
  outb(0x1f6, 0xe0 | ((b->dev&1)<<4) | ((sector>>24)&0x0f));
  //当DIRTY时才需要写
  if(b->flags & B_DIRTY){
    outb(0x1f7, write_cmd);
	//提供需要写的数据
    outsl(0x1f0, b->data, BSIZE/4);
  } else {
	//所要读的数据
    outb(0x1f7, read_cmd);
  }
}

// Interrupt handler.
//通过trap来跳到这里进行中断响应
void
ideintr(void)
{
  struct buf *b;

  // First queued buffer is the active request.
  //先申请一个锁
  acquire(&idelock);

  //无等待队列时释放锁
  if((b = idequeue) == 0){
    release(&idelock);
    return;
  }
  
  //上面的赋值使b为队列头，然后将队列往后移一位，b就是被取出的第一个缓冲了
  idequeue = b->qnext;

  // Read data if needed.
  //在需要(DIRTY)时读取数据
  if(!(b->flags & B_DIRTY) && idewait(1) >= 0)
	//将数据读入b的缓冲区
    insl(0x1f0, b->data, BSIZE/4);

  // Wake process waiting for this buf.
  //设置B_VALID并清除B_DIRTY
  b->flags |= B_VALID;
  b->flags &= ~B_DIRTY;
  //唤醒idestart中睡眠中的队列
  wakeup(b);

  // Start disk on next buf in queue.
  //开始下一个缓冲等待
  if(idequeue != 0)
    idestart(idequeue);

  //释放锁
  release(&idelock);
}

//PAGEBREAK!
// Sync buf with disk.
// If B_DIRTY is set, write buf to disk, clear B_DIRTY, set B_VALID.
// Else if B_VALID is not set, read buf from disk, set B_VALID.
//在磁盘中扫描此缓冲区
void
iderw(struct buf *b)
{
  struct buf **pp;

  //确保有带锁而入
  if(!holdingsleep(&b->lock))
    panic("iderw: buf not locked");
  //当既可用也不脏时，代表不知道要干什么
  if((b->flags & (B_VALID|B_DIRTY)) == B_VALID)
    panic("iderw: nothing to do");
  //找不到磁盘异常
  if(b->dev != 0 && !havedisk1)
    panic("iderw: ide disk 1 not present");

  //求一个ide锁（IDE控制器）
  acquire(&idelock);  //DOC:acquire-lock

  // Append b to idequeue.
  //只会处理最前面的缓冲区
  b->qnext = 0;
  //循环到队尾
  for(pp=&idequeue; *pp; pp=&(*pp)->qnext)  //DOC:insert-queue
    ;
  //将缓冲区b放到队列末
  *pp = b;

  // Start disk if necessary.
  //第一个buf就被请求，在不断的循环下队列前进
  //当这个缓冲区在队首了代表其可以开始处理
  if(idequeue == b)
    idestart(b);

  // Wait for request to finish.
  //等待磁盘请求结束，等ideintr唤醒
  while((b->flags & (B_VALID|B_DIRTY)) != B_VALID){
    sleep(b, &idelock);
  }

  //释放锁
  release(&idelock);
}
