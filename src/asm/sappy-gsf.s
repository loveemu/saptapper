@ Disassembly of relocatable sapphire block

	.global     _start
_start:
	.align
	.code 32

	stmdb	sp!, {pc}
	ldmia	sp!, {r0}
	mov	r1, #0x50

loc_000000C:                            @ save watermark from gsfopt
	ldr	r2, [r0]
	add	r0, r0, #4
	sub	r1, r1, #4
	cmp	r1, #0
	bne	loc_000000C
	b	loc_0000054

byte_0000024:
	.ascii	"Sappy Driver Ripper by CaitSith2\\Zoopd, (c) 2004"

loc_0000054:
	stmdb	sp!, {lr}
	ldr	r0, dword_00000E0       @ call sappy_SoundInit
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
	ldr	r0, dword_00000F4       @ song index
	ldr	r1, dword_00000D8
	bl	bx_r1                   @ call sappy_SelectSongByNum

loc_0000098:
	swi	0x00020000              @ Halt
	b	loc_0000098

sub_00000a0:                            @ IRQ handler
	stmdb	sp!, {lr}
	ldr	r0, dword_00000E4
	bl	bx_r0                   @ call sappy_VSync
	ldr	r0, dword_00000DC
	bl	bx_r0                   @ call sappy_SoundMain
	mov	r0, #0x4000000
	mov	r1, #0x10000
	add	r1, r1, #1
	str	r1, [r0, #0x200]        @ request V-Blank interrupt again
	ldr	r0, num_3007FFC
	str	r1, [r0, #-4]           @ set interrupt check flags at 0x03007ff8
	ldmia	sp!, {r0}
	bx	r0                      @ back to USER funciton

dword_00000D4:
	.word	0x00FFFFFF

dword_00000D8:                          @ sappy_SelectSongByNum function
	.word	0x08038159
dword_00000DC:                          @ sappy_SoundMain function
	.word	0x0803814D
dword_00000E0:                          @ sappy_SoundInit function
	.word	0x080380D5
dword_00000E4:                          @ sappy_VSync function
	.word	0x08037A89

bx_r0:
		bx	r0

bx_r1:
		bx	r1

num_3007FFC:                            @ address of pointer to user IRQ handler
	.word	0x03007FFC
dword_00000F4:                          @ song index
	.word	0x00000030
