#include "bootpack.h"
#include <stdio.h>

void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	char s[40], mcursor[256], keybuf[32], mousebuf[128];
	int mx, my, i;
	struct MOUSE_DEC mdec;
	
	init_gdtidt();		/*GDT(global (segment) descripter table) 全局段号记录表
						  IDT(interrupt descripter table) 中断记录表 */
						  
	init_pic();			/*PIC(programmable interrupt controller) 可编程中断控制器
						  由外设发出的中断请求必须经过中断控制器处理，该函数对PIC
						  芯片进行初始化 */
						  
	io_sti();			/*CPU将能接受外部设备的中断*/
	
	fifo8_init(&keyfifo, 32, keybuf);
	fifo8_init(&mousefifo, 128, mousebuf);
	io_out8(PIC0_IMR, 0xf9);		/*允许PIC接受键盘的中断请求*/
	io_out8(PIC1_IMR, 0xef);		/*允许PIC接受鼠标的中断请求*/
	
	init_keyboard();
	
	init_palette();		/*为了使用320*200的8位(0-255)颜色模式，需要对0-255的标号
						  进行颜色设定，该函数只设定了16种颜色*/
						 
	init_screen8(binfo->vram, binfo->scrnx, binfo->scrny);
	mx = (binfo->scrnx - 16) / 2;		/*屏幕中央位置*/
	my = (binfo->scrny - 28 - 16) / 2;
	init_mouse_cursor8(mcursor, COL8_008484);
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
						/*在屏幕中央显示鼠标光标*/
	sprintf(s, "(%d, %d)", mx, my);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);
	
	enable_mouse(&mdec);
	
	for(;;)
	{
		io_cli();
		if(fifo8_status(&keyfifo) + fifo8_status(&mousefifo) == 0)
		{
			io_stihlt();
		}else
		{
			if(fifo8_status(&keyfifo) != 0)
			{
				i = fifo8_get(&keyfifo);
				io_sti();
				sprintf(s, "%02X", i);
				boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 16, 15, 31);
				putfonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF,s);
			}
			else if(fifo8_status(&mousefifo) != 0)
			{
				i = fifo8_get(&mousefifo);
				io_sti();
				if(mouse_decode(&mdec, i) != 0)
				{
					sprintf(s, "[lcr %4d %4d]", mdec.x, mdec.y);
					if((mdec.btn & 0x01) != 0)
						s[1] = 'L';
					if((mdec.btn & 0x02) != 0)
						s[3] = 'R';
					if((mdec.btn & 0x04) != 0)
						s[2] = 'C';
					boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 32, 16, 32 + 15 * 8 - 1, 31);
					putfonts8_asc(binfo->vram, binfo->scrnx, 32, 16, COL8_FFFFFF, s);
					
					boxfill8(binfo->vram, binfo->scrnx, COL8_008484, mx, my, mx + 15, my + 15);
					mx += mdec.x;
					my += mdec.y;
					if(mx < 0)
						mx = 0;
					if(my < 0)
						my = 0;
					if(mx > binfo->scrnx - 16)
						mx = binfo->scrnx - 16;
					if(my > binfo->scrny - 16)
						my = binfo->scrny - 16;
					sprintf(s, "(%3d, %3d)", mx, my);
					boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 0, 79, 15);
					putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);
					putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
				}
			}
		}
	}
}