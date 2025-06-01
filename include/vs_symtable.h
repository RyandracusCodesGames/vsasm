#ifndef VS_SYMTABLE_H
#define VS_SYMTABLE_H

/********************************************
*   VideoStation Assembler
*
*   Copyright (c) 2025 Ryandracus Chapman
*
*   File: vs_symtable.h
*   Date: 4/23/2025
*   Version: 1.0
*   Updated: 6/1/2025
*   Author: Ryandracus Chapman
*
********************************************/

#define VS_MAX_SYMBOLS 2048

typedef enum VS_SYM_TYPE{
	VS_SYM_OBJ  = 0x1,
	VS_SYM_FUNC = 0x2,
	VS_SYM_SECT = 0x3,
}VS_SYM_TYPE;

typedef enum VS_SYM_SCOPE{
	VS_SCOPE_LOCAL = 0x0,
	VS_SCOPE_GLOBAL = 0x1,
}VS_SYM_SCOPE;

typedef struct VS_SYM{
	char* name;
	unsigned long addr;
	unsigned long size;
	unsigned long instruction_count;
	unsigned long string_table_index;
	unsigned long num_of_instructions;
	VS_SYM_TYPE type;
	VS_SYM_SCOPE scope;
}VS_SYM;

typedef struct VS_SYM_TABLE{
	VS_SYM symbols[VS_MAX_SYMBOLS];
	unsigned long size;
}VS_SYM_TABLE;

void VS_InitSymbolTable();
int VS_AddSymbol(char* name, unsigned long instruction_count, unsigned long size, VS_SYM_TYPE type, VS_SYM_SCOPE scope);
int VS_AddDataSymbol(char* name, unsigned long addr, unsigned long instruction_count, unsigned long size, VS_SYM_TYPE type, VS_SYM_SCOPE scope);
void VS_UpdateDataSize(unsigned long index, unsigned long size);
void VS_UpdateDataAddr(unsigned long index, unsigned long addr);
void VS_UpdateNumberOfInstructions(int index, unsigned long num_of_instructions);
int VS_GetIndexOfLastFuncSymbol();
unsigned int VS_FindSymbol(char* name);
VS_SYM VS_GetSymbol(char* name);
int VS_GetSymbolIndex(char* name);
VS_SYM VS_GetSymbolFromIndex(unsigned long index);
unsigned long VS_GetSymbolTableSize();
void VS_ExpandAddrForAllDataSymbols(int expand);
unsigned long VS_GetSymbolAddr(unsigned long index);
void VS_PrintSymbolTable();
void VS_DestroySymbolTable();

#endif