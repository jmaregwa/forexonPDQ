.text
    .global  entry_point
	.extern  AppInfo
	.global  AppInfo

	.extern  main
	.global  main

entry_point:
 	B        PosMain
	B        AppInfo
	
	.section	.rodata
	.align	2
.LC0:
	.ascii	"  Divided by 0  \000"
	.align	2
.LC1:
	.ascii	"----------------\000"
	.align	2
.LC2:
	.ascii	"Program abort at\000"
	.align	2
.LC3:
	.ascii	"   0x%08X\000"
	.text
	.align	2
	.global	__div0
	.type	__div0, %function
__div0:
	@ args = 0, pretend = 0, frame = 4
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
	stmfd	sp!, {fp, ip, lr, pc}
	sub	fp, ip, #4
	sub	sp, sp, #8
#APP
	mov r0, sp
ldr r3, [sp, #24]
	str	r3, [fp, #-16]
	
	@ lcdCls();
	bl	lcdCls
	
	@ lcdDisplay(0, 0, DISP_CFONT|DISP_CLRLINE, "  Divided by 0  ");
	ldr	r3, =.LC0
	mov	r0, #0
	mov	r1, #0
	mov	r2, #33
	bl	lcdDisplay
	
	@lcdDisplay(0, 2, DISP_CFONT|DISP_CLRLINE, "----------------");
	ldr	r3, =.LC1
	mov	r0, #0
	mov	r1, #2
	mov	r2, #33
	bl	lcdDisplay
	
	@lcdDisplay(0, 4, DISP_CFONT|DISP_CLRLINE, "Program abort at");
	ldr	r3, =.LC2
	mov	r0, #0
	mov	r1, #4
	mov	r2, #33
	bl	lcdDisplay
	
	@lcdDisplay(0, 6, DISP_CFONT|DISP_CLRLINE, "   0x%08X", lr);
	ldr	ip, =.LC3
	ldr	r3, [fp, #-16]
	str	r3, [sp, #0]
	mov	r0, #0
	mov	r1, #6
	mov	r2, #33
	mov	r3, ip
	bl	lcdDisplay
	
	@ while (1) ;
dead:
	b	dead

PosMain:
    STMFD	  SP!, {LR}	 @ Store the return address
    BL        INT_Clear_BSS
    BL        main
    LDMFD     SP!, {PC}^

INT_bss_start:
    .word     Image__bss__ZI__Base
INT_bss_end:
    .word     Image__bss__ZI__Limit
INT_Clear_BSS:
    @ Get BSS start and end addresses
	LDR     r0,INT_bss_start
    LDR     r1,INT_bss_end
   @ Get a value to clear BSS
	MOV     r2,#0
   @ Clear the entire memory range
INT_BSS_Clear_Loop:
	CMP     r0,r1                           @ Are the start and end equal?
	STRNE   r2,[r0],#4                      @ Clear 4-bytes and move to next 4-bytes
	BNE     INT_BSS_Clear_Loop              @ Continue clearing until done
	MOV     pc, lr
