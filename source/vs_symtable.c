#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vs_symtable.h>

/********************************************
*   VideoStation Assembler
*
*   Copyright (c) 2025 Ryandracus Chapman
*
*   File: vs_symtable.c
*   Date: 4/23/2025
*   Version: 1.0
*   Updated: 6/1/2025
*   Author: Ryandracus Chapman
*
********************************************/

VS_SYM_TABLE sym_table;

void VS_InitSymbolTable(){
	sym_table.size = 0;
}

int VS_AddSymbol(char* name, unsigned long instruction_count, unsigned long size, VS_SYM_TYPE type, VS_SYM_SCOPE scope){
	unsigned long table_size = sym_table.size, len = strlen(name);
	
	if(size + 1 < VS_MAX_SYMBOLS){
		sym_table.symbols[table_size].name = malloc(len+1);
		memcpy(sym_table.symbols[table_size].name,name,len);
		sym_table.symbols[table_size].name[len] = '\0';

		sym_table.symbols[table_size].addr = instruction_count * 4;
		sym_table.symbols[table_size].instruction_count = instruction_count;
		sym_table.symbols[table_size].size = size;
		sym_table.symbols[table_size].type = type;
		sym_table.symbols[table_size].scope = scope;
		
		sym_table.size++;
	}
	
	return table_size;
}

int VS_AddDataSymbol(char* name, unsigned long addr, unsigned long instruction_count, unsigned long size, VS_SYM_TYPE type, VS_SYM_SCOPE scope){
	unsigned long table_size = sym_table.size, len = strlen(name);
	
	if(table_size + 1 < VS_MAX_SYMBOLS){
		sym_table.symbols[table_size].name = malloc(len+1);
		memcpy(sym_table.symbols[table_size].name,name,len);
		sym_table.symbols[table_size].name[len] = '\0';
		
		sym_table.symbols[table_size].addr = addr;
		sym_table.symbols[table_size].instruction_count = instruction_count;
		sym_table.symbols[table_size].size = size;
		sym_table.symbols[table_size].type = type;
		sym_table.symbols[table_size].scope = scope;
		
		sym_table.size++;
	}
	
	return table_size;
}

void VS_UpdateDataSize(unsigned long index, unsigned long size){
	if(index < VS_MAX_SYMBOLS){
		sym_table.symbols[index].size = size;
	}
}

void VS_UpdateDataAddr(unsigned long index, unsigned long addr){
	if(index < VS_MAX_SYMBOLS){
		sym_table.symbols[index].addr = addr;
	}
}

void VS_IncrDataAddr(unsigned long index, unsigned long num){
	if(index < VS_MAX_SYMBOLS){
		sym_table.symbols[index].addr += num;
	}
}

void VS_UpdateNumberOfInstructions(int index, unsigned long num_of_instructions){
	if(index >= 0 && index < VS_MAX_SYMBOLS){
		sym_table.symbols[index].num_of_instructions = num_of_instructions;
	}
}

int VS_GetIndexOfLastFuncSymbol(){
	int size = sym_table.size, i;
	for(i = size; i >= 0; i--){
		if(sym_table.symbols[i].type == VS_SYM_FUNC){
			return i;
		}
	}
	
	return -1;
}

unsigned int VS_FindSymbol(char* name){
	unsigned long size = sym_table.size, i;
	for(i = 0; i < size; i++){
		if(!strcmp(name,sym_table.symbols[i].name)){
			return 1;
		}
	}
	
	return 0;
}

int VS_GetSymbolIndex(char* name){
	unsigned long size = sym_table.size, i;
	for(i = 0; i < size; i++){
		if(!strcmp(name,sym_table.symbols[i].name)){
			return i;
		}
	}
	
	return -1;
}

void VS_ExpandAddrForAllDataSymbols(int expand){
	unsigned long size = sym_table.size, i;
	for(i = 0; i < size; i++){
		if(sym_table.symbols[i].type == VS_SYM_OBJ){
			sym_table.symbols[i].addr += expand;
		}
	}
}

VS_SYM VS_GetSymbol(char* name){
	VS_SYM sym;
	sym.name = NULL;
	
	unsigned long size = sym_table.size, i;
	for(i = 0; i < size; i++){
		if(!strcmp(name,sym_table.symbols[i].name)){
			return sym_table.symbols[i];
		}
	}
	
	return sym;
}

VS_SYM VS_GetSymbolFromIndex(unsigned long index){
	VS_SYM sym;
	sym.name = NULL;

	if(index < sym_table.size){
		return sym_table.symbols[index];
	}
	else return sym;
}

unsigned long VS_GetSymbolTableSize(){
	return sym_table.size;
}


unsigned long VS_GetSymbolAddr(unsigned long index){
	if(index < VS_MAX_SYMBOLS){
		return sym_table.symbols[index].addr;
	}
	else return 0;
}

void VS_PrintSymbolTable(){
	unsigned long size = sym_table.size, i, scope, type;
	for(i = 0; i < size; i++){
		scope = sym_table.symbols[i].scope;
		type = sym_table.symbols[i].type;
		
		printf("Symbol Name: %s\n",sym_table.symbols[i].name);
		printf("Symbol Size: %ld\n",sym_table.symbols[i].size);
		
		if(scope == 0){
			printf("Symbol Scope: Local\n");
		}
		else{
			printf("Symbol Scope: Global\n");
		}
		
		if(type == 1){
			printf("Symbol Type: Object\n");
		}
		else{
			printf("Symbol Type: Function\n");
			printf("Symbol Number Of Instructions: %ld\n",sym_table.symbols[i].num_of_instructions);
		}
		
		printf("Symbol Address: %ld\n",sym_table.symbols[i].addr);
		
		printf("\n");
	}
}

void VS_DestroySymbolTable(){
	unsigned long size = sym_table.size, i;
	for(i = 0; i < size; i++){
		if(sym_table.symbols[i].name != NULL){
			free(sym_table.symbols[i].name);
			sym_table.symbols[i].name = NULL;
		}
	}
	
	sym_table.size = 0;
}