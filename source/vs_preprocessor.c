#include <vs_preprocessor.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <vs_opcode.h>
#include <vs_symtable.h>
#include <vs_parser.h>
#include <vs_exp_parser.h>

/********************************************
*   VideoStation Assembler
*
*   Copyright (c) 2025 Ryandracus Chapman
*
*   File: vs_parser.c
*   Date: 4/23/2025
*   Version: 1.1
*   Updated: 6/11/2025
*   Author: Ryandracus Chapman
*
********************************************/

extern VS_OPCODE vs_opcode_table[];
extern VS_SYM_TABLE sym_table;

int nowarnings;

VS_MACRO_TABLE macro_table;

char* reserved_words[] = {
	"$zero","$at","$sp","$ra","$gp","$k1","$k2","$t0","$t1","$t2","$t3","$t4","$t5","$t6","$t7","$t8","$t9","$v0",
	"$v1","$a0","$a1","$a2","$a3","$s0","$s1","$s2", "$s3","$s4","$s5","$s6","$s7","$f0","$f1","$f2","$f3","$f4","$f5",
	"$f6","$f7","$f8","$f9","$f10","$f11","$f12","$f13","$f14","$f15","$f16","$f17","$f18","$f19","$f20","$f21","$f22",
	"$f23","$f24","$f25","$f26","$f27","$f28","$f29","$f30","$f31","r0","r1","r2","r3","r4","r5","r6","r7","r8","r9",
	"r10","r11","r12","r13","r14","r15","r16","r18","r19","r20","r21","r22","r23","r24","r25","r26","r27","r28","r29",
	"r30","r31","zero","at","sp","ra","gp","k1","k2","t0","t1","t2","t3","t4","t5","t6","t7","t8","t9","v0","v1","a0",
	"a1","a2","a3","s0","s1","s2","s3","s4","s5","s6","s7","f0","f1","f2","f3","f4","f5","f6","f7","f8","f9","f10","f11",
	"f12","f13","f14","f15","f16","f17","f18","f19","f20","f21","f22","f23","f24","f25","f26","f27","f28","f29","f30",
	"f31",".text",".data",".ktext",".globl",".global", ".word",".byte",".half",".dw",".dh",".db",".arch",".float",".syntax",
	".undefsym",".type",".section",".bss",".include", ".align", ".incbin",".incasm",".ascii",".empty",".safeloaddelay",
	".safeloadoff",".org","equ","equr","set",".inject",
};

char* directives[] = {
	".text",".data",".ktext",".globl",".global", ".word",".byte",".half",".dw",".dh",".db",".arch",".float",".syntax",".undefsym",
	".type",".section",".bss",".include", ".align", ".incbin",".incasm",".ascii",".empty",".safeloaddelay",".safeloadoff",".org",
	".inject",
};

int VS_StringIsReservedWord(char* name){
	int i, num_reserved_words = sizeof(reserved_words) / sizeof(reserved_words[0]), size = VS_GetNumberOfInstructions();
	for(i = 0; i < num_reserved_words; i++){
		if(!strcmp(name,reserved_words[i])){
			return 1;
		}
	}
	
	for(i = 0; i < size; i++){
		if(!strcmp(name,vs_opcode_table[i].name)){
			return 1;
		}
	}
	
	return 0;
}

int VS_MacroIsReservedWord(char* name, char* value){
	int i, num_reserved_words = sizeof(reserved_words) / sizeof(reserved_words[0]), size = VS_GetNumberOfInstructions();
	for(i = 0; i < num_reserved_words; i++){
		if(!strcmp(name,reserved_words[i])){
			return 1;
		}
		
		if(!strcmp(value,reserved_words[i])){
			return 1;
		}
	}
	
	for(i = 0; i < size; i++){
		if(!strcmp(name,vs_opcode_table[i].name)){
			return 1;
		}
		
		if(!strcmp(value,vs_opcode_table[i].name)){
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
	else{
		if(!nowarnings){
			printf("Warning: Macro not added. Macro name or value is a reserved word.\n");
			printf("Attempted macro addition name and value pair (%s,%s)\n\n",name,value);
		}
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
	else{
		if(!nowarnings){
			printf("Warning: Macro not added. Macro name is a reserved word or the value is not a valid register value.\n");
			printf("Attempted macro addition name and register pair (%s,%s)\n\n",name,value);
		}
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
	else{
		if(!nowarnings){
			printf("Warning: Macro not added. Macro name or value is a reserved word.\n");
			printf("Attempted macro addition name and value pair (%s,%s)\n\n",name,value);
		}
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
	int i, size = 0, len = strlen(src);
	for(i = 0; i < len; i++){
		if(src[i] != ' ' && src[i] != '\r' && src[i] != '\n' && src[i] != '\t' && src[i] != 13 && src[i] != '\v'){
			dest[size++] = src[i];
		}
	}
	
	dest[size] = '\0';
}

void VS_TrimCommentFromLine(char* dest){
	char cpy[VS_MAX_LINE+1], c;
	
	strcpy(cpy,dest);
	
	int i, size = 0, len = strlen(dest);
	for(i = 0; i < len; i++){
		c = cpy[i];
		if(c == '#' || c == ';'){
			break;
		}
			
		dest[size++] = c;
	}
	
	dest[size] = '\0';
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

int VS_LineContainsInstruction(char* line, VS_ASM_PARAMS* params){
	int i, j, ind = 0, size = VS_GetNumberOfInstructions();
	char instruction[12], reg[3], bypass = 0;
	
	for(i = 0; i < 10; i++){
		if(params->syntax == VS_GNU_SYNTAX){
			if(line[i] == '$' || line[i] == '\n' || line[i] == ','){
				break;
			}
		}
		else{
			memcpy(reg,line+i,2);
			reg[2] = '\0';
			
			if(line[0] == 'v' && params->architecture == VS_MIPS_PSP_ARCH){
				memcpy(instruction,line,10);
				instruction[11] = '\0';
				ind = 11;
				break;
			}
			
			if(!strncasecmp(line,"la",2)){
				instruction[0] = 'l'; instruction[1] = 'a';
				ind = 1;
				break;
			}
			
			if((line[0] == 's' || line[0] == 'l') && !strncasecmp(line+1,"wra",3) && !isdigit(line[4])){
				instruction[0] = tolower(line[0]); instruction[1] = 'w';
				ind = 1;
				break;
			}
			
			if(tolower(line[i]) == 'r' &&  tolower(line[i+1]) == 'a'){
				VS_LowercaseAndCopyLine(instruction,line,3);
				ind = 3;
				bypass = 1;
				break;
			}

			if((VS_LineContainsRegister(reg)) || line[i] == '\n' || line[i] == '$' || line[i] == ','){
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
		|| instruction[0] == 'b' || instruction[0] == 'j' || params->architecture == VS_MIPS_PSP_ARCH || bypass){
			
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

void VS_PasteSymbolNames(int has_sym, char* line, char* dest, char* name){
	VS_SYM sym;
	int size, itr = 0;
	
	memcpy(dest, line, VS_MAX_LINE);

	while(has_sym != -1){
		itr++;
		
		sym = VS_GetSymbolFromIndex(has_sym);
		
		char* sym_name = sym.name;
		
		//printf("sym_name = %s\n",sym_name);
		
		size = (int)(strstr(dest,sym_name) - dest);
		
		strncpy(name, dest, size);
		name[size] = '\0';

		//printf("name = %s\n",name);
		//printf("value = %ld\n",sym.addr);
		
		char* after = strstr(dest,sym_name) + strlen(sym_name);
		
		strncpy(dest, after, strlen(after));
		dest[strlen(after)] = '\0';

		//printf("after = %s\n",dest);

		sprintf(name,"%s0x%lx%s",name,sym.addr,dest);
		
		//printf("name = %s\n",name);
		
		VS_TrimStrictLine(dest,name);
	
		has_sym = VS_LineContainsSymbol(dest);
		
		if(itr >= 1000){
			break;
		}
	}
	
	memcpy(line, dest, VS_MAX_LINE);
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
	char imm_str[256];
	unsigned char zero = 0;
	unsigned char binarr[500];
	VS_SYM sym;
	VS_SYM_TYPE type;
	VS_SYM_SCOPE scope;
	
	macro_table.size = 0;
	safe_load_delay = 0;
	line_count = 0;
	has_data_section = 0;
	line_start = 0;
	line_end = 0;
	prev_instruction_count = 0;
	dat = NULL;
	nowarnings = params->nowarnings;
	
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

			char* include_dir = strstr(name,".include");
		
			//printf("include_dir = %s\n",include_dir);

			if(include_dir != NULL){
				char* path =  include_dir + strlen(".include");
			//	printf("path = %s\n",path);
				
				if(!VS_VerifyPathSyntax(path,".include",line_count)){
					return -1;
				}
				
				if(path[0] == '\"'){
					end_quotes = strstr(path + 1,"\"");
				}
				else{
					end_quotes = strstr(path + 1,"\'");
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
						VS_TrimStrictLine(trim,dest);
						
						//printf("Line = %s\n",trim);
						
						if(strstr(trim, ".include") != NULL){
							char* incdir = strstr(trim,".include") + strlen(".include");
							
							int errorcode = VS_GrabMacrosFromIncludeEntry(incdir,incindex,line_count);
							
							if(errorcode == -1){
								return -1;
							}
						}
			
						line_end++;
						
						if(strstr(dest,"#") != NULL || strstr(dest,";") != NULL){
							VS_TrimCommentFromLine(dest);
						}
						
						VS_PrintAndStoreTrimmedLine(preprocess,dest,trim);
						
					}
					
					VS_UpdateIncludeEntry(incindex,line_start,line_end);
					
					fclose(inc);
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
		VS_TrimStrictLine(name,line);
		
		char* include_dir = strstr(line,".include");
		char* equ = strstr(name,"equr");
		char* tok = strstr(name,"equ");
		
		if(include_dir == NULL){
			macro_index = VS_LineContainsMacro(line);
			
			if(equ != NULL && strstr(line,"equal") == NULL){
				char* value = equ + strlen("equr");
				memset(dest,'\0',VS_MAX_LINE);
				memcpy(dest,name,(int)(equ - name));
				VS_AddRegMacro(dest,value);
				VS_SortMacroTable();
				fprintf(pre,"\n");
			}
			else if(tok != NULL && strstr(line,"equal") == NULL){
				char* value = tok + strlen("equ");
				memset(dest,'\0',VS_MAX_LINE);
				memcpy(dest,name,(int)(tok - name));
				VS_AddMacro(dest,value);
				VS_SortMacroTable();
				fprintf(pre,"\n");
			}
			else if(strstr(line,"set") != NULL && VS_LineContainsInstruction(line,params) == -1 && VS_LineContainsDirective(line) == -1 && strstr(line,":") == NULL){
				char* token = strstr(line,"set");
				char* value = token + strlen("set");
				memset(dest,'\0',VS_MAX_LINE);
				memcpy(dest,line,(int)(token - line));
				VS_AddSetMacro(dest,value);
				VS_SortMacroTable();
				fprintf(pre,"\n");
			}
			else if(VS_LineContainsDirective(line) != -1 || strstr(line,":") != NULL){
				fprintf(pre,"%s",line);
			}
			else{			
				if(macro_index != -1){
					memcpy(dest, line, VS_MAX_LINE);
					
					unsigned long itr = 0;
					
					while(macro_index != -1){
						itr++;
						
						char* macro_name = macro_table.macro[macro_index].name;
						
					//	printf("macro_name = %s\n",macro_name);
						
						size = (int)(strstr(dest,macro_name) - dest);
						
						strncpy(name, dest, size);
						name[size] = '\0';

					//	printf("name = %s\n",name);
					//	printf("value = %s\n",macro_table.macro[macro_index].value);
						
						char* after = strstr(dest,macro_name) + strlen(macro_name);
						
						strncpy(dest, after, strlen(after));
						dest[strlen(after)] = '\0';
				
					//	printf("after = %s\n",dest);
						
						strcat(name,macro_table.macro[macro_index].value);
						strcat(name,dest);
						
						VS_TrimStrictLine(dest,name);
					
						macro_index = VS_LineContainsMacro(dest);
						
						if(macro_index == -1){
							fprintf(pre,"%s",dest);
						}
						
						if(itr >= 1000){
							break;
						}
					}
					
					fprintf(pre,"\n");
				}
				else{
					VS_PrintStrictTrimmedLine(pre,line);
					fprintf(pre,"\n");
				}
			}
		}
		else{
			fprintf(pre,"\n");
		}
	}
	
	memset(dest,0x0,VS_MAX_LINE);
	memset(line,0x0,VS_MAX_LINE);
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
		
		if(strstr(line,".undefsym")){
			params->undefsym = 1;
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
			else if(strstr(directive,"ps1") != NULL){
				params->architecture = VS_MIPS_PSX_ARCH;
			}
			else if(strstr(directive,"psp") != NULL){
				params->architecture = VS_MIPS_PSP_ARCH;
			}
			else{
				printf("Warning: Unrecognized architecture. Must be set to mips1, mips2, psx, ps1, or psp\n");
				
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
		
		if(strstr(line,".inject") != NULL){
			char* path =  strstr(line,".inject") + strlen(".inject");
			char* end_quotes;
			
			if(!VS_VerifyPathSyntax(path,".inject",line_count)){
				fclose(pre);
	
				if(has_data_section)
					fclose(dat);
				
				return -1;
			}
			
			if(path[0] == '\"'){
				end_quotes = strstr(path + 1,"\"");
			}
			else{
				end_quotes = strstr(path + 1,"\'");
			}
			
			size = end_quotes - path;
				
			if(size >= 1){
				size = size - 1;
			}
			
			strncpy(dest, path + 1, size);
			
			dest[size] = '\0';
			
			inc = fopen(dest,"rb");
			
			if(inc == NULL){
				printf("Warning: The file path of the inject directive could not be found!\n");
				
				int err = VS_ErrorOccuredInIncludeEntry(line_count);
			
				if(err != -1){
					VS_PrintErrorFromIncludeEntry(err,line_count);
				}
				else{
					printf("Line %d: %s\n",line_count,path);
				}
			}
			else{
				fseek(inc,0x0,SEEK_END);
				size = ftell(inc);
				fseek(inc,0x0,SEEK_SET);
				
				if(size % 4){
					printf("Warning: The contents of %s have not been injected into the program. File size must be a multiple of four.\n",dest);
				}
				else{
					instruction_count += (size >> 2);
				}
				
				fclose(inc);
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
			
			if(op.itype == VS_ADDR_INSTRUCTION && strcmp(op.name,"la") != 0 && strcmp(op.name,"li") != 0  && strcmp(op.name,"cache") != 0){
				if(strstr(line,"(") == NULL){
					instruction_count++;
				}
			}
			
			if(safe_load_delay){
				if(((!strcmp(op.name,"lw") || !strcmp(op.name,"lh") || !strcmp(op.name,"lhu") || !strcmp(op.name,"lbu") | !strcmp(op.name,"lb")) 
					&& params->architecture != VS_MIPS_II_ARCH && params->architecture != VS_MIPS_PSP_ARCH) || op.itype == VS_B_INSTRUCTION || 
				!strcmp(op.name,"jr") || !strcmp(op.name,"jalr") || op.itype == VS_J_INSTRUCTION){
					instruction_count++;
				}
			}
			
			if(!strcmp(op.name,"li") && strstr(line,",") != NULL){
				char* immediate = strstr(line,",") + 1;
				int neg = 0;
				int is_valid_imm;
				
				if(immediate[0] == '-'){
					immediate++;
					neg = 1;
				}
					
				if(VS_LineContainsOperator(immediate)){
					long expr;
					
					VS_InitExprParser();

					expr = VS_IsValidExpression(immediate, params->syntax);
					
					if(expr){
						expr = VS_EvaluateExpr(immediate, params->syntax);
						sprintf(immediate,"%ld",expr);
					}
				}
		
				is_valid_imm = VS_IsValidImmediate(immediate, params);

				if(is_valid_imm != 1 && is_valid_imm != 2){
					instruction_count++;
				}
				else{
					if(neg){
						signed int imm;
						
						if(params->syntax == VS_ASMPSX_SYNTAX && immediate[0] == '$'){
							memset(imm_str,'\0',256);
							imm_str[0] = '-'; imm_str[1] = '0'; imm_str[2] = 'x';
							imm = (signed int)strtol(strcat(imm_str,immediate+1), NULL, 0);
						}
						else{
							imm = (signed int)strtol(immediate, NULL, 0);
						}
						
						imm = -imm;
						
						if(imm <= -32678 || imm >= 32767){
							instruction_count++;
						}
					}
					else{
						unsigned long imm;
						
						if(params->syntax == VS_ASMPSX_SYNTAX && immediate[0] == '$'){
							memset(imm_str,'\0',256);
							imm_str[0] = '0'; imm_str[1] = 'x';
							imm = (unsigned int)strtoul(strcat(imm_str,immediate+1), NULL, 0);
						}
						else{
							imm = (unsigned int)strtoul(immediate, NULL, 0);
						}
						

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
						signed int imm;
						
						if(params->syntax == VS_ASMPSX_SYNTAX && dest[1] == '$'){
							memset(imm_str,'\0',256);
							imm_str[0] = '0'; imm_str[1] = 'x';
							imm = (signed int)strtoul(strcat(imm_str,dest+2), NULL, 0);
						}
						else{
							imm = (signed int)strtol(dest, NULL, 0);
						}
						
						imm = -imm;
						
						if(imm <= -32678 || imm >= 32767){
							instruction_count += 2;
						}
					}
					else{
						unsigned long integer;
						
						if(params->syntax == VS_ASMPSX_SYNTAX && dest[0] == '$'){
							memset(imm_str,'\0',256);
							imm_str[0] = '0'; imm_str[1] = 'x';
							integer = (unsigned int)strtoul(strcat(imm_str,dest+1), NULL, 0);
						}
						else{
							integer = (unsigned long)strtoul(dest, NULL, 0);
						}

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
						signed int imm;
						
						if(params->syntax == VS_ASMPSX_SYNTAX && dest[1] == '$'){
							memset(imm_str,'\0',256);
							imm_str[0] = '0'; imm_str[1] = 'x';
							imm = (signed int)strtoul(strcat(imm_str,dest+2), NULL, 0);
						}
						else{
							imm = (signed int)strtol(dest, NULL, 0);
						}
						
						imm = -imm;
						
						if(imm <= -32678 || imm >= 32767){
							instruction_count += 2;
						}
					}
					else{
						unsigned long integer;
						
						if(params->syntax == VS_ASMPSX_SYNTAX && dest[0] == '$'){
							memset(imm_str,'\0',256);
							imm_str[0] = '0'; imm_str[1] = 'x';
							integer = (unsigned int)strtoul(strcat(imm_str,dest+1), NULL, 0);
						}
						else{
							integer = (unsigned long)strtoul(dest, NULL, 0);
						}

						if(integer >= 65536){
							instruction_count += 2;
						}
					}
				}
			}
			
			if(op.itype == VS_B_INSTRUCTION && op.optype == VS_PSEUDO_OPCODE && strcmp(op.name,"beqz") != 0 && strcmp(op.name,"beqzl") != 0 
			&& strcmp(op.name,"bnez") != 0 && strcmp(op.name,"bnezl") != 0 && strcmp(op.name,"b") != 0){
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
					scope = VS_SCOPE_LOCAL;
					size = 0;
				}
			}
		}
	}
	
	memset(dest,0x0,VS_MAX_LINE);
	memset(line,0x0,VS_MAX_LINE);
	fseek(pre,0x0,SEEK_SET);
	line_count = 0;
	
	if(params->oexe){
		data_offset = params->org + (instruction_count << 2);
	}
	
	while(VS_ReadLine(pre, line)){
		line_count++;
		
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
		
		char* label = strstr(line,":");
		
		if(label != NULL){
			first_occurence = VS_GetFirstOccurenceIndex(line,':');
			memcpy(dest,line,first_occurence);
			dest[first_occurence] = '\0';
			
			sym = VS_GetSymbol(dest);
			
			if(VS_FindSymbol(dest) && sym.type == VS_SYM_OBJ){
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
				if(type == VS_SYM_OBJ){
					data_index = VS_AddDataSymbol(dest,data_offset,instruction_count,size,type,scope);
					data_size = 0;
					scope = VS_SCOPE_LOCAL;
				}
			}
		}
		
		if(strstr(line,".word") != NULL || strstr(line,".dw") != NULL){
			int has_sym = VS_LineContainsSymbol(line);
			
			if(has_sym != -1){
				VS_PasteSymbolNames(has_sym,line,dest,name);
			}
			
			if(strstr(line,",")){
				char* arr;
				if(strstr(line,".word") != NULL){
					arr = strstr(line,".word") + 5;
				}
				else{
					arr = strstr(line,".dw") + 3;
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
					
					if(count-old_count-1 > 0){
						if(dest[count-old_count-1] == ','){
							dest[count-old_count-1] = '\0';
						}
						else{
							dest[count-old_count] = '\0';
						}
					}
					else{
						dest[count-old_count] = '\0';
					}
					
					int neg = 0;
					if(dest[0] == '-'){
						neg = 1;
					}
					
					if(VS_LineContainsOperator(dest+neg)){
						long expr;
						
						VS_InitExprParser();

						expr = VS_IsValidExpression(dest, params->syntax);
						
						if(expr){
							expr = VS_EvaluateExpr(dest, params->syntax);
							sprintf(dest,"%ld",expr);
						}
					}
					
					data_offset += 4;
					data_size += 4;
				
					int num = VS_ParseImmediateValue(dest, params);
				
					//printf("dest = %s\n",dest);
					fwrite(&num,4,1,dat);
				}
			}
			else{
				char* arr;
				if(strstr(line,".word") != NULL){
					arr = strstr(line,".word") + 5;
				}
				else{
					arr = strstr(line,".dw") + 3;
				}
				
				int neg = 0;
				if(arr[0] == '-'){
					neg = 1;
				}
				
				if(VS_LineContainsOperator(arr+neg)){
					long expr;
					
					VS_InitExprParser();

					expr = VS_IsValidExpression(arr, params->syntax);
					
					if(expr){
						expr = VS_EvaluateExpr(arr, params->syntax);
						sprintf(arr,"%ld",expr);
					}
				}
				
				data_offset += 4;
				data_size += 4;
				
				int num = VS_ParseImmediateValue(arr, params);
				
				fwrite(&num,4,1,dat);
			}

			VS_UpdateDataSize(data_index, data_size);
		}
		
		if(strstr(line,".half") != NULL || strstr(line,".dh") != NULL){
			int has_sym = VS_LineContainsSymbol(line);
			
			if(has_sym != -1){
				VS_PasteSymbolNames(has_sym,line,dest,name);
			}
			
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
					
					int neg = 0;
					if(dest[0] == '-'){
						neg = 1;
					}
					
					if(VS_LineContainsOperator(dest+neg)){
						long expr;
						
						VS_InitExprParser();

						expr = VS_IsValidExpression(dest, params->syntax);
						
						if(expr){
							expr = VS_EvaluateExpr(dest, params->syntax);
							sprintf(dest,"%ld",expr);
						}
					}
					
					int num = VS_ParseImmediateValue(dest, params);
					
					num &= 0xFFFF;
					//printf("dest = %s\n",dest);
					fwrite(&num,2,1,dat);
				}
			}
			else{
				char* arr;
				if(strstr(line,".half") != NULL){
					arr = strstr(line,".half") + 5;
				}
				else{
					arr = strstr(line,".dh") + 3;
				}
				
				int neg = 0;
				if(arr[0] == '-'){
					neg = 1;
				}
				
				if(VS_LineContainsOperator(arr+neg)){
					long expr;
					
					VS_InitExprParser();

					expr = VS_IsValidExpression(arr, params->syntax);
					
					if(expr){
						expr = VS_EvaluateExpr(arr, params->syntax);
						sprintf(arr,"%ld",expr);
					}
				}
				
				data_offset += 2;
				data_size += 2;
				
				int num = VS_ParseImmediateValue(arr, params);
				
				num &= 0xFFFF;
				fwrite(&num,2,1,dat);
			}
			
			VS_UpdateDataSize(data_index, data_size);
		}
		
		if(strstr(line,".byte") != NULL || strstr(line,".db") != NULL){
			int has_sym = VS_LineContainsSymbol(line);
			
			if(has_sym != -1){
				VS_PasteSymbolNames(has_sym,line,dest,name);
			}
			
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
					
					int neg = 0;
					if(dest[0] == '-'){
						neg = 1;
					}
					
					if(VS_LineContainsOperator(dest+neg)){
						long expr;
						
						VS_InitExprParser();

						expr = VS_IsValidExpression(dest, params->syntax);
						
						if(expr){
							expr = VS_EvaluateExpr(dest, params->syntax);
							sprintf(dest,"%ld",expr);
						}
					}
					
					data_offset++;
					data_size++;
					
					//printf("data_size = %d\n",data_size);
					int num = VS_ParseImmediateValue(dest, params);
					
					//printf("dest = %s\n",dest);
					num &= 0xFF;
					fwrite(&num,1,1,dat);
				}
			}
			else{
				char* arr;
				if(strstr(line,".byte") != NULL){
					arr = strstr(line,".byte") + 5;
				}
				else{
					arr = strstr(line,".db") + 3;
				}
				
				data_offset++;
				data_size++;
				
				int neg = 0;
				if(arr[0] == '-'){
					neg = 1;
				}
				
				if(VS_LineContainsOperator(arr+neg)){
					long expr;
					
					VS_InitExprParser();

					expr = VS_IsValidExpression(arr, params->syntax);
					
					if(expr){
						expr = VS_EvaluateExpr(arr, params->syntax);
						sprintf(arr,"%ld",expr);
					}
				}
				
				int num = VS_ParseImmediateValue(arr, params);
				num &= 0xFF;
				fwrite(&num,1,1,dat);
			}

			VS_UpdateDataSize(data_index, data_size);
		}
		
		if(strstr(line,".float") != NULL){
			int has_sym = VS_LineContainsSymbol(line);
			
			if(has_sym != -1){
				VS_PasteSymbolNames(has_sym,line,dest,name);
			}
			
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
						
						if(string[i+1] == 'r'){
							dest[len++] = '\r';
							i++;
							continue;
						}
						
						if(string[i+1] == 't'){
							dest[len++] = '\t';
							i++;
							continue;
						}
						
						if(string[i+1] == 'v'){
							dest[len++] = '\v';
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
	
	fclose(pre);
	
	if(has_data_section)
		fclose(dat);

	return data_size;
}