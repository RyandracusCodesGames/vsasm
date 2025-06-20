#-----------------------------------------------------------
# VideoStation Assembler
# (C) 2025 Ryandracus Chapman
#-----------------------------------------------------------
# PlayStation Graphics Program 3: Draw a Gouraud-Shaded Triangle
#-----------------------------------------------------------
# Video Mode: 256x240 NTSC 16-BIT COLOR
#-----------------------------------------------------------
.syntax asmpsx
.arch psx

VS_IO equ $1F800000
VS_GP0 equ $1810
VS_GP1 equ $1814
VS_FILL_SCREEN equ $2000000
VS_SHADE_TRIANGLE equ $30000000
VS_MODE equ 0 
VS_WIDTH equ 256 
VS_HEIGHT equ 240

; CHOOSE R,G, B VALUES FROM 0 TO 255

VS_R equ 0
VS_G equ 0
VS_B equ 0

; vs_gp0 = vs_io_addr + vs_gp0;
; vs_gp1 = vs_io_addr + vs_gp1;

InitVideo:
	li t0, VS_IO           ; vs_io_addr = $1F800000; (base i/o address of memory map) 
	sw zero, VS_GP1(t0)    ; *vs_gp1 = vs_cmd_gpu_reset;
	li t1, $4000000        ; vs_cmd_dma_req = $4000000 (no dma)
	sw t1, VS_GP1(t0)      ; *vs_gp0 = vs_cmd_dma_req;
	li t1, $8000000        ; vs_cmd_display_mode = $8000000 + mode;
	addiu t1, t1, VS_MODE  ; vs_cmd_display_enable += mode;
	sw t1, VS_GP1(t0)      ; *vs_gp1 = vs_cmd_display_enable;
	li t5, $C4E24E         ; vs_hrange = $C4E24E;
	li t6, $040010         ; vs_vrange = $040010;
	li t1, $06000000       ; vs_cmd_horizontal_range = $6000000
	addu t1, t1, t5        ; vs_cmd_horizontal_range += vs_hrange;
	sw t1, VS_GP1(t0)      ; *vs_gp1 = vs_cmd_horizontal_range;
	li t1, $07000000       ; vs_cmd_vertical_range = $7000000;
	addu t1, t1, t6        ; vs_cmd_vertical_range += vs_vrange;
	sw t1, VS_GP1(t0)      ; *vs_gp1 = vs_cmd_vertical_range;
	li t1, $05000000       ; vs_cmd_display_x1y1 = $5000000; (start x, y of display = 0)
	sw t1, VS_GP1(t0)      ; *vs_gp1 = vs_cmd_display_x1y1;
	li   t1, $E1000000     ; cmd = gpu0_cmd_tex_page;
	addi t1, t1, $000508   ; cmd += $000508;  
	sw   t1, VS_GP0(t0)    ; *vs_gpu0 = cmd;
	li   t1, $E3000000     ; cmd = gpu0_cmd_draw_area_top_left;
	sw   t1, VS_GP0(t0)    ; *vs_gpu0 = cmd;
	li   t1, $E4000000     ; cmd = gpu0_cmd_draw_area_bot_right;
	li   t2, VS_HEIGHT
	li   t3, VS_WIDTH
	sll  t2, t2, $0A      ; height <<= 10;
	addu t2, t2, t3       ; height += width;
	addu t1, t1, t2       ; cmd += height;
	sw   t1, VS_GP0(t0)   ; *vs_gpu0 = cmd;
	li   t1, $E5000000    ; cmd = gpu0_cmd_draw_offset;
	sw   t1, VS_GP0(t0)   ; *vs_gpu0 = cmd;
	li   t1, $03000000    ; cmd = gpu1_cmd_display_enable;
	sw   t1, VS_GP1(t0)   ; *vs_gpu1 = cmd;
FillScreen:
	li t1, VS_FILL_SCREEN  ; vs_cmd_fill_screen = VS_FILL_SCREEN;
	li t2, VS_B 
	andi t2, t2, $ff       ; b &= $ff;
	sll t2, t2, $10        ; b <<= 16;
	li t3, VS_G 
	andi t3, t3, $ff       ; g &= $ff;
	sll t3, t3, $8         ; g <<= 8;
	addu t2, t2, t3        ; b += g;
	addiu t2, t2, VS_R     ; b += r;
	addu t1, t1, t2        ; vs_cmd_fill_screen += b;
	sw t1, VS_GP0(t0)      ; *vs_gp0 = vs_cmd_fill_screen;
	sw zero, VS_GP0(t0)    ; *vs_gp0 = 0; (x1, y1 = 0) 
	li t2, VS_HEIGHT
	andi t2, t2, $FFFF     ; height &= $FFFF;
	sll t2, t2, $10        ; height <<= 16;
	li t3, VS_WIDTH
	andi t3, t3, $FFFF     ; width &= $FFFF;
	addu t2, t2, t3        ; height += width; 
	sw t2, VS_GP0(t0)      ; *vs_gp0 = height;
	subi sp, sp, 36
	li a0, 80
	li a1, 140 
	li a2, 130
	li a3, 50 
	li t1, 180
	li t2, 140
	li t3, $0000FF
	li t4, $00FF00
	li t5, $FF0000
	sw t1, 16(sp) 
	sw t2, 20(sp) 
	sw t3, 24(sp) 
	sw t4, 28(sp) 
	sw t5, 32(sp) 
	jal ShadeTriangle
	nop
main:
	b main 
	nop
	addi sp, sp, 36

# Function: ShadeTriangle
# Purpose: Draws a gouraud-shaded triange to the display area
# a0: x1, a1: y1, a2: x2, a3: y2, 16(sp): x3, 20(sp): y3, 24(sp): color1, 28(sp): color2, 32(sp): color3
ShadeTriangle:
	lw t2, 24(sp)
	li t1, VS_SHADE_TRIANGLE ; vs_cmd_shade_triangle = $3000000; (delay slot)
	addu t1, t1, t2          ; vs_cmd_shade_triangle += color1;
	sw t1, VS_GP0(t0)        ; *vs_gp0 = vs_cmd_shade_triangle;
	andi a0, a0, $FFFF       ; x1 &= $FFFF;
	sll a1, a1, $10          ; y1 <<= 16;
	addu a1, a1, a0          ; y1 += x1; 
	sw a1, VS_GP0(t0)        ; *vs_gp0 = y1;
	lw t2, 28(sp)
	andi a2, a2, $FFFF       ; x2 &= $FFFF; (delay slot)
	sw t2, VS_GP0(t0)        ; *vs_gp0 = color2;
	sll a3, a3, $10          ; y2 <<= 16;
	addu a3, a3, a2          ; y2 += x2; 
	lw t1, 32(sp)
	lw a0, 16(sp)
	sw a3, VS_GP0(t0)        ; *vs_gp0 = y2;
	sw t1, VS_GP0(t0)        ; *vs_gp0 = color3;
	lw a1, 20(sp)
	andi a0, a0, $FFFF       ; x3 &= $FFFF;
	sll a1, a1, $10          ; y3 <<= 16;
	addu a1, a1, a0          ; y3 += x3; 
	sw a1, VS_GP0(t0)        ; *vs_gp0 = y3;
	jr ra 
	nop