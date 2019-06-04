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
//�ȴ�Ӳ��׼����
static int
idewait(int checkerr)
{
  int r;

  //0x1f7�˿ڱ�ʾӲ��Ӳ��״̬λ
  //�ȴ�BUSY�������READY������
  while(((r = inb(0x1f7)) & (IDE_BSY|IDE_DRDY)) != IDE_DRDY)
    ;
  //���󷵻�
  if(checkerr && (r & (IDE_DF|IDE_ERR)) != 0)
    return -1;
  return 0;
}

void
ideinit(void)
{
  int i;

  initlock(&idelock, "ide");
  //��IDE_IRQ�ദ�����жϣ�����ֵ�����һ��CPUר�Ŵ����ж�
  ioapicenable(IRQ_IDE, ncpu - 1);
  //�ȴ����̽�������
  idewait(0);

  //����ж��ٴ��̣�����ϵͳ�Ӵ���0��������Ĭ��0�Ǵ��ڵ�
  //ͨ���˿�0x1f6�ȳ��Դ���1��������1�������ʹ�����̲�����
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
//��ʼ��b������Ҳ���ǶԻ���������Ĵ��̶�д
static void
idestart(struct buf *b)
{
  if(b == 0)
    panic("idestart");
  if(b->blockno >= FSSIZE)
    panic("incorrect blockno");
  int sector_per_block =  BSIZE/SECTOR_SIZE;
  int sector = b->blockno * sector_per_block;
  //ͨ����־λ���ж��Ƕ�����д
  //�������Ҫд����Ч������Ҫ��
  int read_cmd = (sector_per_block == 1) ? IDE_CMD_READ :  IDE_CMD_RDMUL;
  int write_cmd = (sector_per_block == 1) ? IDE_CMD_WRITE : IDE_CMD_WRMUL;

  if (sector_per_block > 7) panic("idestart");

  idewait(0);
  //��ʼ���жϣ���������Ӳ���˿�
  outb(0x3f6, 0);  // generate interrupt
  outb(0x1f2, sector_per_block);  // number of sectors
  outb(0x1f3, sector & 0xff);
  outb(0x1f4, (sector >> 8) & 0xff);
  outb(0x1f5, (sector >> 16) & 0xff);
  outb(0x1f6, 0xe0 | ((b->dev&1)<<4) | ((sector>>24)&0x0f));
  //��DIRTYʱ����Ҫд
  if(b->flags & B_DIRTY){
    outb(0x1f7, write_cmd);
	//�ṩ��Ҫд������
    outsl(0x1f0, b->data, BSIZE/4);
  } else {
	//��Ҫ��������
    outb(0x1f7, read_cmd);
  }
}

// Interrupt handler.
//ͨ��trap��������������ж���Ӧ
void
ideintr(void)
{
  struct buf *b;

  // First queued buffer is the active request.
  //������һ����
  acquire(&idelock);

  //�޵ȴ�����ʱ�ͷ���
  if((b = idequeue) == 0){
    release(&idelock);
    return;
  }
  
  //����ĸ�ֵʹbΪ����ͷ��Ȼ�󽫶���������һλ��b���Ǳ�ȡ���ĵ�һ��������
  idequeue = b->qnext;

  // Read data if needed.
  //����Ҫ(DIRTY)ʱ��ȡ����
  if(!(b->flags & B_DIRTY) && idewait(1) >= 0)
	//�����ݶ���b�Ļ�����
    insl(0x1f0, b->data, BSIZE/4);

  // Wake process waiting for this buf.
  //����B_VALID�����B_DIRTY
  b->flags |= B_VALID;
  b->flags &= ~B_DIRTY;
  //����idestart��˯���еĶ���
  wakeup(b);

  // Start disk on next buf in queue.
  //��ʼ��һ������ȴ�
  if(idequeue != 0)
    idestart(idequeue);

  //�ͷ���
  release(&idelock);
}

//PAGEBREAK!
// Sync buf with disk.
// If B_DIRTY is set, write buf to disk, clear B_DIRTY, set B_VALID.
// Else if B_VALID is not set, read buf from disk, set B_VALID.
//�ڴ�����ɨ��˻�����
void
iderw(struct buf *b)
{
  struct buf **pp;

  //ȷ���д�������
  if(!holdingsleep(&b->lock))
    panic("iderw: buf not locked");
  //���ȿ���Ҳ����ʱ������֪��Ҫ��ʲô
  if((b->flags & (B_VALID|B_DIRTY)) == B_VALID)
    panic("iderw: nothing to do");
  //�Ҳ��������쳣
  if(b->dev != 0 && !havedisk1)
    panic("iderw: ide disk 1 not present");

  //��һ��ide����IDE��������
  acquire(&idelock);  //DOC:acquire-lock

  // Append b to idequeue.
  //ֻ�ᴦ����ǰ��Ļ�����
  b->qnext = 0;
  //ѭ������β
  for(pp=&idequeue; *pp; pp=&(*pp)->qnext)  //DOC:insert-queue
    ;
  //��������b�ŵ�����ĩ
  *pp = b;

  // Start disk if necessary.
  //��һ��buf�ͱ������ڲ��ϵ�ѭ���¶���ǰ��
  //������������ڶ����˴�������Կ�ʼ����
  if(idequeue == b)
    idestart(b);

  // Wait for request to finish.
  //�ȴ����������������ideintr����
  while((b->flags & (B_VALID|B_DIRTY)) != B_VALID){
    sleep(b, &idelock);
  }

  //�ͷ���
  release(&idelock);
}
