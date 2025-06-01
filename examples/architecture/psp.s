.arch psp

.text
.globl TestPSPOpcodes
TestPSPOpcodes:
	vadd.s S123, S110, S122
	vsub.s S123, S110, S122
	lv.s   S110, 0x20($s0)
	svl.q	C220,0x40($s1)
	vsub.p	R122,C430,C010
	vsin.s S123, S110
	vsqrt.t C121,C430
	vdiv.q	R001,C430,C010
	jr $ra
	vnop