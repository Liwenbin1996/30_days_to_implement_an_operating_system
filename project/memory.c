/* 内存管理 */

#include "bootpack.h"

#define EFLAGS_AC_BIT		0x00040000
#define CR0_CACHE_DISABLE	0x60000000

/* 486以上的CPU使用高速缓冲存储器，为了测试内存是否可以读写，必须先禁止掉 */
unsigned int memtest(unsigned int start, unsigned int end)
{
	char flg486 = 0;
	unsigned int eflg, cr0, i;
	
	/* 确认CPU是386还是486以上 */
	eflg = io_load_eflags();
	eflg |= EFLAGS_AC_BIT;				/* AC-bit = 1 */
	io_store_eflags(eflg);
	eflg = io_load_eflags();
	if((eflg & EFLAGS_AC_BIT) != 0)
		flg486 = 1;				/* 如果是386，即使设定AC=1，AC的值还会自动回到0 */
	eflg &= ~EFLAGS_AC_BIT;
	io_store_eflags(eflg);
	
	if(flg486 != 0)
	{
		cr0 = load_cr0();
		cr0 |= CR0_CACHE_DISABLE;		/* 禁止缓存 */
		store_cr0(cr0);
	}
	
	i = memtest_sub(start, end);
	
	if(flg486 != 0)
	{
		cr0 = load_cr0();
		cr0 &= ~CR0_CACHE_DISABLE;		/* 允许缓存 */
		store_cr0(cr0);
	}
	
	return i;
}

void memman_init(struct MEMMAN *man)
{
	man->frees = 0;
	man->maxfrees = 0;
	man->lostsize = 0;
	man->losts = 0;
	return;
}

unsigned int memman_total(struct MEMMAN *man)
{
	unsigned int i, t = 0;
	for(i = 0; i < man->frees; i++)
	{
		t += man->free[i].size;
	}
	return t;
}

unsigned int memman_alloc(struct MEMMAN *man, unsigned int size)
{
	unsigned int i, a;
	for(i = 0; i < man->frees; i++)		/* 找到足够大的内存 */
	{
		if(man->free[i].size >= size)
		{
			a = man->free[i].addr;
			man->free[i].addr += size;
			man->free[i].size -= size;
			if(man->free[i].size == 0)
			{
				man->frees--;
				for(; i < man->frees; i++)
					man->free[i] = man->free[i+1];
			}
			return a;
		}
	}
	return 0;		/* 无可用空间 */
}

int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size)
{
	int i, j;
	
	for(i = 0; i < man->frees; i++)
		if(man->free[i].addr > addr)
			break;
	
	/* 可以与前后两块内存合并 */
	if(i > 0)
	{
		if(man->free[i-1].addr + man->free[i-1].size == addr)
		{
			man->free[i-1].size += size;
			if(i < man->frees)
			{
				if(addr + size == man->free[i].addr)
				{
					man->free[i-1].size += man->free[i].size;
					man->frees--;
					for(; i < man->frees; i++)
					{
						man->free[i] = man->free[i+1];
					}
				}
			}
			return 0;
		}
	}
	
	/* 可以与后一块内存合并 */
	if(i < man->frees)
	{
		if(addr + size == man->free[i].addr)
		{
			man->free[i].addr = addr;
			man->free[i].size += size;
			return 0;
		}
	}
	
	/* 不可合并 */
	if(man->frees < MEMMAN_FREES)
	{
		for(j = man->frees; j > i; j--)
		{
			man->free[j] = man->free[j-1];
		}
		man->frees++;
		if(man->maxfrees < man->frees)
			man->maxfrees = man->frees;
		man->free[i].addr = addr;
		man->free[i].size = size;
		return 0;
	}
	/* 内存块达到管理上线只好暂时丢弃 */
	man->losts++;
	man->lostsize += size;
	return -1;
}

unsigned int memman_alloc_4k(struct MEMMAN *man, unsigned int size)
{
	unsigned int a;
	/* 4KB为单位向上取整 */
	size = (size + 0xfff) & 0xfffff000;
	a = memman_alloc(man, size);
	return a;
}

int memman_free_4k(struct MEMMAN *man, unsigned int addr, unsigned int size)
{
	int i;
	size = (size + 0xfff) & 0xfffff000;
	i = memman_free(man, addr, size);
	return i;
}
















