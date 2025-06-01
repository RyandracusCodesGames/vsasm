.arch mips2

.text
.globl example
example:
	ll $a1, 4($a0)
	lw $a2, 4($a1)
	addi $a2, 0x2       ; mips2 removes load delays, can access loaded register from next opcode
	li.s $f2, 1.5
	li.s $f4, 2.5
	mul.s $f2, $f2, $f4
	sqrt.s $f2
	li $t0, 1
	li $t1, 2
	umul $t0, $t0, $t1 
	addi $t0, 0x10
	subu $t0, $t1 
	sub.s $f2, $f6
	jr $ra
	nop