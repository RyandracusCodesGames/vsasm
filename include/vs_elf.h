#ifndef VS_ELF_H
#define VS_ELF_H

#include <stdio.h>
#include <vs_utils.h>

/********************************************
*   VideoStation Assembler
*
*   Copyright (c) 2025 Ryandracus Chapman
*
*   File: vs_elf.h
*   Date: 4/30/2025
*   Version: 1.1
*   Updated: 6/6/2025
*   Author: Ryandracus Chapman
*
********************************************/

#include <vs_utils.h>

#define VS_MAX_RELOCS 4096

#define VS_MAX_INSTRUCTION_READ 2048
#define VS_MAX_READ 4096
#define VS_ELF32_ST_INFO(b,t) (((b)<<4)+((t)&0xf))
#define VS_ELF32_R_INFO(s,t) (((s)<<8)+(unsigned char)(t))

typedef struct VS_ELF{
	char magic[4];
	unsigned char bits;
	unsigned char endian;
	unsigned char version;
	unsigned char os;
	unsigned char abi_version;
	unsigned char pad[7];
	unsigned short obj_type;
	unsigned short architecture;
	unsigned long elf_version;
	unsigned long entry;
	unsigned long program_header_offset;
	unsigned long section_header_table;
	unsigned long flags;
	unsigned short size;
	unsigned short program_header_size;
	unsigned short program_header_num;
	unsigned short section_header_size;
	unsigned short section_header_num;
	unsigned short shstrndx;
}VS_ELF;

typedef struct VS_SECTION_HEADER{
	unsigned long sh_name;
	unsigned long sh_type;
	unsigned long sh_flags;
	unsigned long sh_addr;
	unsigned long sh_offset;
	unsigned long sh_size;
	unsigned long sh_link;
	unsigned long sh_info;
	unsigned long sh_addralign;
	unsigned long sh_entsize;
}VS_SECTION_HEADER;

typedef struct VS_ELF_SYMBOL{
	unsigned long st_name;
	unsigned long st_value;
	unsigned long st_size;
	unsigned char st_info;
	unsigned char st_other;
	unsigned short st_shndx;
}VS_ELF_SYMBOL;

typedef enum VS_RELOC_TYPE{
	VS_MIPS_HI_16 = 5,
	VS_MIPS_LO_16 = 6,
	VS_MIPS_26    = 4,
	VS_MIPS_PC16  = 10,
}VS_RELOC_TYPE;

typedef struct VS_ELF_RELOC{
	unsigned long r_offset;
	unsigned long r_info;
}VS_ELF_RELOC;

int VS_FilenameHasExtension(char* filename);
void VS_AlignOffset(FILE* file, unsigned long* offset, unsigned long align);
int VS_WriteELF(char* filename, VS_ASM_PARAMS params);
void VS_WriteELFHeader(FILE* file, VS_ELF elf);
void VS_WriteELFSymbol(FILE* file, VS_ELF_SYMBOL symbol);
void VS_WriteSectionHeader(FILE* file, VS_SECTION_HEADER sh);
void VS_InitRelocTable();
void VS_SetRelocTrue();
void VS_AddRelocEntry(unsigned long offset, unsigned long reloc_type, int is_jump);
void VS_AddUndefinedRelocEntry(unsigned long offset, unsigned long reloc_type, int index);
void VS_PrintRelocTable();

#endif