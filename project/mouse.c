/* 鼠标中断处理 */

#include "bootpack.h"

struct FIFO8 mousefifo;

void inthandler2c(int *esp)
{
	unsigned char data;
	io_out8(PIC1_OCW2, 0x64);		/* 通知PIC IRQ-12接收完成,否则PIC不再监视该中断 */
	io_out8(PIC0_OCW2, 0x62);		/* 通知PIC IRQ-02接收完成,否则PIC不再监视该中断 */
	data = io_in8(PORT_KEYDAT);
	fifo8_put(&mousefifo, data);
	return;
}

#define KEYCMD_SENDTO_MOUSE		0xd4
#define MOUSECMD_ENABLE			0xf4

/* 激活鼠标 */
void enable_mouse(struct MOUSE_DEC *mdec)
{
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);		/* 如果有效，将返回ACK（0xfa） */
	mdec->phase = 0;		/* 目前的状态为：等待鼠标传回ACK */
	return;
}

int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat)
{
	if(mdec->phase == 0)		/* 等待鼠标传回ACK */
	{
		if(dat == 0xfa)
			mdec->phase = 1;
		return 0;
	}
	if(mdec->phase == 1)		/* 等待鼠标的第一个字节 */
	{
		/* 判断接受的第一个字节是否正确 第一字节应该在“[0~3][8~f]” */
		if((dat & 0xc8) == 0x08)
		{
			mdec->buf[0] = dat;
			mdec->phase = 2;
		}
		return 0;
	}
	if(mdec->phase == 2)		/* 等待鼠标的第二个字节 */
	{
		mdec->buf[1] = dat;
		mdec->phase = 3;
		return 0;
	}
	if(mdec->phase == 3)		/* 等待鼠标的第三个字节 */
	{
		mdec->buf[2] = dat;
		mdec->phase = 1;
		mdec->btn = mdec->buf[0] & 0x07;
		mdec->x = mdec->buf[1];
		mdec->y = mdec->buf[2];
		if((mdec->buf[0] & 0x10) != 0)
			mdec->x |= 0xffffff00;
		if((mdec->buf[0] & 0x20) != 0)
			mdec->y |= 0xffffff00;
		mdec->y = - mdec->y;		/* 在鼠标中，y方向上的符号与屏幕相反 */
		return 1;
	}
	return -1;
}