// �����ṹ�������豸ʹ��buf����ʾһ����������
struct buf {
	//����λ����ʾ�ڴ�����̵���ϵ
	int flags;
	//�豸��
	uint dev;
	//������
	uint blockno;
	//�˿������ı����˯���������ڵȴ�������Ӧʱ˯��
	struct sleeplock lock;
	//���ü�������ǰ�ж��ٽ�����Ҫ�˿�
	uint refcnt;
	//������˫������������ǰ��ָ��ͺ���ָ��
	struct buf *prev; // LRU cache list
	struct buf *next;
	//��������������õ�ָ��
	struct buf *qnext; // disk queue
	//�˿������
	uchar data[BSIZE];
};
//B_VALID λ���������Ѿ�������
//B_DIRTY λ����������Ҫ��д��
//rev7����--B_BUSY λ��һ������������ĳ����������ʹ��������������������̱���ȴ��������ü���refcnt����
#define B_VALID 0x2  // buffer has been read from disk
#define B_DIRTY 0x4  // buffer needs to be written to disk

