
	.text
	.global	main

	.ascii	"__DRIVER_START__"

.thumb
.align 2
main:
	adr	r0, a_GsfDriverMark
	bl	read_watermark

	ldr	r3, sub_init_irq
	bl	bx_r3

	ldr	r0, ofs_sound_work
	ldr	r3, sub_init_sound
	bl	bx_r3

	ldr	r0, num_songindex
	ldr	r3, sub_selectsong
	bl	bx_r3

infinite_loop:
	swi	5                       @ VSyncIntrWait
	b	infinite_loop

.align
a_GsfDriverMark:
	.asciz	"Natsume Driver Ripper by loveemu"

.align
sub_init_irq:
	.word	0

sub_init_sound:
	.word	0

ofs_sound_work:
	.word	0

sub_selectsong:
	.word	0

num_songindex:
	.word	0

bx_r3:
	cmp	r3, #0
	beq	locret_1
	bx	r3
locret_1:
	bx	lr

read_watermark:
loc_read_watermark:
	ldr	r1, [r0]
	add	r0, r0, #4
	lsr	r2, r1, #24
	bne	loc_read_watermark      @ repeat until NUL
	bx	lr

.align
	.ascii	"___DRIVER_END___"
