@ Relocatable sapphire block

	.global     main

	.ascii	"**DRIVER_START**"

main:
	.align
	.code 32

	stmdb	sp!, {pc}
	ldmia	sp!, {r0}
	mov	r1, #0x60

loc_readmark:                           @ save watermark from gsfopt
	ldr	r2, [r0]
	add	r0, r0, #4
	sub	r1, r1, #4
	cmp	r1, #0
	bne	loc_readmark
	b	loc_init

	.string	"Sappy Driver Ripper by CaitSith2\\Zoopd, (c) 2004, loveemu 2014."

loc_init:
	stmdb	sp!, {lr}
	ldr	r0, sappy_soundinit
	bl	bx_r0
	ldr	r0, num_3007FFC
	stmdb	sp!, {pc}
	ldmia	sp!, {r1}
	add	r1, r1, #0x30
	str	r1, [r0]                @ set IRQ handler address
	mov	r0, #0x4000000
	mov	r1, #8
	str	r1, [r0, #4]            @ set DISPSTAT (enable V-Blank IRQ)
	mov	r1, #1
	str	r1, [r0, #0x200]        @ set IE (request V-Blank interrupt)
	str	r1, [r0, #0x208]        @ set IME (activate interrupts)
	ldr	r0, num_songindex
	ldr	r1, sappy_selectsong
	bl	bx_r1

loc_main_loop:
	swi	0x00020000              @ Halt
	b	loc_main_loop

irq_handler:
	stmdb	sp!, {lr}
	ldr	r0, sappy_vsync
	bl	bx_r0
	ldr	r0, sappy_soundmain
	bl	bx_r0
	mov	r0, #0x4000000
	mov	r1, #0x10000
	add	r1, r1, #1
	str	r1, [r0, #0x200]        @ request V-Blank interrupt again
	ldr	r0, num_3007FFC
	str	r1, [r0, #-4]           @ set interrupt check flags at 0x03007ff8
	ldmia	sp!, {r0}
	bx	r0                      @ back to USER funciton

num_FFFFFFFF:
	.word	0xFFFFFFFF

sappy_selectsong:
	.word	0

sappy_soundmain:
	.word	0

sappy_soundinit:
	.word	0

sappy_vsync:
	.word	0

bx_r0:
	cmp	r0, #0
	beq	quit_sub
	bx	r0

bx_r1:
	cmp	r1, #0
	beq	quit_sub
	bx	r1

quit_sub:
	bx	lr

num_3007FFC:                            @ address of pointer to user IRQ handler
	.word	0x03007FFC

num_songindex:
	.word	1

	.ascii	"***DRIVER_END***"
