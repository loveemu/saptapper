
	.text
	.global	main

	.ascii	"__DRIVER_START__"

.arm
.align
main:
	adr	r1, main_thumb+1        @ set THUMB flag
	bx	r1

.thumb
.align 2
main_thumb:
	adr	r0, a_GsfDriverMark

loc_read_watermark:
	ldr	r1, [r0]
	add	r0, r0, #4
	lsr	r2, r1, #24
	bne	loc_read_watermark      @ repeat until NUL
	b	init

.align
a_GsfDriverMark:
	.asciz	"Sappy Driver Ripper by CaitSith2\\Zoopd, (c) 2004, 2014 loveemu"

.align
init:
	push	{lr}
	ldr	r3, sappy_soundinit
	bl	bx_r3
	ldr	r0, num_3007FFC
	adr	r1, irq_handler
	str	r1, [r0]                @ set IRQ handler address
	ldr	r0, num_4000000
	mov	r1, #8
	str	r1, [r0, #0x4]          @ set DISPSTAT (enable V-Blank IRQ)
	mov	r1, #1
	ldr	r0, num_4000200
	str	r1, [r0, #0x0]          @ set IE (request V-Blank interrupt)
	str	r1, [r0, #0x8]          @ set IME (activate interrupts)
	ldr	r0, num_songindex
	ldr	r3, sappy_selectsong
	bl	bx_r3                   @ call sappy_SelectSongByNum

main_loop:
	ldr	r3, sappy_soundmain
	bl	bx_r3
	swi	5                       @ VSyncIntrWait
	b	main_loop

.arm
.align
irq_handler:                            @ reference: gbatek, devkitadv/crtls/isr.S
	ldr	r3, num_4000200
	ldr	r1, [r3, #0x0]
	and	r0, r1, r1, lsr #16

	stmfd	sp!, {r0-r3, lr}

	adr	r1, vsync_callback+1
	ands	r2, r0, #0x0001
	bne	irq_callhandler
	b	irq_nohandler

irq_callhandler:
	mov	lr, pc
	bx	r1

irq_nohandler:
	ldmfd	sp!, {r0-r3, lr}

	strh	r0, [r3, #0x2]          @ acknowledge Interrupt
	ldr	r3, num_3007FFC
	str	r0, [r3, #-4]           @ set BIOS IRQ flags for (VBlank)IntrWait

	bx	lr

.thumb
.align 2
vsync_callback:
	push	{lr}

	ldr	r3, sappy_vsync
	bl	bx_r3

	pop	{r0}
	bx	r0

.align 2
num_3007FFC:
	.word	0x03007FFC

num_4000000:
	.word	0x04000000

num_4000200:
	.word	0x04000200

sappy_soundinit:
	.word	0

sappy_selectsong:
	.word	0

sappy_soundmain:
	.word	0

sappy_vsync:
	.word	0

num_songindex:
	.word	1

.thumb
.align 2
bx_r3:
	cmp	r3, #0
	beq	locret_1
	bx	r3
locret_1:
	bx	lr

.align
	.ascii	"___DRIVER_END___"
