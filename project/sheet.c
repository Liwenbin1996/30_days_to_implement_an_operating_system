/* 鼠标和窗口的叠加处理 */

#include "bootpack.h"

#define SHEET_USE	1

struct SHTCTL *shtctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize)
{
	struct SHTCTL *ctl;
	int i;
	ctl = (struct SHTCTL *) memman_alloc_4k(memman, sizeof(struct SHTCTL));
	if(ctl == 0)
		goto err;
	ctl->map = (unsigned char *) memman_alloc_4k(memman, xsize * ysize);
	ctl->vram = vram;
	ctl->xsize = xsize;
	ctl->ysize = ysize;
	ctl->top = -1;		/* 一个图层也没有 */
	for(i = 0; i < MAX_SHEETS; i++)
	{
		ctl->sheets0[i].flags = 0;		/* 未使用 */
		ctl->sheets0[i].ctl = ctl;
	}
		
err:
	return ctl;
}

struct SHEET *sheet_alloc(struct SHTCTL *ctl)
{
	struct SHEET *sht;
	int i;
	for(i = 0; i < MAX_SHEETS; i++)
	{
		if(ctl->sheets0[i].flags == 0)
		{
			sht = &ctl->sheets0[i];
			sht->flags = SHEET_USE;		/* 标记为使用 */
			sht->height =  -1;			/* 隐藏 */
			return sht;
		}
	}
	return 0;		/* 没有空图层可使用 */
}

void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize, int col_inv)
{
	sht->buf = buf;
	sht->bxsize = xsize;			/* 图层大小 */
	sht->bysize = ysize;
	sht->col_inv = col_inv;			/* 透明色号 */
	return;
}

/* map上每一个点有一个图层号码，表示该点属于哪个图层 */
void sheet_refreshmap(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0)
{
	int h, bx, by, vx, vy, bx0, by0, bx1, by1;
	unsigned char *buf, sid, *map = ctl->map;
	struct SHEET *sht;
	if(vx0 < 0) { vx0 = 0; }
	if(vy0 < 0) { vy0 = 0; }
	if(vx1 > ctl->xsize) { vx1 = ctl->xsize; }
	if(vy1 > ctl->ysize) { vy1 = ctl->ysize; }
	for(h = h0; h <= ctl->top; h++)
	{
		sht = ctl->sheets[h];
		sid = sht - ctl->sheets0;		/* 图层号码 */
		buf = sht->buf;
		bx0 = vx0 - sht->vx0;			/* 屏幕上的位置对应到当前图层的位置 */
		by0 = vy0 - sht->vy0;
		bx1 = vx1 - sht->vx0;
		by1 = vy1 - sht->vy0;
		if(bx0 < 0) { bx0 = 0; }		/* 只在当前层的范围内处理 */
		if(by0 < 0) { by0 = 0; }
		if(bx1 > sht->bxsize) { bx1 = sht->bxsize; }
		if(by1 > sht->bysize) { by1 = sht->bysize; }
		for(by = by0; by < by1; by++)
		{
			vy = sht->vy0 + by;
			for(bx = bx0; bx < bx1; bx++)
			{
				vx = sht->vx0 + bx;
				if(buf[by * sht->bxsize + bx] != sht->col_inv)
					map[vy * ctl->xsize + vx] = sid;
			}
		}
	}
	return;
}

/* 重新绘制部分画面 */
/*vx,vy: 在屏幕上的位置; bx,by: 在图层上的位置*/
void sheet_refreshsub(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0, int h1)
{
	int h, bx, by, vx, vy, bx0, by0, bx1, by1;
	unsigned char *buf, *vram = ctl->vram, *map = ctl->map, sid;
	struct SHEET *sht;
	
	if(vx0 < 0)
		vx0 = 0;
	if(vy0 < 0)
		vy0 = 0;
	if(vx1 > ctl->xsize)
		vx1 = ctl->xsize;
	if(vy1 > ctl->ysize)
		vy1 = ctl->ysize;
	
	for(h = h0; h <= h1; h++)
	{
		sht = ctl->sheets[h];
		buf = sht->buf;
		sid = sht - ctl->sheets0;
		
		bx0 = vx0 - sht->vx0;		/* 屏幕上的位置对应到当前图层的位置 */
		by0 = vy0 - sht->vy0;
		bx1 = vx1 - sht->vx0;
		by1 = vy1 - sht->vy0;
		if(bx0 < 0) { bx0 = 0; }	/* 只在当前层的范围内处理 */
		if(by0 < 0) { by0 = 0; }
		if(bx1 > sht->bxsize) { bx1 = sht->bxsize; }
		if(by1 > sht->bysize) { by1 = sht->bysize; }
		for(by = by0; by < by1; by++)
		{
			vy = sht->vy0 + by;
			for(bx = bx0; bx < bx1; bx++)
			{
				vx = sht->vx0 + bx;
				if(map[vy * ctl->xsize + vx] == sid)
					vram[vy * ctl->xsize + vx] = buf[by * sht->bxsize + bx];
			}
		}
	}
}

/* 调整图层高度 */
void sheet_updown(struct SHEET *sht, int height)
{
	struct SHTCTL *ctl = sht->ctl;
	int h, old = sht->height;
	
	/* 如果指定的高度过高或过低，则需要修正 */
	if(height > ctl->top + 1)
		height = ctl->top + 1;
	if(height < -1)
		height = -1;
	sht->height = height;
	
	if(old > height)		/* 图层降低了 */
	{
		if(height >= 0)
		{
			for(h = old; h > height; h--)
			{
				ctl->sheets[h] = ctl->sheets[h - 1];
				ctl->sheets[h]->height = h;
			}
			ctl->sheets[height] = sht;
			sheet_refreshmap(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height + 1);
			sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height + 1, old);
		}
		else		/* 隐藏 */
		{
			for(h = old; h < ctl->top; h++)
			{
				ctl->sheets[h] = ctl->sheets[h + 1];
				ctl->sheets[h]->height = h;
			}
			ctl->top--;
			sheet_refreshmap(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, 0);
			sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, 0, old - 1);
		}
	}
	else if(old < height)     /* 图层提高了 */
	{
		if(old >= 0)
		{
			for(h = old; h < height; h++)
			{
				ctl->sheets[h] = ctl->sheets[h + 1];
				ctl->sheets[h]->height = h;
			}
			ctl->sheets[height] = sht;
		}
		else
		{
			for(h = ctl->top; h >= height; h--)
			{
				ctl->sheets[h + 1] = ctl->sheets[h];
				ctl->sheets[h + 1] = h + 1;
			}
			ctl->sheets[height] = sht;
			ctl->top++;
		}
		sheet_refreshmap(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height);
		sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height, height);
	}
	return;
}

/* 重新绘制画面，支持部分更新 */
void sheet_refresh(struct SHEET *sht, int bx0, int by0, int bx1, int by1)
{
	if(sht->height >= 0)
		sheet_refreshsub(sht->ctl, sht->vx0 + bx0, sht->vy0 + by0, sht->vx0 + bx1, sht->vy0 + by1, sht->height, sht->height);
	return;
}

/* 图层平移 */
void sheet_slide(struct SHEET *sht, int vx0, int vy0)
{
	struct SHTCTL *ctl = sht->ctl;
	int old_vx0 = sht->vx0, old_vy0 = sht->vy0;
	sht->vx0 = vx0;
	sht->vy0 = vy0;
	if(sht->height >= 0)
	{
		sheet_refreshmap(ctl, old_vx0, old_vy0, old_vx0 + sht->bxsize, old_vy0 + sht->bysize, 0);
		sheet_refreshmap(ctl, vx0, vy0, vx0 + sht->bxsize, vy0 + sht->bysize, sht->height);
		sheet_refreshsub(ctl, old_vx0, old_vy0, old_vx0 + sht->bxsize, old_vy0 + sht->bysize, 0, sht->height - 1);
		sheet_refreshsub(ctl, vx0, vy0, vx0 + sht->bxsize, vy0 + sht->bysize, sht->height, sht->height);
	}
	return;
}

void sheet_free(struct SHEET * sht)
{
	if(sht->height >= 0)
		sheet_updown(sht, -1);
	sht->flags = 0;
	return;
}


















