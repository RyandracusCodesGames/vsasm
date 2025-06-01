#-----------------------------------------------------------
# VideoStation Assembler
# (C) 2025 Ryandracus Chapman
#-----------------------------------------------------------
# PlayStation Graphics Program 1: Draw A Solid Color Rectangle
#-----------------------------------------------------------
# Video Mode: 256x240 NTSC 16-BIT COLOR
#-----------------------------------------------------------

VS_IO equ 0x1F800000
VS_GP0 equ 0x1810
VS_GP1 equ 0x1814
VS_FILL_SCREEN equ 0x2000000
VS_FILL_RECT equ 0x60000000
VS_MODE equ 0 
VS_WIDTH equ 256 
VS_HEIGHT equ 240

; CHOOSE R,G, B VALUES FROM 0 TO 255

VS_R equ 75
VS_G equ 0
VS_B equ 130

; CHOOSE RECTANGLE COLOR AND AN X VALUE(FROM 0 TO 255) AND A Y VALUE(FROM 0 TO 239)
; CHOOSE RECTANGLE WIDTH AND HEIGHT

VS_RECTR equ 0
VS_RECTG equ 255 
VS_RECTB equ 0
VS_RECTX equ 50 
VS_RECTY equ 50
VS_RECTW equ 64 
VS_RECTH equ 32

; vs_gp0 = vs_io_addr + vs_gp0;
; vs_gp1 = vs_io_addr + vs_gp1;

InitVideo:
	li $t0, VS_IO           ; vs_io_addr = 0x1F800000; (base i/o address of memory map) 
	sw $zero, VS_GP1($t0)   ; *vs_gp1 = vs_cmd_gpu_reset;
	li $t1, 0x3000000       ; vs_cmd_display_enable = 0x3000000;
	sw $t1, VS_GP1($t0)     ; *vs_gp1 = vs_cmd_display_enable;
	li $t1, 0x8000000       ; vs_cmd_display_mode = 0x8000000 + mode;
	addiu $t1, $t1, VS_MODE ; vs_cmd_display_enable += mode;
	sw $t1, VS_GP1($t0)     ; *vs_gp1 = vs_cmd_display_enable;
	li $t5, 0xC4E24E        ; vs_hrange = 0xC4E24E;
	li $t6, 0x040010        ; vs_vrange = 0x040010;
	li $t1, 0x6000000       ; vs_cmd_horizontal_range = 0x6000000
	addu $t1, $t1, $t5      ; vs_cmd_horizontal_range += vs_hrange;
	sw $t1, VS_GP1($t0)     ; *vs_gp1 = vs_cmd_horizontal_range;
	li $t1, 0x7000000       ; vs_cmd_vertical_range = 0x7000000;
	addu $t1, $t1, $t6      ; vs_cmd_vertical_range += vs_vrange;
	sw $t1, VS_GP1($t0)     ; *vs_gp1 = vs_cmd_vertical_range;
	li $t1, 0x4000001       ; vs_cmd_dma_req = 0x4000001 (fifo dma)
	sw $t1, VS_GP1($t0)      ; *vs_gp0 = vs_cmd_dma_req;
	li $t1, 0x5000000       ; vs_cmd_display_x1y1 = 0x5000000; (start x, y of display = 0)
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
FillRect:
	li $t1, VS_FILL_RECT    ; vs_cmd_fill_rect;
	li $t2, VS_RECTB 
	andi $t2, $t2, 0xff     ; b &= 0xff;
	sll $t2, $t2, 0x10      ; b <<= 16;
	li $t3, VS_RECTG 
	andi $t3, $t3, 0xff     ; g &= 0xff;
	sll $t3, $t3, 0x8       ; g <<= 8;
	addu $t2, $t2, $t3      ; b += g;
	addiu $t2, $t2, VS_RECTR ; b += r;
	addu $t1, $t1, $t2      ; vs_cmd_fill_rect += b;
	sw $t1, VS_GP0($t0)     ; *vs_gp0 = vs_cmd_fill_rect;
	li $t1, VS_RECTY        ; y = VS_RECTY;
	li $t2, VS_RECTX        ; x = VS_RECTX;
	andi $t1, $t1, 0xFFFF   ; y &= 0xFFFF;
	andi $t2, $t2, 0xFFFF   ; x &= 0xFFFF;
	sll  $t1, $t1, 0x10     ; y <<= 16;
	addu $t1, $t1, $t2      ; y += x;
	sw $t1, VS_GP0($t0)     ; *vs_gp0 = y;
	li $t1, VS_RECTH        ; h = VS_RECTH
	li $t2, VS_RECTW        ; w = VS_RECTW
	andi $t1, $t1, 0xFFFF   ; h &= 0xFFFF;
	andi $t2, $t2, 0xFFFF   ; w &= 0xFFFF;
	sll  $t1, $t1, 0x10     ; h <<= 16;
	addu $t1, $t1, $t2      ; h += w;
	sw $t1, VS_GP0($t0)     ; *vs_gp0 = h;
main:
	b main 
	nop                     ; (delay slot)