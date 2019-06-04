// 缓冲块结构，磁盘设备使用buf来表示一个磁盘扇区
struct buf {
	//几个位来表示内存与磁盘的联系
	int flags;
	//设备号
	uint dev;
	//扇区号
	uint blockno;
	//此块所带的本身的睡眠锁，用于等待磁盘响应时睡眠
	struct sleeplock lock;
	//引用计数，当前有多少进程需要此块
	uint refcnt;
	//由于是双向链表，所以有前向指针和后向指针
	struct buf *prev; // LRU cache list
	struct buf *next;
	//磁盘申请队列里用的指针
	struct buf *qnext; // disk queue
	//此块的数据
	uchar data[BSIZE];
};
//B_VALID 位代表数据已经被读入
//B_DIRTY 位代表数据需要被写出
//rev7才有--B_BUSY 位是一个锁；它代表某个进程正在使用这个缓冲区，其他进程必须等待，被引用技术refcnt代替
#define B_VALID 0x2  // buffer has been read from disk
#define B_DIRTY 0x4  // buffer needs to be written to disk

