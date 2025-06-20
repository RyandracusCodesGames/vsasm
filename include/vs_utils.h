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
*   Version: 1.1
*   Updated: 6/10/2025
*   Author: Ryandracus Chapman
*
********************************************/

#define VS_MAX_INCLUDES 100

typedef struct FileIncludeEntry{
	char* name;
	unsigned long line_start;
	unsigned long line_end;
	int num_of_lines;
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

typedef enum VS_ENDIAN{
	VS_LITTLE_ENDIAN = 0x0,
	VS_BIG_ENDIAN = 0x1,
}VS_ENDIAN;

typedef enum VS_EXE_FORMAT{
	VS_EXE_PSX = 0x0,
	VS_EXE_CPE = 0x1,
}VS_EXE_FORMAT;

typedef struct VS_ASM_PARAMS{
	unsigned long org;
	unsigned long syntax;
	unsigned long endian;
	unsigned long architecture;
	int oexe;
	int exe_fmt;
	int undefsym;
	int nowarnings;
}VS_ASM_PARAMS;

void VS_InitFileIncludeTable();
void VS_DestroyFileIncludeTable();
unsigned long VS_AddIncludeEntry(char* name, unsigned long line_start, unsigned long line_end);
void VS_UpdateIncludeEntry(unsigned long index, unsigned long line_start, unsigned long line_end);
int VS_ErrorOccuredInIncludeEntry(unsigned long line_count);
int VS_GetActualLineCount(unsigned long line_count);
void VS_PrintErrorFromIncludeEntry(unsigned long entry, unsigned long line_count);
void VS_PrintError(FILE* in, char* line, unsigned long line_count);
int VS_GrabMacrosFromIncludeEntry(char* incdir, unsigned long incindex, unsigned long line_count);
int VS_VerifyPathSyntax(char* path, char* directive, unsigned long line_count);
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
void VS_LowercaseLine(char* line);
void VS_LowercaseAndCopyLine(char* dest, char* src, int n);
void VS_AppendVFPUEncoding(const char* name, unsigned long* instruction);
void VS_PrintErrorString(int error);
void VS_WriteInstruction(FILE* file, unsigned long instruction, VS_ENDIAN endian);
void VS_WriteNop(FILE* file);

#endif