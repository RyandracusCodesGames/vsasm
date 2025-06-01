#-----------------------------------------------------------
# VideoStation Assembler
# (C) 2025 Ryandracus Chapman
#-----------------------------------------------------------
# PlayStation Graphics Program 12: Double Buffering
#-----------------------------------------------------------
# Video Mode: 256x240 NTSC 16-BIT COLOR
#-----------------------------------------------------------

VS_IO equ 0x1F800000
VS_GP0 equ 0x1810 
VS_GP1 equ 0x1814 
VS_FILL_SCREEN equ 0x2000000
VS_CPU_TO_VRAM equ 0xA0000000
VS_VRAM_TO_VRAM equ 0x80000000
VS_GPU_DMA equ 0x10A0
VS_GPU_BCR equ 0x10A4
VS_GPU_CHCR equ 0x10A8
VS_CMD_STAT_READY equ 0x4000000
VS_DMA_ENABLE equ 0x1000000
VS_DRAW_LINE equ 0x40000000
VS_SHADE_TRIANGLE equ 0x30000000

VS_MODE equ 0

VS_DISPLAY_X1 equ 0 
VS_DISPLAY_Y1 equ 240 

VS_WIDTH equ 256 
VS_HEIGHT equ 240

; CHOOSE R,G, B VALUES FROM 0 TO 255

VS_RED equ 75
VS_GREEN equ 0
VS_BLUE equ 130

; vs_gp0 = (unsigned long*)(vs_io_addr + vs_gp0);
; vs_gp1 = (unsigned long*)(vs_io_addr + vs_gp1);

InitVideo:
	li $t0, VS_IO             ; vs_io_addr = (unsigned long*)0x1F800000;
	sw $zero, VS_GP1($t0)     ; *vs_gpu1 = vs_cmd_gpu_reset;
	li $t1, 0x3000000         ; gpu1_cmd = vs_cmd_display_enable;
	sw $t1, VS_GP1($t0)       ; *vs_gp1 = gpu1_cmd;
	li $t1, 0x4000000         ; gpu1_cmd = vs_cmd_dma_off;
	sw $t1, VS_GP1($t0)       ; *vs_gp1 = gpu1_cmd;
	li $t1, 0x8000000         ; vs_cmd_display_mode = 0x8000000 + mode;
	addiu $t1, $t1, VS_MODE   ; vs_cmd_display_enable += mode;
	li   $t1, 0x05000000      ; gpu1_cmd = vs_cmd_display_area;
	sw   $t1, VS_GP1($t0)     ; *gpu1 = gpu1_cmd;
	li $t5, 0xC4E24E          ; vs_hrange = 0xC4E24E;
	li $t6, 0x040010          ; vs_vrange = 0x040010;
	li $t1, 0x06000000        ; vs_cmd_horizontal_range = 0x6000000
	addu $t1, $t1, $t5        ; vs_cmd_horizontal_range += vs_hrange;
	sw $t1, VS_GP1($t0)       ; *vs_gp1 = vs_cmd_horizontal_range;
	li $t1, 0x07000000        ; vs_cmd_vertical_range = 0x7000000;
	addu $t1, $t1, $t6        ; vs_cmd_vertical_range += vs_vrange;
	sw $t1, VS_GP1($t0)       ; *vs_gpu1 = vs_cmd_vertical_range;
	li $t1, 0xE1000000        ; gpu0_cmd = vs_cmd_draw_mode;
	addi $t1, $t1, 0x000508   ; gpu0_cmd += 0x000508;  
	sw   $t1, VS_GP0($t0)     ; *vs_gpu0 = gpu0_cmd;
	li $t1, 0xE2000000        ; gpu0_cmd = vs_cmd_texture_window;
	sw   $t1, VS_GP0($t0)     ; *vs_gpu0 = gpu0_cmd;
	li $t1, 0xE3000000        ; gpu0_cmd = vs_cmd_display_x1y1;
	li $t2, VS_DISPLAY_X1     ; x1 = VS_DISPLAY_X1;
	li $t3, VS_DISPLAY_Y1     ; y1 = VS_DISPLAY_Y1;
	andi $t2, $t2, 0x3FF      ; x1 &= 0x3FF;
	andi $t3, $t3, 0x1FF      ; y1 &= 0x1FF;
	sll $t3, $t3, 0x0A        ; y1 <<= 10;
	addu $t3, $t3, $t2        ; y1 += x1;
	addu $t1, $t1, $t3        ; gpu0_cmd += y1;
	sw   $t1, VS_GP0($t0)     ; *vs_gpu0 = gpu0_cmd;
	li $t1, 0xE4000000        ; gpu0_cmd = vs_cmd_display_x2y2;
	li $t2, VS_DISPLAY_X1     ; x2 = VS_DISPLAY_X1;
	li $t3, VS_DISPLAY_Y1     ; y2 = VS_DISPLAY_Y1;
	addiu $t2, $t2, VS_WIDTH  ; x2 += width;
	addiu $t3, $t3, VS_HEIGHT ; y2 += height;
	andi $t2, $t2, 0x3FF      ; x2 &= 0x3FF;
	andi $t3, $t3, 0x1FF      ; y2 &= 0x1FF;
	sll $t3, $t3, 0x0A        ; y2 <<= 10;
	addu $t3, $t3, $t2        ; y2 += x2;
	addu $t1, $t1, $t3        ; gpu0_cmd += y2;
	sw   $t1, VS_GP0($t0)     ; *vs_gpu0 = gpu0_cmd;
	li   $t1, 0xE5000000      ; gpu0_cmd = gpu0_cmd_draw_offset;
	li $t2, VS_DISPLAY_X1     ; x1 = VS_DISPLAY_X1;
	li $t3, VS_DISPLAY_Y1     ; y1 = VS_DISPLAY_Y1;
	sll  $t3, $t3, 11 
	addu $t3, $t3, $t2 
	addu $t1, $t1, $t3
	sw   $t1, VS_GP0($t0)     ; *gpu0 = cmd;
	li   $t1, 0x03000000      ; cmd = gpu1_cmd_display_enable;
	sw   $t1, VS_GP1($t0)     ; *vs_gpu1 = cmd;
	li $t2, 0x1F8010F0        ; dma_address = 0x1F8010F0;
	li $t1, 0x300             ; dma_priority = 0x300;
	sw $t1, 0($t2)            ; *dma_address = dma_priority;
	li $t1, 0x800             ; gpu_dma_enable = 0x800;
	sw $t1, 0($t2)            ; *dma_address = gpu_dma_enable;
FillScreen:
	li $t1, VS_FILL_SCREEN  ; vs_cmd_fill_screen = VS_FILL_SCREEN;
	li $t2, VS_BLUE
	andi $t2, $t2, 0xff     ; b &= 0xff;
	sll $t2, $t2, 0x10      ; b <<= 16;
	li $t3, VS_GREEN 
	andi $t3, $t3, 0xff     ; g &= 0xff;
	sll $t3, $t3, 0x8       ; g <<= 8;
	addu $t2, $t2, $t3      ; b += g;
	addiu $t2, $t2, VS_RED  ; b += r;
	addu $t1, $t1, $t2      ; vs_cmd_fill_screen += b;
	sw $t1, VS_GP0($t0)     ; *vs_gp0 = vs_cmd_fill_screen;
	li $t1, VS_DISPLAY_X1
	li $t2, VS_DISPLAY_Y1
	andi $t1, $t1, 0xFFFF   ; x1 &= 0xFFFF;
	sll $t2, $t2, 0x10      ; y1 <<= 16;
	addu $t2, $t2, $t1      ; y1 += x1; 
	sw $t2, VS_GP0($t0)     ; *vs_gp0 = y1;
	li $t2, VS_HEIGHT
	andi $t2, $t2, 0xFFFF   ; height &= 0xFFFF;
	sll $t2, $t2, 0x10      ; height <<= 16;
	li $t3, VS_WIDTH
	andi $t3, $t3, 0xFFFF   ; width &= 0xFFFF;
	addu $t2, $t2, $t3      ; height += width; 
	sw $t2, VS_GP0($t0)     ; *vs_gp0 = height;
DrawCommands:
	li $a0, 50
	li $a1, 200 
	li $a2, 200
	li $a3, 100 
	li $t1, 0x00FFFF
	sw $t1, 16($sp) 
	jal DrawLine
	nop
	jal DrawSync
	nop
	li $a0, 80
	li $a1, 140 
	li $a2, 130
	li $a3, 50 
	li $t1, 180
	li $t2, 140
	li $t3, 0x0000FF
	li $t4, 0x00FF00
	li $t5, 0xFF0000
	sw $t1, 16($sp) 
	sw $t2, 20($sp) 
	sw $t3, 24($sp) 
	sw $t4, 28($sp) 
	sw $t5, 32($sp) 
	jal ShadeTriangle
	nop
	jal DrawSync
	nop
BufferSwap:
	li $t1, VS_VRAM_TO_VRAM ; gpu0_cmd = VS_VRAM_TO_VRAM;
	sw $t1, VS_GP0($t0)     ; *vs_gp0 = gpu0_cmd;
	li $t1, VS_DISPLAY_X1   ; x1 = VS_DISPLAY_X1;
	li $t2, VS_DISPLAY_Y1   ; y1 = VS_DISPLAY_Y1;
	andi $t1, $t1, 0xFFFF   ; x1 &= 0xFFFF;
	sll $t2, $t2, 0x10      ; y1 <<= 16;
	addu $t2, $t2, $t1      ; y1 += x1;
	sw $t2, VS_GP0($t0)     ; *vs_gp0 = y1;
	sw $zero, VS_GP0($t0)   ; x2 = 0; y2 = 0; *vs_gp0 = y2;
	li $t1, VS_WIDTH        ; w = VS_WIDTH;
	li $t2, VS_HEIGHT       ; h = VS_HEIGHT;
	andi $t1, $t1, 0xFFFF   ; w &= 0xFFFF;
	sll $t2, $t2, 0x10      ; h <<= 16;
	addu $t2, $t2, $t1      ; h += w;
	sw $t2, VS_GP0($t0)     ; *vs_gp0 = h;
	jal DMASync
	nop
main:
	b main 
	nop
	
	
# Function: DrawSync
# Purpose: Halts program execution until all drawing commands have been executed by the gpu 
DrawSync:
	li $t0, VS_IO             ; vs_io_addr = (unsigned long*)0x1F800000;
DrawSyncLoop:
	lw $t1, VS_GP1($t0)       ; gpu1 = *vs_gpu1;
	li $t2, VS_CMD_STAT_READY ; gpu1_cmd = VS_CMD_STAT_READY; (delay slot)
	and $t1, $t1, $t2         ; gpu1 &= gpu1_cmd;
	beqz $t1, DrawSyncLoop    ; if(gpu1 == 0) { goto DrawSyncLoop; }
	nop 
	jr $ra
	nop
	
# Function: DMASync
# Purpose: Halts program execution until all gpu dma transfers have completed
DMASync:
	li $t0, VS_IO             ; vs_io_addr = (unsigned long*)0x1F800000;
DMASyncLoop:
	lw $t1, VS_GPU_CHCR($t0)  ; gpu0 = *vs_gpu0;
	li $t2, VS_DMA_ENABLE    ; gpu0_cmd = VS_CMD_STAT_READY; (delay slot)
	and $t1, $t1, $t2         ; gpu0 &= gpu0_cmd;
	bnez $t1, DMASyncLoop    ; if(gpu0 == 0) { goto DrawSyncLoop; }
	nop 
	jr $ra
	nop
	
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
	
# Function: ShadeTriangle
# Purpose: Draws a gouraud-shaded triange to the display area
# a0: x1, a1: y1, a2: x2, a3: y2, 16($sp): x3, 20($sp): y3, 24($sp): color1, 28($sp): color2, 32($sp): color3
ShadeTriangle:
	lw $t2, 24($sp)
	li $t1, VS_SHADE_TRIANGLE ; vs_cmd_shade_triangle = 0x3000000; (delay slot)
	addu $t1, $t1, $t2        ; vs_cmd_shade_triangle += color1;
	sw $t1, VS_GP0($t0)       ; *vs_gp0 = vs_cmd_shade_triangle;
	andi $a0, $a0, 0xFFFF     ; x1 &= 0xFFFF;
	sll $a1, $a1, 0x10        ; y1 <<= 16;
	addu $a1, $a1, $a0        ; y1 += x1; 
	sw $a1, VS_GP0($t0)       ; *vs_gp0 = y1;
	lw $t2, 28($sp)
	andi $a2, $a2, 0xFFFF     ; x2 &= 0xFFFF; (delay slot)
	sw $t2, VS_GP0($t0)       ; *vs_gp0 = color2;
	sll $a3, $a3, 0x10        ; y2 <<= 16;
	addu $a3, $a3, $a2        ; y2 += x2; 
	lw $t1, 32($sp)
	lw $a0, 16($sp)
	sw $a3, VS_GP0($t0)       ; *vs_gp0 = y2;
	sw $t1, VS_GP0($t0)       ; *vs_gp0 = color3;
	lw $a1, 20($sp)
	andi $a0, $a0, 0xFFFF     ; x3 &= 0xFFFF;
	sll $a1, $a1, 0x10        ; y3 <<= 16;
	addu $a1, $a1, $a0        ; y3 += x3; 
	sw $a1, VS_GP0($t0)       ; *vs_gp0 = y3;
	jr $ra 
	nop