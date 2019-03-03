; haribote-os boot asm
; TAB=4

BOTPAK	EQU		0x00280000		; bootpack.hrb(包含文件头信息)存放的位置
DSKCAC	EQU		0x00100000		; 最终保存软盘内容的位置
DSKCAC0	EQU		0x00008000		; 启动程序将软盘512字节之后的10个柱面读取到内存0x8200的位置，
								  0x8000~0x81ff预留，我认为只是为了方便之后对地址的计算
								  
; BOOT_INFO
CYLS	EQU		0x0ff0			; 启动区从软盘读入的柱面数
LEDS	EQU		0x0ff1			; 键盘的LED指示灯信息
VMODE	EQU		0x0ff2			; 画面模式 颜色的位数
SCRNX	EQU		0x0ff4			; 屏幕分辨率
SCRNY	EQU		0x0ff6
VRAM	EQU		0x0ff8			; 显存起始位置

		ORG		0xc200			; haribote.sys文件保存在软盘的0x4c00位置，对应到内存中就是0x8000+0x4c00，
								  我认为这句多余了
								  
; 设置屏幕模式

		MOV		AL,0x13			; VGA图形，320 x 200 x 8位颜色
		MOV		AH,0x00
		INT 	0x10
		MOV		BYTE [VMODE],8
		MOV		WORD [SCRNX],320
		MOV		WORD [SCRNY],200
		MOV		DWORD [VRAM],0x000a0000
		
; 获取键盘的LED状态

		MOV		AH,0x02
		INT		0x16			; 键盘中断
		MOV		[LEDS],AL

; ===========================================================================================================
; 以下为进入进入32位模式做准备，之后便由C语言来进行开发
; ===========================================================================================================

; 禁止所有中断

		MOV		AL,0xff
		OUT		0x21,AL			; 禁止主PIC中断
		NOP						; 如果连续执行OUT指令，有些机种可能无法运行
		OUT		0xa1,AL			; 禁止从PIC中断
		
		CLI						; 禁止CPU级别的中断
		
; 为了能让CPU访问1MB以上的空间，设定A20GATE，该电路集成在键盘控制电路上
; 为了兼容早期的16位系统，在执行激活命令前，电路被限制为只能使用1MB内存
	
		CALL	waitkbdout		; 确保向键盘控制电路传输数据的时候键盘电路已经做好接收准备
		MOV		AL,0xd1
		OUT		0x64,AL			; 发送指令
		CALL	waitkbdout
		MOV		AL,0xdf			; 发送数据(enable A20)
		OUT		0x60,AL
		CALL	waitkbdout
		
; 切换到保护模式
; 保护模式下段寄存器的值不再是实际地址的一部分，而是段表的编号

[INSTRSET "i486p"]				; 使用到486为止的指令

		LGDT	[GDTR0]			; 设定临时GDT
		MOV		EAX,CR0			; 通过设置CR0寄存器，切换至32位保护模式
		AND		EAX,0x7fffffff	; bit31=0(禁止分页)
		OR		EAX,0x00000001	; bit0=1(切换到保护模式)
		MOV		CR0,EAX
		JMP		pipelineflush	; 更新所有的段寄存器的初值
pipelineflush:
		MOV		AX,1 << 3		; 设置段号为1(整块内存)，段寄存器低3位有其他含义
		MOV		DS,AX
		MOV		ES,AX
		MOV		FS,AX
		MOV		GS,AX
		MOV		SS,AX
		
; 将bootpack.hrb文件复制到0x00280000
; haribote.sys=asmhead.bin+bootpack.hrb，所以bootpack标签后的内容就是bootpack.hrb

		MOV		ESI,bootpack	; 源地址
		MOV		EDI,BOTPAK		; 目标地址
		MOV		ECX,512*1024/4	; 复制次数，一次4字节
		CALL	memcpy
		
; 将启动区数据复制到0x00100000

		MOV		ESI,0x7c00
		MOV		EDI,DSKCAC
		MOV		ECX,512/4
		CALL	memcpy
		
; 将导入的10个柱面的内容复制到0x00100200

		MOV		ESI,DSKCAC0+512
		MOV		EDI,DSKCAC+512
		MOV		ECX,0
		MOV		CL,BYTE [CYLS]
		IMUL	ECX,512*18*2/4	; 柱面数 -> 字节数
		SUB		ECX,512/4		; 启动区已经导入，不要重复导入
		CALL	memcpy
		

; asmhead的任务已经完成，现在将控制权交给bootpack
; 看不懂看不懂
		
		MOV		EBX,BOTPAK
		MOV		ECX,[EBX+16]
		ADD		ECX,3
		SHR		ECX,2			; ECX /= 4
		JZ		skip
		MOV		ESI,[EBX+20]
		ADD		ESI,EBX
		MOV		EDI,[EBX+12]
		CALL	memcpy
skip:
		MOV		ESP,[EBX+12]	; 初始化栈
		JMP		DWORD 2*8:0x0000001b	; 开始执行bootpack.hrb的内容
	
	
waitkbdout:
		IN		AL,0x64
		AND		AL,0x02	
		JNZ		waitkbdout		; 键盘控制电路未准备好
		RET
		
memcpy:
		MOV		EAX,[ESI]
		ADD		ESI,4
		MOV		[EDI],EAX
		ADD		EDI,4
		SUB		ECX,1
		JNZ		memcpy
		RET
		
; 看不懂看不懂
		ALIGNB	16
GDT0:
		RESB	8
		DW		0xffff,0x0000,0x9200,0x00cf
		DW		0xffff,0x0000,0x9a28,0x0047
		
		DW		0
GDTR0:
		DW		8*3-1
		DD		GDT0
		
		ALIGNB	16
		
bootpack:





















		
		
		
		
		
		
		
		