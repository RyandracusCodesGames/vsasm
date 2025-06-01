#-----------------------------------------------------------
# VideoStation Assembler
# (C) 2025 Ryandracus Chapman
#-----------------------------------------------------------
# Milestone Program 1: Catch The Block
#-----------------------------------------------------------
# PlayStation Graphics Program 5: 
# Rectangle randomly generates on the screen, use the d-pad
# to collide your rectangle with it, and it will change locations.
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

VS_RED equ 255
VS_GREEN equ 255
VS_BLUE equ 255

; CHOOSE RECTANGLE COLOR AND AN X VALUE(FROM 0 TO 255) AND A Y VALUE(FROM 0 TO 239)
; CHOOSE RECTANGLE WIDTH AND HEIGHT

VS_RECTR equ 0
VS_RECTG equ 200 
VS_RECTB equ 0
VS_RECTX equ 10 
VS_RECTY equ 50
VS_RECTW equ 32 
VS_RECTH equ 32

VS_RECTR2 equ 255
VS_RECTG2 equ 0 
VS_RECTB2 equ 0

VS_RECTX2 equ 90
VS_RECTY2 equ 150

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
InitPad: 
    li $t1,0x15
    li $a0, 0x20000001
    li $t2,0xB0
    li $a1, 0x1f800000      ; Set Pad Buffer Address To Automatically Update Each Frame
    jalr $t2                ; Jump To BIOS Routine OutdatedPadInitAndStart()
    nop ; Delay Slot
	addi $sp, $sp, -20
	li $s0, 0x1f800000
	sw $zero, 8($s0) 
	sw $zero, 12($s0) 
	li $t0, VS_RECTX        ; x = VS_RECTX;
	sw $t0, 16($s0)
	li $t1, VS_RECTY        ; y = VS_RECTY;
	sw $t1, 20($s0)
	li $t4, VS_RECTX2 
	li $t5, VS_RECTY2
Input:
PRESSRIGHT:
    lw $t0, 8($s0)
    nop 
    andi $t0, $t0, 0x2000
    beqz $t0, PRESSLEFT
    nop 
    lw $t0, 16($s0)
    nop
    addi $t0, $t0, 2        ; x += 2;
    sw $t0, 16($s0)
    nop
PRESSLEFT:
    lw $t0, 8($s0)
    nop 
    andi $t0, $t0, 0x8000
    beqz $t0, PRESSUP
    nop 
    lw $t0, 16($s0)
    nop
    addi $t0, $t0, -2       ; x -= 2;
    sw $t0, 16($s0)
    nop
PRESSUP:
    lw $t0, 8($s0)
    nop 
    andi $t0, $t0, 0x1000
    beqz $t0, PRESSDOWN
    nop 
    lw $t0, 20($s0)          
    nop
    addi $t0, $t0, -2       ; y -= 2;
    sw $t0, 20($s0)
    nop
PRESSDOWN:
    lw $t0, 8($s0)
    nop 
    andi $t0, $t0, 0x4000
    beqz $t0, FillScreen
    nop 
    lw $t0, 20($s0)
    nop
    addi $t0, $t0, 2        ; y += 2;
    sw $t0, 20($s0)
    nop
FillScreen:
	li $t1, VS_FILL_RECT    ; vs_cmd_fill_screen = VS_FILL_SCREEN;
	li $t2, VS_BLUE 
	andi $t2, $t2, 0xff     ; b &= 0xff;
	sll $t2, $t2, 0x10      ; b <<= 16;
	li $t3, VS_GREEN 
	andi $t3, $t3, 0xff     ; g &= 0xff;
	sll $t3, $t3, 0x8       ; g <<= 8;
	addu $t2, $t2, $t3      ; b += g;
	addiu $t2, $t2, VS_RED  ; b += r;
	addu $t1, $t1, $t2      ; vs_cmd_fill_screen += b;
	sw $t1, VS_GP0($s0)     ; *vs_gp0 = vs_cmd_fill_screen;
	sw $zero, VS_GP0($s0)   ; *vs_gp0 = 0; (x1, y1 = 0) 
	li $t2, VS_HEIGHT
	andi $t2, $t2, 0xFFFF   ; height &= 0xFFFF;
	sll $t2, $t2, 0x10      ; height <<= 16;
	li $t3, VS_WIDTH
	andi $t3, $t3, 0xFFFF   ; width &= 0xFFFF;
	addu $t2, $t2, $t3      ; height += width; 
	sw $t2, VS_GP0($s0)     ; *vs_gp0 = height;
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
	sw $t1, VS_GP0($s0)     ; *vs_gp0 = vs_cmd_fill_rect;
	lw $t1, 20($s0)
	lw $t2, 16($s0)
	andi $t1, $t1, 0xFFFF   ; y &= 0xFFFF;
	andi $t2, $t2, 0xFFFF   ; x &= 0xFFFF;
	sll  $t1, $t1, 0x10     ; y <<= 16;
	addu $t1, $t1, $t2      ; y += x;
	sw $t1, VS_GP0($s0)     ; *vs_gp0 = y;
	li $t1, VS_RECTH        ; h = VS_RECTH
	li $t2, VS_RECTW        ; w = VS_RECTW
	andi $t1, $t1, 0xFFFF   ; h &= 0xFFFF;
	andi $t2, $t2, 0xFFFF   ; w &= 0xFFFF;
	sll  $t1, $t1, 0x10     ; h <<= 16;
	addu $t1, $t1, $t2      ; h += w;
	sw $t1, VS_GP0($s0)     ; *vs_gp0 = h;
FillTargetRect:
	li $t1, VS_FILL_RECT    ; vs_cmd_fill_rect;
	li $t2, VS_RECTB2 
	andi $t2, $t2, 0xff     ; b &= 0xff;
	sll $t2, $t2, 0x10      ; b <<= 16;
	li $t3, VS_RECTG2 
	andi $t3, $t3, 0xff     ; g &= 0xff;
	sll $t3, $t3, 0x8       ; g <<= 8;
	addu $t2, $t2, $t3      ; b += g;
	addiu $t2, $t2, VS_RECTR2 ; b += r;
	addu $t1, $t1, $t2      ; vs_cmd_fill_rect += b;
	sw $t1, VS_GP0($s0)     ; *vs_gp0 = vs_cmd_fill_rect;
	andi $t5, $t5, 0xFFFF   ; y &= 0xFFFF;
	andi $t4, $t4, 0xFFFF   ; x &= 0xFFFF;
	sll  $t1, $t5, 0x10     ; y <<= 16;
	addu $t1, $t1, $t4      ; y += x;
	sw $t1, VS_GP0($s0)     ; *vs_gp0 = y;
	li $t1, VS_RECTH        ; h = VS_RECTH
	li $t2, VS_RECTW        ; w = VS_RECTW
	andi $t1, $t1, 0xFFFF   ; h &= 0xFFFF;
	andi $t2, $t2, 0xFFFF   ; w &= 0xFFFF;
	sll  $t1, $t1, 0x10     ; h <<= 16;
	addu $t1, $t1, $t2      ; h += w;
	sw $t1, VS_GP0($s0)     ; *vs_gp0 = h;
DetectCollision:
	lw $t2, 16($s0)
	nop 
	addi $t3, $t2, VS_RECTW ; size1 = rectx + VS_RECTW;
	ble $t3, $t4, WaitVSync ; if(size1 < rectx2) { collide = false; goto Wait; }
	lw  $t1, 20($s0)
	addi $t3, $t4, VS_RECTW ; size2 = rectx2 + VS_RECTW;
	bge $t2, $t3, WaitVSync
	nop
	addi $t3, $t1, VS_RECTH ; size1 = recty + VS_RECTH;
	ble $t3, $t5, WaitVSync ; if(size1 < recty2) { collide = false; goto Wait; }
	addi $t2, $t5, VS_RECTH ; size2 = recty2 + VS_RECTW;
	bge $t1, $t2, WaitVSync ; if(y2 < recty2) { collide = false; goto Wait; }
	nop
	b SwitchLocation
	nop
WaitVSync:                   ; Wait For Vertical Retrace Period & Store XOR Pad Data
    lw $t0, 0($s0)      ; Load Pad Buffer
    nop               
    beqz $t0, WaitVSync ; if(pad_buffer == 0){ goto Wait; }
    nor $t0, $t0, $zero ; pad_buffer = !(pad_buffer | 0);
    sw $zero, 0($s0)    ; Store Zero To Pad Buffer
    sw $t0, 8($s0)      ; Store Pad Data
main:
	b Input 
	nop                     ; (delay slot)
	addi $sp, $sp, 20
	
SwitchLocation:
	jal VS_Rand
	nop
	andi $v0, $v0, 0xff
	move $t4, $v0
	jal VS_Rand
	nop 
	andi $v0, $v0, 200 
	move $t5, $v0
	b WaitVSync
	nop
	
# Function: VS_Rand
# Purpose: Utilizes the original Doom random function to generate a random unsigned 8-bit integer
	.text 
	.globl
VS_Rand:
	la    $t0, vs_rand_index
	lw    $t1, 0($t0)
	nop 
	addiu $t1, $t1, 0x1       ; vs_rand_index++;
	andi  $t1, $t1, 0xFF      ; vs_rand_index &= 0xff;
	la    $t2, rndtable
	addu  $t2, $t2, $t1       ; rndtable += vs_rand_index;
	lbu   $v0, 0($t2)         ; num = *rndtable;
	sw    $t1, 0($t0)
	jr    $ra 
	nop

	.data
	.align, 4
vs_rand_index:
	.empty, 4
	
	.data
	.align, 4
rndtable: 
	.byte 0x0, 0x8, 0x6d, 0xdc, 0xde, 0xf1, 0x95, 0x6b, 0x4b, 0xf8, 0xfe, 0x8c, 0x10, 0x42, 0x4a, 0x15, 0xd3, 0x2f, 0x50, 0xf2, 0x9a, 0x1b, 0xcd, 0x80, 0xa1, 
	.byte 0x59, 0x4d, 0x24, 0x5f, 0x6e, 0x55, 0x30, 0xd4, 0x8c, 0xd3, 0xf9, 0x16, 0x4f, 0xc8, 0x32, 0x1c, 0xbc, 0x34, 0x8c, 0xca, 0x78, 0x44, 0x91, 0x3e, 0x46,
	.byte 0xb8, 0xbe, 0x5b, 0xc5, 0x98, 0xe0, 0x95, 0x68, 0x19, 0xb2, 0xfc, 0xb6, 0xca, 0xb6, 0x8d, 0xc5, 0x4, 0x51, 0xb5, 0xf2, 0x91, 0x2a, 0x27, 0xe3, 0x9c, 
	.byte 0xc6, 0xe1, 0xc1, 0xdb, 0x5d, 0x7a, 0xaf, 0xf9, 0x0, 0xaf, 0x8f, 0x46, 0xef, 0x2e, 0xf6, 0xa3, 0x35, 0xa3, 0x6d, 0xa8, 0x87, 0x2, 0xeb, 0x19, 0x5c, 
	.byte 0x14, 0x91, 0x8a, 0x4d, 0x45, 0xa6, 0x4e, 0xb0, 0xad, 0xd4, 0xa6, 0x71, 0x5e, 0xa1, 0x29, 0x32, 0xef, 0x31, 0x6f, 0xa4, 0x46, 0x3c, 0x2, 0x25, 0xab, 
	.byte 0x4b, 0x88, 0x9c, 0xb, 0x38, 0x2a, 0x92, 0x8a, 0xe5, 0x49, 0x92, 0x4d, 0x3d, 0x62, 0xc4, 0x87, 0x6a, 0x3f, 0xc5, 0xc3, 0x56, 0x60, 0xcb, 0x71, 0x65, 
	.byte 0xaa, 0xf7, 0xb5, 0x71, 0x50, 0xfa, 0x6c, 0x7, 0xff, 0xed, 0x81, 0xe2, 0x4f, 0x6b, 0x70, 0xa6, 0x67, 0xf1, 0x18, 0xdf, 0xef, 0x78, 0xc6, 0x3a, 0x3c, 
	.byte 0x52, 0x80, 0x3, 0xb8, 0x42, 0x8f, 0xe0, 0x91, 0xe0, 0x51, 0xce, 0xa3, 0x2d, 0x3f, 0x5a, 0xa8, 0x72, 0x3b, 0x21, 0x9f, 0x5f, 0x1c, 0x8b, 0x7b, 0x62, 
	.byte 0x7d, 0xc4, 0xf, 0x46, 0xc2, 0xfd, 0x36, 0xe, 0x6d, 0xe2, 0x47, 0x11, 0xa1, 0x5d, 0xba, 0x57, 0xf4, 0x8a, 0x14, 0x34, 0x7b, 0xfb, 0x1a, 0x24, 0x11, 
	.byte 0x2e, 0x34, 0xe7, 0xe8, 0x4c, 0x1f, 0xdd, 0x54, 0x25, 0xd8, 0xa5, 0xd4, 0x6a, 0xc5, 0xf2, 0x62, 0x2b, 0x27, 0xaf, 0xfe, 0x91, 0xbe, 0x54, 0x76, 0xde, 
	.byte 0xbb, 0x88, 0x78, 0xa3, 0xec, 0xf9