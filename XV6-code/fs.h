// On-disk file system format.
// Both the kernel and user programs use this header file.


#define ROOTINO 1  // root i-number
#define BSIZE 512  // block size

// Disk layout:
//磁盘的布局
// [ boot引导块 |    超级块    | 日志 |   i节点块    |
//                                          空闲块位图    |   数据块   ]
// [ boot block | super block | log | inode blocks |
//                                          free bit map | data blocks]
//
// mkfs computes the super block and builds an initial file system. The
// super block describes the disk layout:
//超级块布局
struct superblock {
	uint size;         // Size of file system image (blocks)
	uint nblocks;      // Number of data blocks
	uint ninodes;      // Number of inodes.
	uint nlog;         // Number of log blocks
	uint logstart;     // Block number of first log block
	uint inodestart;   // Block number of first inode block
	uint bmapstart;    // Block number of first free map block
};

#define NDIRECT 12
#define NINDIRECT (BSIZE / sizeof(uint))
//最大的文件
#define MAXFILE (NDIRECT + NINDIRECT)

// On-disk inode structure
// 硬盘节点的格式
//指的是磁盘上的记录文件大小、数据块扇区号的数据结构。
struct dinode {
	//type=0 空闲节点
	short type;           // File type区分类型
	//主次设备号
	short major;          // Major device number (T_DEV only)
	short minor;          // Minor device number (T_DEV only)
	//i节点的目录项，判断是否应该被释放
	short nlink;          // Number of links to inode in file system
	//文件的大小
	uint size;            // Size of file (bytes)
	//对应数据块的块号
	uint addrs[NDIRECT + 1];   // Data block addresses
};

// Inodes per block.
#define IPB           (BSIZE / sizeof(struct dinode))

// Block containing inode i
#define IBLOCK(i, sb)     ((i) / IPB + sb.inodestart)

// Bitmap bits per block
//每个块的位图
#define BPB           (BSIZE*8)

// Block of free map containing bit for block b
#define BBLOCK(b, sb) (b/BPB + sb.bmapstart)

// Directory is a file containing a sequence of dirent structures.
#define DIRSIZ 14

struct dirent {
	ushort inum;
	char name[DIRSIZ];
};

