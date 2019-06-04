// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.
//
// The implementation uses two state flags internally:
// * B_VALID: the buffer data has been read from the disk.
// * B_DIRTY: the buffer data has been modified
//     and needs to be written to disk.
//块缓冲层：
//（1）同步对磁盘的访问，使得对于每一个块，
//     同一时间只有一份拷贝放在内存中并且只有一个内核线程使用这份拷贝；
//（2）缓存常用的块以提升性能

#include "types.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "buf.h"

//此结构体将所有的缓冲buf都串在一起，也就是缓冲链表
//是不是储存在device位置？
struct {
  //用于同步这个链表的锁
  struct spinlock lock;
  //缓冲块数组，用于储存
  struct buf buf[NBUF];

  // Linked list of all buffers, through prev/next.
  // head.next is most recently used.
  //当前最前面的缓冲块，块与块间的顺序由其内部的prev/next指针决定，用于访问
  //但是块又都是存于上面的数组中的
  //头部指针指向了最近被使用的缓冲块，加速搜索(先来先)
  struct buf head;
} bcache;

//缓冲区初始化
void
binit(void)
{
  struct buf *b;

  //先初始化这个锁
  initlock(&bcache.lock, "bcache");

//PAGEBREAK!
  // Create linked list of buffers
  //构成循环链表先
  bcache.head.prev = &bcache.head;
  bcache.head.next = &bcache.head;
  //遍历缓冲数组，顺序链接一个缓冲块双向链表，链表尾指向头
  for(b = bcache.buf; b < bcache.buf+NBUF; b++){
	//将当前遍历到的缓冲块指向头的下一个
    b->next = bcache.head.next;
    b->prev = &bcache.head;
	//初始化单个块自己的缓冲块锁
    initsleeplock(&b->lock, "buffer");
	//将头的下一个指向此缓冲块
    bcache.head.next->prev = b;
    bcache.head.next = b;
  }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
// 在设备区内存寻找目标设备dev的标号为blockno的一个缓冲区
// 没有则创建，有则返回并加锁
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;

  //先申请一个缓冲区锁，这个链表也需要同步
  acquire(&bcache.lock);

  // Is the block already cached?
  //利用链表遍历缓冲区，查找是否已经存在需要的缓冲区
  //这里是从链表头部开始找，这是因为头部是最近被使用的块，加速搜索
  for(b = bcache.head.next; b != &bcache.head; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
	  //找到时，引用计数加一
      b->refcnt++;
	  //证明不需要修改到缓冲区表，释放锁
      release(&bcache.lock);
	  //请求一个睡眠锁
      acquiresleep(&b->lock);
	  //返回此块
      return b;
    }
  }

  // Not cached; recycle an unused buffer.
  // Even if refcnt==0, B_DIRTY indicates a buffer is in use
  // because log.c has modified it but not yet committed it.
  //遍历缓冲区寻找一个可用的缓冲块,这里是从链表尾部开始往前找
  //由于链表尾部是距离上次使用最久的块，加速了搜索的速度
  for(b = bcache.head.prev; b != &bcache.head; b = b->prev){
	//必须是没有被引用(空闲中)且已经提交给磁盘(可从内存中删除)的块
	//if (!(b−>flags & B_BUSY)) {
	//rev7中没有引用计数，而是使用了BUSY位
    if(b->refcnt == 0 && (b->flags & B_DIRTY) == 0) {
	  //将请求的信息输入个这个块
      b->dev = dev;
      b->blockno = blockno;
      b->flags = 0;
      b->refcnt = 1;
	  //修改完成就释放缓冲区锁
      release(&bcache.lock);
      acquiresleep(&b->lock);
	  //返回
      return b;
    }
  }

  //没有可用的缓冲块异常
  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
// 从磁盘读取所需的块，返回一个有锁有内容的缓冲块
// (缓冲块锁)：一块缓冲块只允许一个内核线程访问以保持同步
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  //请求到一个可用的缓冲块(或者已有的缓冲块)
  b = bget(dev, blockno);
  //当此缓冲块不可用时，表示需要从磁盘读取内容
  if((b->flags & B_VALID) == 0) {
	//块缓冲，维护一个磁盘请求队列，用中断来恢复，返回时就是读取完成了
    iderw(b);
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
// 将缓冲区中的一块写入磁盘中，也需要锁
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  //修改为DIRTY代表将要被写入
  b->flags |= B_DIRTY;
  //块缓冲，维护一个磁盘请求队列，用中断来恢复，返回时就是写入完成了
  iderw(b);
}

// Release a locked buffer.
// Move to the head of the MRU list.
// 调用此函数释放处理好的缓冲块
void
brelse(struct buf *b)
{
  //确保锁
  if(!holdingsleep(&b->lock))
    panic("brelse");

  //释放锁使得此块又可以被用了
  releasesleep(&b->lock);

  //由于需要来修改缓冲区了申请缓冲区锁
  acquire(&bcache.lock);
  //引用结束，计数--
  b->refcnt--;
  if (b->refcnt == 0) {
	//计数为0代表空闲了，重新加到缓冲区队列的头部，加快下次搜索的效率
    // no one is waiting for it.
    b->next->prev = b->prev;
    b->prev->next = b->next;
    b->next = bcache.head.next;
    b->prev = &bcache.head;
    bcache.head.next->prev = b;
    bcache.head.next = b;
  }
  
  release(&bcache.lock);
}
//PAGEBREAK!
// Blank page.

