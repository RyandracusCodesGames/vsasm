.syntax asmpsx
.arch ps1

VS_IO equ $1F800000
VS_GP1 equ  $1814

.text
Work:
	li a0, VS_IO
	addi a1, $80<<24
	li t0, 12 + ((4*5) << 2)
	addi t0, 4 / 2
	addi t0, 4 + $40
	sw t0, VS_GP1-4(a0)
	jr ra
	nop