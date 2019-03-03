/*关于图像显示的处理*/

#include "bootpack.h"

void init_palette(void)
{
	static unsigned char table_rgb[16 * 3] = {
		0x00, 0x00, 0x00,	/*  0:黑 */
		0xff, 0x00, 0x00,	/*  1:亮红 */
		0x00, 0xff, 0x00,	/*  2:亮绿 */
		0xff, 0xff, 0x00,	/*  3:亮黄 */
		0x00, 0x00, 0xff,	/*  4:亮蓝 */
		0xff, 0x00, 0xff,	/*  5:亮紫 */
		0x00, 0xff, 0xff,	/*  6:浅亮蓝 */
		0xff, 0xff, 0xff,	/*  7:白 */
		0xc6, 0xc6, 0xc6,	/*  8:亮灰 */
		0x84, 0x00, 0x00,	/*  9:暗红 */
		0x00, 0x84, 0x00,	/* 10:暗绿 */
		0x84, 0x84, 0x00,	/* 11:暗黄 */
		0x00, 0x00, 0x84,	/* 12:暗蓝 */
		0x84, 0x00, 0x84,	/* 13:暗紫 */
		0x00, 0x84, 0x84,	/* 14:浅暗蓝 */
		0x84, 0x84, 0x84	/* 15:暗灰 */		
	};
	set_palette(0, 15, table_rgb);
	return;
}

/*设置调色板的步骤
1、屏蔽外部中断
2、将要设定的调色板号码写入0x03c8，紧接着按R，G，B的顺序写入0x03c9.如果
还想继续设定下一个调色板，则省略调色板号码，再按照RGB的顺序写入0x03c9
3、恢复外部中断
*/
void set_palette(int start, int end, unsigned char *rgb)
{
	int i, eflags;
	eflags = io_load_eflags();		/*记录中断许可标志位的值，虽然将EFLAGS寄存器的值都保存了，
									  但实际只是为了保存IF位(inerrupt flag)*/
	io_cli();		/*IF置0，屏蔽中断*/
	io_out8(0x03c8, start);
	for(i = start; i<= end; i++)
	{
		io_out8(0x03c9, rgb[0] / 4);		/*为什么要除以4?*/
		io_out8(0x03c9, rgb[1] / 4);
		io_out8(0x03c9, rgb[2] / 4);
		rgb += 3;
	}
	io_store_eflags(eflags);		/*恢复IF原有的值*/
	return;
}

/*
vram：显存起始地址
xsize: 屏幕宽度
x,y: 坐标
c: 色号
*/
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1)
{
	int x, y;
	for(y = y0; y <= y1; y++)
	{
		for(x = x0; x <= x1; x++)
			vram[y * xsize + x] = c;
	}
	return;
}

void init_screen8(char *vram, int x, int y)
{
	boxfill8(vram, x, COL8_008484,  0,     0,      x -  1, y - 29);
	boxfill8(vram, x, COL8_C6C6C6,  0,     y - 28, x -  1, y - 28);
	boxfill8(vram, x, COL8_FFFFFF,  0,     y - 27, x -  1, y - 27);
	boxfill8(vram, x, COL8_C6C6C6,  0,     y - 26, x -  1, y -  1);

	boxfill8(vram, x, COL8_FFFFFF,  3,     y - 24, 59,     y - 24);
	boxfill8(vram, x, COL8_FFFFFF,  2,     y - 24,  2,     y -  4);
	boxfill8(vram, x, COL8_848484,  3,     y -  4, 59,     y -  4);
	boxfill8(vram, x, COL8_848484, 59,     y - 23, 59,     y -  5);
	boxfill8(vram, x, COL8_000000,  2,     y -  3, 59,     y -  3);
	boxfill8(vram, x, COL8_000000, 60,     y - 24, 60,     y -  3);

	boxfill8(vram, x, COL8_848484, x - 47, y - 24, x -  4, y - 24);
	boxfill8(vram, x, COL8_848484, x - 47, y - 23, x - 47, y -  4);
	boxfill8(vram, x, COL8_FFFFFF, x - 47, y -  3, x -  4, y -  3);
	boxfill8(vram, x, COL8_FFFFFF, x -  3, y - 24, x -  3, y -  3);
	return;
}

/*打印字符
vram：显存起始地址
xsize: 屏幕宽度
x,y: 坐标
c: 色号
font: 字符的表示向量
*/
void putfont8(char *vram, int xsize, int x, int y, char c, char *font)
{
	int i;
	char *p, d;
	for(i = 0; i < 16; i++)
	{
		p = vram + (y + i) * xsize + x;
		d = font[i];
		if((d & 0x80) != 0) { p[0] = c; }
		if((d & 0x40) != 0) { p[1] = c; }
		if((d & 0x20) != 0) { p[2] = c; }
		if((d & 0x10) != 0) { p[3] = c; }
		if((d & 0x08) != 0) { p[4] = c; }
		if((d & 0x04) != 0) { p[5] = c; }
		if((d & 0x02) != 0) { p[6] = c; }
		if((d & 0x01) != 0) { p[7] = c; }
	}
	return;
}

/*打印字符串
vram：显存起始地址
xsize: 屏幕宽度
x,y: 坐标
c: 色号
s: 字符串
*/
void putfonts8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s)
{
	extern char hankaku[4096];		/*表示所有ASCII字符的向量集 大小为 8*16*/
	for(; *s != 0x00; s++)
	{
		putfont8(vram, xsize, x, y, c, hankaku + *s * 16);
		x += 8;
	}
	return;
}

void init_mouse_cursor8(char *mouse, char bc)
{
	static char cursor[16][16] = {
		"**************..",
		"*OOOOOOOOOOO*...",
		"*OOOOOOOOOO*....",
		"*OOOOOOOOO*.....",
		"*OOOOOOOO*......",
		"*OOOOOOO*.......",
		"*OOOOOOO*.......",
		"*OOOOOOOO*......",
		"*OOOO**OOO*.....",
		"*OOO*..*OOO*....",
		"*OO*....*OOO*...",
		"*O*......*OOO*..",
		"**........*OOO*.",
		"*..........*OOO*",
		"............*OO*",
		".............***"		
	};
	int x, y;
	
	for(y = 0; y < 16; y++)
	{
		for(x = 0; x < 16; x++)
		{
			if(cursor[y][x] == '*')
				mouse[y * 16 + x] = COL8_000000;
			if(cursor[y][x] == 'O')
				mouse[y * 16 + x] = COL8_FFFFFF;
			if(cursor[y][x] == '.')
				mouse[y * 16 + x] = bc;
		}
	}
	return;
}

/*打印一个图标
vram: 显存起始地址
vxsize: 屏幕宽度
pxsize: 图标宽度
pysize: 图标长度
buf: 图标的表示向量
bxsize: 图标每一行的像素数，一般与pxsize相等
*/
void putblock8_8(char *vram, int vxsize, int pxsize, 
	int pysize, int px0, int py0, char *buf, int bxsize)
{
	int x, y;
	for(y = 0; y < pysize; y++)
	{
		for(x = 0; x < pxsize; x++)
			vram[(py0 + y) * vxsize + (px0 + x)] = buf[y * bxsize + x];
	}
	return;
}


















