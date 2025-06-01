#-------------------------------------
# The original Sony PlayStation has no FPU
# but it does have a math coprocessor that 
# operates on fixed-point numbers
#-------------------------------------

.arch psx

VS_CMD_INIT_GTE equ 0x40000000

.text 
.globl InitGTE
InitGTE:
	li $8, VS_CMD_INIT_GTE   ; gte_cmd = VS_CMD_INIT_GTE;
	mtc0 $8, $12             ; *cop2r12 = gte_cmd;
	nop 
	jr $ra 
	nop

.globl FixedMul
FixedMul:
	mult $a0, $a1
	mflo $v0
	sra $v0, 0x0C
	jr $ra
	nop
	
.globl FixedDiv
FixedDiv:
	sll $a0, 0x0C 
	div $a0, $a1 
	mflo $v0 
	jr $ra
	nop