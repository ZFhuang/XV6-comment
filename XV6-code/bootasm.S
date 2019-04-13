#include "asm.h"
#include "memlayout.h"
#include "mmu.h"

# Start the first CPU : switch to 32 - bit protected mode, jump into C.
# The BIOS loads this code from the first sector of the hard disk into
# memory at physical address 0x7c00 and starts executing in real mode
# with %cs = 0 % ip = 7c00.

.code16                       # Assemble for 16 - bit mode
.globl start
start :
cli                         # BIOS enabled interrupts; disable

# Zero data segment registers DS, ES, and SS.
xorw    %ax, %ax             # Set %ax to zero
movw    %ax, %ds             # ->Data Segment
movw    %ax, %es             # ->Extra Segment
movw    %ax, %ss             # ->Stack Segment

# Physical address line A20 is tied to zero so that the first PCs
# with 2 MB would run software that assumed 1 MB.Undo that.
//第一个等待
seta20.1:
//inb->写入byte
//从8042的0x64端口读取信息
inb     $0x64, %al               # Wait for not busy
//测试是否busy(与操作，主要是为了判断1号位，也就是输入缓冲器是否有值)
testb   $0x2, %al
//有值时跳回seta20.1，循环等待
jnz     seta20.1

//没有值(空闲)时，将指令$0xd1存入寄存器
movb    $0xd1, %al               # 0xd1->port 0x64
//向0x64写信息，这个命令的意思是准备写Output端口。
//随后通过60h端口写入的字节，会被放置在Output Port中。Output Port中包含了GateA20
outb    %al, $0x64

//第二个等待
seta20.2:
//类似的要判断是否busy
inb     $0x64, %al               # Wait for not busy
testb   $0x2, %al
jnz     seta20.2

//空闲时将0xdf放入寄存器，含义是打开端口
movb    $0xdf, %al               # 0xdf->port 0x60
//向读写端口0x60中写信息0xdf，这样就打开了GateA20的开关
outb    %al, $0x60

# Switch from real to protected mode.Use a bootstrap GDT that makes
# virtual addresses map directly to physical addresses so that the
# effective memory map doesn't change during the transition.
lgdt    gdtdesc
movl    %cr0, %eax
orl     $CR0_PE, %eax
movl    %eax, %cr0

//PAGEBREAK!
# Complete the transition to 32 - bit protected mode by using a long jmp
# to reload %cs and %eip.The segment descriptors are set up with no
# translation, so that the mapping is still the identity mapping.
ljmp    $(SEG_KCODE << 3), $start32

.code32  # Tell assembler to generate 32 - bit code now.
start32:
# Set up the protected - mode data segment registers
movw    $(SEG_KDATA << 3), %ax    # Our data segment selector
movw    %ax, %ds                # ->DS: Data Segment
movw    %ax, %es                # ->ES : Extra Segment
movw    %ax, %ss                # ->SS : Stack Segment
movw    $0, %ax                 # Zero segments not ready for use
movw    %ax, %fs                # ->FS
movw    %ax, %gs                # ->GS

# Set up the stack pointer and call into C.
movl    $start, %esp
call    bootmain

# If bootmain returns(it shouldn't), trigger a Bochs
	# breakpoint if running under Bochs, then loop.
	movw    $0x8a00, %ax            # 0x8a00->port 0x8a00
	movw    %ax, %dx
	outw    %ax, %dx
	movw    $0x8ae0, %ax            # 0x8ae0->port 0x8a00
	outw    %ax, %dx
	spin :
jmp     spin

# Bootstrap GDT
.p2align 2                                # force 4 byte alignment
gdt :
SEG_NULLASM                             # null seg
SEG_ASM(STA_X | STA_R, 0x0, 0xffffffff)   # code seg
SEG_ASM(STA_W, 0x0, 0xffffffff)         # data seg

gdtdesc :
.word(gdtdesc - gdt - 1)             # sizeof(gdt) - 1
.long   gdt                             # address gdt

