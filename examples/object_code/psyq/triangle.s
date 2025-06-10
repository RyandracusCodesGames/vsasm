.syntax asmpsx
.arch psx

VS_IO equ $1F800000
VS_GP0 equ $1810
VS_GP1 equ $1814
VS_SHADE_TRIANGLE equ $30000000

.text

# Purpose: Draws a gouraud-shaded triange to the display area
# a0: x1, a1: y1, a2: x2, a3: y2, 16(sp): x3, 20(sp): y3, 24(sp): color1, 28(sp): color2, 32(sp): color3
.globl GouraudShadeTriangle
GouraudShadeTriangle:
	li t0, VS_IO 
	lw t2, 24(sp)
	li t1, VS_SHADE_TRIANGLE ; vs_cmd_shade_triangle = 0x3000000; (delay slot)
	addu t1, t2              ; vs_cmd_shade_triangle += color1;
	sw t1, VS_GP0(t0)        ; *vs_gp0 = vs_cmd_shade_triangle;
	andi a0, $FFFF     	     ; x1 &= 0xFFFF;
	sll a1, $10        	     ; y1 <<= 16;
	addu a1, a0              ; y1 += x1; 
	sw a1, VS_GP0(t0)        ; *vs_gp0 = y1;
	lw t2, 28(sp)
	andi a2, $FFFF           ; x2 &= 0xFFFF; (delay slot)
	sw t2, VS_GP0(t0)        ; *vs_gp0 = color2;
	sll a3, $10              ; y2 <<= 16;
	addu a3, a2              ; y2 += x2; 
	lw t1, 32(sp)
	lw a0, 16(sp)
	sw a3, VS_GP0(t0)        ; *vs_gp0 = y2;
	sw t1, VS_GP0(t0)        ; *vs_gp0 = color3;
	lw a1, 20(sp)
	andi a0, $FFFF           ; x3 &= 0xFFFF;
	sll a1, $10              ; y3 <<= 16;
	addu a1, a0              ; y3 += x3; 
	sw a1, VS_GP0(t0)        ; *vs_gp0 = y3;
	jr ra 
	nop	