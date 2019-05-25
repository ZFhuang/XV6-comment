// Memory layout
//内存布局
//虚拟地址空间KERNBASE以上部分映射到物理内存低地址相应位置
//内核末尾物理地址到物理地址PHYSTOP的内存空间未使用 

#define EXTMEM  0x100000            // Start of extended memory内核长度
#define PHYSTOP 0xE000000           // Top physical memory物理地址的结束
#define DEVSPACE 0xFE000000         // Other devices are at high addresses虚拟内存中其他设备的地址

// Key addresses for address space layout (see kmap in vm.c for layout)
#define KERNBASE 0x80000000         // First kernel virtual address初始的内核虚拟内存地址
#define KERNLINK (KERNBASE+EXTMEM)  // Address where kernel is linked内核结束地址

//使用预处理函数，将虚拟地址转为物理地址
#define V2P(a) (((uint) (a)) - KERNBASE)
//将物理地址转为虚拟地址，由于xv6的内存结构，物理地址是被映射在虚拟地址的高2G的位置的
#define P2V(a) ((void *)(((char *) (a)) + KERNBASE))

#define V2P_WO(x) ((x) - KERNBASE)    // same as V2P, but without casts 无类型转换的V2P版本
#define P2V_WO(x) ((x) + KERNBASE)    // same as P2V, but without casts 无类型转换的P2V版本
