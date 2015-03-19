
	.text
	.global	main

	.ascii	"__DRIVER_START__"

.thumb
.align 2
main:
	ldr	r3, sub_init
	bl	bx_r3

	ldr	r0, ofs_init_2_a0
	ldr	r3, sub_init_2
	bl	bx_r3

	ldr	r0, num_songindex
	ldr	r3, sub_selectsong
	bl	bx_r3

infinite_loop:
	swi	5                       @ VSyncIntrWait
	b	infinite_loop

.align 2
sub_init:
	.word	0

sub_init_2:
	.word	0

ofs_init_2_a0:
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
