/*关于GDT,IDT等descriptor table的处理*/

#include "bootpack.h"

void init_gdtidt(void)
{
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;
	struct GATE_DESCRIPTOR *idt = (struct GATE_DESCRIPTOR *) ADR_IDT;
	int i;
	
	/*GDT的初始化*/
	for(i = 0; i <= LIMIT_GDT / 8; i++)
	{
		set_segmdesc(gdt + i, 0, 0, 0);		/*段表清0*/
	}
	set_segmdesc(gdt + 1, 0xffffffff,   0x00000000, AR_DATA32_RW);		/*4G，CPU能管理的全部内存*/	
	set_segmdesc(gdt + 2, LIMIT_BOTPAK, ADR_BOTPAK, AR_CODE32_ER);		/*bootpack目标文件保存的位置*/
	load_gdtr(LIMIT_GDT, ADR_GDT);		/*设定GDTR寄存器，这是一个专门保存段表位置和大小信息的特殊寄存器*/
	
	/*IDT的初始化*/
	for(i = 0; i <= LIMIT_IDT / 8; i++)
	{
		set_gatedesc(idt + i, 0, 0, 0);		/*中断表清0*/
	}
	load_idtr(LIMIT_IDT, ADR_IDT);		/*设定IDTR寄存器，这是一个专门保存中断表位置和大小信息的特殊寄存器*/
	
	/*将asm_inthandler21注册为第0x21号中断，中断处理程序所在段号为2，
	但是段寄存器低三位有其他意思必须为0，需要向左移3位*/
	set_gatedesc(idt + 0x21, (int)asm_inthandler21, 2 << 3, AR_INTGATE32);		/*21号键盘中断*/
	set_gatedesc(idt + 0x27, (int)asm_inthandler27, 2 << 3, AR_INTGATE32);		/*27号鼠标中断*/
	set_gatedesc(idt + 0x2c, (int)asm_inthandler2c, 2 << 3, AR_INTGATE32);
	
}

/*段的信息分为：
1、段的大小 limit
2、段的起始地址 base
3、段的管理属性(禁止写入，禁止执行，系统专用等) ar
总共使用8个字节保存这些信息，这些信息都是多部分拼接而成，目的是兼容16位CPU

limit使用20位保存，20位只能指定1MB，所以这里采用了一个方法：
在段的属性中设置一个标志位Gbit，如果标志位是1，limit的单位不再是字节，而是页(4KB)
所以limit分为两部分：limit_low的两个字节和limit_high的低半个字节

base分为三部分：base_low, base_mid, base_high

ar分为两部分：access_right和limit_high的高半字节，共12位
limit_high中的高半字节是"扩展访问权"，这四位是由"GD00"组成，G表示Gbit，D表示段的模式，
1是指32位模式，0是指16位模式;access_right就是原始的管理属性：

00000000(0x00): 未使用的记录表
10010010(0x92): 系统专用，可读写的段。不可执行
10011010(0x9a): 系统专用，可执行的段。可写不可读
11110010(0xf2): 应用程序用，可读写的段。不可执行
11111010(0xfa): 应用程序用，可执行的段。可读不可写
*/
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar)
{
	if(limit > 0xffffff)
	{
		ar |= 0x8000;		/*设置Gbit位*/
		limit /= 0x1000;	/*limit单位改为页*/
	}
	sd->limit_low    = limit & 0xffff;
	sd->base_low     = base & 0xffff;
	sd->base_mid     = (base >> 16) & 0xff;
	sd->access_right = ar & 0xff;
	sd->limit_high   = ((limit >> 16) & 0x0f) | ((ar >> 8) & 0xf0);
	sd->base_high    = (base >> 24) & 0xff;
}

/*初始化IDT
gd 			注册地址
offset 		中断处理程序入口地址
selector 	中断处理程序所在的段号
ar 			中断属性
*/
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar)
{
	gd->offset_low = offset & 0xffff;
	gd->selector = selector;
	gd->dw_count = (ar >> 8) & 0xff;
	gd->access_right = ar & 0xff;
	gd->offset_high = (offset >> 16) & 0xffff;
}


















