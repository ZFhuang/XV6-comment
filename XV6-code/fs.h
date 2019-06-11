// On-disk file system format.
// Both the kernel and user programs use this header file.


#define ROOTINO 1  // root i-number
#define BSIZE 512  // block size

// Disk layout:
//���̵Ĳ���
// [ boot������ |    ������    | ��־ |   i�ڵ��    |
//                                          ���п�λͼ    |   ���ݿ�   ]
// [ boot block | super block | log | inode blocks |
//                                          free bit map | data blocks]
//
// mkfs computes the super block and builds an initial file system. The
// super block describes the disk layout:
//�����鲼��
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
//�����ļ�
#define MAXFILE (NDIRECT + NINDIRECT)

// On-disk inode structure
// Ӳ�̽ڵ�ĸ�ʽ
//ָ���Ǵ����ϵļ�¼�ļ���С�����ݿ������ŵ����ݽṹ��
struct dinode {
	//type=0 ���нڵ�
	short type;           // File type��������
	//�����豸��
	short major;          // Major device number (T_DEV only)
	short minor;          // Minor device number (T_DEV only)
	//i�ڵ��Ŀ¼��ж��Ƿ�Ӧ�ñ��ͷ�
	short nlink;          // Number of links to inode in file system
	//�ļ��Ĵ�С
	uint size;            // Size of file (bytes)
	//��Ӧ���ݿ�Ŀ��
	uint addrs[NDIRECT + 1];   // Data block addresses
};

// Inodes per block.
#define IPB           (BSIZE / sizeof(struct dinode))

// Block containing inode i
#define IBLOCK(i, sb)     ((i) / IPB + sb.inodestart)

// Bitmap bits per block
//ÿ�����λͼ
#define BPB           (BSIZE*8)

// Block of free map containing bit for block b
#define BBLOCK(b, sb) (b/BPB + sb.bmapstart)

// Directory is a file containing a sequence of dirent structures.
#define DIRSIZ 14

struct dirent {
	ushort inum;
	char name[DIRSIZ];
};

