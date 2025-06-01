#ifndef VS_PSYQOBJ_H
#define VS_PSYQOBJ_H

#include <stdio.h>

/********************************************
*   VideoStation Assembler
*
*   Copyright (c) 2025 Ryandracus Chapman
*
*   File: vs_psyqobj.h
*   Date: 5/25/2025
*   Version: 1.0
*   Updated: 6/1/2025
*   Author: Ryandracus Chapman
*
********************************************/

typedef struct VS_PSYQ_HEADER{
	char magic[4];
	unsigned char processor_type;
}VS_PSYQ_HEADER;

typedef struct VS_PSYQ_SECTION_HEADER{
	unsigned char magic;
	unsigned char section_number;
	unsigned short group;
	unsigned short alignment;
	unsigned char symbol_strlen;
	char symbol_name[8];
}VS_PSYQ_SECTION_HEADER;

typedef struct VS_PSYQ_SYMBOL{
	unsigned char cmd;
	unsigned short symbol_num;
	unsigned short section_number;
	unsigned long offset;
	unsigned char symbol_strlen;
}VS_PSYQ_SYMBOL;

typedef enum VS_PSYQ_RELOC_TYPE{
	VS_PSYQ_HI_16 = 82,
	VS_PSYQ_LO_16 = 84,
	VS_PSYQ_26    = 74,
}VS_PSYQ_RELOC_TYPE;

typedef struct VS_PSYQ_RELOC{
	unsigned long index;
	unsigned char reloc_type;
	unsigned short reloc_offset;
	unsigned short section_num;
	unsigned long dest_symbol_addr;
}VS_PSYQ_RELOC;

void VS_InitPSYQRelocTable();
void VS_SetPSYQRelocTrue();
void VS_AddPSYQRelocEntry(unsigned long index, unsigned char reloc_type, unsigned short offset, unsigned short section_number, unsigned short dest_symbol_addr);
void VS_PrintPSYQRelocTable();
void VS_WritePSYQHeader(FILE* file, VS_PSYQ_HEADER header);
void VS_WritePSYQSectionHeader(FILE* file, VS_PSYQ_SECTION_HEADER header);
void VS_SwitchSection(FILE* file, unsigned short section);
void VS_WriteCodeCmd(FILE* file, unsigned short size);
void VS_WriteSLDInfoCmd(FILE* file, unsigned short offset);
void VS_WriteSymbol(FILE* file, VS_PSYQ_SYMBOL symbol);
void VS_WritePSYQObj(char* filename);

#endif