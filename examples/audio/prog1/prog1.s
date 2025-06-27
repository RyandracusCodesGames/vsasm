#-----------------------------------------------------------
# VideoStation Assembler
# (C) 2025 Ryandracus Chapman
#-----------------------------------------------------------
# PlayStation Audio Program 1: Initialize the Sound Processor
# and Play an Audio Sample
#-----------------------------------------------------------
# Video Mode: 256x240 NTSC 16-BIT COLOR
#-----------------------------------------------------------

.arch psx
.syntax asmpsx
.text

; PSX'S BASE I/O ADDRESS

VS_IO equ $1F800000

; GPU'S MEMORY MAPPED I/O ADDRESSES = vs_io_addr + gpu_io_addr
; ex: vs_gpu0_addr = VS_IO + VS_GP0

VS_GP0 equ $1810
VS_GP1 equ $1814
VS_FILL_SCREEN equ $2000000
VS_FILL_RECT equ $60000000
VS_CPU_TO_VRAM equ $A0000000
VS_VRAM_TO_VRAM equ $80000000
VS_GPU_DMA equ $10A0
VS_GPU_BCR equ $10A4
VS_GPU_CHCR equ $10A8
VS_CMD_STAT_READY equ $4000000
VS_DMA_ENABLE equ $1000000
VS_TEXTURE_FOUR_POINT_POLY equ $2D000000
VS_DRAW_LINE equ $40000000
VS_MODE equ 0
VS_DISPLAY_X1 equ 0 
VS_DISPLAY_Y1 equ 240 
VS_WIDTH equ 256 
VS_HEIGHT equ 240
VS_RED equ 56 
VS_GREEN equ 56
VS_BLUE equ 89
VS_FONTW equ 8 
VS_FONTH equ 11

; SPU'S MEMORY MAPPED I/O ADDRESSES = vs_io_addr + spu_io_addr
; ex: vs_spu_status_addr = VS_IO + VS_SPU_STATUS_ADDR

VS_SPU_STATUS_ADDR equ $1DAE
VS_SPU_CTRL_ADDR equ $1DAA 
VS_SPU_STATUS_TIMEOUT equ $100000
VS_SPU_BUS_CONFIG_ADDR equ $1014
VS_SPU_MASTER_VOLUME_LEFT_ADDR	equ $1D80
VS_SPU_MASTER_VOLUME_RIGHT_ADDR	equ $1D82
VS_SPU_REVERB_VOLUME_LEFT_ADDR	equ $1D84
VS_SPU_REVERB_VOLUME_RIGHT_ADDR	equ $1D86
VS_SPU_NOISE_MODE1 equ $1D94
VS_SPU_NOISE_MODE2 equ $1D96
VS_SPU_REVERB_MODE1 equ $1D98
VS_SPU_REVERB_MODE2 equ $1D9A
VS_SPU_CD_VOLUME_LEFT_ADDR equ $1DB0
VS_SPU_CD_VOLUME_RIGHT_ADDR equ $1DB2
VS_SPU_EXT_VOLUME_LEFT_ADDR equ $1DB4
VS_SPU_EXT_VOLUME_RIGHT_ADDR equ $1DB6
VS_SPU_KEY_ON1_ADDR	equ $1D88
VS_SPU_KEY_ON2_ADDR	equ $1D8A
VS_SPU_KEY_OFF1_ADDR equ $1D8C
VS_SPU_KEY_OFF2_ADDR equ $1D8E
VS_SPU_FM_MODE1_ADDR equ $1D90
VS_SPU_FM_MODE2_ADDR equ $1D92
VS_DPCR_ADDR equ $10F0
VS_SPU_DMA_M_ADDR equ $10C0
VS_SPU_DMA_BCR_ADDR equ $10C4
VS_SPU_DMA_CHCR_ADDR equ $10C8
VS_SPU_CHANNEL_VOLUME_LEFT_ADDR equ $1F801C00
VS_SPU_CHANNEL_VOLUME_RIGHT_ADDR equ $1F801C02
VS_SPU_CHANNEL_SAMPLE_RATE_ADDR equ $1F801C04
VS_SPU_CHANNEL_ADPCM_ADDR equ $1F801C06
VS_SPU_CHANNEL_ADPCM_REPEAT_ADDR equ $1F801C0E
VS_SPU_DATA_TRANSFER_ADDR equ $1DA6

; SOUND.ADPCM VARIABLES

VS_SAMPLE_RATE equ 44100 
VS_SAMPLE_SIZE equ 109842
VS_MASTER_VOLUME equ $3FFF
VS_CHANNEL_VOLUME equ $3FFF

; UI VARIABLES

VS_BURGER_X equ 10 
VS_BURGER_Y equ 60 

VS_BIMGW equ 32 
VS_BIMGH equ 32

InitAudio:
	li t0, VS_IO                           ; vs_io_addr = (unsigned long*)$1F800000;
	li t1, $200931e1                      
	sw t1, VS_SPU_BUS_CONFIG_ADDR(t0)      ; *(unsigned long*)VS_SPU_BUS_CONFIG_ADDR = $200931e1;
	sh zero, VS_SPU_CTRL_ADDR(t0)          ; *(unsigned short*)VS_SPU_CTRL_ADDR = 0;
	li t1, VS_SPU_STATUS_TIMEOUT           ; spu_status_timeout = VS_SPU_STATUS_TIMEOUT;
	li t2, $001f                           ; mask = $001f;
WaitInitAudio:
	beqz t1, ContinueInitAudio             ; if(spu_status_timeout == 0) { goto ContinueInitAudio; }
	lhu t3, VS_SPU_STATUS_ADDR(t0)         ; vs_spu_stat = *(unsigned short*)VS_SPU_STATUS_ADDR;
	subi t1, t1, $1                        ; spu_status_timeout--;
	and t3, t3, t2                         ; vs_spu_stat &= mask;
	beqz t3, WaitInitAudio
	nop
ContinueInitAudio:
	sh  zero, VS_SPU_MASTER_VOLUME_LEFT_ADDR(t0)      ; *(unsigned short*)VS_SPU_MASTER_VOLUME_LEFT_ADDR = 0;
	sh  zero, VS_SPU_MASTER_VOLUME_RIGHT_ADDR(t0)     ; *(unsigned short*)VS_SPU_MASTER_VOLUME_RIGHT_ADDR = 0;
	sh  zero, VS_SPU_REVERB_VOLUME_LEFT_ADDR(t0)      ; *(unsigned short*)VS_SPU_REVERB_VOLUME_LEFT_ADDR = 0;
	sh  zero, VS_SPU_REVERB_VOLUME_RIGHT_ADDR(t0)     ; *(unsigned short*)VS_SPU_REVERB_VOLUME_RIGHT_ADDR = 0;
	sw  zero, VS_SPU_NOISE_MODE1(t0)                  ; *(unsigned long*)VS_SPU_NOISE_MODE1 = 0;
	sw  zero, VS_SPU_REVERB_MODE1(t0)                 ; *(unsigned long*)VS_SPU_REVERB_MODE1 = 0;
	sh  zero, VS_SPU_CD_VOLUME_LEFT_ADDR(t0)          ; *(unsigned short*)VS_SPU_CD_VOLUME_LEFT_ADDR  = 0;
	sh  zero, VS_SPU_CD_VOLUME_RIGHT_ADDR(t0)         ; *(unsigned short*)VS_SPU_CD_VOLUME_RIGHT_ADDR  = 0;
	sh  zero, VS_SPU_EXT_VOLUME_LEFT_ADDR(t0)         ; *(unsigned short*)VS_SPU_EXT_VOLUME_LEFT_ADDR  = 0;
	sh  zero, VS_SPU_EXT_VOLUME_RIGHT_ADDR(t0)        ; *(unsigned short*)VS_SPU_EXT_VOLUME_RIGHT_ADDR  = 0;
	li  t1, $FFFF
	sh  t1, VS_SPU_KEY_OFF1_ADDR(t0)                  ; *(unsigned short*)VS_SPU_KEY_OFF1_ADDR  = $FFFF;
	li  t1, $00FF
	sh  t1, VS_SPU_KEY_OFF2_ADDR(t0)                  ; *(unsigned short*)VS_SPU_KEY_OFF2_ADDR  = $00FF;
	sh  zero, VS_SPU_FM_MODE1_ADDR(t0)                ; *(unsigned short*)VS_SPU_FM_MODE1_ADDR  = 0;
	sh  zero, VS_SPU_FM_MODE2_ADDR(t0)                ; *(unsigned short*)VS_SPU_FM_MODE2_ADDR  = 0;
	lw  t1,   VS_DPCR_ADDR(t0)                        ; dpcr = *(unsigned long*)VS_DPCR_ADDR;
	nop 
	sra t2, t1, $10                                   ; channel = dpcr >> 16;
	li t2, $fff0ffff
	and t1, t1, t2                                    ; dpcr &= $fff0ffff;
	li t2, $b0000
	or  t1, t1, t2
	sw t1, VS_DPCR_ADDR(t0)
	li t1, $00000201
	sw t1, VS_SPU_DMA_CHCR_ADDR(t0)                   ; *(unsigned long*)VS_SPU_DMA_CHCR_ADDR = $00000201;
	li t1, $0004                                         
	sh t1, $1DAC(t0)                                  ; dma_ctrl_addr = $0004;
	li t1, $0707
	sh t1, $1DA8(t0)
	li t1, $C000
	sh t1, VS_SPU_CTRL_ADDR(t0)                       ; *(unsigned short*)VS_SPU_CTRL_ADDR = $C000;
	li t1, VS_SPU_STATUS_TIMEOUT     		          ; spu_status_timeout = VS_SPU_STATUS_TIMEOUT;
	li t3, $1
FinishWait:
	beqz t1, InitVoiceChannels                   ; if(spu_status_timeout == 0) { goto InitVoiceChannels; }
	lhu  t2, VS_SPU_STATUS_ADDR(t0) 		     ; spu_stat = *VS_SPU_STATUS_ADDR;
	subi t1, t1, $1                              ; spu_status_timeout--; (delay slot)
	andi t2, t2, $003f                           ; spu_stat &= $001f;
	bne  t3, t3, FinishWait                      ; if(spu_stat != 1) { goto finish_wait; }
	nop
	li t1, $0                                    ; i = 0;
InitVoiceChannels:
	li    t2, VS_SPU_CHANNEL_VOLUME_LEFT_ADDR   ; VS_SPU_CHANNEL_VOLUME_LEFT_ADDR = (unsigned short*)$1F801C00;
	sll   t3, t1, $4                            ; channel = i * 16;
	addu  t2, t2, t3                            ; VS_SPU_CHANNEL_VOLUME_LEFT_ADDR += channel;
	sh    zero, 0(t2)                           ; *(unsigned short*)VS_SPU_CHANNEL_VOLUME_LEFT_ADDR = 0;
	addiu t2, t2, $2                            ; VS_SPU_CHANNEL_VOLUME_LEFT_ADDR += 2;
	sh    zero, 0(t2)                           ; *(unsigned short*)VS_SPU_CHANNEL_VOLUME_LEFT_ADDR = 0;
	li    t2, VS_SPU_CHANNEL_SAMPLE_RATE_ADDR   ; *(unsigned short*)VS_SPU_CHANNEL_SAMPLE_RATE_ADDR = (u16*)$1F801C04;
	addu  t2, t2, t3                            ; VS_SPU_CHANNEL_SAMPLE_RATE_ADDR += channel;
	li    t4, $1000
	sh    t4, 0(t2)                             ; *(unsigned short*)VS_SPU_CHANNEL_SAMPLE_RATE_ADDR = $1000;
	li    t2, VS_SPU_CHANNEL_ADPCM_ADDR         ; VS_SPU_CHANNEL_ADPCM_ADDR = (u16*)$1F801C06;
	addu  t2, t2, t3                            ; VS_SPU_CHANNEL_ADPCM_ADDR += channel;
	li    t4, $200
	sh    t4, 0(t2)                             ; *(unsigned short*)VS_SPU_CHANNEL_ADPCM_ADDR = $200;
	li    t2, VS_SPU_CHANNEL_ADPCM_REPEAT_ADDR  ; VS_SPU_CHANNEL_ADPCM_REPEAT_ADDR = (u16*)$1F801C0E;
	addu  t2, t2, t3                            ; VS_SPU_CHANNEL_ADPCM_REPEAT_ADDR += channel;
	sh    t4, 0(t2)                             ; *(unsigned short*)VS_SPU_CHANNEL_ADPCM_REPEAT_ADDR = $200;
	addiu t1, t1, $1                            ; i++;
	li t2, 24
	bne   t1, t2, InitVoiceChannels             ; if(i != 24) { goto InitVoiceChannels; }
	nop 
	addi sp, sp, -100
TransferADPCMtoSPU:
	li t1, $0A                  ; counter = 10;
	la t2, VolumeCounter
	sw t1, 0(t2)                ; *VolumeCounter = counter;
	la t2, ChannelVolumeCounter
	sw t1, 0(t2)                ; *ChannelVolumeCounter = counter;
	li a0, VS_MASTER_VOLUME     ; master_volume = $3FFF;
	jal VS_SetMasterVolume      ; VS_SetMasterVolume(master_volume);
	nop 
	li a0, $0                   ; reverb_volume = 0;
	jal VS_SetReverbVolume      ; VS_SetReverbVolume(reverb_volume);
	nop
	li a0, $0                   ; channel = 0;
	li a1, VS_CHANNEL_VOLUME    ; volume = $3FFF;
	jal VS_SetChannelVolume     ; VS_SetChannelVolume(channel,volume);
	nop
	li a0, $1010                ; addr = $1010;
	jal VS_SetADPCMAddr         ; VS_SetADPCMAddr(addr);
	nop
	li a0, $0                   ; channel = 0;
	li a1, VS_SAMPLE_RATE       ; sample_rate = VS_SAMPLE_RATE;
	jal VS_SetChannelSampleRate ; VS_SetChannelSampleRate(channel, sample_rate);
	nop 
	li a0, $0 
	li a1, $f
	jal VS_SetChannelSustainLevel
	nop
	la a0, SampleADPCM         ; adpcm_addr = SampleADPCM;
	li a1, VS_SAMPLE_SIZE      ; size = VS_SAMPLE_SIZE;
	jal VS_WriteADPCM
	nop
InitVideo:
	li t0, VS_IO            ; vs_io_addr = (unsigned long*)$1F800000;
	sw zero, VS_GP1(t0)     ; *vs_gpu1 = vs_cmd_gpu_reset;
	li t1, $3000000         ; gpu1_cmd = vs_cmd_display_enable;
	sw t1, VS_GP1(t0)       ; *vs_gp1 = gpu1_cmd;
	li t1, $4000000         ; gpu1_cmd = vs_cmd_dma_off;
	sw t1, VS_GP1(t0)       ; *vs_gp1 = gpu1_cmd;
	li t1, $8000000         ; vs_cmd_display_mode = $8000000 + mode;
	addiu t1, t1, VS_MODE   ; vs_cmd_display_enable += mode;
	li   t1, $05000000      ; gpu1_cmd = vs_cmd_display_area;
	sw   t1, VS_GP1(t0)     ; *gpu1 = gpu1_cmd;
	li t5, $C4E24E          ; vs_hrange = $C4E24E;
	li t6, $040010          ; vs_vrange = $040010;
	li t1, $06000000        ; vs_cmd_horizontal_range = $6000000
	addu t1, t1, t5         ; vs_cmd_horizontal_range += vs_hrange;
	sw t1, VS_GP1(t0)       ; *vs_gp1 = vs_cmd_horizontal_range;
	li t1, $07000000        ; vs_cmd_vertical_range = $7000000;
	addu t1, t1, t6         ; vs_cmd_vertical_range += vs_vrange;
	sw t1, VS_GP1(t0)       ; *vs_gpu1 = vs_cmd_vertical_range;
	li t1, $E1000000        ; gpu0_cmd = vs_cmd_draw_mode;
	addi t1, t1, $000508    ; gpu0_cmd += $000508;  
	sw   t1, VS_GP0(t0)     ; *vs_gpu0 = gpu0_cmd;
	li t1, $E2000000        ; gpu0_cmd = vs_cmd_texture_window;
	sw   t1, VS_GP0(t0)     ; *vs_gpu0 = gpu0_cmd;
	li t1, $E3000000        ; gpu0_cmd = vs_cmd_display_x1y1;
	li t2, VS_DISPLAY_X1    ; x1 = VS_DISPLAY_X1;
	li t3, VS_DISPLAY_Y1    ; y1 = VS_DISPLAY_Y1;
	andi t2, t2, $3FF       ; x1 &= $3FF;
	andi t3, t3, $1FF       ; y1 &= $1FF;
	sll t3, t3, $0A         ; y1 <<= 10;
	addu t3, t3, t2         ; y1 += x1;
	addu t1, t1, t3         ; gpu0_cmd += y1;
	sw   t1, VS_GP0(t0)     ; *vs_gpu0 = gpu0_cmd;
	li t1, $E4000000        ; gpu0_cmd = vs_cmd_display_x2y2;
	li t2, VS_DISPLAY_X1    ; x2 = VS_DISPLAY_X1;
	li t3, VS_DISPLAY_Y1    ; y2 = VS_DISPLAY_Y1;
	addiu t2, t2, VS_WIDTH  ; x2 += width;
	addiu t3, t3, VS_HEIGHT ; y2 += height;
	andi t2, t2, $3FF       ; x2 &= $3FF;
	andi t3, t3, $1FF       ; y2 &= $1FF;
	sll t3, t3, $0A         ; y2 <<= 10;
	addu t3, t3, t2         ; y2 += x2;
	addu t1, t1, t3         ; gpu0_cmd += y2;
	sw   t1, VS_GP0(t0)     ; *vs_gpu0 = gpu0_cmd;
	li   t1, $E5000000      ; gpu0_cmd = gpu0_cmd_draw_offset;
	li t2, VS_DISPLAY_X1    ; x1 = VS_DISPLAY_X1;
	li t3, VS_DISPLAY_Y1    ; y1 = VS_DISPLAY_Y1;
	sll  t3, t3, 11         ; y1 <<= 11;
	addu t3, t3, t2         ; y1 += x;
	addu t1, t1, t3         ; gpu0_cmd += y;
	sw   t1, VS_GP0(t0)     ; *gpu0 = gpu0_cmd;
	li   t1, $03000000      ; cmd = gpu1_cmd_display_enable;
	sw   t1, VS_GP1(t0)     ; *vs_gpu1 = cmd;
	li t2, $1F8010F0        ; dma_address = $1F8010F0;
	li t1, $300             ; dma_priority = $300;
	sw t1, 0(t2)            ; *dma_address = dma_priority;
	li t1, $800             ; gpu_dma_enable = $800;
	sw t1, 0(t2)            ; *dma_address = gpu_dma_enable;
UploadImageDataToVram:
	li a0, 256 
	li a1, 0 
	li a2, 32 
	li a3, 32 
	la t1, Burger
	sw t1, 16(sp)
	jal VS_TransferImageDataToVram
	nop
InitPad: 
    li t1,$15
    li a0, $20000001
    li t2,$B0
    li a1, VS_IO           ; Set Pad Buffer Address To Automatically Update Each Frame
    jalr t2                ; Jump To BIOS Routine OutdatedPadInitAndStart()
    nop ; Delay Slot
	li t0, VS_IO
	li a0, $0 
	jal VS_TurnOnChannel
	nop
	sw zero, 8(t0) 
	sw zero, 12(t0) 
	sw zero, 16(t0)
Input:
PRESSRIGHT:
    lw t1, 8(t0)
    nop 
    andi t1, t1, $2000
    beqz t1, PRESSLEFT
    nop 
	lw t1, 16(t0)
	nop 
	beqz t1, IncreaseMasterVolume
	nop 
	la t2, ChannelVolumeCounter
	lw t3, 0(t2)
	nop 
	addi t3, t3, $1 
	move a0, t3 
	li a1, $0A
	jal VS_ClampMax
	nop
	sw v0, 0(t2)
	li t1, 1638 
	mult v0, t1 
	mflo a1 
	li a0, $0
	jal VS_SetChannelVolume
	nop
	b PRESSLEFT
	nop
IncreaseMasterVolume:
	la t2, VolumeCounter
	lw t3, 0(t2)
	nop 
	addi t3, t3, $1 
	move a0, t3 
	li a1, $0A
	jal VS_ClampMax
	nop
	sw v0, 0(t2)
	li t1, 1638 
	mult v0, t1 
	mflo a0 
	jal VS_SetMasterVolume
	nop
PRESSLEFT:
    lw t1, 8(t0)
    nop 
    andi t1, t1, $8000
    beqz t1, PRESSUP
    nop 
	lw t1, 16(t0)
	nop 
	beqz t1, DecreaseMasterVolume
	nop 
	la t2, ChannelVolumeCounter
	lw t3, 0(t2)
	nop 
	subi t3, t3, $1 
	move a0, t3 
	li a1, $0
	jal VS_ClampMin
	nop
	sw v0, 0(t2)
	li t1, 1638 
	mult v0, t1 
	mflo a1 
	li a0, $0
	jal VS_SetChannelVolume
	nop
	b PRESSUP
	nop
DecreaseMasterVolume:
	la t2, VolumeCounter
	lw t3, 0(t2)
	nop 
	subi t3, t3, $1 
	move a0, t3 
	li a1, $0
	jal VS_ClampMin
	nop
	sw v0, 0(t2)
	li t1, 1638 
	mult v0, t1 
	mflo a0 
	jal VS_SetMasterVolume
	nop
PRESSUP:
    lw t1, 8(t0)
    nop 
    andi t1, t1, $1000
    beqz t1, PRESSDOWN
    nop
	sw zero, 16(t0)
PRESSDOWN:
    lw t1, 8(t0)
    nop 
    andi t1, t1, $4000
    beqz t1, PRESSX
	nop
	li t1, $1
	sw t1, 16(t0)
PRESSX:
    lw t1, 8(t0)
    nop 
    andi t1, t1, $0040 
    beqz t1, FillScreen
    nop 
    li a0, $0 
	jal VS_TurnOnChannel
	nop
FillScreen:
	li t1, VS_FILL_SCREEN  ; vs_cmd_fill_screen = VS_FILL_SCREEN;
	li t2, VS_BLUE
	andi t2, t2, $ff     ; b &= $ff;
	sll t2, t2, $10      ; b <<= 16;
	li t3, VS_GREEN 
	andi t3, t3, $ff     ; g &= $ff;
	sll t3, t3, $8       ; g <<= 8;
	addu t2, t2, t3      ; b += g;
	addiu t2, t2, VS_RED ; b += r;
	addu t1, t1, t2      ; vs_cmd_fill_screen += b;
	sw t1, VS_GP0(t0)    ; *vs_gp0 = vs_cmd_fill_screen;
	li t1, VS_DISPLAY_X1
	li t2, VS_DISPLAY_Y1
	andi t1, t1, $FFFF   ; x1 &= $FFFF;
	sll t2, t2, $10      ; y1 <<= 16;
	addu t2, t2, t1      ; y1 += x1; 
	sw t2, VS_GP0(t0)    ; *vs_gp0 = y1;
	li t2, VS_HEIGHT
	andi t2, t2, $FFFF   ; height &= $FFFF;
	sll t2, t2, $10      ; height <<= 16;
	li t3, VS_WIDTH
	andi t3, t3, $FFFF   ; width &= $FFFF;
	addu t2, t2, t3      ; height += width; 
	sw t2, VS_GP0(t0)    ; *vs_gp0 = height;
TextureXButton:
	li a0, 15 
	li a1, 390 
	li a2, 18 
	li a3, 23 
	la t1, XButton
	sw t1, 16(sp)
	jal VS_TransferImageDataToVram
	nop
TextureBurger:
	li a0, 2 
	li a1, $1 
	li a2, 256 
	li a3, 0 
	jal VS_GetTexturePage
	nop 
	lw t1, 16(t0)
	nop 
	bnez t1, TextureBurger2
	nop
	li a0, VS_BURGER_X 
	li a1, VS_BURGER_Y
	move a2, v0 
	jal VS_TextureBurger
	nop
	b DrawSoundBars
	nop
TextureBurger2:
	li a0, VS_BURGER_X 
	li a1, VS_BURGER_Y
	addi a1, a1, 40
	move a2, v0 
	jal VS_TextureBurger
	nop
DrawSoundBars:
	li a0, 50 
	li a1, 68 
	li a2, 100
	li a3, 10 
	li t1, $000000
	sw t1, 16(sp)
	jal FillRect
	nop
	li a0, 50 
	li a1, 110 
	li a2, 100
	li a3, 10 
	li t1, $000000
	sw t1, 16(sp)
	jal FillRect
	nop
	la t1, VolumeCounter
	lw t2, 0(t1)
	li t3, $0A 
	mult t2, t3 
	mflo a2 
	li a0, 50 
	li a1, 68 
	li a3, 10 
	li t1, $00FF00
	sw t1, 16(sp)
	jal FillRect
	nop
	la t1, ChannelVolumeCounter
	lw t2, 0(t1)
	li t3, $0A 
	mult t2, t3 
	mflo a2 
	li a0, 50 
	li a1, 110 
	li a3, 10 
	li t1, $00FF00
	sw t1, 16(sp)
	jal FillRect
	nop
DrawText:
	li a0, 50 
	li a1, 270 
	la a2, MVolumeString 
	li a3, 12 
	jal VS_DrawString 
	nop
	li a0, 50 
	li a1, 330 
	la a2, CVolumeString 
	li a3, 15 
	jal VS_DrawString 
	nop
	li a0, 50 
	li a1, 395
	la a2, PlaySampleString 
	li a3, 10 
	jal VS_DrawString 
	nop
BufferSwap:
	li t1, VS_VRAM_TO_VRAM ; gpu0_cmd = VS_VRAM_TO_VRAM;
	sw t1, VS_GP0(t0)      ; *vs_gp0 = gpu0_cmd;
	li t1, VS_DISPLAY_X1   ; x1 = VS_DISPLAY_X1;
	li t2, VS_DISPLAY_Y1   ; y1 = VS_DISPLAY_Y1;
	andi t1, t1, $FFFF     ; x1 &= $FFFF;
	sll t2, t2, $10        ; y1 <<= 16;
	addu t2, t2, t1        ; y1 += x1;
	sw t2, VS_GP0(t0)      ; *vs_gp0 = y1;
	sw zero, VS_GP0(t0)    ; x2 = 0; y2 = 0; *vs_gp0 = y2;
	li t1, VS_WIDTH        ; w = VS_WIDTH;
	li t2, VS_HEIGHT       ; h = VS_HEIGHT;
	andi t1, t1, $FFFF     ; w &= $FFFF;
	sll t2, t2, $10        ; h <<= 16;
	addu t2, t2, t1        ; h += w;
	sw t2, VS_GP0(t0)      ; *vs_gp0 = h;
	jal DMASync
	nop
WaitVSync:              ; Wait For Vertical Retrace Period & Store XOR Pad Data
    lw t1, 0(t0)        ; Load Pad Buffer
    nop               
    beqz t1, WaitVSync ; if(pad_buffer == 0){ goto Wait; }
    nor t1, t1, zero   ; pad_buffer = !(pad_buffer | 0);
    sw zero, 0(t0)     ; Store Zero To Pad Buffer
    sw t1, 8(t0)       ; Store Pad Data
main:
	b Input 
	nop
	addi sp, sp, 100
	
# Function: VS_SetLeftMasterVolume
# Purpose: Sets the volume of the left master channel
# a0: volume 
	.text 
	.globl VS_SetLeftMasterVolume
	.type VS_SetLeftMasterVolume, @function
VS_SetLeftMasterVolume:
	li t0, VS_IO                               ; vs_io_addr = (unsigned long*)VS_IO;
	sh a0, VS_SPU_MASTER_VOLUME_LEFT_ADDR(t0)  ; *(unsigned short*)VS_SPU_MASTER_VOLUME_LEFT_ADDR = volume;
	jr ra 
	nop
	
# Function: VS_SetRightMasterVolume
# Purpose: Sets the volume of the right master channel
# a0: volume 
	.text 
	.globl VS_SetRightMasterVolume
	.type VS_SetRightMasterVolume, @function
VS_SetRightMasterVolume:
	li t0, VS_IO                               ; vs_io_addr = (unsigned long*)VS_IO;
	sh a0, VS_SPU_MASTER_VOLUME_LEFT_ADDR(t0)  ; *(unsigned short*)VS_SPU_MASTER_VOLUME_LEFT_ADDR = volume;
	jr ra 
	nop
	
# Function: VS_SetMasterVolume
# Purpose: Sets the volume of both the left and right master channels
# a0: volume 
	.text 
	.globl VS_SetMasterVolume
	.type VS_SetMasterVolume, @function
VS_SetMasterVolume:
	li t0, VS_IO                                ; vs_io_addr = (unsigned long*)VS_IO;
	sh a0, VS_SPU_MASTER_VOLUME_LEFT_ADDR(t0)   ; *(unsigned short*)VS_SPU_MASTER_VOLUME_LEFT_ADDR = volume;
	sh a0, VS_SPU_MASTER_VOLUME_RIGHT_ADDR(t0)  ; *(unsigned short*)VS_SPU_MASTER_VOLUME_RIGHT_ADDR = volume;
	jr ra 
	nop
	
# Function: VS_SetReverbVolume
# Purpose: Sets the volume of both the left and right reverb channels
# a0: volume 
	.text 
	.globl VS_SetReverbVolume
	.type VS_SetReverbVolume, @function
VS_SetReverbVolume:
	li t0, VS_IO                                ; vs_io_addr = (unsigned long*)VS_IO;
	sh a0, VS_SPU_REVERB_VOLUME_LEFT_ADDR(t0)   ; *(unsigned short*)VS_SPU_REVERB_VOLUME_LEFT_ADDR = volume;
	sh a0, VS_SPU_REVERB_VOLUME_RIGHT_ADDR(t0)  ; *(unsigned short*)VS_SPU_REVERB_VOLUME_RIGHT_ADDR = volume;
	jr ra 
	nop
	
# Function: VS_SetChannelSampleRate
# Purpose: Sets the audio sample rate of an SPU voice channel
# a0: channel, a1: sample_rate
	.text 
	.globl VS_SetChannelSampleRate
	.type VS_SetChannelSampleRate, @function
VS_SetChannelSampleRate:
	sll  a1, a1, $0C                          ; sample_rate <<= 12;
	li t1, 44100
	divu a1, t1                               ; sample_rate /= 44100;
	mflo a1  
	la   t1, VS_SPU_CHANNEL_SAMPLE_RATE_ADDR  ; channel_sample_rate = $1F801C04;
	li t2, $10
	mult  a0, t2                              ; channel *= 16;
	mflo a0
	addu t1, t1, a0                           ; channel_sample_rate += channel;
	sh   a1, 0(t1)                            ; *(unsigned short*)channel_sample_rate = sample_rate;
	jr   ra 
	nop
	
# Function: VS_GetSPUSampleRate
# Purpose: Converts a given 16-bit integer audio sample rate into an equivalent value for an SPU voice channel 
# a0: sample_rate
	.text 
	.globl VS_GetSPUSampleRate
	.type VS_GetSPUSampleRate, @function
VS_GetSPUSampleRate:
	sll  a0, a0, $0C      ; sample_rate <<= 12;
	li t1, 44100
	divu a0, t1           ; sample_rate /= 44100;
	mflo v0 
	jr   ra               ; return sample_rate;
	nop
	
# Function: VS_SetChannelVolume
# Purpose: Sets the volume of an SPU voice channel 
# a0: channel, a1: volume
	.text 
	.globl VS_SetChannelVolume
	.type VS_SetChannelVolume, @function
VS_SetChannelVolume:
	la   t1, $1F801C00   ; VS_SPU_CHANNEL_VOLUME_LEFT_ADDR = (u16*)$1F801C00;
	li   t2, $10
	mult a0, t2          ; channel *= 16;
	mflo a0
	addu t1, t1, a0      ; VS_SPU_CHANNEL_VOLUME_LEFT_ADDR += channel;
	sh   a1, 0(t1)       ; *VS_SPU_CHANNEL_VOLUME_LEFT_ADDR = volume;
	addi t1, t1, $2
	sh   a1, 0(t1)       ; *VS_SPU_CHANNEL_VOLUME_RIGHT_ADDR = volume;
	jr   ra 
	nop 
	
# Function: VS_SetChannelSustainLevel
# Purpose: Sets the sustain level of an SPU voice channel 
# a0: channel, a1: sustain 
	.text 
	.globl VS_SetChannelSustainLevel
	.type VS_SetChannelSustainLevel, @function
VS_SetChannelSustainLevel:
	li   t1, $1F801C08  ; adsr_channel_addr = (u32*)$1F801C08;
	li t2, $010
	mult  a0, t2        ; channel *= 16;
	mflo a0
	addu t1, t1, a0     ; adsr_channel_addr += channel;
	lhu  t2, 0(t1)      ; adsr = *adsr_channel_addr;
	andi a1, a1, $f     ; sustain &= $f;
	or   a1, a1, t2     ; adsr |= sustain;
	sh   a1, 0(t1)      ; *adsr_channel_addr = adsr;
	jr   ra
	nop
	
# Function: VS_TurnOnChannel
# Purpose: Turns on the audio playback of an SPU voice channel 
# a0: channel
	.text 
	.globl VS_TurnOnChannel
	.type VS_TurnOnChannel, @function
VS_TurnOnChannel:
	li   t1, $1F801D88 ; spu_key_on_addr = (u16*)$1F801D88;
	li   t2, $1        ; bit = 1;
	sll  t2, t2, a0
	sh   t2, 0(t1)     ; *spu_key_on_addr = bit;
	li   t1, $1F801D8A
	sra  t2, t2, $10
	sh   t2, 0(t1)
	jr  ra 
	nop
	
# Function: VS_TurnOffChannel
# Purpose: Turns off the audio playback of an SPU voice channel 
# a0: channel
	.text 
	.globl VS_TurnOffChannel
	.type VS_TurnOffChannel, @function
VS_TurnOffChannel:
	li   t0, VS_IO
	li   t1, $0         ; bit = 0;
	sh   t1, $1D8C(t0)  ; *spu_key_off = 0;
	sra  t1, t1, $10
	sh   t1, $1D8E(t0)
	jr  ra 
	nop
	
# Function: VS_GetChannelStatus
# Purpose: Returns the current playback status of the audio channel where 1 is finished and 0 is playing 
# a0: channel 
	.text 
	.globl VS_GetChannelStatus
	.type VS_GetChannelStatus, @function 
VS_GetChannelStatus:
	li   t1, $1F801D9C      ; key_status_addr = (u32*)$1F801D9C;
	lw   v0, 0(t1)          ; value = *key_status_addr;
	nop 
	sra  v0, v0, a0         ; value >>= channel;
	andi v0, v0, $1         ; value &= 1;
	jr ra 
	nop
	
# Function: VS_SetADPCMAddr
# Purpose: Sets the sound ram data transfer address
# a0: addr
	.text 
	.globl VS_SetADPCMAddr
	.type VS_SetADPCMAddr, @function
VS_SetADPCMAddr:
	li t0, VS_IO                           ; vs_io_addr = VS_IO;
	sra a0, a0, $3                         ; addr >>= 3;
	sh a0, VS_SPU_DATA_TRANSFER_ADDR(t0)   ; *(unsigned short*)spu_trans_addr = addr;
	jr ra
	nop 
	
# Function: VS_WaitForSpuDMATransfer
# Purpose: Pause program execution until APCM data has completed its data transfer to the sound ram of the SPU via dma
	.text 
	.globl VS_WaitForSpuDMATransfer
	.type VS_WaitForSpuDMATransfer, @function
VS_WaitForSpuDMATransfer:
	li t0, VS_IO
	li t1, VS_SPU_STATUS_TIMEOUT     ; i = VS_SPU_STATUS_TIMEOUT;
vs_wait_loop:
	beqz t1, vs_wait_loop_end        ; if(i == 0) { goto vs_wait_loop_end; }
	lhu  t2, VS_SPU_STATUS_ADDR(t0)  ; stat = *(unsigned short*)VS_SPU_STATUS_ADDR;
	subi  t1, t1, $1                 ; i--; (delay slot)
	andi t2, t2, $0400               ; stat &= mask;
	bnez t2, vs_wait_loop            ; if(stat != 0) { goto vs_wait_loop; }
	nop
vs_wait_loop_end:
	jr ra
	nop
	
# Function: VS_ClearADSR
# Purpose: Sets the attack, sustain, decay, and release rates to zero for a specific SPU channel
# a0: channel
	.text 
	.globl VS_ClearADSR
	.type VS_ClearADSR, @function
VS_ClearADSR:
	li  t1, $1F801C08
	li t2, $10
	mult  a0, t2
	mflo a0
	addu t1, t1, a0 
	sh   zero, 0(t1)
	jr   ra
	nop
	
# Function: VS_SetDMAWrite
# Purpose: Sets the sound ram transfer mode to dma write
	.text 
	.globl VS_SetDMAWrite
	.type VS_SetDMAWrite, @function 
VS_SetDMAWrite:
	li   t0, VS_IO                             ; vs_io_addr = VS_IO;
	lw   t1, VS_SPU_BUS_CONFIG_ADDR(t0)        ; bus_config = *(unsigned long*)VS_SPU_BUS_CONFIG_ADDR;
	nop
	li   t2, $f0ffffff
	and  t1, t1, t2                            ; bus_config &= $f0ffffff;
	sw   t1, VS_SPU_BUS_CONFIG_ADDR(t0)        ; *(unsigned long*)VS_SPU_BUS_CONFIG_ADDR = bus_config;
	lhu  t1, VS_SPU_CTRL_ADDR(t0)  	           ; ctrl = *(unsigned short*)VS_SPU_CTRL_ADDR;
	nop
	andi t1, t1, $ffcf                         ; ctrl &= disable_current_dma_req;
	sh   t1, VS_SPU_CTRL_ADDR(t0)              ; *(unsigned short*)VS_SPU_CTRL_ADDR = ctrl;
	li   t1, VS_SPU_STATUS_TIMEOUT             ; spu_status_timeout = VS_SPU_STATUS_TIMEOUT;
WaitSpuDMA:
	beqz t1, FinishInitSPUDMA                  ; if(spu_status_timeout == 0) { goto FinishInitSPUDMA; }
	lhu  t2, VS_SPU_STATUS_ADDR(t0)            ; spu_stat = *(unsigned short*)VS_SPU_STATUS_ADDR;
	subi  t1, t1, $1       	                   ; spu_status_timeout--; (delay slot)
	andi t2, t2, $0030                         ; cond = spu_stat & $0030;
	bnez t2, WaitSpuDMA                        ; if(!cond) { goto WaitSpuDMA; }
	nop 
FinishInitSPUDMA:
	lhu  t1, VS_SPU_CTRL_ADDR(t0)              ; ctrl = *(unsigned short*)VS_SPU_CTRL_ADDR;
	nop
	ori  t1, t1, $0020                         ; ctrl |= write_mode;
	sh   t1, VS_SPU_CTRL_ADDR(t0)              ; *(unsigned short*)VS_SPU_CTRL_ADDR = ctrl;
	li   t1, VS_SPU_STATUS_TIMEOUT             ; spu_status_timeout = VS_SPU_STATUS_TIMEOUT;
WaitCtrlReg:
	beqz t1, FinishSetDMAWrite                 ; if(spu_status_timeout == 0) { goto dma_end; }
	lhu  t2, VS_SPU_STATUS_ADDR(t0)            ; spu_stat = *(unsigned short*)VS_SPU_STATUS_ADDR;
	subi t1, t1, $1                            ; spu_status_timeout-- (delay slot)
	andi t2, t2, $0030                         ; spu_stat &= $0030;
	li t3, $0020
	bne  t2, t3, WaitCtrlReg                   ; if(spu_stat != $0020) { goto WaitCtrlReg; }  
	nop
FinishSetDMAWrite:
	jr   ra
	nop
	
# Function: VS_SetDMAOff
# Purpose:
	.text 
	.globl VS_SetDMAOff
	.type VS_SetDMAOff, @function
VS_SetDMAOff:
	li   t0, VS_IO                  ; vs_io_addr = VS_IO;
	lhu  t1, VS_SPU_CTRL_ADDR(t0)  ; ctrl = *(unsigned short*)VS_SPU_CTRL_ADDR;
	nop
	andi t1, t1, $ffcf            ; ctrl &= disable_current_dma_req;
	sh   t1, VS_SPU_CTRL_ADDR(t0)  ; *(unsigned short*)VS_SPU_CTRL_ADDR = ctrl;
	jr   ra 
	nop
	
# Function: VS_WriteADPCM
# Purpose: Writes APCM data via a dma request to the spu transfer address
# a0: apcm_addr, a1: size
	.text 
	.globl VS_WriteADPCM
	.type VS_WriteADPCM, @function
VS_WriteADPCM:
	addi sp, sp, -4 
	sw   ra, 4(sp)
	jal  VS_SetDMAWrite                ; VS_SetDMAWrite();
	nop
	addi a1, a1, 56
	li   t0, VS_IO                    ; vs_io_addr = VS_IO;
	sw   a0, VS_SPU_DMA_M_ADDR(t0)   ; *(unsigned long*)VS_SPU_DMA_M_ADDR = apcm_addr;
	sra  a1, a1, $2                 ; size /= 4;
	andi t1, a1, $f                 
	beqz t1, align_size               ; if(size % 16) { goto align_size; }
	li   t2, $01000201 
	sra a1, a1, $4                  ; size /= 16;
	sll a1, a1, $10                 ; size <<= 16;
	ori a1, a1, $10
	sw  a1, VS_SPU_DMA_BCR_ADDR(t0)  ; *(unsigned long*)VS_SPU_DMA_BCR_ADDR = size;
	sw  t2, VS_SPU_DMA_CHCR_ADDR(t0) ; *(unsigned long*)VS_SPU_DMA_CHCR_ADDR  = $01000201;
	jal VS_WaitForSpuDMATransfer       ; VS_WaitForSpuDMATransfer();
	nop
	jal VS_SetDMAOff                   ; VS_SetDMAOff();
	nop
	lw   ra, 4(sp)
	addi sp, sp, 4
	jr  ra
	nop
align_size:
	addiu a1, a1, 15                 ; size += 15;
	sra a1, a1, $4                  ; size /= 16;
	sll a1, a1, $10                 ; size <<= 16;
	ori a1, a1, $10                 ; size |= 16;
	sw  a1, VS_SPU_DMA_BCR_ADDR(t0)  ; *(unsigned long*)VS_SPU_DMA_BCR_ADDR = size;
	sw  t2, VS_SPU_DMA_CHCR_ADDR(t0) ; *(unsigned long*)VS_SPU_DMA_CHCR_ADDR  = $01000201;
	jal VS_WaitForSpuDMATransfer       ; VS_WaitForSpuDMATransfer();
	nop
	jal VS_SetDMAOff                   ; VS_SetDMAOff();
	nop
	lw   ra, 4(sp)
	addi sp, sp, 4
	jr  ra
	nop
	
# Function: VS_ManuallyWriteADPCM
# a0: adpcm, a1: size 
VS_ManuallyWriteADPCM:
	li t0, VS_IO 
	lhu t1, VS_SPU_CTRL_ADDR(t0)    		 ; ctrl = *(unsigned short*)VS_SPU_CTRL_ADDR;
	sra a1, a1, $1                 		 ; size /= 2;
	andi t1, t1, $ffcf             		 ; ctrl &= $ffcf;
	sh t1, VS_SPU_CTRL_ADDR(t0)     		 ; *(unsigned shor*)VS_SPU_CTRL_ADDR = ctrl;
	li t1, VS_SPU_STATUS_TIMEOUT     		 ; spu_status_timeout = VS_SPU_STATUS_TIMEOUT;
WaitManualSPUDMA:
	beqz t1, FinishInitManualSpuDMA  		 ; if(spu_status_timeout == 0) { goto FinishInitSPUDMA; }
	lhu  t2, VS_SPU_STATUS_ADDR(t0) 		 ; spu_stat = *(unsigned short*)VS_SPU_STATUS_ADDR;
	subi  t1, t1, $1       	      		 ; spu_status_timeout--; (delay slot)
	andi t2, t2, $0030              		 ; cond = spu_stat & $0030;
	bnez t2, WaitManualSPUDMA               ; if(!cond) { goto WaitManualSPUDMA; }
	nop 
FinishInitManualSpuDMA:
	lhu t1, VS_SPU_DATA_TRANSFER_ADDR(t0)  ; addr = *(unsigned short*)VS_SPU_DATA_TRANSFER_ADDR;
	nop
ManualDMALoop:
	li t2, 32 
	blt t2, a1, vs_min_a                   ; if(32 < size) { goto vs_min_a; }
	nop 
	move v0, a1   					     ; min = size;
	sub a1, a1, v0                        ; size -= min;
	b vs_min_b 
	nop
vs_min_a:
	move v0, t2                            ; min = 32;
	sub a1, a1, v0                        ; size -= min;
vs_min_b:
	sh t1, VS_SPU_DATA_TRANSFER_ADDR(t0)   ; *(unsigned short*)VS_SPU_DATA_TRANSFER_ADDR = addr;
	sra t2, v0, $2                        ; incr = min >>= 2;
	addu t1, t1, t2                       ; addr += incr;
WriteDataLoop:
	lhu t2, 0(a0)                          ; half = *adpcm;
	addi a0, a0, $2                       ; adpcm += 2;
	sh t2, $1da8(t0)                      ; *(unsigned short*)VS_SPU_DATA_ADDR = half;
	subi v0, v0, $1                       ; min--;
	bnez v0, WriteDataLoop                  ; if(min != 0) { goto WriteDataLoop; }
	nop
	lhu t2, VS_SPU_CTRL_ADDR(t0)    		 ; ctrl = *(unsigned short*)VS_SPU_CTRL_ADDR;
	nop
	ori t2, t2, $0010             		 ; ctrl |= $0010;
	sh t2, VS_SPU_CTRL_ADDR(t0)     		 ; *(unsigned shor*)VS_SPU_CTRL_ADDR = ctrl;
	li t3, VS_SPU_STATUS_TIMEOUT
WaitDMABusy:
	beqz t3, FinishStatusReg  		         ; if(spu_status_timeout == 0) { goto FinishStatusReg; }
	lhu  t2, VS_SPU_STATUS_ADDR(t0) 		 ; spu_stat = *(unsigned short*)VS_SPU_STATUS_ADDR;
	subi  t3, t3, $1       	      		 ; spu_status_timeout--; (delay slot)
	andi t2, t2, $0400              		 ; cond = spu_stat & $0400;
	bnez t2, WaitDMABusy                    ; if(!cond) { goto WaitDMABusy; }
	nop 
FinishStatusReg:
	li t2, $1000 
Delay:
	subi t2, t2, $1 
	bnez t2, Delay
	nop	
FinalCheck:
	bnez a1, ManualDMALoop
	nop 
	lhu  t1, VS_SPU_CTRL_ADDR(t0)  ; ctrl = *(unsigned short*)VS_SPU_CTRL_ADDR;
	nop
	andi t1, t1, $ffcf            ; ctrl &= disable_current_dma_req;
	sh   t1, VS_SPU_CTRL_ADDR(t0)  ; *(unsigned short*)VS_SPU_CTRL_ADDR = ctrl;
	jr ra 
	nop

# a0: mask, a1: value
WaitStatus:	
	li   t0, VS_IO
	li   t3, VS_SPU_STATUS_TIMEOUT              ; spu_status_timeout = VS_SPU_STATUS_TIMEOUT;
WaitStatusLoop:
	beqz t3, FinishWaitStatus                   ; if(spu_status_timeout == 0) { goto FinishWaitStatus; }
	lhu  t2, VS_SPU_STATUS_ADDR(t0)            ; spu_stat = *(unsigned short*)VS_SPU_STATUS_ADDR;
	subi t3, t3, $1       	                 ; spu_status_timeout--; (delay slot)
	and  t2, t2, a0                           ; cond = spu_stat & mask;
	bne t2, a1, WaitStatusLoop                 ; if(cond != value) { goto WaitStatusLoop; }
	nop 
FinishWaitStatus:
	jr ra 
	nop

# Function: DrawSync
# Purpose: Halts program execution until all drawing commands have been executed by the gpu 
DrawSync:
	li t0, VS_IO             ; vs_io_addr = (unsigned long*)$1F800000;
DrawSyncLoop:
	lw t1, VS_GP1(t0)       ; gpu1 = *vs_gpu1;
	li t2, VS_CMD_STAT_READY ; gpu1_cmd = VS_CMD_STAT_READY; (delay slot)
	and t1, t1, t2         ; gpu1 &= gpu1_cmd;
	beqz t1, DrawSyncLoop    ; if(gpu1 == 0) { goto DrawSyncLoop; }
	nop 
	jr ra
	nop
	
# Function: DMASync
# Purpose: Halts program execution until all gpu dma transfers have completed
DMASync:
	li t0, VS_IO             ; vs_io_addr = (unsigned long*)$1F800000;
DMASyncLoop:
	lw t1, VS_GPU_CHCR(t0)  ; gpu0 = *vs_gpu0;
	li t2, VS_DMA_ENABLE     ; gpu0_cmd = VS_CMD_STAT_READY; (delay slot)
	and t1, t1, t2         ; gpu0 &= gpu0_cmd;
	bnez t1, DMASyncLoop     ; if(gpu0 == 0) { goto DrawSyncLoop; }
	nop 
	jr ra
	nop
	
# Function: VS_GetTexturePage
# Purpose: Gets the texture page of the texture given the texture parameters
# a0: mode, a1: a, a2: x, a3: y 
	.text 
	.globl
	.type, @function
VS_GetTexturePage:
	andi a0, a0, $3    # mode &= 3; 
	sll  a0, a0, $7    # mode <<= 7;
	andi a1, a1, $3    # a &= 3; 
	sll  a1, a1, $5    # a <<= 5;
	or   a0, a0, a1    # mode |= a;
	andi t1, a3, $100  # y &= $100;
	sra  t1, t1, $4    # y >>= 4;
	or   a0, a0, t1    # mode |= y;
	andi a2, a2, $3ff  # x &= $3ff;
	sra  a2, a2, $6    # x >>= 6;
	or   a0, a0, a2    # mode |= x;
	andi a3, a3, $200  # y &= $200;
	sll  a3, a3, $2    # y <<= 2;
	or   v0, a0, a3    # mode |= y;
	jr   ra 
	nop

# Function: VS_GetCLUT
# Purpose: Gets the color palette coordinates in a format that can be given to the GPU 
# a0: x, a1: y 
	.text 
	.globl VS_GetCLUT
	.type, @function
VS_GetCLUT:
	sll  a1, a1, $6  # y <<= 6;
	sra  a0, a0, $4  # x >>= 4;
	andi a0, a0, $3f # x &= $3f;
	or   v0, a0, a1  # y |= x;
	jr ra 
	nop
	
# Function: VS_TransferImageDataToVram
# Purpose: Manually writes image data to the GPU's video memory
# a0: x, a1: y, a2: width, a3: height, 16(sp): data 
VS_TransferImageDataToVram:
	li t0, VS_IO           ; vs_io_addr = (unsigned long*)$1F800000;
	li t1, VS_CPU_TO_VRAM  ; vs_cmd_cpu_to_vram = $A0000000;
	sw t1, VS_GP0(t0)     ; *vs_gp0 = vs_cmd_cpu_to_vram;
	sll a1, a1, $10      ; y <<= 16;
	addu a1, a1, a0      ; y += x;
	sw a1, VS_GP0(t0)     ; *vs_gp0 = y;
	sll t1, a3, $10      ; h <<= 16;
	addu t1, t1, a2      ; h += w;
	sw t1, VS_GP0(t0)     ; *vs_gp0 = h;
	mult a2, a3           ; size = w * h;
	mflo t1 
	sll t1, t1, $1       ; size <<= 1;
	lw  a0, 16(sp)        
	sra t1, t1, $2       ; size /= 4;
	subi sp, sp, 8         
	sw ra, 4(sp)
TransferDataLoop:
	lw t2,0(a0)           ; word = (unsigned long*)data;
	addiu a0, a0, $4     ; data += 4;
	sw t2, VS_GP0(t0)     ; *vs_gpu0 = word;
	bnez t1, TransferDataLoop ; if(size != 0) { goto TransferDataLoop; }
	subi t1, t1, $1         ; size--; (delay slot)
	jal DMASync                ; DMASync();
	nop
	lw ra, 4(sp)
	addi sp, sp, 8
	jr ra 
	nop
	
# Function: VS_TextureFourPointPoly
# Purpose: Draws a textured four-point polygon, a quad, to the display area using the GPU 
# a0: x1, a1: y1, a2: palette, a3: u1, 16(sp): v1, 20(sp): x2, 24(sp): y2, 28(sp): texpage, 32(sp): u2, 36(sp): v2, 40(sp): x3, 44(sp): y3, 48(sp): u3, 52(sp): v3
# 56(sp): x4, 60(sp): y4, 64(sp): u4, 68(sp): v4
VS_TextureFourPointPoly:         
	li   t0, VS_IO                           ; vs_io_addr = (unsigned long*)$1F800000;
	li   t1, VS_TEXTURE_FOUR_POINT_POLY      ; gpu0_cmd = VS_TEXTURE_FOUR_POINT_POLY;
	sw   t1, VS_GP0(t0)                     ; *vs_gpu0 = gpu0_cmd;
	andi a0, a0, $FFFF                     ; x1 &= $FFFF;
	sll  a1, a1, $10                       ; y1 <<= 16;
	or   a1, a1, a0                        ; y1 |= x1;
	sw   a1, VS_GP0(t0)                     ; *vs_gpu0 = y1;
	sll  a2, a2, $10                       ; palette <<= 16;
	lhu  a1, 16(sp)
	andi a3, a3, $FF                       ; u1 &= $FF; 
	andi a1, a1, $FF                       ; v1 &= $FF;
	sll  a1, a1, $8                        ; v1 <<= 8;
	or   a1, a1, a3                        ; v1 |= u1;
	or   a1, a1, a2                        ; v1 |= palette;
	sw   a1, VS_GP0(t0)                     ; *vs_gpu0 = v1;
	lhu  a0, 20(sp)
	lhu  a1, 24(sp)
	andi a0, a0, $FFFF                     ; x2 &= $FFFF;
	sll  a1, a1, $10                       ; y2 <<= 16;
	or   a1, a1, a0                        ; y2 |= x2;
	sw   a1, VS_GP0(t0)                     ; *vs_gpu0 = y2;
	lhu  a1, 36(sp)
	lhu  a2, 28(sp)
	lhu  a3, 32(sp)
	sll  a2, a2, $10                       ; texpage <<= 16;
	andi a3, a3, $FF                       ; u2 &= $FF; 
	andi a1, a1, $FF                       ; v2 &= $FF;
	sll  a1, a1, $8                        ; v2 <<= 8;
	or   a1, a1, a3                        ; v2 |= u2;
	or   a1, a1, a2                        ; v2 |= texpage;
	sw   a1, VS_GP0(t0)                     ; *vs_gpu0 = v2;
	lhu  a0, 40(sp)
	lhu  a1, 44(sp)
	andi a0, a0, $FFFF                     ; x3 &= $FFFF;
	sll  a1, a1, $10                       ; y3 <<= 16;
	or   a1, a1, a0                        ; y3 |= x3;
	sw   a1, VS_GP0(t0)                     ; *vs_gpu0 = y3;
	lhu  a3, 48(sp)
	lhu  a1, 52(sp)
	andi a3, a3, $FF                       ; u3 &= $FF; 
	andi a1, a1, $FF                       ; v3 &= $FF;
	sll  a1, a1, $8                        ; v3 <<= 8;
	or   a1, a1, a3                        ; v3 |= u3;
	sw   a1, VS_GP0(t0)                     ; *vs_gpu0 = v3;
	lhu  a0, 56(sp)
	lhu  a1, 60(sp)
	andi a0, a0, $FFFF                    ; x4 &= $FFFF;
	sll  a1, a1, $10                      ; y4 <<= 16;
	or   a1, a1, a0                       ; y4 |= x4;
	sw   a1, VS_GP0(t0)                    ; *vs_gpu0 = y4;
	lhu  a2, 64(sp)
	lhu  a3, 68(sp)
	andi a2, a2, $FF                      ; u4 &= $FF;
	sll  a3, a3, $8                       ; v4 <<= 8;
	or   a3, a3, a2                       ; v4 |= u4;
	sw   a3, VS_GP0(t0)                    ; *vs_gpu0 = v4;
	jr ra
	nop
	
# Function: DrawLine
# Purpose: Draws a single colored line to the display area 
# a0: x1, a1: y1, a2: x2, a3: y2, 16(sp): color 
DrawLine:
	li t0, VS_IO 
	lw t2, 16(sp)
	li t1, VS_DRAW_LINE     ; gpu0_cmd_draw_line;
	addu t1, t1, t2       ; gpu0_cmd_draw_line += color;
	sw t1, VS_GP0(t0)      ; *vs_gpu0 - gpu0_cmd_draw_line;
	andi a0, a0, $FFFF    ; x1 &= $FFFF;
	sll a1, a1, $10       ; y1 <<= 16;
	addu a1, a1, a0       ; y1 += x1;
	sw a1, VS_GP0(t0)      ; *vs_gp0 = y1;
	andi a2, a2, $FFFF    ; x2 &= $FFFF;
	sll a3, a3, $10       ; y2 <<= 16;
	addu a3, a3, a2       ; y2 += x2;
	sw a3, VS_GP0(t0)      ; *vs_gp0 = y2;
	jr ra 
	nop
	
# Function: FillRect
# Purpose: Draws a monochrome rectangle to the display area
# a0: x1, a1: y1, a2: width, a3: height, 16(sp): color 
FillRect:
	li t0, VS_IO 
	lw t2, 16(sp)
	li t1, VS_FILL_RECT     ; gpu0_cmd = gpu0_cmd_fill_rect;
	addu t1, t1, t2       ; gpu0_cmd += color;
	sw t1, VS_GP0(t0)      ; *vs_gpu0 = gpu0_cmd;
	andi a0, a0, $FFFF    ; x1 &= $FFFF;
	sll a1, a1, $10       ; y1 <<= 16;
	addu a1, a1, a0       ; y1 += x1;
	sw a1, VS_GP0(t0)      ; *vs_gp0 = y1;
	andi a2, a2, $FFFF    ; width &= $FFFF;
	sll a3, a3, $10       ; height <<= 16;
	addu a3, a3, a2       ; height += width;
	sw a3, VS_GP0(t0)      ; *vs_gp0 = height;
	jr ra 
	nop
	
# Function: VS_TextureBurger
# Purpose: Textures a burger sprite to the X,Y screen coordinates with alpha transparency turned on 
# a0: x, a1: y, a2: texpage
VS_TextureBurger:
	subi sp, sp, 80 
	sw ra, 4(sp)
	move t2, a2
	li t0, VS_IO
	li   t1, $E6000000   ; cmd = gpu0_cmd_mask_bits;
	addi t1, t1, $1
	sw t1, VS_GP0(t0)    ; *vs_gp0 = cmd;
	li a2, $0            ; palette = 0;
	li a3, $0            ; u1 = 0;
	sw zero, 16(sp)      ; v1 = 0;
	sw a0, 20(sp)        ; x2 = x;
	li t1, VS_BIMGH
	add t1, t1, a1 
	sw t1, 24(sp)        ; y2 = y + VS_BIMGH;
	sw t2, 28(sp)        ; texpage = GetTexturePage(2,1,texx,texy); 
	sw zero, 32(sp)      ; u2 = 0;
	li t1, VS_BIMGH 
	sw t1, 36(sp)        ; v2 = VS_BIMGH;
	li t1, VS_BIMGW 
	addu t1, t1, a0
	sw t1, 40(sp)        ; x3 = x + VS_BIMGW;
	sw a1, 44(sp)        ; y3 = y;
	li t1, VS_BIMGW 
	sw t1, 48(sp)        ; u3 = VS_BIMGW;
	sw zero, 52(sp)      ; v3 = 0;
	li t1, VS_BIMGW 
	add t1, t1, a0
	sw t1, 56(sp)        ; x4 = x + VS_BIMGW;
	li t1, VS_BIMGH 
	add t1, t1, a1 
	sw t1, 60(sp)        ; y4 = y + VS_BIMGH;
	li t1, VS_BIMGW 
	sw t1, 64(sp)        ; u4 = VS_BIMGW;
	li t1, VS_BIMGH 
	sw t1, 68(sp)        ; v4 = VS_BIMGH;
	jal VS_TextureFourPointPoly
	nop
	jal DrawSync
	nop
	lw ra, 4(sp)
	addi sp, sp, 80
	jr ra 
	nop 
	
# Function: VS_ClampMax 
# Purpose: Clamps a target value to a maximum integer
# a0: target, a1: max 
VS_ClampMax:
	bgt a0, a1, ClampToMax
	nop 
	move v0, a0 
	jr ra 
	nop
ClampToMax:
	move v0, a1 
	jr ra 
	nop
	
# Function: VS_ClampMin 
# Purpose: Clamps a target value to a minimum integer
# a0: target, a1: max 
VS_ClampMin:
	blt a0, a1, ClampToMin
	nop 
	move v0, a0 
	jr ra 
	nop
ClampToMin:
	move v0, a1 
	jr ra 
	nop
	

	
.data 
.align, 4 
SampleADPCM:
	.incbin "sound.adpcm"
	
.data 
.align, 4 
XButton:
	.incbin "xbut.bin"
	
.data 
.align, 4 
Burger:
	.incbin "burger.bin"
	
.data 
.align, 4 
VolumeCounter:
.empty, 4

.data 
.align, 4 
ChannelVolumeCounter:
.empty, 4

.data 
MVolumeString:
	.ascii "MASTER VOLUME"
	
.data 
CVolumeString:
	.ascii "CHANNEL 0 VOLUME"
	
.data 
PlaySampleString:
	.ascii "PLAY SAMPLE"
	
# Function: VS_DrawString
# Purpose: Draws a string to the display area 
# a0: x, a1: y, a2: string, a3: strlen 
	.text 
VS_DrawString:
	addiu sp, sp, -8
    sw ra, 0(sp)
    sw s0, 4(sp)
	move t1, a0          ; orgx = x;
	move t2, a1          ; orgy = y;
DrawChar:
	lbu a0, 0(a2)        ; c = *string;
	addiu a2, a2, $1    ; string++; (delay slot)
	li t3, 32
	beq  a0, t3, vs_draw_space
	nop
	jal VS_CharData        ; data = VS_CharData(c);
	nop
	li t3, VS_CPU_TO_VRAM ; gpu0_cmd = VS_CPU_TO_VRAM; (delay slot)
	sw t3, VS_GP0(t0)    ; *vs_gp0 = gpu0_cmd;
	andi t1, t1, $FFFF  ; x &= $FFFF;
	sll t3, a1, $10     ; y <<= 16;
	addu t3, t3, t1     ; y += x;
	sw t3, VS_GP0(t0)    ; *vs_gp0 = y;
	li t3, VS_FONTW       ; w = VS_IMGW;
	li t4, VS_FONTH       ; h = VS_IMGH;
	sll t4, t4, $10     ; h <<= 16;
	addu t4, t4, t3     ; h += w;
	sw t4, VS_GP0(t0)    ; *vs_gp0 = h;
	li t3, VS_FONTW       ; w = VS_FONTW;
	li t4, VS_FONTH       ; h = VS_FONTH;
	addu t1, t1, t3     ; x += w;
	mult t3, t4          ; size = w * h;
	mflo t3 
	sll t3, t3, $1      ; size <<= 1;
	sra t3, t3, $2      ; size /= 4;
TransferLoop:
	lw t4,0(v0)
	addiu v0, v0, $4
	sw t4, VS_GP0(t0)
	bnez t3, TransferLoop
	subi t3, t3, $1
	blez a3, end
	subi a3, a3, $1     ; strlen--; (delay slot)
	nop
	b DrawChar
	nop
	
vs_draw_space:
	addi t1, t1, $8     ; x += 8;
	b   DrawChar
	subi a3, a3, $1     ; strlen--;
	
end:
	lw ra, 0(sp)
    lw s0, 4(sp)
    addiu sp, sp, 8
    jr ra
	nop

# Function: VS_CharData
# Purpose: Returns the image data of the input character
# a0: c
	.text
VS_CharData:
	li  t7, 48
	beq a0, t7, vs_char_zero 
	li  t8, 49
	beq a0, t8, vs_char_one 
	li  t7, 50 
	beq a0, t7, vs_char_two
	li  t8, 51 
	beq a0, t8, vs_char_three
	li  t7, 52 
	beq a0, t7, vs_char_four
	li  t8, 53 
	beq a0, t8, vs_char_five
	li  t7, 54 
	beq a0, t7, vs_char_six
	li  t8, 55 
	beq a0, t8, vs_char_seven
	li  t7, 56 
	beq a0, t7, vs_char_eight
	li  t8, 57 
	beq a0, t8, vs_char_nine
	li  t7, 65 
	beq a0, t7, vs_char_A
	li  t8, 66
	beq a0, t8, vs_char_B
	li  t7, 67
	beq a0, t7, vs_char_C
	li  t8, 68
	beq a0, t8, vs_char_D
	li  t7, 69
	beq a0, t7, vs_char_E
	li  t8, 70
	beq a0, t8, vs_char_F
	li  t7, 71
	beq a0, t7, vs_char_G
	li  t8, 72
	beq a0, t8, vs_char_H
	li  t7, 73
	beq a0, t7, vs_char_I
	li  t8, 74
	beq a0, t8, vs_char_J
	li  t7, 75
	beq a0, t7, vs_char_K
	li  t8, 76
	beq a0, t8, vs_char_L
	li  t7, 77
	beq a0, t7, vs_char_M
	li  t7, 78
	beq a0, t7, vs_char_N
	li  t8, 79
	beq a0, t8, vs_char_O
	li  t7, 80
	beq a0, t7, vs_char_P
	li  t8, 81
	beq a0, t8, vs_char_Q
	li  t7, 82
	beq a0, t7, vs_char_R
	li  t8, 83
	beq a0, t8, vs_char_S
	li  t7, 84
	beq a0, t7, vs_char_T
	li  t8, 85
	beq a0, t8, vs_char_U
	li  t7, 86
	beq a0, t7, vs_char_V
	li  t8, 87
	beq a0, t8, vs_char_W
	li  t7, 88
	beq a0, t7, vs_char_X
	li  t8, 89
	beq a0, t8, vs_char_Y
	li  t7, 90
	beq a0, t7, vs_char_Z
	li  t8, 46
	beq a0, t8, vs_char_period
	li  t7, 33
	beq a0, t7, vs_char_exclamation
	li  t8, 44
	beq a0, t8, vs_char_comma
	li  t7, 39
	beq a0, t7, vs_char_apostrophe
	li  t8, 43
	beq a0, t8, vs_char_plus 
	li  t7, 45
	beq a0, t7, vs_char_minus
	li  t8, 61
	beq a0, t8, vs_char_equal
	li  t7, 123
	beq a0, t7, vs_char_left_curl
	li  t8, 40
	beq a0, t8, vs_char_left_para
	li  t7, 125
	beq a0, t7, vs_char_right_curl
	li  t8, 41
	beq a0, t8, vs_char_right_para
	li  t7, 42
	beq a0, t7, vs_char_star
	li  t8, 59
	beq a0, t8, vs_char_semi
	li  t7, 47
	beq a0, t7, vs_char_slash 
	li  t8, 35
	beq a0, t8, vs_char_pound
	li  t7, 95
	beq a0, t7, vs_char_underscore
	li  t8, 62
	beq a0, t8, vs_char_great
	li  t7, 60
	beq a0, t7, vs_char_less
	nop
	la v0, VS_A
	jr ra
	nop
vs_char_zero:
	la v0, VS_0
	jr ra
	nop
	
vs_char_one:
	la v0, VS_1
	jr ra
	nop
	
vs_char_two:
	la v0, VS_2
	jr ra
	nop
	
vs_char_three:
	la v0, VS_3
	jr ra
	nop
	
vs_char_four:
	la v0, VS_4
	jr ra
	nop
	
vs_char_five:
	la v0, VS_5
	jr ra
	nop
	
vs_char_six:
	la v0, VS_6
	jr ra
	nop
	
vs_char_seven:
	la v0, VS_7
	jr ra
	nop
	
vs_char_eight:
	la v0, VS_8
	jr ra
	nop
	
vs_char_nine:
	la v0, VS_9
	jr ra
	nop

vs_char_A:
	la v0, VS_A
	jr ra
	nop

vs_char_B:
	la v0, VS_B
	jr ra
	nop

vs_char_C:
	la v0, VS_C
	jr ra
	nop

vs_char_D:
	la v0, VS_D
	jr ra
	nop

vs_char_E:
	la v0, VS_E
	jr ra
	nop

vs_char_F:
	la v0, VS_F
	jr ra
	nop

vs_char_G:
	la v0, VS_G
	jr ra
	nop

vs_char_H:
	la v0, VS_H
	jr ra
	nop

vs_char_I:
	la v0, VS_I
	jr ra
	nop

vs_char_J:
	la v0, VS_J
	jr ra
	nop

vs_char_K:
	la v0, VS_K
	jr ra
	nop

vs_char_L:
	la v0, VS_L
	jr ra
	nop

vs_char_M:
	la v0, VS_M
	jr ra
	nop

vs_char_N:
	la v0, VS_N
	jr ra
	nop

vs_char_O:
	la v0, VS_O
	jr ra
	nop

vs_char_P:
	la v0, VS_P
	jr ra
	nop

vs_char_Q:
	la v0, VS_Q
	jr ra
	nop

vs_char_R:
	la v0, VS_R
	jr ra
	nop

vs_char_S:
	la v0, VS_S
	jr ra
	nop

vs_char_T:
	la v0, VS_T
	jr ra
	nop

vs_char_U:
	la v0, VS_U
	jr ra
	nop

vs_char_V:
	la v0, VS_V
	jr ra
	nop

vs_char_W:
	la v0, VS_W
	jr ra
	nop

vs_char_X:
	la v0, VS_X
	jr ra
	nop

vs_char_Y:
	la v0, VS_Y
	jr ra
	nop

vs_char_Z:
	la v0, VS_Z
	jr ra
	nop

vs_char_period:
	la v0, VS_period
	jr ra
	nop
	
vs_char_exclamation:
	la v0, VS_excl
	jr ra
	nop
	
vs_char_quotes:
	la v0, VS_quotes
	jr ra
	nop
	
vs_char_comma:
	la v0, VS_comma
	jr ra
	nop
	
vs_char_semi:
	la v0, VS_semi 
	jr ra
	nop
	
vs_char_star:
	la v0, VS_star
	jr ra
	nop
	
vs_char_great:
	la v0, VS_great
	jr ra
	nop
	
vs_char_less:
	la v0, VS_less
	jr ra
	nop
	
vs_char_equal:
	la v0, VS_eq
	jr ra
	nop
	
vs_char_apostrophe:
	la v0, VS_apostrophe
	jr ra
	nop
	
vs_char_left_curl:
	la v0, VS_left_curl
	jr ra
	nop
	
vs_char_left_para:
	la v0, VS_left_para
	jr ra
	nop
	
vs_char_right_curl:
	la v0, VS_right_curl
	jr ra
	nop
	
vs_char_right_para:
	la v0, VS_right_para
	jr ra
	nop
	
vs_char_plus:
	la v0, VS_plus
	jr ra
	nop
	
vs_char_minus:
	la v0, VS_minus
	jr ra
	nop
	
vs_char_underscore:
	la v0, VS_underscore
	jr ra
	nop

vs_char_slash:
	la v0, VS_slash
	jr ra
	nop
	
vs_char_pound:
	la v0, VS_pound
	jr ra
	nop
	
.data
.align, 4
VS_0: 
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $7fff, $7fff, $7fff, $0, $0, $0, $7fff, $0, $0, $0, $0 
	.half $7fff, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $7fff, $7fff, $0, $0, $7fff, $0
	.half $0, $7fff, $0, $7fff, $0, $0, $7fff, $0, $7fff, $0, $0, $7fff, $0, $0, $7fff, $7fff, $0, $0, $0, $7fff, $0 
	.half $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $0, $7fff, $7fff, $7fff, $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0

	.data
	.align, 4
VS_1: 
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $7fff, $7fff, $0, $0, $0, $0, $0 
	.half $7fff, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0 
	.half $7fff, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $7fff, $7fff, $7fff 
	.half $7fff, $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $0

	.data
	.align, 4
VS_2: 
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $7fff, $7fff, $7fff, $0, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0
	.half $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $7fff
	.half $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $7fff, $7fff, $7fff, $7fff, $7fff, $7fff
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0

	.data
	.align, 4
VS_3: 
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $7fff, $7fff, $7fff, $0, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0 
	.half $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0 
	.half $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $0, $7fff 
	.half $7fff, $7fff, $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0

	.data
	.align, 4
VS_4: 
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $7fff, $0, $0, $0, $7fff, $0, $0, $0, $7fff, $0, $0, $0, $7fff
	.half $0, $0, $0, $7fff, $0, $0, $0, $7fff, $0, $0, $0, $7fff, $0, $0, $0, $7fff, $7fff, $7fff, $7fff, $7fff, $7fff, $0, $0, $0 
	.half $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0
	.half $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0
	
	.data
	.align, 4
VS_5: 
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $7fff, $7fff, $7fff, $7fff, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0 
	.half $7fff, $0, $0, $0, $0, $0, $0, $0, $7fff, $7fff, $7fff, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $0, $7fff, $0, $0, $0, $0
	.half $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $0, $7fff, $7fff 
	.half $7fff, $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0

	.data
	.align, 4
VS_6: 
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $7fff, $7fff, $7fff, $0, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff
	.half $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $7fff, $7fff, $7fff, $7fff, $7fff, $0, $0, $0, $7fff, $0, $0
	.half $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $0, $7fff, $7fff, $7fff 
	.half $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0


	.data
	.align, 4
VS_7: 
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $7fff, $7fff, $7fff, $7fff, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0 
	.half $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $7fff, $0
	.half $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0 
	.half $0, $0, $0, $0, $0, $0, $0
	
	.data
	.align, 4
VS_8: 
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $7fff, $7fff, $7fff, $0, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff
	.half $0, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $0, $7fff, $7fff, $7fff, $7fff, $0, $0, $0, $7fff, $0 
	.half $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $0, $7fff, $7fff 
	.half $7fff, $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0


	.data
	.align, 4
VS_9: 
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $7fff, $7fff, $7fff, $0, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0 
	.half $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $0, $7fff, $7fff, $7fff, $7fff, $7fff, $0, $0 
	.half $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $7fff, $7fff 
	.half $7fff, $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0
	
	.data
	.align, 4
VS_A: 
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $7fff, $0, $0, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $7fff, $0 
	.half $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff, $7fff, $7fff, $7fff, $7fff, $7fff, $0, $0, $7fff, $0 
	.half $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0 
	.half $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $0
	
	.data
	.align, 4
VS_B: 
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $7fff, $7fff, $7fff, $7fff, $0, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0
	.half $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff, $7fff, $7fff, $7fff, $7fff, $0, $0, $0 
	.half $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff 
	.half $7fff, $7fff, $7fff, $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0
	
	.data
	.align, 4
VS_C: 
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $7fff, $7fff, $7fff, $0, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff
	.half $0, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0
	.half $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $0, $7fff, $7fff, $7fff, $7fff
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $0

	.data
	.align, 4
VS_D: 
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $7fff, $7fff, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $0, $7fff, $0, $0, $0, $7fff
	.half $0, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0
	.half $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $7fff, $0, $0, $0, $7fff, $7fff, $7fff, $7fff
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0
	
	.data
	.align, 4
VS_E: 
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $7fff, $7fff, $7fff, $7fff, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0 
	.half $7fff, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $7fff, $7fff, $7fff, $7fff, $7fff, $7fff, $0, $0 
	.half $7fff, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $7fff, $7fff 
	.half $7fff, $7fff, $7fff, $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $0
	
	.data
	.align, 4
VS_F: 
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $7fff, $7fff, $7fff, $7fff, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0 
	.half $7fff, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $7fff, $7fff, $7fff, $7fff, $0, $0, $0, $0, $7fff
	.half $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $0

	.data
	.align, 4
VS_G: 
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $7fff, $7fff, $7fff, $0, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff
	.half $0, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $7fff 
	.half $7fff, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $0, $7fff, $7fff, $7fff 
	.half $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0

	.data
	.align, 4
VS_H: 
.half 
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff 
	.half $0, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff, $7fff, $7fff, $7fff, $7fff, $7fff, $0, $0 
	.half $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff
	.half $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $0

	.data
	.align, 4
VS_I: 
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $7fff, $7fff, $7fff, $7fff, $7fff, $0, $0, $0, $0, $7fff, $7fff, $0, $0, $0, $0 
	.half $0, $0, $7fff, $7fff, $0, $0, $0, $0, $0, $0, $7fff, $7fff, $0, $0, $0, $0, $0, $0, $7fff, $7fff, $0, $0, $0, $0, $0, $0 
	.half $7fff, $7fff, $0, $0, $0, $0, $0, $0, $7fff, $7fff, $0, $0, $0, $0, $0, $0, $7fff, $7fff, $0, $0, $0, $0, $7fff, $7fff, $7fff 
	.half $7fff, $7fff, $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $0

	.data
	.align, 4
VS_J: 
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $7fff, $7fff, $7fff, $7fff, $7fff, $7fff, $0, $0, $0, $0, $7fff, $7fff, $0, $0, $0
	.half $0, $0, $0, $7fff, $7fff, $0, $0, $0, $0, $0, $0, $7fff, $7fff, $0, $0, $0, $0, $0, $0, $7fff, $7fff, $0, $0, $0, $0, $0 
	.half $0, $7fff, $7fff, $0, $0, $0, $7fff, $0, $0, $7fff, $7fff, $0, $0, $0, $7fff, $7fff, $0, $7fff, $7fff, $0, $0, $0, $0, $7fff 
	.half $7fff, $7fff, $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0

	.data
	.align, 4
VS_K: 
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $7fff, $0, $0, $0, $7fff, $0
	.half $0, $7fff, $0, $0, $0, $0, $7fff, $0, $7fff, $0, $0, $0, $0, $0, $7fff, $7fff, $0, $0, $0, $0, $0, $0, $7fff, $0, $7fff, $0
	.half $0, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $0, $7fff, $0, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $0
	
	.data
	.align, 4
VS_L: 
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0 
	.half $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0
	.half $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $7fff, $7fff, $7fff, $7fff, $7fff, $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $0

	.data
	.align, 4
VS_M: 
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff, $7fff, $0, $0, $7fff, $7fff, $0, $0, $7fff
	.half $0, $7fff, $7fff, $0, $7fff, $0, $0, $7fff, $0, $7fff, $7fff, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff 
	.half $0, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $0

	.data
	.align, 4
VS_N: 
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff, $7fff, $0, $0, $0, $7fff, $0, $0, $7fff
	.half $0, $7fff, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $7fff, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $7fff, $7fff, $0, $0, $7fff
	.half $0, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $0

	.data
	.align, 4
VS_O: 
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $7fff, $7fff, $7fff, $0, $0, $0, $7fff, $0
	.half $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0 
	.half $0, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $0, $7fff, $7fff, $7fff, $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0

	.data
	.align, 4
VS_P: 
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $7fff, $7fff, $7fff, $7fff, $0, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0
	.half $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff, $7fff, $7fff, $7fff, $7fff, $0, $0, $0 
	.half $7fff, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0

	.data
	.align, 4
VS_Q: 
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $7fff, $7fff, $7fff, $0, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff
	.half $0, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $7fff
	.half $0, $0, $7fff, $0, $0, $7fff, $0, $0, $7fff, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $7fff, $7fff
	.half $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $7fff,

	.data
	.align, 4
VS_R: 
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $7fff, $7fff, $7fff, $7fff, $0, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0 
	.half $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff, $7fff, $7fff, $7fff, $7fff, $0, $0, $0 
	.half $7fff, $7fff, $7fff, $0, $0, $0, $0, $0, $7fff, $0, $7fff, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff, $7fff, $0, $0, $0 
	.half $7fff, $0, $0, $0, $7fff, $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $0

	.data
	.align, 4
VS_S: 
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $7fff, $7fff, $7fff, $0, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0 
	.half $7fff, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $7fff, $7fff, $7fff, $0, $0, $0, $0
	.half $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $0, $7fff, $7fff 
	.half $7fff, $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0

	.data
	.align, 4
VS_T: 
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $7fff, $7fff, $7fff, $7fff, $7fff, $0, $0, $7fff, $7fff, $7fff, $7fff, $7fff, $7fff
	.half $0, $0, $0, $0, $7fff, $7fff, $0, $0, $0, $0, $0, $0, $7fff, $7fff, $0, $0, $0, $0, $0, $0, $7fff, $7fff, $0, $0, $0, $0
	.half $0, $0, $7fff, $7fff, $0, $0, $0, $0, $0, $0, $7fff, $7fff, $0, $0, $0, $0, $0, $0, $7fff, $7fff, $0, $0, $0, $0, $0, $0 
	.half $7fff, $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0

	.data
	.align, 4
VS_U: 
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff
	.half $0, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0
	.half $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $0, $7fff, $7fff, $7fff, $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0

	.data
	.align, 4
VS_V: 
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff, $0
	.half $0, $0, $0, $7fff, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0 
	.half $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $7fff, $7fff, $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0

	.data
	.align, 4
VS_W: 
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff, $0
	.half $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0
	.half $0, $7fff, $0, $0, $7fff, $0, $7fff, $7fff, $0, $7fff, $0, $0, $7fff, $0, $7fff, $7fff, $0, $7fff, $0, $0, $7fff, $7fff, $0, $0, $7fff, $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $0

	.data
	.align, 4
VS_X: 
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $7fff, $0, $0, $0, $7fff, $0, $0, $0, $7fff, $0, $0, $0, $7fff, $0
	.half $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $7fff, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $7fff, $0, $7fff
	.half $0, $0, $0, $0, $7fff, $0, $0, $0, $7fff, $0, $0, $0, $7fff, $0, $0, $0, $7fff, $0, $0, $0, $7fff, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0

	.data
	.align, 4
VS_Y: 
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff, $0
	.half $0, $0, $0, $7fff, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $0, $7fff, $7fff, $0, $0, $0, $0, $0, $0, $7fff, $7fff
	.half $0, $0, $0, $0, $0, $0, $7fff, $7fff, $0, $0, $0, $0, $0, $0, $7fff, $7fff, $0, $0, $0, $0, $0, $0, $7fff, $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0

	.data
	.align, 4
VS_Z: 
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $7fff, $7fff, $7fff, $7fff, $7fff, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0
	.half $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0
	.half $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $7fff, $7fff, $7fff, $7fff, $7fff, $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $0
	
	.data
	.align, 4
VS_excl: 
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $7fff
	.half $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0
	.half $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0
	
	.data
	.align, 4
VS_apostrophe: 
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $7fff, $0, $0, $0, $0, $0, $0, $7fff, $7fff, $0, $0, $0, $0, $0
	.half $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0
	
	.data
	.align, 4
VS_comma: 
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $7fff, $0, $0, $0, $0, $0, $0
	.half $7fff, $7fff, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0
	
	.data
	.align, 4
VS_eq:
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0
	.half $0, $0, $0, $7fff, $7fff, $7fff, $7fff, $7fff, $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0 
	.half $0, $7fff, $7fff, $7fff, $7fff, $7fff, $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0
	
	.data
	.align, 4
VS_quotes: 
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $7fff, $0, $0, $0, $0, $0, $7fff, $0, $7fff, $0, $0, $0, $0, $0 
	.half $7fff, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0
	
	.data
	.align, 4
VS_great: 
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $0
	.half $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $7fff, $0
	.half $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0
	
	.data
	.align, 4
VS_left_curl: 
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $7fff, $7fff, $7fff, $0, $0, $0, $7fff, $7fff, $0, $0, $0, $0
	.half $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $7fff, $7fff, $0, $0, $0, $0, $0, $0, $0 
	.half $7fff, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $7fff, $7fff, $0, $0, $0, $0, $0, $0, $0, $7fff, $7fff, $7fff, $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $0
	
	.data
	.align, 4
VS_left_para: 
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $7fff
	.half $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0
	.half $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0
	
	.data
	.align, 4
VS_less: 
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $7fff
	.half $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0 
	.half $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0

	.data
	.align, 4
VS_minus: 
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0 
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $7fff, $7fff, $7fff, $7fff, $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $0
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0

	.data
	.align, 4
VS_period: 
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0 
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0
	.half $0, $0, $0, $0, $0, $0, $0, $7fff, $7fff, $0, $0, $0, $0, $0, $0, $7fff, $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0
	
	.data
	.align, 4
VS_plus: 
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $7fff
	.half $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $7fff, $7fff, $7fff, $7fff, $7fff, $0, $0, $0, $0, $0, $7fff, $0, $0
	.half $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0
	
	.data
	.align, 4
VS_pound: 
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $7fff
	.half $7fff, $7fff, $7fff, $7fff, $7fff, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $7fff 
	.half $7fff, $7fff, $7fff, $7fff, $7fff, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $7fff, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0
	
	.data
	.align, 4
VS_right_curl: 
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $7fff, $7fff, $7fff, $0, $0, $0, $0, $0, $0, $0, $7fff, $7fff, $0, $0, $0, $0 
	.half $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $7fff, $7fff, $0, $0, $0, $0, $0, $0
	.half $7fff, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $7fff, $7fff, $0, $0, $0, $7fff, $7fff, $7fff, $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0
	
	.data
	.align, 4
VS_right_para: 
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $0
	.half $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $7fff, $0
	.half $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0

	.data
	.align, 4
VS_semi: 
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $7fff, $0, $0, $0, $0, $0, $0
	.half $7fff, $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0
	
	.data
	.align, 4
VS_slash: 
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0 
	.half $0, $7fff, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0 
	.half $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0

	.data
	.align, 4
VS_star: 
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0, $7fff, $7fff, $7fff, $0, $0, $0, $0, $7fff
	.half $7fff, $7fff, $7fff, $7fff, $0, $0, $0, $0, $7fff, $7fff, $7fff, $0, $0, $0, $0, $0, $0, $7fff, $0, $0, $0, $0, $0, $0
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0
	
	.data
	.align, 4
VS_underscore: 
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0 
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0 
	.half $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $0, $7fff, $7fff, $7fff, $7fff, $7fff, $7fff, $0, $0, $0, $0, $0, $0, $0, $0, $0