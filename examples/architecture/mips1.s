.arch mips1

.text
.globl example
example:
	li.s $f2, 1.5
	li.s $f4, 2.5
	lw $a2, 4($a1)
	nop
	addi $a2, 0x2
	mul.s $f2, $f2, $f4
	li $t0, 1
	li $t1, 2
	umul $t0, $t0, $t1 
	addi $t0, 0x10
	subu $t0, $t1 
	sub.s $f2, $f6
	jr $ra