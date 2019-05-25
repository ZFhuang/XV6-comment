// Memory layout
//�ڴ沼��
//�����ַ�ռ�KERNBASE���ϲ���ӳ�䵽�����ڴ�͵�ַ��Ӧλ��
//�ں�ĩβ�����ַ�������ַPHYSTOP���ڴ�ռ�δʹ�� 

#define EXTMEM  0x100000            // Start of extended memory�ں˳���
#define PHYSTOP 0xE000000           // Top physical memory�����ַ�Ľ���
#define DEVSPACE 0xFE000000         // Other devices are at high addresses�����ڴ��������豸�ĵ�ַ

// Key addresses for address space layout (see kmap in vm.c for layout)
#define KERNBASE 0x80000000         // First kernel virtual address��ʼ���ں������ڴ��ַ
#define KERNLINK (KERNBASE+EXTMEM)  // Address where kernel is linked�ں˽�����ַ

//ʹ��Ԥ���������������ַתΪ�����ַ
#define V2P(a) (((uint) (a)) - KERNBASE)
//�������ַתΪ�����ַ������xv6���ڴ�ṹ�������ַ�Ǳ�ӳ���������ַ�ĸ�2G��λ�õ�
#define P2V(a) ((void *)(((char *) (a)) + KERNBASE))

#define V2P_WO(x) ((x) - KERNBASE)    // same as V2P, but without casts ������ת����V2P�汾
#define P2V_WO(x) ((x) + KERNBASE)    // same as P2V, but without casts ������ת����P2V�汾
