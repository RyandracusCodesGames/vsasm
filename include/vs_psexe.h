#ifndef VS_PSEXE_H
#define VS_PSEXE_H

/********************************************
*   VideoStation Assembler
*
*   Copyright (c) 2025 Ryandracus Chapman
*
*   File: vs_psexe.h
*   Date: 5/3/2025
*   Version: 1.0
*   Updated: 6/1/2025
*   Author: Ryandracus Chapman
*
********************************************/

typedef struct VS_PSEXE{
	char psexe_magic_id[8];
	unsigned char zero_filled[8];
	unsigned long pc;
	unsigned long gp;
	unsigned long ram_dest_addr;
	unsigned long file_size;
	unsigned long unknown_1;
	unsigned long unknown_2;
	unsigned long mem_fill_start_addr;
	unsigned long mem_fill_size;
	unsigned long init_sp_base;
	unsigned long init_sp_offset;
	unsigned char reserved[20];
}VS_PSEXE;

void VS_WritePSEXE(char* filename, unsigned long org, int output);

#endif