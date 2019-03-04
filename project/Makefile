OBJS_BOOTPACK = bootpack.obj naskfunc.obj hankaku.obj graphic.obj \
		dsctbl.obj int.obj fifo.obj keyboard.obj mouse.obj memory.obj
		
TOOLPATH = ../z_tools/
INCPATH  = ../z_tools/haribote/

MAKE     = $(TOOLPATH)make.exe -r
NASK     = $(TOOLPATH)nask.exe
CC1      = $(TOOLPATH)cc1.exe -I$(INCPATH) -Os -Wall -quiet		# I: 指定寻找头文件的第一个目录 
																# O: 代码优化等级
																# Wall：生成所有警告信息
GAS2NASK = $(TOOLPATH)gas2nask.exe -a
OBJ2BIM  = $(TOOLPATH)obj2bim.exe
MAKEFONT = $(TOOLPATH)makefont.exe
BIN2OBJ  = $(TOOLPATH)bin2obj.exe
BIM2HRB  = $(TOOLPATH)bim2hrb.exe
RULEFILE = $(TOOLPATH)haribote/haribote.rul
EDIMG    = $(TOOLPATH)edimg.exe
IMGTOL   = $(TOOLPATH)imgtol.com
COPY     = copy
DEL      = del


default:
	$(MAKE) img
	
	
ipl10.bin : ipl10.nas
	$(NASK) ipl10.nas ipl10.bin ipl10.lst
	
asmhead.bin : asmhead.nas
	$(NASK) asmhead.nas asmhead.bin asmhead.lst
	
hankaku.bin : hankaku.txt
	$(MAKEFONT) hankaku.txt hankaku.bin
	
hankaku.obj : hankaku.bin
	$(BIN2OBJ) hankaku.bin hankaku.obj _hankaku
	
bootpack.bim : $(OBJS_BOOTPACK) 
	$(OBJ2BIM) @$(RULEFILE) out:bootpack.bim stack:3136k map:bootpack.map \
		$(OBJS_BOOTPACK)
		
bootpack.hrb : bootpack.bim 
	$(BIM2HRB) bootpack.bim bootpack.hrb 0
	
haribote.sys : asmhead.bin bootpack.hrb
	copy /B asmhead.bin+bootpack.hrb haribote.sys
	
haribote.img : ipl10.bin haribote.sys
	$(EDIMG) imgin:../z_tools/fdimg0at.tek \
		wbinimg src:ipl10.bin len:512 from:0 to:0 \
		copy from:haribote.sys to:@: \
		imgout:haribote.img

#.c文件 -> gas汇编语言 -> nask汇编语言 -> .obj目标文件 -> .bim链接文件 -> .exe可执行文件
		
%.gas : %.c
	$(CC1) -o $*.gas $*.c

%.nas : %.gas
	$(GAS2NASK) $*.gas $*.nas

%.obj : %.nas
	$(NASK) $*.nas $*.obj $*.lst
	
	
img :
	$(MAKE) haribote.img
	
run :
	$(MAKE) img
	$(COPY) haribote.img ..\z_tools\qemu\fdimage0.bin
	$(MAKE) -C ../z_tools/qemu
	
clean :
	-$(DEL) *.bin		#"-"表示忽略当前此行命令执行时候所遇到的错误
	-$(DEL) *.lst
	-$(DEL) *.obj
	-$(DEL) bootpack.map
	-$(DEL) bootpack.bim
	-$(DEL) bootpack.hrb
	-$(DEL) haribote.sys
	
src_only : 
	$(MAKE) clean
	-$(DEL) haribote.img
	
	












