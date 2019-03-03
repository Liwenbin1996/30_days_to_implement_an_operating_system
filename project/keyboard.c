/* 键盘中断处理 */

#include "bootpack.h"

struct FIFO8 keyfifo;

void inthandler21(int *esp)
{
	unsigned char data;
	io_out8(PIC0_OCW2, 0x61);	/* 通知PIC IRQ-01接收完成,否则PIC不再监视该中断 */
	data = io_in8(PORT_KEYDAT);
	fifo8_put(&keyfifo, data);
	return;
}

#define PORT_KEYSTA				0x0064
#define KEYSTA_SEND_NOTREADY	0x02
#define KEYCMD_WRITE_MODE		0x60
#define KBC_MODE				0x47

/* 等待键盘控制器能够传输数据 */
void wait_KBC_sendready(void)
{
	for(;;)
	{
		if((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0)
			break;
	}
	return;
}

/* 初始化键盘电路 */
/* 要使用鼠标，必须激活鼠标控制电路以及鼠标本身 */
void init_keyboard(void)
{
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);	/* 设定模式 */
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, KBC_MODE);			/* 激活鼠标控制电路 */
	return;
}
