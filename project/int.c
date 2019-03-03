/*中断处理相关*/

#include "bootpack.h"
#include <stdio.h>


/*PIC是将8个中断信号集合成一个中断信号的装置。PIC监视着输入管脚的8个中断信号，
只要有一个中断信号进来，就将唯一的输出管脚信号变成ON，并通知CPU。IBM的大叔们
想要通过增加PIC来处理更多的中断信号，他们认为电脑会有8个以上的外部设备，所以
就把中断信号设计成15个，并为此增设了2个PIC。

PIC分主从PIC，从PIC1与主PIC0的第二个引脚连接
PIC寄存器都是8位寄存器，其中：
IMR(interrupt mask register) 中断屏蔽寄存器
ICW(initial control word) 初始化控制数据，其中ICW2决定了中断信号(IRQ)以哪一号
中断通知CPU，IRQ0~15分别对应INT 0x20~2f
*/
void init_pic(void)
{
	io_out8(PIC0_IMR,  0xff  ); /* 禁止所有中断 */
	io_out8(PIC1_IMR,  0xff  ); /* 禁止所有中断 */

	io_out8(PIC0_ICW1, 0x11  ); /* 边沿触发模式 */
	io_out8(PIC0_ICW2, 0x20  ); /* IRQ0-7由INT20-27接收 */
	io_out8(PIC0_ICW3, 1 << 2); /* PIC1由IRQ2连接 */
	io_out8(PIC0_ICW4, 0x01  ); /* 无缓冲模式 */

	io_out8(PIC1_ICW1, 0x11  ); /* 边沿触发模式 */
	io_out8(PIC1_ICW2, 0x28  ); /* IRQ8-15由INT28-2f接收 */
	io_out8(PIC1_ICW3, 2     ); /* PIC1由IRQ2连接 */
	io_out8(PIC1_ICW4, 0x01  ); /* 无缓冲模式 */

	io_out8(PIC0_IMR,  0xfb  ); /* 11111011 除PIC1以外全部禁止 */
	io_out8(PIC1_IMR,  0xff  ); /* 11111111 禁止所有中断 */

	return;	
}

 /*键盘中断 IRQ1 对应 INT 0x21*/
/* void inthandler21(int *esp)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	boxfill8(binfo->vram, binfo->scrnx, COL8_000000, 0, 0, 32 * 8 - 1, 15);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, "INT 21 (IRQ-1) : PS/2 keyboard");
	for(;;)
		io_hlt();
} */

/*鼠标中断 IRQ12 对应 INT 0x2C*/
/* void inthandler2c(int *esp)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	boxfill8(binfo->vram, binfo->scrnx, COL8_000000, 0, 0, 32 * 8 - 1, 15);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, "INT 2C (IRQ-12) : PS/2 mouse");
	for(;;)
		io_hlt();
}  */

void inthandler27(int *esp)
{
	io_out8(PIC0_ICW2, 0x67);
	return;
}






