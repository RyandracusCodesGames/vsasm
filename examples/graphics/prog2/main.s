#-----------------------------------------------------------
# VideoStation Assembler
# (C) 2025 Ryandracus Chapman
#-----------------------------------------------------------
# PlayStation Graphics Program 2: Draw flat-shaded/gouraud-shaded lines
#-----------------------------------------------------------
# Video Mode: 256x240 NTSC 16-BIT COLOR
#-----------------------------------------------------------

VS_IO equ 0x1F800000
VS_GP0 equ 0x1810
VS_GP1 equ 0x1814
VS_FILL_SCREEN equ 0x2000000
VS_DRAW_LINE equ 0x40000000
VS_SHADE_LINE equ 0x50000000
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
	li $t0, VS_IO           ; vs_io_addr = 0x1F800000; (base i/o address of memory map) 
	sw $zero, VS_GP1($t0)   ; *vs_gp1 = vs_cmd_gpu_reset;
	li $t1, 0x4000000       ; vs_cmd_dma_req = 0x4000000 (no dma)
	sw $t1, VS_GP1($t0)      ; *vs_gp0 = vs_cmd_dma_req;
	li $t1, 0x8000000       ; vs_cmd_display_mode = 0x8000000 + mode;
	addiu $t1, $t1, VS_MODE ; vs_cmd_display_enable += mode;
	sw $t1, VS_GP1($t0)     ; *vs_gp1 = vs_cmd_display_enable;
	li $t5, 0xC4E24E        ; vs_hrange = 0xC4E24E;
	li $t6, 0x040010        ; vs_vrange = 0x040010;
	li $t1, 0x06000000      ; vs_cmd_horizontal_range = 0x6000000
	addu $t1, $t1, $t5      ; vs_cmd_horizontal_range += vs_hrange;
	sw $t1, VS_GP1($t0)     ; *vs_gp1 = vs_cmd_horizontal_range;
	li $t1, 0x07000000      ; vs_cmd_vertical_range = 0x7000000;
	addu $t1, $t1, $t6      ; vs_cmd_vertical_range += vs_vrange;
	sw $t1, VS_GP1($t0)     ; *vs_gp1 = vs_cmd_vertical_range;
	li $t1, 0x05000000      ; vs_cmd_display_x1y1 = 0x5000000; (start x, y of display = 0)
	sw $t1, VS_GP1($t0)     ; *vs_gp1 = vs_cmd_display_x1y1;
	li   $t1, 0xE1000000    ; cmd = gpu0_cmd_tex_page;
	addi $t1, $t1, 0x000508 ; cmd += 0x000508;  
	sw   $t1, VS_GP0($t0)   ; *vs_gpu0 = cmd;
	li   $t1, 0xE3000000    ; cmd = gpu0_cmd_draw_area_top_left;
	sw   $t1, VS_GP0($t0)   ; *vs_gpu0 = cmd;
	li   $t1, 0xE4000000    ; cmd = gpu0_cmd_draw_area_bot_right;
	li   $t2, VS_HEIGHT
	li   $t3, VS_WIDTH
	sll  $t2, $t2, 0x0A     ; height <<= 10;
	addu $t2, $t2, $t3      ; height += width;
	addu $t1, $t1, $t2      ; cmd += height;
	sw   $t1, VS_GP0($t0)   ; *vs_gpu0 = cmd;
	li   $t1, 0xE5000000    ; cmd = gpu0_cmd_draw_offset;
	sw   $t1, VS_GP0($t0)   ; *vs_gpu0 = cmd;
	li   $t1, 0x03000000    ; cmd = gpu1_cmd_display_enable;
	sw   $t1, VS_GP1($t0)   ; *vs_gpu1 = cmd;
FillScreen:
	li $t1, VS_FILL_SCREEN  ; vs_cmd_fill_screen = VS_FILL_SCREEN;
	li $t2, VS_B 
	andi $t2, $t2, 0xff     ; b &= 0xff;
	sll $t2, $t2, 0x10      ; b <<= 16;
	li $t3, VS_G 
	andi $t3, $t3, 0xff     ; g &= 0xff;
	sll $t3, $t3, 0x8       ; g <<= 8;
	addu $t2, $t2, $t3      ; b += g;
	addiu $t2, $t2, VS_R    ; b += r;
	addu $t1, $t1, $t2      ; vs_cmd_fill_screen += b;
	sw $t1, VS_GP0($t0)     ; *vs_gp0 = vs_cmd_fill_screen;
	sw $zero, VS_GP0($t0)   ; *vs_gp0 = 0; (x1, y1 = 0) 
	li $t2, VS_HEIGHT
	andi $t2, $t2, 0xFFFF   ; height &= 0xFFFF;
	sll $t2, $t2, 0x10      ; height <<= 16;
	li $t3, VS_WIDTH
	andi $t3, $t3, 0xFFFF   ; width &= 0xFFFF;
	addu $t2, $t2, $t3      ; height += width; 
	sw $t2, VS_GP0($t0)     ; *vs_gp0 = height;
	subi $sp, $sp, 24
	li $a0, 50
	li $a1, 50 
	li $a2, 100
	li $a3, 100 
	li $t1, 0x00FF00
	sw $t1, 16($sp) 
	jal DrawLine
	nop
	li $a0, 50
	li $a1, 0 
	li $a2, 50
	li $a3, 50 
	li $t1, 0x00FFFF
	sw $t1, 16($sp) 
	jal DrawLine
	nop
	li $a0, 100
	li $a1, 0 
	li $a2, 100
	li $a3, 50 
	li $t1, 0x00FFFF
	sw $t1, 16($sp) 
	jal DrawLine
	nop
	li $a0, 100
	li $a1, 50 
	li $a2, 150
	li $a3, 100 
	li $t1, 0x0000FF
	li $t2, 0xFF0000
	sw $t1, 16($sp) 
	sw $t2, 20($sp) 
	jal ShadeLine 
	nop
	li $a0, 150
	li $a1, 100 
	li $a2, 150
	li $a3, 239 
	li $t1, 0x00FFFF
	li $t2, 0xFF0000
	sw $t1, 16($sp) 
	sw $t2, 20($sp) 
	jal ShadeLine 
	nop
	li $a0, 100
	li $a1, 100 
	li $a2, 100
	li $a3, 239 
	li $t1, 0x00FFFF
	li $t2, 0xFF0000
	sw $t1, 16($sp) 
	sw $t2, 20($sp) 
	jal ShadeLine 
	nop
main:
	b main 
	nop
	addi $sp, $sp, 24
	
# Function: DrawLine 
# Purpose: Draws a flat-shaded line from x1, y1 to x2, y2
# a0: x1, a1: y1, a2: x2, a3: y2, 16($sp): color
DrawLine:
	lw $t2, 16($sp)
	li $t1, VS_DRAW_LINE    ; vs_cmd_draw_line = 0x4000000; (delay slot)
	addu $t1, $t1, $t2      ; vs_cmd_draw_line += color;
	sw $t1, VS_GP0($t0)     ; *vs_gp0 = vs_cmd_draw_line;
	andi $a0, $a0, 0xFFFF   ; x1 &= 0xFFFF;
	sll $a1, $a1, 0x10      ; y1 <<= 16;
	addu $a1, $a1, $a0      ; y1 += x1; 
	sw $a1, VS_GP0($t0)     ; *vs_gp0 = y1;
	andi $a2, $a2, 0xFFFF   ; x2 &= 0xFFFF;
	sll $a3, $a3, 0x10      ; y2 <<= 16;
	addu $a3, $a3, $a2      ; y2 += x2; 
	sw $a3, VS_GP0($t0)     ; *vs_gp0 = y2;
	jr $ra 
	nop
	
# Function: ShadeLine 
# Purpose: Draws a gouraud-shaded line from x1, y1 to x2, y2
# a0: x1, a1: y1, a2: x2, a3: y2, 16($sp): color1, 20($sp): color2
ShadeLine:
	lw $t2, 16($sp)
	li $t1, VS_SHADE_LINE   ; vs_cmd_shade_line = 0x5000000; (delay slot)
	addu $t1, $t1, $t2      ; vs_cmd_shade_line += color1;
	sw $t1, VS_GP0($t0)     ; *vs_gp0 = vs_cmd_shade_line;
	andi $a0, $a0, 0xFFFF   ; x1 &= 0xFFFF;
	sll $a1, $a1, 0x10      ; y1 <<= 16;
	addu $a1, $a1, $a0      ; y1 += x1; 
	sw $a1, VS_GP0($t0)     ; *vs_gp0 = y1;
	lw $t2, 20($sp)
	andi $a2, $a2, 0xFFFF   ; x2 &= 0xFFFF; (delay slot)
	sw $t2, VS_GP0($t0)     ; *vs_gp0 = color2;
	sll $a3, $a3, 0x10      ; y2 <<= 16;
	addu $a3, $a3, $a2      ; y2 += x2; 
	sw $a3, VS_GP0($t0)     ; *vs_gp0 = y2;
	jr $ra 
	nop