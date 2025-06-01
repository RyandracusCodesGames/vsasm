#ifndef VS_PREPROCESSER_H
#define VS_PREPROCESSER_H

#include <stdio.h>
#include <vs_utils.h>

/********************************************
*   VideoStation Assembler
*
*   Copyright (c) 2025 Ryandracus Chapman
*
*   File: vs_parser.h
*   Date: 4/23/2025
*   Version: 1.0
*   Updated: 6/1/2025
*   Author: Ryandracus Chapman
*
********************************************/

#define VS_MAX_LINE 1024
#define VS_MAX_MACROS 5000

typedef struct VS_MACRO{
	char* name;
	char* value;
}VS_MACRO;

typedef struct VS_MACRO_TABLE{
	VS_MACRO macro[VS_MAX_MACROS];
	unsigned long size;
}VS_MACRO_TABLE;

int VS_StringIsReservedWord(char* name);
int VS_MacroIsReservedWord(char* name, char* value);
int VS_MacroIsAlreadyInTable(char* name);
void VS_AddMacro(char* name, char* value);
void VS_AddRegMacro(char* name, char* value);
void VS_AddSetMacro(char* name, char* value);
void VS_SortMacroTable();
void VS_PrintMacroTable();
int VS_LineContainsMacro(char* line);
void VS_DestroyMacroTable();
int VS_ReadLine(FILE* in, char* line);
void VS_TrimLine(char* dest, char* src);
void VS_TrimStrictLine(char* dest, const char* src);
void VS_PrintTrimmedLine(FILE* out, const char* line);
void VS_PrintStrictTrimmedLine(FILE* out, const char* line);
int VS_IsStringBlank(const char* line);
int VS_StrictIsStringBlank(const char* line);
int VS_LineContainsInstruction(const char* line, VS_ASM_PARAMS* params);
int VS_LineContainsDirective(char* line);
int VS_PreproccessAssemblyFile(FILE* in, FILE* preprocess, VS_ASM_PARAMS* params);
int VS_GetFirstOccurenceIndex(char* str, char c);
int VS_GetNumberOfCommas(char* str);

#endif