
	.text
	.global	main

	.ascii	"__DRIVER_START__"

.thumb
.align 2
main:
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

.align 2
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

	.ascii	"___DRIVER_END___"
