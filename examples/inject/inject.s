.syntax asmpsx
.arch mips1

VS_IO equ $1F800000
VS_GP0 equ $1810
VS_GP1 equ $1814
VS_MODE equ 0 
VS_WIDTH equ 256 
VS_HEIGHT equ 240

InitVideo:
	li t0, VS_IO           ; vs_io_addr = $1F800000; (base i/o address of memory map) 
	sw zero, VS_GP1(t0)   ; *vs_gp1 = vs_cmd_gpu_reset;
	li t1, $4000000       ; vs_cmd_dma_req = $4000000 (no dma)
	sw t1, VS_GP1(t0)      ; *vs_gp0 = vs_cmd_dma_req;
	li t1, $8000000       ; vs_cmd_display_mode = $8000000 + mode;
	addiu t1, t1, VS_MODE ; vs_cmd_display_enable += mode;
	sw t1, VS_GP1(t0)     ; *vs_gp1 = vs_cmd_display_enable;
	li t5, $C4E24E        ; vs_hrange = $C4E24E;
	li t6, $040010        ; vs_vrange = $040010;
	li t1, $06000000      ; vs_cmd_horizontal_range = $6000000
	addu t1, t1, t5      ; vs_cmd_horizontal_range += vs_hrange;
	sw t1, VS_GP1(t0)     ; *vs_gp1 = vs_cmd_horizontal_range;
	li t1, $07000000      ; vs_cmd_vertical_range = $7000000;
	addu t1, t1, t6      ; vs_cmd_vertical_range += vs_vrange;
	sw t1, VS_GP1(t0)     ; *vs_gp1 = vs_cmd_vertical_range;
	li t1, $05000000      ; vs_cmd_display_x1y1 = $5000000; (start x, y of display = 0)
	sw t1, VS_GP1(t0)     ; *vs_gp1 = vs_cmd_display_x1y1;
	li   t1, $E1000000    ; cmd = gpu0_cmd_tex_page;
	addi t1, t1, $000508 ; cmd += $000508;  
	sw   t1, VS_GP0(t0)   ; *vs_gpu0 = cmd;
	li   t1, $E3000000    ; cmd = gpu0_cmd_draw_area_top_left;
	sw   t1, VS_GP0(t0)   ; *vs_gpu0 = cmd;
	li   t1, $E4000000    ; cmd = gpu0_cmd_draw_area_bot_right;
	li   t2, VS_HEIGHT
	li   t3, VS_WIDTH
	sll  t2, t2, $0A     ; height <<= 10;
	addu t2, t2, t3      ; height += width;
	addu t1, t1, t2      ; cmd += height;
	sw   t1, VS_GP0(t0)   ; *vs_gpu0 = cmd;
	li   t1, $E5000000    ; cmd = gpu0_cmd_draw_offset;
	sw   t1, VS_GP0(t0)   ; *vs_gpu0 = cmd;
	li   t1, $03000000    ; cmd = gpu1_cmd_display_enable;
	sw   t1, VS_GP1(t0)   ; *vs_gpu1 = cmd;