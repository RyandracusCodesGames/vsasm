#include <vs_preprocessor.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <vs_opcode.h>
#include <vs_symtable.h>
#include <vs_parser.h>

/********************************************
*   VideoStation Assembler
*
*   Copyright (c) 2025 Ryandracus Chapman
*
*   File: vs_parser.c
*   Date: 4/23/2025
*   Version: 1.0
*   Updated: 6/1/2025
*   Author: Ryandracus Chapman
*
********************************************/

extern VS_OPCODE vs_opcode_table[];
extern VS_SYM_TABLE sym_table;

VS_MACRO_TABLE macro_table;

char* reserved_words[] = {
	"add","addu","addi","addiu","and","andi","b","beq","beqz","bgez","bgtz","ble","blez","blt","bltz","bne",
	"bnez","cfc2","ctc2","div","divu","j","jal","jalr","jr","la","li","lb","lbu","lh","lhu","lui","lw","lwc2",
	"lwl","lwr","mfc2","mfhi","mthi","mflo","mtlo","mtc2","mul","mult","multu","move","nor","nop","xor","or",
	"ori","sb","sh","sw","slt","slti","sltiu","sltu","sll","srl","sra","sub","subu","$0","$1","$2","$3","$4",
	"$zero","$at","$sp","$ra","$gp","$k1","$k2","$5","$6","$7","$8","$9","$10","$11","$12","$13","$14","$15",
	"$16","$17","$18","$19","$20","$21","$22","$23","$24","$25","$26","$27","$28","$29","$30","$31","$t0","$t1",
	"$t2","$t3","$t4","$t5","$t6","$t7","$t8","$t9","$v0","$v1","$a0","$a1","$a2","$a3","$s0","$s1","$s2","$s3",
	"$s4","$s5","$s6","$s7","text",".text","data",".data","ktext",".ktext","globl",".globl","global",".global",
	".word","word",".byte","byte",".half","half","type",".type","section",".section","bss",".bss","@function",
	"include",".include","@object", ".align", ".incbin",".incasm",".ascii",".empty",".org",".arch",".float",
	".syntax",".undefsym",
};

char* directives[] = {
	".text",".data",".ktext",".globl",".global", ".word",".byte",".half",".dw",".dh",".db",".arch",".float",".syntax",".undefsym",
	".type",".section",".bss",".include", ".align", ".incbin",".incasm",".ascii",".empty",".safeloaddelay",".safeloadoff",".org"
};

int VS_StringIsReservedWord(char* name){
	int i, num_reserved_words = sizeof(reserved_words) / sizeof(reserved_words[0]);
	for(i = 0; i < num_reserved_words; i++){
		if(!strcmp(name,reserved_words[i])){
			return 1;
		}
	}
	
	return 0;
}

int VS_MacroIsReservedWord(char* name, char* value){
	int i, num_reserved_words = sizeof(reserved_words) / sizeof(reserved_words[0]);
	for(i = 0; i < num_reserved_words; i++){
		if(!strcmp(name,reserved_words[i])){
			return 1;
		}
		
		if(!strcmp(value,reserved_words[i])){
			return 1;
		}
	}
	
	return 0;
}

int VS_MacroIsAlreadyInTable(char* name){
	unsigned long i, size = macro_table.size;
	for(i = 0; i < size; i++){
		if(!strcmp(name,macro_table.macro[i].name)){
			return 1;
		}
	}
	
	return 0;
}

void VS_AddMacro(char* name, char* value){
	unsigned long size = macro_table.size;
	
	if(!VS_MacroIsReservedWord(name,value) && !VS_MacroIsAlreadyInTable(name)){
		macro_table.macro[size].name  = malloc(strlen(name)+1);
		macro_table.macro[size].value = malloc(strlen(value)+1);
		
		strcpy(macro_table.macro[size].name, name);
		strcpy(macro_table.macro[size].value, value);
		
		macro_table.size++;
	}
}

void VS_AddRegMacro(char* name, char* value){
	unsigned long i, size = macro_table.size;
	
	if(!VS_StringIsReservedWord(name) && (VS_IsValidRegister(value,VS_GNU_SYNTAX) || VS_IsValidRegister(value,VS_ASMPSX_SYNTAX))){
		
		for(i = 0; i < size; i++){
			if(!strcmp(name,macro_table.macro[i].name)){
				free(macro_table.macro[i].value);
				macro_table.macro[i].value = malloc(strlen(value)+1);
				strcpy(macro_table.macro[i].value, value);
				return;
			}
		}
		
		macro_table.macro[size].name  = malloc(strlen(name)+1);
		macro_table.macro[size].value = malloc(strlen(value)+1);
		
		strcpy(macro_table.macro[size].name, name);
		strcpy(macro_table.macro[size].value, value);
		
		macro_table.size++;
	}
}

void VS_AddSetMacro(char* name, char* value){
	unsigned long i, size = macro_table.size;
	
	if(!VS_MacroIsReservedWord(name,value)){
		
		for(i = 0; i < size; i++){
			if(!strcmp(name,macro_table.macro[i].name)){
				free(macro_table.macro[i].value);
				macro_table.macro[i].value = malloc(strlen(value)+1);
				strcpy(macro_table.macro[i].value, value);
				return;
			}
		}
		
		macro_table.macro[size].name  = malloc(strlen(name)+1);
		macro_table.macro[size].value = malloc(strlen(value)+1);
		
		strcpy(macro_table.macro[size].name, name);
		strcpy(macro_table.macro[size].value, value);
		
		macro_table.size++;
	}
}

int VS_LineContainsDirective(char* line){
	int i, size = sizeof(directives) / sizeof(directives[0]);
	char instruction[15];
	memcpy(instruction,line,14);
	instruction[14] = '\0';
	
	for(i = 0; i < size; i++){
		if(strstr(instruction,directives[i]) != NULL){
			return i;
		}
	}
	
	return -1;
}

void VS_SortMacroTable(){
	int i, j, size = macro_table.size;
	for(i = 0; i < size - 1; i++){
		for(j = 0; j < size - 1; j++){
			int leni = strlen(macro_table.macro[j].name);
			int lenj = strlen(macro_table.macro[j+1].name);
			
			if(leni < lenj){
				char* temp = macro_table.macro[j].name;
				macro_table.macro[j].name = macro_table.macro[j+1].name;
				macro_table.macro[j+1].name = temp;
				
				temp = macro_table.macro[j].value;
				macro_table.macro[j].value = macro_table.macro[j+1].value;
				macro_table.macro[j+1].value = temp;
			}
		}
	}
}

void VS_PrintMacroTable(){
	unsigned long i, size = macro_table.size;
	for(i = 0; i < size; i++){
		printf("Macro %ld:\n",i);
		printf("Name = %s\n",macro_table.macro[i].name);
		printf("Value = %s\n",macro_table.macro[i].value);
	}
}

int VS_LineContainsMacro(char* line){
	unsigned long i, size = macro_table.size;
	for(i = 0; i < size; i++){
		if(strstr(line,macro_table.macro[i].name) != NULL){
			return i;
		}
	}
	
	return -1;
}

void VS_DestroyMacroTable(){
	unsigned long i, size = macro_table.size;
	for(i = 0; i < size; i++){
		if(macro_table.macro[i].name != NULL){
			free(macro_table.macro[i].name);
			macro_table.macro[i].name = NULL;
		}
		
		if(macro_table.macro[i].value != NULL){
			free(macro_table.macro[i].value);
			macro_table.macro[i].value = NULL;
		}
	}
	
	macro_table.size = 0;
}

int VS_ReadLine(FILE* in, char* line){
	if(fgets(line, VS_MAX_LINE, in) == NULL){
		return 0;
	}
	else return 1;
}

void VS_TrimLine(char* dest, char* src){
	int i, size = 0, len = strlen(src);
	for(i = 0; i < len; i++){
		if(src[i] != ' '){
			dest[size++] = src[i];
		}
	}
}

void VS_TrimStrictLine(char* dest, const char* src){
	int i, index = 0, size = 0, len = strlen(src);
	for(i = 0; i < len; i++){
		if(src[i] != ' ' && src[i] != '\r' && src[i] != '\n' && src[i] != '\t' && src[i] != 13 && src[i] != '\v'){
			dest[size++] = src[i];
			index = i;
		}
	}
	
	dest[index+1] = '\0';
}

int VS_GetFirstOccurenceIndex(char* str, char c){
	int i;
	for(i = 0; i < VS_MAX_LINE; i++){
		if(str[i] == c){
			return i;
		}
	}
	
	return -1;
}

int VS_GetNumberOfCommas(char* str){
	int i, len = strlen(str), count = 0;
	for(i = 0; i < len; i++){
		if(str[i] == ','){
			count++;
		}
	}
	
	return count;
}

void VS_PrintTrimmedLine(FILE* out, const char* line){
	int i, len = strlen(line);
	for(i = 0; i < len; i++){
		if(line[i] != ' ' && line[i] != '\r' && line[i] != '\t' && line[i] != 13 && line[i] != '\v')
			fprintf(out,"%c",line[i]);
	}
}

void VS_PrintAndStoreTrimmedLine(FILE* out, const char* line, char* dest){
	int i, len = strlen(line), count;
	
	if(strstr(line,".ascii") != NULL){
		char* ascii = strstr(line,".ascii");
		char* string = ascii + strlen(".ascii");
		string += VS_GetFirstOccurenceIndex(string,'\"');
		memset(dest,0x0,VS_MAX_LINE);
		memcpy(dest,".ascii",strlen(".ascii"));
		memcpy(dest + strlen(".ascii"),string,strlen(string));
		dest[strlen(".ascii") + strlen(string)] = '\0';
		fprintf(out,"%s",dest);
		return;
	}
	
	for(i = 0, count = 0; i < len; i++){
		if(line[i] != ' ' && line[i] != '\n' && line[i] != '\r' && line[i] != '\t' && line[i] != 13 && line[i] != '\v'){
			dest[count++] = line[i];
		}
	}
	
	dest[count] = '\0';
	
	fprintf(out,"%s\n",dest);
}

void VS_PrintStrictTrimmedLine(FILE* out, const char* line){
	int i, len = strlen(line);
	for(i = 0; i < len; i++){
		if(line[i] != ' ' && line[i] != '\r' && line[i] != '\n' && line[i] != '\t' && line[i] != 13 && line[i] != '\v')
			fprintf(out,"%c",line[i]);
	}
}

int VS_IsStringBlank(const char* line){
	int i, len = strlen(line);
	for(i = 0; i < len; i++){
		if(line[i] != ' ' && line[i] != '\r' && line[i] != '\t' && line[i] != 13 && line[i] != '\v')
			return 0;
	}
	
	return 1;
}

int VS_StrictIsStringBlank(const char* line){
	int i, len = strlen(line);
	for(i = 0; i < len; i++){
		if(line[i] != ' ' && line[i] != '\n' && line[i] != '\r' && line[i] != '\t' && line[i] != 13 && line[i] != '\v')
			return 0;
	}
	
	return 1;
}

int VS_LineContainsInstruction(const char* line, VS_ASM_PARAMS* params){
	int i, j, ind = 0, size = VS_GetNumberOfInstructions();
	char instruction[12], reg[3];
	
	for(i = 0; i < 10; i++){
		if(params->syntax == VS_GNU_SYNTAX){
			if(line[i] == '$' || line[i] == '\n'){
				break;
			}
		}
		else{
			memcpy(reg,line+i,2);
			reg[2] = '\0';
			
			if(line[0] == 'v' && params->architecture == VS_MIPS_PSP_ARCH){
				VS_UnderlineLine(line);
				memcpy(instruction,line,10);
				instruction[11] = '\0';
				ind = 11;
				break;
			}
		
			if(line[0] != 's' && line[i] == 'l' && line[i+1] == 'a'){
				instruction[0] = 'l';instruction[1] = 'a';
				ind = 1;
				break;
			}
			
			if(line[i] == 'n' && line[i+1] == 'o' && line[i+2] == 'r'){
				instruction[0] = 'n';instruction[1] = 'o';instruction[2] = 'r';
				ind = 2;
				break;
			}
			
			if(line[i] == 'o' && line[i+1] == 'r' && line[i+2] != 'i'){
				instruction[0] = 'o';instruction[1] = 'r';
				ind = 1;
				break;
			}
			
			if(line[i] == 's' && line[i+1] == 'r' && line[i+2] == 'a'){
				instruction[0] = 's';instruction[1] = 'r';instruction[2] = 'a';
				ind = 2;
				break;
			}
			
			if(line[i] == 'j' && line[i+1] == 'a' && line[i+2] == 'l' && line[i+3] != 'r'){
				instruction[0] = 'j';instruction[1] = 'a';instruction[2] = 'l';
				ind = 2;
				break;
			}
			
			if(line[i] == 's' && line[i+1] == 'r' && line[i+2] == 'a' && line[i+3] == 'v'){
				instruction[0] = 's';instruction[1] = 'r';instruction[2] = 'a';instruction[3] = 'v';
				ind = 3;
				break;
			}
			
			if((VS_LineContainsRegister(reg)) || line[i] == '\n' || line[i] == '$'){
				break;
			}
		}
		
		instruction[i] = tolower(line[i]);
		ind = i;
	}
	
	instruction[ind+1] = '\0';

	if(params->syntax == VS_ASMPSX_SYNTAX && strstr(instruction,"zero")){
		char* zero = strstr(line,"zero");
		int len = (int)(zero - line);
		memcpy(instruction,line,len);
		instruction[len] = '\0';
	}
	
	for(i = 0; i < size; i++){
		if(!strcmp(instruction,vs_opcode_table[i].name)){
			return i;
		}
	}
	
	if(strstr(instruction,"cop2") != NULL || strstr(instruction,"syscall") != NULL || strstr(instruction,"bal") || strstr(instruction,"jal") 
		|| instruction[0] == 'b' || instruction[0] == 'j' || params->architecture == VS_MIPS_PSP_ARCH){
			
		for(i = ind; i >= 0; i--){
			instruction[i] = '\0';
			
			for(j = 0; j < size; j++){
				if(!strcmp(instruction,vs_opcode_table[j].name)){
					return j;
				}
			}		
		}
	}
	
	return -1;
}

int VS_LLineContainsInstruction(const char* line){
	int i, index = -1, ind = 0, size = VS_GetNumberOfInstructions();
	char instruction[9];
	
	for(i = 0; i < 8; i++){
		if(line[i] == '$' || line[i] == '\n'){
			break;
		}
		
		instruction[i] = tolower(line[i]);
		ind = i;
	}
	
	instruction[ind+1] = '\0';

	for(i = 0; i < size; i++){
		if(strstr(instruction,vs_opcode_table[i].name) != NULL){
			index = i;
			break;
		}
	}

	if(index == -1){	
		return -1;
	}
	
	if(!strcmp(instruction,vs_opcode_table[index].name)){
		return index;
	}
	
	if(strstr(instruction,"cop2") != NULL || strstr(instruction,"syscall") != NULL || strstr(instruction,"bal")
		|| strstr(instruction,"jal") || instruction[0] == 'b' || instruction[0] == 'j'){
		if(instruction[0] == 'b' && instruction[1] != 'a' && instruction[2] != 'l'){
			return VS_GetOpcode("b"); 
		}
		else if(instruction[0] == 'j' && instruction[1] != 'a' && instruction[2] != 'l'){
			return VS_GetOpcode("j"); 
		}
		else return index;
	}
	else return -1;
}

int VS_PreproccessAssemblyFile(FILE* in, FILE* preprocess, VS_ASM_PARAMS* params){
	FILE* inc, *pre, *dat;
	unsigned long size, line_start, line_end, incindex, num_of_instructions;
	int macro_index, index;
	int first_occurence, line_count, instruction_count, prev_instruction_count, data_offset;
	int data_index, old_data_index, data_size;
	int has_data_section, safe_load_delay;
	char line[VS_MAX_LINE+1];
	char dest[VS_MAX_LINE+1];
	char name[VS_MAX_LINE+1];
	unsigned char zero = 0;
	unsigned char binarr[500];
	VS_SYM_TYPE type;
	VS_SYM_SCOPE scope;
	
	macro_table.size = 0;
	safe_load_delay = 0;
	line_count = 0;
	has_data_section = 0;
	line_start = 0;
	line_end = 0;
	prev_instruction_count = 0;
	
	/* BEGIN PREPROCESSING BY SEARCHING FOR AND REMOVING COMMENTS ( '#' OR ';' SYMBOLS) */
	/* BEGIN SEARCHING FOR .INCLUDE DIRECTIVES AND REPLACING MACROS WITH THEIR SET VALUES */
	while(VS_ReadLine(in, line)){
		char* comment = strstr(line, "#");
		
		line_count++;
		
		/* IF '#' COMMENT TYPE NOT FOUND, TRY ';' COMMENT TYPE */
		if(comment == NULL){
			comment = strstr(line,";");
			
			if(comment != NULL){
				first_occurence = VS_GetFirstOccurenceIndex(line,';');
				memcpy(dest, line, first_occurence);
				dest[first_occurence] = '\0';
			}
			else{
				/* THE CURRENT LINE CONTAINS NO COMMENTS */
				memcpy(dest, line, VS_MAX_LINE);
			}
		}
		else{
			first_occurence = VS_GetFirstOccurenceIndex(line,'#');
			memcpy(dest, line, first_occurence);
			
			if(!first_occurence){
				dest[first_occurence] = '\n';	
			}
			else{
				dest[first_occurence] = '\0';
			}
		}

		if(strstr(line,".data") != NULL){
			has_data_section = 1;
		}
		
		if(!VS_IsStringBlank(dest) && dest != NULL && dest[0] != '\n'){
			char* end_quotes;
			char trim[VS_MAX_LINE];
	
			VS_PrintAndStoreTrimmedLine(preprocess,dest,name);
			
			char* token = strstr(name,"equr");
			char* include_dir = strstr(name,".include");
			
			if(token != NULL && strstr(name,"equal") == NULL){
				char* value = token + strlen("equr");
				memset(dest,'\0',VS_MAX_LINE);
				memcpy(dest,name,(int)(token - name));
				VS_AddRegMacro(dest,value);
			}
			
			token = strstr(name,"equ");
			
			if(token != NULL && strstr(name,"equal") == NULL && strstr(name,"equr") == NULL){
				char* value = token + strlen("equ");
				memset(dest,'\0',VS_MAX_LINE);
				memcpy(dest,name,(int)(token - name));
				VS_AddMacro(dest,value);
			}
			
			token = strstr(name,"set");
			
			if(token != NULL && VS_LineContainsInstruction(name,params) == -1 && strstr(name,":") == NULL && VS_LineContainsDirective(name) == -1){
				char* value = token + strlen("set");
				memset(dest,'\0',VS_MAX_LINE);
				memcpy(dest,name,(int)(token - name));
				VS_AddSetMacro(dest,value);
			}
			
			//printf("include_dir = %s\n",include_dir);
			
			if(include_dir != NULL){
				char* path =  include_dir + strlen(".include");
			//	printf("path = %s\n",path);
				
				if(path[0] != '\'' && path[0] != '\"'){
					printf("Syntax Error: The file path of an include directive must be encased in double quotes or single quotes\n");
					printf("Line %d: %s\n",line_count,path);
					return -1;
				}
				else{
					if(path[0] == '\"'){
						end_quotes = strstr(path + 1,"\"");
						
						if(end_quotes == NULL){
							printf("Syntax Error: The file path of an include directive must be encased in double quotes or single quotes\n");
							printf("Line %d: %s\n",line_count,path);
							return -1;
						}
					}
					else{
						end_quotes = strstr(path + 1,"\'");
						
						if(end_quotes == NULL){
							printf("Syntax Error: The file path of an include directive must be encased in double quotes or single quotes\n");
							printf("Line %d: %s\n",line_count,path);
							return -1;
						}
					}
					
					size = end_quotes - path;
					
					if(size >= 1){
						size = size - 1;
					}
					
					strncpy(dest, path + 1, size);
					
					dest[size] = '\0';
					
				//	printf("dest = %s\n",dest);
					
					inc = fopen(dest,"rb");
					
					if(inc == NULL){
						printf("Warning: The file path of the include directive could not be found!\n");
						printf("Line %d: %s\n",line_count,dest);
					}
					else{
						if(line_end == 0){
							line_start = line_count + line_end;
						}
						else{
							line_start = line_end + 1;
						}
						
						line_end = line_start;
						
						incindex = VS_AddIncludeEntry(dest,line_start,line_end);
						
						while(VS_ReadLine(inc, dest)){
							memset(trim,0x0,VS_MAX_LINE);
							VS_TrimLine(trim,dest);
							
							//printf("Line = %s\n",trim);
							
							token = strstr(trim,"equr");
							
							if(token != NULL){
								memset(name,0x0,VS_MAX_LINE);
								strncpy(name,trim,VS_GetFirstOccurenceIndex(trim,'e'));
								char* value = token + strlen("equr");
								
							//	printf("Name = %s\n",name);
							//	printf("Token = %s\n",value);
								
								VS_AddRegMacro(name,value);
								
								fprintf(preprocess,"\n");
								line_end++;
							}
							else if(strstr(trim,"equ") != NULL && strstr(line,"equr") == NULL){
								token = strstr(trim,"equ");
								
								char* value = token + strlen("equ");
								memset(name,'\0',VS_MAX_LINE);
								memcpy(name,trim,(int)(token - trim));
								
								//printf("Name = %s\n",name);
								//printf("Token = %s\n",value);
								
								VS_AddMacro(name,value);
								
								fprintf(preprocess,"\n");
								line_end++;
							}
							else if(strstr(trim,"set") != NULL && VS_LineContainsInstruction(line,params) == -1 && strstr(line,":") == NULL && VS_LineContainsDirective(line) == -1){
								token = strstr(trim,"set");
								
								char* value = token + strlen("set");
								memset(name,'\0',VS_MAX_LINE);
								memcpy(name,trim,(int)(token - trim));
								
								//printf("Name = %s\n",name);
								//printf("Token = %s\n",value);
								
								VS_AddSetMacro(name,value);
								
								fprintf(preprocess,"\n");
								line_end++;
							}
							else if(strstr(dest, ".include") != NULL){
								char* incdir = strstr(trim,".include") + strlen(".include");
								
								int errorcode = VS_GrabMacrosFromIncludeEntry(incdir,incindex,line_count, params);
								
								if(errorcode == -1){
									return -1;
								}
								
								fprintf(preprocess,"\n");
								line_end++;
							}
							else if(!VS_IsStringBlank(dest) && dest != NULL && dest[0] != '\n'){
								line_end++;
								VS_PrintAndStoreTrimmedLine(preprocess,dest,trim);
							}
						}
						
						VS_UpdateIncludeEntry(incindex,line_start,line_end);
						
						fclose(inc);
					}
				}
			}
		}
		else{
			fprintf(preprocess,"\n");
		}
	}
	
	VS_SortMacroTable();
	
	memset(dest,0x0,VS_MAX_LINE);
	fseek(in, 0x0, SEEK_SET);
	fseek(preprocess, 0x0, SEEK_SET);
	
	line_count = 0;
	instruction_count = 0;
	
	VS_InitSymbolTable();
	
	scope = VS_SCOPE_LOCAL;
	type = VS_SYM_FUNC;
	size = 0;
	data_offset = data_index = data_size = old_data_index = 0;
	
	if(has_data_section){
		dat = fopen("datasec.dat","wb");
	}
	
	/* ITERATE THROUGH MACRO TABLE AND REPLACE MACROS WITH THEIR RESPECTIVE VALUES, AND REMOVE .INCLUDE DIRECTIVES */
	
	pre = fopen("preprocessor.i","wb+");
	
	while(VS_ReadLine(preprocess,line)){
		char* include_dir = strstr(line,".include");
		char* equ = strstr(line,"equ");
		
		if(include_dir == NULL){
			macro_index = VS_LineContainsMacro(line);
			
			if(equ != NULL){
				fprintf(pre,"\n");
			}
			else if(strstr(line,"set") != NULL && VS_LineContainsInstruction(line,params) != -1 && VS_LineContainsDirective(line) != -1 && strstr(name,":") == NULL){
				fprintf(pre,"\n");
			}
			else{
				if(macro_index != -1){
					memcpy(dest, line, VS_MAX_LINE);
					
					while(macro_index != -1){
						char* macro_name = macro_table.macro[macro_index].name;
						
						size = (int)(strstr(dest,macro_name) - dest);
						
						strncpy(name, dest, size);
						name[size] = '\0';
						
						VS_PrintStrictTrimmedLine(pre,name);
						VS_PrintStrictTrimmedLine(pre,macro_table.macro[macro_index].value);
						
					//	printf("name = %s\n",name);
					//	printf("value = %s\n",macro_table.macro[macro_index].value);
						
						char* after = strstr(dest,macro_name) + strlen(macro_name);
						
						strncpy(dest, after, strlen(after));
						dest[strlen(after)] = '\0';
						
						macro_index = VS_LineContainsMacro(dest);
						
						if(macro_index == -1)
							VS_PrintStrictTrimmedLine(pre,dest);
						
						macro_index = VS_LineContainsMacro(dest);
					}
					
					fprintf(pre,"\n");
				}
				else{
					fprintf(pre,"%s",line);
				}
			}
		}
		else{
			fprintf(pre,"\n");
		}
	}
	
	memset(dest,0x0,VS_MAX_LINE);
	fseek(pre,0x0,SEEK_SET);
	
	while(VS_ReadLine(pre, line)){
		line_count++;
		
		int instr_index = VS_LineContainsInstruction(line,params);
		int dir = VS_LineContainsDirective(line);
		
		if(strstr(line,".globl") != NULL){
			scope = VS_SCOPE_GLOBAL;
		}
		
		if(strstr(line,".global") != NULL){
			scope = VS_SCOPE_GLOBAL;
		}
		
		if(strstr(line,".data") != NULL){
			type = VS_SYM_OBJ;
		}
		
		if(strstr(line,".text") != NULL){
			type = VS_SYM_FUNC;
		}

		if(strstr(line,".safeloaddelay") != NULL){
			safe_load_delay = 1;
		}
		
		if(strstr(line,".safeloadoff") != NULL){
			safe_load_delay = 0;
		}
		
		if(strstr(line,".arch") != NULL){
			char* directive = strstr(line,".arch") + strlen(".arch");
			
			if(strstr(directive,"mips1") != NULL){
				params->architecture = VS_MIPS_I_ARCH;
			}
			else if(strstr(directive,"mips2") != NULL){
				params->architecture = VS_MIPS_II_ARCH;
			}
			else if(strstr(directive,"psx") != NULL){
				params->architecture = VS_MIPS_PSX_ARCH;
			}
			else if(strstr(directive,"psp") != NULL){
				params->architecture = VS_MIPS_PSP_ARCH;
			}
			else{
				printf("Warning: Unrecognized architecture. Must be set to mips1, mips2, psx, or psp\n");
				
				int err = VS_ErrorOccuredInIncludeEntry(line_count);
				
				if(err != -1){
					VS_PrintErrorFromIncludeEntry(err,line_count);
				}
				else{
					VS_PrintError(in,line,line_count);
				}
			}
		}
		
		if(strstr(line,".syntax") != NULL){
			char* directive = strstr(line,".syntax") + strlen(".syntax");
			
			if(strstr(directive,"gnu") != NULL){
				params->syntax = VS_GNU_SYNTAX;
			}
			else if(strstr(directive,"asmpsx") != NULL){
				params->syntax = VS_ASMPSX_SYNTAX;
			}
			else{
				printf("Warning: Unrecognized syntax. Must be set to gnu or asmpsx\n");
				
				int err = VS_ErrorOccuredInIncludeEntry(line_count);
				
				if(err != -1){
					VS_PrintErrorFromIncludeEntry(err,line_count);
				}
				else{
					VS_PrintError(in,line,line_count);
				}
			}
		}
		
		char* align = strstr(line,".align");
		
		if(align != NULL){
			char* align_value = strstr(align,",");
			
			if(align_value == NULL){
				printf("Syntax Error: .align directive must contain a comma followed by a value '.align, <value>'\n");
				
				int err = VS_ErrorOccuredInIncludeEntry(line_count);
				
				if(err != -1){
					VS_PrintErrorFromIncludeEntry(err,line_count);
				}
				else{
					VS_PrintError(in,line,line_count);
				}
				
				fclose(pre);
				
				if(has_data_section){
					fclose(dat);
				}
				
				return -1;
			}
			else{
				align = align_value + 1;
				int result;
				int align_num;
				
				if(align[0] == '0' && align[1] == 'x'){
					result = VS_HexDFA(align);
				}
				else{
					result = VS_IntDFA(align);
				}
				
				if(!result){
					printf("Syntax Error: .align directive must have a valid positive, immediate value\n");
					int err = VS_ErrorOccuredInIncludeEntry(line_count);
				
					if(err != -1){
						VS_PrintErrorFromIncludeEntry(err,line_count);
					}
					else{
						VS_PrintError(in,line,line_count);
					}
					
					fclose(pre);
					
					if(has_data_section){
						fclose(dat);
					}
				
					return -1;
				}
				
				align_num = (int)strtoul(align, NULL, 0);
				
				while(data_offset % align_num){
					data_offset++;
					fwrite(&zero,1,1,dat);
				}
			}
		}
		
		char* inclubin = strstr(line,".incbin");

		if(inclubin != NULL){
			char* path =  inclubin + strlen(".incbin");
			char* end_quotes;
		//	printf("path = %s\n",path);
			
			if(path[0] != '\'' && path[0] != '\"'){
				printf("Syntax Error: The file path of an include directive must be encased in double quotes or single quotes\n");
				
				int err = VS_ErrorOccuredInIncludeEntry(line_count);
				
				if(err != -1){
					VS_PrintErrorFromIncludeEntry(err,line_count);
				}
				else{
					printf("Line %d: %s\n",line_count,path);
				}
				
				return -1;
			}
			else{
				if(path[0] == '\"'){
					end_quotes = strstr(path + 1,"\"");
					
					if(end_quotes == NULL){
						printf("Syntax Error: The file path of an include directive must be encased in double quotes or single quotes\n");
						
						int err = VS_ErrorOccuredInIncludeEntry(line_count);
				
						if(err != -1){
							VS_PrintErrorFromIncludeEntry(err,line_count);
						}
						else{
							printf("Line %d: %s\n",line_count,path);
						}
						
						return -1;
					}
				}
				else{
					end_quotes = strstr(path + 1,"\'");
					
					if(end_quotes == NULL){
						printf("Syntax Error: The file path of an include directive must be encased in double quotes or single quotes\n");
						
						int err = VS_ErrorOccuredInIncludeEntry(line_count);
				
						if(err != -1){
							VS_PrintErrorFromIncludeEntry(err,line_count);
						}
						else{
							printf("Line %d: %s\n",line_count,path);
						}

						return -1;
					}
				}
				
				size = end_quotes - path;
				
				if(size >= 1){
					size = size - 1;
				}
				
				strncpy(dest, path + 1, size);
				
				dest[size] = '\0';
				
			//	printf("dest = %s\n",dest);
				
				inc = fopen(dest,"rb");
				
				if(inc == NULL){
					printf("Warning: The file path of the include directive could not be found!\n");
					
					int err = VS_ErrorOccuredInIncludeEntry(line_count);
				
					if(err != -1){
						VS_PrintErrorFromIncludeEntry(err,line_count);
					}
					else{
						printf("Line %d: %s\n",line_count,path);
					}
				}
				else{
					has_data_section = 1;
					
					fseek(inc, 0x0, SEEK_END);
					size = ftell(inc);
					fseek(inc, 0x0, SEEK_SET);
					
					unsigned long i, count;
					for(i = 0; i < size;){
						count = fread(binarr,1,500,inc);
						fwrite(binarr,1,count,dat);
						i += count;
					}
					
					data_offset += size;
					data_size += size;
					VS_UpdateDataSize(data_index, data_size);
					
					fclose(inc);
				}
			}
		}
		
		if(strstr(line,".empty") != NULL){
			char* size_str = strstr(line,".empty");
			char* str = strstr(size_str,",");
			
			if(str == NULL){
				printf("Syntax Error: .empty directive must contain a comma followed by a value '.empty, <value>'\n");
				
				int err = VS_ErrorOccuredInIncludeEntry(line_count);
				
				if(err != -1){
					VS_PrintErrorFromIncludeEntry(err,line_count);
				}
				else{
					VS_PrintError(in,line,line_count);
				}
				
				fclose(pre);
				
				if(has_data_section){
					fclose(dat);
				}
				
				return -1;
			}
			else{
				char* num = str + 1;
				int result;
				
				if(num[0] == '0' && num[1] == 'x'){
					result = VS_HexDFA(num);
				}
				else{
					result = VS_IntDFA(num);
				}
				
				if(!result){
					printf("Syntax Error: .empty directive must have a valid immediate value\n");
					
					int err = VS_ErrorOccuredInIncludeEntry(line_count);
				
					if(err != -1){
						VS_PrintErrorFromIncludeEntry(err,line_count);
					}
					else{
						VS_PrintError(in,line,line_count);
					}
					
					fclose(pre);
					
					if(has_data_section){
						fclose(dat);
					}
				
					return -1;
				}
				
				size = (int)strtoul(num, NULL, 0);
				data_offset += size;
				VS_UpdateDataSize(data_index,size);
				
				unsigned long i;
				for(i = 0; i < size; i++)
					fwrite(&zero,1,1,dat);
			}
		}
		
		if(strstr(line,".type") != NULL){
			char* str = strstr(line,",");
			
			if(str == NULL){
				printf("Syntax Error: .type directive must contain a comma followed by a value '.type, <value>'\n");
				
				int err = VS_ErrorOccuredInIncludeEntry(line_count);
				
				if(err != -1){
					VS_PrintErrorFromIncludeEntry(err,line_count);
				}
				else{
					VS_PrintError(in,line,line_count);
				}
				
				fclose(pre);
				
				if(has_data_section){
					fclose(dat);
				}
				
				return -1;
			}
			
			if(strstr(line,"@function") != NULL){
				type = VS_SYM_FUNC;
			}
			else if(strstr(line,"@object") != NULL){
				type = VS_SYM_OBJ;
			}
			else{
				printf("Syntax Error: .type directive can only be set to @function or @object '.type <func>, @function' or '.type <object>, @object\n");
				
				int err = VS_ErrorOccuredInIncludeEntry(line_count);
				
				if(err != -1){
					VS_PrintErrorFromIncludeEntry(err,line_count);
				}
				else{
					VS_PrintError(in,line,line_count);
				}
				
				fclose(pre);
				
				if(has_data_section){
					fclose(dat);
				}
				
				return -1;
			}
		}
		
		if(strstr(line,".size") != NULL){
			char* size_str = strstr(line,".size");
			char* str = strstr(size_str,",");;
			
			if(str == NULL){
				printf("Syntax Error: .size directive must contain a comma followed by a value '.size, <value>'\n");
				
				int err = VS_ErrorOccuredInIncludeEntry(line_count);
				
				if(err != -1){
					VS_PrintErrorFromIncludeEntry(err,line_count);
				}
				else{
					VS_PrintError(in,line,line_count);
				}
				
				fclose(pre);
				
				if(has_data_section){
					fclose(dat);
				}
				
				return -1;
			}
			else{
				char* num = str + 1;
				int result;
				
				if(num[0] == '0' && num[1] == 'x'){
					result = VS_HexDFA(num);
				}
				else{
					result = VS_IntDFA(num);
				}
				
				if(!result){
					printf("Syntax Error: .size directive must have a valid immediate value\n");
					
					int err = VS_ErrorOccuredInIncludeEntry(line_count);
				
					if(err != -1){
						VS_PrintErrorFromIncludeEntry(err,line_count);
					}
					else{
						VS_PrintError(in,line,line_count);
					}
					
					fclose(pre);
					
					if(has_data_section){
						fclose(dat);
					}
				
					return -1;
				}
				
				size = (int)strtoul(num, NULL, 0);
			}
		}
		
		if(strstr(line,".org") != NULL){
			char* org_str = strstr(line,".org");

			char* num = org_str + strlen(".org");
			int result;
			
			if(num[0] == '0' && num[1] == 'x'){
				result = VS_HexDFA(num);
			}
			else{
				result = VS_IntDFA(num);
			}
			
			if(!result){
				printf("Syntax Error: .org directive must have a valid immediate value\n");
				
				int err = VS_ErrorOccuredInIncludeEntry(line_count);
				
				if(err != -1){
					VS_PrintErrorFromIncludeEntry(err,line_count);
				}
				else{
					VS_PrintError(in,line,line_count);
				}
				
				fclose(pre);
				
				if(has_data_section){
					fclose(dat);
				}
			
				return -1;
			}
			
			params->org = (int)strtoul(num, NULL, 0);
			params->org = (((params->org) + ((4)-1)) & ~((4)-1));
		}
		
		if(instr_index != -1 && strstr(line,":") == NULL && dir == -1){
			VS_OPCODE op;
			VS_GetOpcodeFromIndex(&op,instr_index);
			
			if(safe_load_delay){
				if(!strcmp(op.name,"lw") || !strcmp(op.name,"lh") || !strcmp(op.name,"lhu") || !strcmp(op.name,"lbu") | !strcmp(op.name,"lb") ||
				op.itype == VS_B_INSTRUCTION || op.itype == VS_COP_INSTRUCTION || !strcmp(op.name,"jr") || !strcmp(op.name,"jalr") || op.itype == VS_J_INSTRUCTION){
					instruction_count++;
				}
			}
			
			if(!strcmp(op.name,"li") && strstr(line,",") != NULL){
				char* immediate = strstr(line,",") + 1;
				int neg = 0;
				
				if(immediate[0] == '-'){
					immediate++;
					neg = 1;
				}
				
				if(!VS_HexDFA(immediate) && !VS_IntDFA(immediate)){
					instruction_count++;
				}
				else{
					if(neg){
						signed int imm = (signed int)strtol(immediate, NULL, 0);
						imm = -imm;
						
						if(imm <= -32678 || imm >= 32767){
							instruction_count++;
						}
					}
					else{
						unsigned long imm = (unsigned int)strtoul(immediate, NULL, 0);

						if(imm >= 65536){
							instruction_count++;
						}
					}
				}
			}
			
			if((op.itype == VS_I_INSTRUCTION && strcmp(op.name,"lui") != 0 && strcmp(op.name,"nop") != 0)){

				if(VS_GetCharCountInLine(line,',') == 2){
					char* fchar = line + VS_GetFirstOccurenceIndex(line,',') + 1;
					int findex = VS_GetFirstOccurenceIndex(fchar,',') + 1, neg = 0;
					memcpy(dest, fchar + findex, strlen(fchar) - findex);
					dest[strlen(fchar) - findex] = '\0';
					
					if(dest[0] == '-'){
						neg = 1;
					}
					
					if(neg){
						signed int imm = (signed int)strtol(dest, NULL, 0);
						imm = -imm;
						
						if(imm <= -32678 || imm >= 32767){
							instruction_count += 2;
						}
					}
					else{
						unsigned long integer = (unsigned long)strtoul(dest, NULL, 0);

						if(integer >= 65536){
							instruction_count += 2;
						}
					}
				}
				else{
					int findex = VS_GetFirstOccurenceIndex(line,',') + 1, neg = 0;
					memcpy(dest, line + findex, strlen(line) - findex);
					dest[strlen(line) - findex] = '\0';
					
					if(dest[0] == '-'){
						neg = 1;
					}
					
					if(neg){
						signed int imm = (signed int)strtol(dest, NULL, 0);
						imm = -imm;
						
						if(imm <= -32678 || imm >= 32767){
							instruction_count += 2;
						}
					}
					else{
						unsigned long integer = (unsigned long)strtoul(dest, NULL, 0);

						if(integer >= 65536){
							instruction_count += 2;
						}
					}
				}
			}
			
			if(op.itype == VS_B_INSTRUCTION && op.optype == VS_PSEUDO_OPCODE && strcmp(op.name,"beqz") != 0 && strcmp(op.name,"bnez") != 0 && strcmp(op.name,"b") != 0){
				instruction_count++;
			}
			
			if(!strcmp(op.name,"la")  || !strcmp(op.name,"mul") || !strcmp(op.name,"umul")){
				instruction_count++;
			}
			
			if(!strcmp(op.name,"li.s")){
				instruction_count += 3;
			}
			
			instruction_count++;
		}
		else if(instr_index == -1 && strstr(line,":") == NULL && dir == -1 && strlen(line) > 1){
			char* string = strstr(line,"jal");
			
			if(string != NULL){
				instruction_count++;
			}
			else if(line[0] == 'b' || line[0] == 'j'){
				instruction_count++;
			}
		}
		
		char* label = strstr(line,":");
		
		if(label != NULL){
			first_occurence = VS_GetFirstOccurenceIndex(line,':');
			memcpy(dest,line,first_occurence);
			dest[first_occurence] = '\0';
			
			if(VS_FindSymbol(dest)){
				printf("Error: The label '%s' is already defined!\n",dest);
				
				int err = VS_ErrorOccuredInIncludeEntry(line_count);
				
				if(err != -1){
					VS_PrintErrorFromIncludeEntry(err,line_count);
				}
				else{
					VS_PrintError(in,line,line_count);
				}
				
				fclose(pre);
				
				if(has_data_section){
					fclose(dat);
				}
				
				return -1;
			}
			else{
				if(type == VS_SYM_FUNC){
					index = VS_GetIndexOfLastFuncSymbol();
					num_of_instructions = instruction_count - prev_instruction_count;
					prev_instruction_count = instruction_count;
					VS_UpdateNumberOfInstructions(index,num_of_instructions);
					VS_AddSymbol(dest,instruction_count,size,type,scope);
				}
				else{
					data_index = VS_AddDataSymbol(dest,data_offset,instruction_count,size,type,scope);
					data_size = 0;
				}
				
				scope = VS_SCOPE_LOCAL;
				size = 0;
			}
		}
		
		if(strstr(line,".word") != NULL || strstr(line,".dw") != NULL ){
			if(strstr(line,",")){
				char* arr;
				
				if(strstr(line,".word") != NULL){
					arr = strstr(line,".word") + 5;
				}
				else{
					arr = strstr(line,".db") + 3;
				}
				
				int num_of_commas = VS_GetNumberOfCommas(arr) + 1, len = strlen(arr);
				
				int i, j, count = 0, old_count = 0;
				for(i = 0; i < num_of_commas; i++){
					
					old_count = count;
					
					for(j = old_count; j < len; j++, count++){
						if(arr[j] == ','){
							count++;
							break;
						}
					}
					
				//	printf("oldcount = %d\n",old_count);
				//	printf("count = %d\n",count);
					
					memcpy(dest, arr + old_count, count - old_count);
					dest[count-old_count] = '\0';
					
					data_offset += 4;
					data_size += 4;
				
					int num = (unsigned long)strtoul(dest, NULL, 0);
					//printf("dest = %s\n",dest);
					fwrite(&num,4,1,dat);
				}
			}
			else{
				data_offset += 4;
				data_size += 4;
				
				int num;
				if(strstr(line,".word") != NULL){
					num = (unsigned long)strtoul(strstr(line,".word") + 5, NULL, 0);
				}
				else{
					(unsigned long)strtoul(strstr(line,".dw") + 3, NULL, 0);
				}
				
				fwrite(&num,4,1,dat);
			}

			VS_UpdateDataSize(data_index, data_size);
		}
		
		if(strstr(line,".half") != NULL){
			if(strstr(line,",")){
				char* arr;
				if(strstr(line,".half") != NULL){
					arr = strstr(line,".half") + 5;
				}
				else{
					arr = strstr(line,".dh") + 3;
				}
				
				int num_of_commas = VS_GetNumberOfCommas(arr) + 1, len = strlen(arr);
				
				int i, j, count = 0, old_count = 0;
				for(i = 0; i < num_of_commas; i++){
					
					old_count = count;
					
					for(j = old_count; j < len; j++, count++){
						if(arr[j] == ','){
							count++;
							break;
						}
					}
					
				//	printf("oldcount = %d\n",old_count);
				//	printf("count = %d\n",count);
					
					memcpy(dest, arr + old_count, count - old_count);
					dest[count-old_count] = '\0';
					
					data_offset += 2;
					data_size += 2;
				
					int num = (unsigned short)strtoul(dest, NULL, 0);
					num &= 0xFFFF;
					//printf("dest = %s\n",dest);
					fwrite(&num,2,1,dat);
				}
			}
			else{
				data_offset += 2;
				data_size += 2;
				
				
				int num;
				if(strstr(line,".half") != NULL){
					num = (unsigned short)strtoul(strstr(line,".half") + 5, NULL, 0);
				}
				else{
					num = (unsigned short)strtoul(strstr(line,".dh") + 3, NULL, 0);
				}
				
				num &= 0xFFFF;
				fwrite(&num,2,1,dat);
			}
			
			VS_UpdateDataSize(data_index, data_size);
		}
		
		if(strstr(line,".byte") != NULL){

			if(strstr(line,",")){
				char* arr;
				if(strstr(line,".byte") != NULL){
					arr = strstr(line,".byte") + 5;
				}
				else{
					arr = strstr(line,".db") + 3;
				}
				
				int num_of_commas = VS_GetNumberOfCommas(arr) + 1, len = strlen(arr);
				
				int i, j, count = 0, old_count = 0;
				for(i = 0; i < num_of_commas; i++){
					
					old_count = count;
					
					for(j = old_count; j < len; j++, count++){
						if(arr[j] == ','){
							count++;
							break;
						}
					}
					
				//	printf("oldcount = %d\n",old_count);
				//	printf("count = %d\n",count);
					
					memcpy(dest, arr + old_count, count - old_count);
					dest[count-old_count] = '\0';
					
					data_offset++;
					data_size++;
					
					//printf("data_size = %d\n",data_size);
				
					int num = (unsigned char)strtoul(dest, NULL, 0);
					//printf("dest = %s\n",dest);
					num &= 0xFF;
					fwrite(&num,1,1,dat);
				}
			}
			else{
				data_offset++;
				data_size++;
				
				int num;
				if(strstr(line,".byte") != NULL){
					num = (unsigned char)strtoul(strstr(line,".byte") + 5, NULL, 0);
				}
				else{
					num = (unsigned char)strtoul(strstr(line,".db") + 3, NULL, 0);
				}
				
				num &= 0xFF;
				fwrite(&num,1,1,dat);
			}

			VS_UpdateDataSize(data_index, data_size);
		}
		
		if(strstr(line,".float") != NULL){
			if(strstr(line,",")){
				char* arr = strstr(line,".float") + 6;
				
				int num_of_commas = VS_GetNumberOfCommas(arr) + 1, len = strlen(arr);
				
				int i, j, count = 0, old_count = 0;
				for(i = 0; i < num_of_commas; i++){
					
					old_count = count;
					
					for(j = old_count; j < len; j++, count++){
						if(arr[j] == ','){
							count++;
							break;
						}
					}
					
				//	printf("oldcount = %d\n",old_count);
				//	printf("count = %d\n",count);
					
					memcpy(dest, arr + old_count, count - old_count);
					dest[count-old_count] = '\0';
					
					data_offset += 4;
					data_size += 4;

					float fvalue = (float)atof(dest);
					int num;
					memcpy(&num,&fvalue,sizeof(fvalue));
					//printf("dest = %s\n",dest);
					fwrite(&num,4,1,dat);
				}
			}
			else{
				data_offset += 4;
				data_size += 4;
				
				float fvalue = (float)atof(strstr(line,".float") + 6);
				int num;
				memcpy(&num,&fvalue,sizeof(fvalue));
				fwrite(&num,4,1,dat);
			}

			VS_UpdateDataSize(data_index, data_size);
		}
		
		char* ascii = strstr(line,".ascii");
		
		if(ascii != NULL){
			char* string = ascii + strlen(".ascii");
			
			if(string[0] != '\"'){
				printf("Syntax Error: The .ascii directive must begin with doubles quotes .ascii \"<example>\"\n");
				
				int err = VS_ErrorOccuredInIncludeEntry(line_count);
				
				if(err != -1){
					VS_PrintErrorFromIncludeEntry(err,line_count);
				}
				else{
					VS_PrintError(in,line,line_count);
				}
				
				fclose(pre);
				
				if(has_data_section){
					fclose(dat);
				}
				
				return -1;
			}
			
			string++;
			
			if(strstr(string,"\"") != NULL){
				int end = VS_GetFirstOccurenceIndex(string,'\"');
				int len = 0;

				memset(dest,0x0,VS_MAX_LINE);
				memcpy(dest,string,end);
				
				int i;
				for(i = 0; i < end; i++){
					if(string[i] == '\\' && i + 1 < VS_MAX_LINE){
						if(string[i+1] == 'n'){
							dest[len++] = '\n';
							i++;
							continue;
						}
					}
					
					dest[len++] = string[i];
				}
				
				dest[len] = '\0';
				
				//printf("%s\n",dest);
				
				data_offset += len;
				data_size += len;
				VS_UpdateDataSize(data_index, data_size);
				
				fwrite(dest,1,len,dat);
				
				data_offset++;
				fwrite(&zero,1,1,dat);
			}
			else{
				printf("Syntax Error: The .ascii directive must end with doubles quotes .ascii \"<example>\"\n");
				
				int err = VS_ErrorOccuredInIncludeEntry(line_count);
				
				if(err != -1){
					VS_PrintErrorFromIncludeEntry(err,line_count);
				}
				else{
					VS_PrintError(in,line,line_count);
				}
				
				fclose(pre);
				
				if(has_data_section){
					fclose(dat);
				}
				
				return -1;
			}
		}
	}
	
	index = VS_GetIndexOfLastFuncSymbol();
	num_of_instructions = instruction_count - prev_instruction_count;
	VS_UpdateNumberOfInstructions(index,num_of_instructions);

	if(params->oexe){
		VS_ExpandAddrForAllDataSymbols((0x80010000) + (instruction_count << 2));
	}
	
	fclose(pre);
	
	if(has_data_section)
		fclose(dat);

	return data_size;
}