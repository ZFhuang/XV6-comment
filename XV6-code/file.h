struct file {
  enum { FD_NONE, FD_PIPE, FD_INODE } type;
  int ref; // reference count
  char readable;
  char writable;
  struct pipe *pipe;
  struct inode *ip;
  uint off;
};


// in-memory copy of an inode
//存放在内存中的dinode，当有指针指向这个i节点时
//系统才把这个节点保留在内存中，因此有类似智能指针的ref
struct inode {
  uint dev;           // Device number
  uint inum;          // Inode number
  int ref;            // Reference count 引用计数，0时丢弃
  struct sleeplock lock; // protects everything below here
  int valid;          // inode has been read from disk?

  //对dinode的一个拷贝
  short type;         // copy of disk inode
  short major;
  short minor;
  short nlink;
  uint size;
  uint addrs[NDIRECT+1];
};

// table mapping major device number to
// device functions
struct devsw {
  int (*read)(struct inode*, char*, int);
  int (*write)(struct inode*, char*, int);
};

extern struct devsw devsw[];

#define CONSOLE 1
