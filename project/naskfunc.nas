; naskfunc
; TAB=4

[FORMAT "WCOFF"]				; 制作目标文件的模式，不链接
[INSTRSET "i486p"]				; 使用到486为止的指令
[BITS 32]						; 32位机器语言模式，为了和C语言链接
[FILE "naskfunc.nas"]			; 目标文件模式下必须设定文件名信息
		; 函数的声明
		GLOBAL	_io_hlt, _io_cli, _io_sti, _io_stihlt
		GLOBAL	_io_in8,  _io_in16,  _io_in32
		GLOBAL	_io_out8, _io_out16, _io_out32
		GLOBAL	_io_load_eflags, _io_store_eflags
		GLOBAL	_load_gdtr, _load_idtr
		GLOBAL	_asm_inthandler21, _asm_inthandler27, _asm_inthandler2c
		EXTERN	_inthandler21, _inthandler27, _inthandler2c
		
[SECTION .text]		; 代码段

_io_hlt:	; void io_hlt(void);
		HLT
		RET

_io_cli:	; void io_cli(void);
		CLI
		RET

_io_sti:	; void io_sti(void);
		STI
		RET

_io_stihlt:	; void io_stihlt(void);
		STI
		HLT
		RET

_io_in8:	; int io_in8(int port);
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,0
		IN		AL,DX
		RET

_io_in16:	; int io_in16(int port);
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,0
		IN		AX,DX
		RET

_io_in32:	; int io_in32(int port);
		MOV		EDX,[ESP+4]		; port
		IN		EAX,DX
		RET

_io_out8:	; void io_out8(int port, int data);
		MOV		EDX,[ESP+4]		; port
		MOV		AL,[ESP+8]		; data
		OUT		DX,AL
		RET

_io_out16:	; void io_out16(int port, int data);
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,[ESP+8]		; data
		OUT		DX,AX
		RET

_io_out32:	; void io_out32(int port, int data);
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,[ESP+8]		; data
		OUT		DX,EAX
		RET

_io_load_eflags:	; int io_load_eflags(void);
		PUSHFD
		POP		EAX				; EAX保存返回值
		RET

_io_store_eflags:	; void io_store_eflags(int eflags);
		MOV		EAX,[ESP+4]
		PUSH	EAX
		POPFD
		RET		
		
; GDTR寄存器是48位寄存器，低16位保存段上限，高32位保存GDT起始地址
; 一共6字节但我们传进来的参数是两个整型8字节，所以需要调整limit的内存分布
; IDTR寄存器同理
_load_gdtr:		; void load_gdtr(int limit, int addr);
		MOV		AX,[ESP+4]		; limit
		MOV		[ESP+6],AX
		LGDT	[ESP+6]
		RET

_load_idtr:		; void load_idtr(int limit, int addr);
		MOV		AX,[ESP+4]		; limit
		MOV		[ESP+6],AX
		LIDT	[ESP+6]
		RET

; 执行中断处理程序前要先保存CPU现场	
_asm_inthandler21:
		PUSH	ES
		PUSH	DS
		PUSHAD				; 按顺序PUSH EAX ECX EDX EBX ESP EBP ESI EDI
		;MOV		EAX,ESP		;注释掉的三行似乎没什么用？
		;PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler21
		;POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD				; 中断返回  弹出三个参数，一个给IP，一个给CS，一个给FLAG标志位
		
_asm_inthandler27:
		PUSH	ES
		PUSH	DS
		PUSHAD
		;MOV		EAX,ESP
		;PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler27
		;POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD

_asm_inthandler2c:
		PUSH	ES
		PUSH	DS
		PUSHAD
		;MOV		EAX,ESP
		;PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler2c
		;POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		