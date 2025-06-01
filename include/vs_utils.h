#ifndef VS_UTILS_H
#define VS_UTILS_H

#include <stdio.h>

/********************************************
*   VideoStation Assembler
*
*   Copyright (c) 2025 Ryandracus Chapman
*
*   File: vs_utils.h
*   Date: 5/21/2025
*   Version: 1.0
*   Updated: 6/1/2025
*   Author: Ryandracus Chapman
*
********************************************/

#define VS_MAX_INCLUDES 100

typedef struct FileIncludeEntry{
	char* name;
	unsigned long line_start;
	unsigned long line_end;
}FileIncludeEntry;

typedef struct FileIncludeTable{
	FileIncludeEntry table[VS_MAX_INCLUDES];
	unsigned size;
}FileIncludeTable;

typedef enum VS_ObjectFormat{
	VS_ELF_OBJ  = 0x0,
	VS_PSYQ_OBJ = 0x1,
}VS_ObjectFormat;

typedef enum VS_SYNTAX{
	VS_GNU_SYNTAX = 0x0,
	VS_ASMPSX_SYNTAX = 0x1,
}VS_SYNTAX;

typedef struct VS_ASM_PARAMS{
	unsigned long org;
	unsigned long syntax;
	unsigned long endian;
	unsigned long architecture;
	int oexe;
}VS_ASM_PARAMS;

void VS_InitFileIncludeTable();
void VS_DestroyFileIncludeTable();
unsigned long VS_AddIncludeEntry(char* name, unsigned long line_start, unsigned long line_end);
void VS_UpdateIncludeEntry(unsigned long index, unsigned long line_start, unsigned long line_end);
int VS_ErrorOccuredInIncludeEntry(unsigned long line_count);
void VS_PrintErrorFromIncludeEntry(unsigned long entry, unsigned long line_count);
void VS_PrintError(FILE* in, char* line, unsigned long line_count);
int VS_GrabMacrosFromIncludeEntry(char* incdir, unsigned long incindex, unsigned long line_count, VS_ASM_PARAMS* params);
unsigned long VS_CountNumberOfCharInLine(char* line, char c);
unsigned long VS_SwapLong(unsigned long dword);
unsigned short VS_SwapShort(unsigned short hword);
unsigned long VS_GetCharCountInLine(char* line, char c);
char* VS_GetArchitectureName(unsigned long architecture);
unsigned char VS_GetConditionField(const char* instruction);
unsigned char VS_IsFloatingPointComparison(const char* instruction);
unsigned char VS_ToDigit(char c);
unsigned char VS_GetVFPUSize(const char* vfpu_instruction);
unsigned char VS_GetVFPUComparison(char* cond);
void VS_UnderlineLine(char* line);
void VS_AppendVFPUEncoding(const char* name, unsigned long* instruction);
void VS_PrintErrorString(int error);

#endif