#include <vs_utils.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <vs_preprocessor.h>

/********************************************
*   VideoStation Assembler
*
*   Copyright (c) 2025 Ryandracus Chapman
*
*   File: vs_utils.c
*   Date: 5/21/2025
*   Version: 1.1
*   Updated: 6/10/2025
*   Author: Ryandracus Chapman
*
********************************************/

FileIncludeTable table;

void VS_InitFileIncludeTable(){
	table.size = 0;
}

void VS_DestroyFileIncludeTable(){
	unsigned long i;
	for(i = 0; i < table.size; i++){
		if(table.table[i].name != NULL){
			free(table.table[i].name);
			table.table[i].name = NULL;
		}
	}
}

unsigned long VS_AddIncludeEntry(char* name, unsigned long line_start, unsigned long line_end){
	unsigned long size = table.size;
	unsigned long len = strlen(name);
	
	if(size < VS_MAX_INCLUDES){
		table.table[size].name = malloc(len+1);
		memcpy(table.table[size].name,name,len);
		table.table[size].name[len] = '\0';
		
		table.table[size].line_start = line_start;
		table.table[size].line_end = line_end;
		table.table[size].num_of_lines = line_end - line_start;
		
		table.size++;
		
		return table.size - 1;
	}
	
	return 0;
}

void VS_UpdateIncludeEntry(unsigned long index, unsigned long line_start, unsigned long line_end){
	if(index < VS_MAX_INCLUDES){
		table.table[index].line_start = line_start;
		table.table[index].line_end = line_end;
		table.table[index].num_of_lines = line_end - line_start;
	}
}

int VS_ErrorOccuredInIncludeEntry(unsigned long line_count){
	unsigned i, size = table.size;
	for(i = 0; i < size; i++){
		if(line_count >= table.table[i].line_start && line_count <= table.table[i].line_end){
			return i;
		}
	}
	
	return -1;
}

int VS_GetActualLineCount(unsigned long line_count){
	unsigned long i, size = table.size;
	int count = line_count;
	for(i = 0; i < size; i++){
		if(line_count > table.table[i].line_end){
			count -= table.table[i].num_of_lines;
		}
	}
	
	return count;
}

void VS_PrintErrorFromIncludeEntry(unsigned long entry, unsigned long line_count){
	int count;
	char line[VS_MAX_LINE+1];
	
	if(entry < VS_MAX_INCLUDES){
		FILE* err = fopen(table.table[entry].name,"rb");
		
		count = line_count - table.table[entry].line_start;

		if(count < 0){
			count = 0;
		}
		
		int i;
		for(i = 0; i < count; i++){
			VS_ReadLine(err,line);
		}
		
		printf("From include file: %s\n",table.table[entry].name);
		printf("Line %d: %s\n",count,line);
		
		fclose(err);
	}
}

void VS_PrintIncludeEntryTable(){
	int i, size = table.size;
	for(i = 0; i < size; i++){
		printf("Include = %s\n",table.table[i].name);
		printf("Line Start = %ld\n",table.table[i].line_start);
		printf("Line End = %ld\n\n",table.table[i].line_end);
	}
}

void VS_PrintError(FILE* in, char* line, unsigned long line_count){
	fseek(in, 0x0, SEEK_SET);

	unsigned long i;
	for(i = 0; i < line_count; i++){
		VS_ReadLine(in,line);
	}

	printf("Line %ld: %s\n",line_count,line);
}

int VS_GrabMacrosFromIncludeEntry(char* incdir, unsigned long incindex, unsigned long line_count){
	FILE* inc;
	char dest[VS_MAX_LINE+1];
	char trim[VS_MAX_LINE+1];
	char name[VS_MAX_LINE+1];
	char* token, *end_quotes;
	int size;
	
	if(incdir[0] != '\'' && incdir[0] != '\"'){
		printf("Syntax Error: The file path of an include directive must be encased in double quotes or single quotes\n");
		VS_PrintErrorFromIncludeEntry(incindex,line_count);
		return -1;
	}
	else{
		if(incdir[0] == '\"'){
			end_quotes = strstr(incdir + 1,"\"");
			
			if(end_quotes == NULL){
				printf("Syntax Error: The file path of an include directive must be encased in double quotes or single quotes\n");
				VS_PrintErrorFromIncludeEntry(incindex,line_count);
				return -1;
			}
		}
		else{
			end_quotes = strstr(incdir + 1,"\'");
			
			if(end_quotes == NULL){
				printf("Syntax Error: The file path of an include directive must be encased in double quotes or single quotes\n");
				VS_PrintErrorFromIncludeEntry(incindex,line_count);
				return -1;
			}
		}
	}
	
	size = end_quotes - incdir;
	
	if(size >= 1){
		size = size - 1;
	}
	
	strncpy(dest, incdir + 1, size);
	
	dest[size] = '\0';
	
	inc = fopen(dest,"rb");

	if(inc == NULL){
		printf("Warning: The file path of the include directive could not be found!\n");
		VS_PrintErrorFromIncludeEntry(incindex,line_count);
	}
	else{
		while(VS_ReadLine(inc, dest)){
			memset(trim,0x0,VS_MAX_LINE);
			VS_TrimLine(trim,dest);
			
			token = strstr(trim,"equr");
			
			if(token != NULL){
				char* value = token + strlen("equr");
				memset(name,'\0',VS_MAX_LINE);
				memcpy(name,trim,(int)(token - trim));

				VS_AddRegMacro(name,value);
			}	
			
			token = strstr(trim,"equ");
			
			if(token != NULL && strstr(trim,"equr") == NULL){
				char* value = token + strlen("equ");
				memset(name,'\0',VS_MAX_LINE);
				memcpy(name,trim,(int)(token - trim));

				VS_AddMacro(name,value);
			}	
		}
		
		fclose(inc);
	}
	
	return 0;
}

int VS_VerifyPathSyntax(char* path, char* directive, unsigned long line_count){
	char* end_quotes;
	
	if(path[0] != '\'' && path[0] != '\"'){
		printf("Syntax Error: The file path of an %s directive must be encased in double quotes or single quotes\n",directive);
		
		int err = VS_ErrorOccuredInIncludeEntry(line_count);
		
		if(err != -1){
			VS_PrintErrorFromIncludeEntry(err,line_count);
		}
		else{
			printf("Line %ld: %s\n",line_count,path);
		}
		
		return 0;
	}
	else{
		if(path[0] == '\"'){
			end_quotes = strstr(path + 1,"\"");
			
			if(end_quotes == NULL){
				printf("Syntax Error: The file path of an %s directive must be encased in double quotes or single quotes\n",directive);
				
				int err = VS_ErrorOccuredInIncludeEntry(line_count);
		
				if(err != -1){
					VS_PrintErrorFromIncludeEntry(err,line_count);
				}
				else{
					printf("Line %ld: %s\n",line_count,path);
				}
				
				return 0;
			}
		}
		else{
			end_quotes = strstr(path + 1,"\'");
			
			if(end_quotes == NULL){
				printf("Syntax Error: The file path of an %s directive must be encased in double quotes or single quotes\n",directive);
				
				int err = VS_ErrorOccuredInIncludeEntry(line_count);
		
				if(err != -1){
					VS_PrintErrorFromIncludeEntry(err,line_count);
				}
				else{
					printf("Line %ld: %s\n",line_count,path);
				}

				return 0;
			}
		}
	}

	return 1;
}

unsigned long VS_CountNumberOfCharInLine(char* line, char c){
	unsigned long i, len = strlen(line), count = 0;
	for(i = 0; i < len; i++){
		if(line[i] == c){
			count++;
		}
	}
	
	return count;
}

unsigned long VS_SwapLong(unsigned long dword){
	unsigned char msb1, msb2, lsb1, lsb2;
	
	msb1 = (dword >> 24) & 0xff;
	msb2 = (dword >> 16) & 0xff;
	lsb1 = (dword >> 8) & 0xff;
	lsb2 = dword & 0xff;
	
	return lsb2 << 24 | lsb1 << 16 | msb2 << 8 | msb1;
}

unsigned short VS_SwapShort(unsigned short hword){
	unsigned char msb, lsb;
	
	msb = (hword >> 8) & 0xff;
	lsb = hword & 0xff;
	
	return lsb << 8 | msb;
}

unsigned long VS_GetCharCountInLine(char* line, char c){
	unsigned long i, len = strlen(line), count = 0;
	for(i = 0; i < len; i++){
		if(line[i] == c){
			count++;
		}
	}
	
	return count;
}

char* VS_GetArchitectureName(unsigned long architecture){
	switch(architecture){
		case 1:{
			return "mips1";
		}
		case 2:{
			return "mips2";
		}
		case 4:{
			return "psx";
		}
		case 8:{
			return "psp";
		}
		default: return "Unrecognized architecture";
	}
}

unsigned char VS_GetConditionField(const char* instruction){
	if(!strcmp(instruction,"c.f.s")){
		return 0;
	}
	else if(!strcmp(instruction,"c.un.s")){
		return 1;
	}
	else if(!strcmp(instruction,"c.eq.s")){
		return 2;
	}
	else if(!strcmp(instruction,"c.ueq.s")){
		return 3;
	}
	else if(!strcmp(instruction,"c.olt.s")){
		return 4;
	}
	else if(!strcmp(instruction,"c.ult.s")){
		return 5;
	}
	else if(!strcmp(instruction,"c.ole.s")){
		return 6;
	}
	else if(!strcmp(instruction,"c.ule.s")){
		return 7;
	}
	else if(!strcmp(instruction,"c.sf.s")){
		return 1 << 3 | 0;
	}
	else if(!strcmp(instruction,"c.ngle.s")){
		return 1 << 3 | 1;
	}
	else if(!strcmp(instruction,"c.seq.s")){
		return 1 << 3 | 2;
	}
	else if(!strcmp(instruction,"c.ngl.s")){
		return 1 << 3 | 3;
	}
	else if(!strcmp(instruction,"c.lt.s")){
		return 1 << 3 | 4;
	}
	else if(!strcmp(instruction,"c.nge.s")){
		return 1 << 3 | 5;
	}
	else if(!strcmp(instruction,"c.le.s")){
		return 1 << 3 | 6;
	}
	else if(!strcmp(instruction,"c.ngt.s")){
		return 1 << 3 | 7;
	}
	else return 0;
}

unsigned char VS_IsFloatingPointComparison(const char* instruction){
	if(!strcmp(instruction,"c.f.s")){
		return 1;
	}
	else if(!strcmp(instruction,"c.un.s")){
		return 1;
	}
	else if(!strcmp(instruction,"c.eq.s")){
		return 1;
	}
	else if(!strcmp(instruction,"c.ueq.s")){
		return 1;
	}
	else if(!strcmp(instruction,"c.olt.s")){
		return 1;
	}
	else if(!strcmp(instruction,"c.ult.s")){
		return 1;
	}
	else if(!strcmp(instruction,"c.ole.s")){
		return 1;
	}
	else if(!strcmp(instruction,"c.ule.s")){
		return 1;
	}
	else if(!strcmp(instruction,"c.sf.s")){
		return 1;
	}
	else if(!strcmp(instruction,"c.ngle.s")){
		return 1;
	}
	else if(!strcmp(instruction,"c.seq.s")){
		return 1;
	}
	else if(!strcmp(instruction,"c.ngl.s")){
		return 1;
	}
	else if(!strcmp(instruction,"c.lt.s")){
		return 1;
	}
	else if(!strcmp(instruction,"c.nge.s")){
		return 1;
	}
	else if(!strcmp(instruction,"c.le.s")){
		return 1;
	}
	else if(!strcmp(instruction,"c.ngt.s")){
		return 1;
	}
	else return 0;
}

unsigned char VS_ToDigit(char c){
	switch(c){
		case '0':{
			return 0;
		}
		case '1':{
			return 1;
		}
		case '2':{
			return 2;
		}
		case '3':{
			return 3;
		}
		case '4':{
			return 4;
		}
		case '5':{
			return 5;
		}
		case '6':{
			return 6;
		}
		case '7':{
			return 7;
		}
		case '8':{
			return 8;
		}
		case '9':{
			return 9;
		}
		default: return 0;
	}
}

unsigned char VS_GetVFPUSize(const char* vfpu_instruction){
	int size;
	unsigned char c;
	
	c = vfpu_instruction[VS_GetFirstOccurenceIndex((char*)vfpu_instruction,'.') + 1];
	
	if(c == 's'){
		size = 0;
	}
	else if(c == 'p'){
		size = 1;
	}
	else if(c == 't'){
		size = 2;
	}
	else{
		size = 3;
	}
	
	return size;
}

unsigned char VS_GetVFPUComparison(char* cond){
	if(!strcasecmp(cond,"fl")){
		return 0;
	}
	else if(!strcasecmp(cond,"eq")){
		return 1;
	}
	else if(!strcasecmp(cond,"lt")){
		return 2;
	}
	else if(!strcasecmp(cond,"le")){
		return 3;
	}
	else if(!strcasecmp(cond,"tr")){
		return 4;
	}
	else if(!strcasecmp(cond,"ne")){
		return 5;
	}
	else if(!strcasecmp(cond,"ge")){
		return 6;
	}else if(!strcasecmp(cond,"gt")){
		return 7;
	}
	else if(!strcasecmp(cond,"ez")){
		return 8;
	}
	else if(!strcasecmp(cond,"en")){
		return 9;
	}
	else if(!strcasecmp(cond,"ei")){
		return 10;
	}
	else if(!strcasecmp(cond,"es")){
		return 11;
	}
	else if(!strcasecmp(cond,"nz")){
		return 12;
	}
	else if(!strcasecmp(cond,"nn")){
		return 13;
	}else if(!strcasecmp(cond,"ni")){
		return 14;
	}
	else if(!strcasecmp(cond,"ns")){
		return 15;
	}
	else return 16;
}

void VS_LowercaseLine(char* line){
	int i, len = strlen(line);
	for(i = 0; i < len; i++){
		line[i] = tolower(line[i]);
	}
}

void VS_LowercaseAndCopyLine(char* dest, char* src, int n){
	int i;
	for(i = 0; i < n; i++){
		dest[i] = tolower(src[i]);
	}
}

void VS_AppendVFPUEncoding(const char* name, unsigned long* instruction){
	if(!strncmp(name,"vabs",4)){
		*instruction |= 1 << 16;
	}
	
	if(!strncmp(name,"vneg",4)){
		*instruction |= 1 << 17;
	}
	
	if(!strncmp(name,"vsat0",5)){
		*instruction |= 1 << 18;
	}
	
	if(!strncmp(name,"vsat1",5)){
		*instruction |= 1 << 16;
		*instruction |= 1 << 18;
	}
	
	if(!strncmp(name,"vrcp",4)){
		*instruction |= 1 << 20;
	}
	
	if(!strncmp(name,"vsin",4)){
		*instruction |= 1 << 17;
		*instruction |= 1 << 20;
	}
	
	if(!strncmp(name,"vcos",4)){
		*instruction |= 1 << 16;
		*instruction |= 1 << 17;
		*instruction |= 1 << 20;
	}
	
	if(!strncmp(name,"vexp2",5)){
		*instruction |= 1 << 18;
		*instruction |= 1 << 20;
	}
	
	if(!strncmp(name,"vlog2",5)){
		*instruction |= 1 << 16;
		*instruction |= 1 << 18;
		*instruction |= 1 << 20;
	}
	
	if(!strncmp(name,"vlgb",4)){
		*instruction |= 1 << 16;
		*instruction |= 1 << 17;
		*instruction |= 1 << 18;
		*instruction |= 1 << 20;
		*instruction |= 1 << 21;
	}
	
	if(!strncmp(name,"vsbz",4)){
		*instruction |= 1 << 17;
		*instruction |= 1 << 18;
		*instruction |= 1 << 20;
		*instruction |= 1 << 21;
	}
	
	if(!strncmp(name,"vsqrt",5)){
		*instruction |= 1 << 17;
		*instruction |= 1 << 18;
		*instruction |= 1 << 20;
	}
	
	if(!strncmp(name,"vasin",5)){
		*instruction |= 1 << 16;
		*instruction |= 1 << 17;
		*instruction |= 1 << 18;
		*instruction |= 1 << 20;
	}
	
	if(!strncmp(name,"vnrcp",5)){
		*instruction |= 1 << 19;
		*instruction |= 1 << 20;
	}
	
	if(!strncmp(name,"vnsin",5)){
		*instruction |= 1 << 17;
		*instruction |= 1 << 19;
		*instruction |= 1 << 20;
	}
	
	if(!strncmp(name,"vrexp2",6)){
		*instruction |= 1 << 18;
		*instruction |= 1 << 19;
		*instruction |= 1 << 20;
	}
	
	if(!strncmp(name,"vsrt1",5)){
		*instruction |= 1 << 22;
	}
	
	if(!strncmp(name,"vsrt2",5)){
		*instruction |= 1 << 16;
		*instruction |= 1 << 22;
	}
	
	if(!strncmp(name,"vsrt3",5)){
		*instruction |= 1 << 19;
		*instruction |= 1 << 22;
	}
	
	if(!strncmp(name,"vsrt4",5)){
		*instruction |= 1 << 16;
		*instruction |= 1 << 19;
		*instruction |= 1 << 22;
	}
	
	if(!strncmp(name,"vbfy1",5)){
		*instruction |= 1 << 17;
		*instruction |= 1 << 22;
	}	
	
	if(!strncmp(name,"vbfy2",5)){
		*instruction |= 1 << 16;
		*instruction |= 1 << 17;
		*instruction |= 1 << 22;
	}
	
	if(!strncmp(name,"vsgn",4)){
		*instruction |= 1 << 17;
		*instruction |= 1 << 19;
		*instruction |= 1 << 22;
	}
	
	if(!strncmp(name,"vocp",4)){
		*instruction |= 1 << 18;
		*instruction |= 1 << 22;
	}
	
	if(!strncmp(name,"vf2iz",5)){
		*instruction |= 1 << 21;
	}
	
	if(!strncmp(name,"vf2iu",5)){
		*instruction |= 1 << 22;
	}
	
	if(!strncmp(name,"vf2id",5)){
		*instruction |= 1 << 21;
		*instruction |= 1 << 22;
	}
	
	if(!strncmp(name,"vrot",4)){
		*instruction |= 1 << 21;
	}
	
	if(!strncmp(name,"vsocp",5)){
		*instruction |= 1 << 16;
		*instruction |= 1 << 18;
		*instruction |= 1 << 22;
	}
	
	if(!strncmp(name,"vavg",4)){
		*instruction |= 1 << 16;
		*instruction |= 1 << 17;
		*instruction |= 1 << 18;
		*instruction |= 1 << 22;
	}
	
	if(!strncmp(name,"vfad",4)){
		*instruction |= 1 << 17;
		*instruction |= 1 << 18;
		*instruction |= 1 << 22;
	}
	
	if(!strncmp(name,"vidt",4)){
		*instruction |= 1 << 16;
		*instruction |= 1 << 17;
	}
	
	if(!strncmp(name,"vzero",5)){
		*instruction |= 1 << 17;
		*instruction |= 1 << 18;
	}
	
	if(!strncmp(name,"vone",4)){
		*instruction |= 1 << 16;
		*instruction |= 1 << 17;
		*instruction |= 1 << 18;
	}
	
	if(!strncmp(name,"vrnds",5)){
		*instruction |= 1 << 21;
	}
	
	if(!strncmp(name,"vrndi",5)){
		*instruction |= 1 << 16;
		*instruction |= 1 << 21;
	}
	
	if(!strncmp(name,"vrndf1",6)){
		*instruction |= 1 << 17;
		*instruction |= 1 << 21;
	}
	
	if(!strncmp(name,"vrndf2",6)){
		*instruction |= 1 << 16;
		*instruction |= 1 << 17;
		*instruction |= 1 << 21;
	}
	
	if(!strncmp(name,"vmidt",5)){
		*instruction |= 1 << 16;
		*instruction |= 1 << 17;
	}
	
	if(!strncmp(name,"vmzero",6)){
		*instruction |= 1 << 17;
		*instruction |= 1 << 18;
	}
	
	if(!strncmp(name,"vmone",5)){
		*instruction |= 1 << 16;
		*instruction |= 1 << 17;
		*instruction |= 1 << 18;
	}
	
	if(!strncmp(name,"vi2uc",5)){
		*instruction |= 1 << 18;
		*instruction |= 1 << 19;
		*instruction |= 1 << 20;
		*instruction |= 1 << 21;
	}
	
	if(!strncmp(name,"vi2c",4)){
		*instruction |= 1 << 16;
		*instruction |= 1 << 18;
		*instruction |= 1 << 19;
		*instruction |= 1 << 20;
		*instruction |= 1 << 21;
	}
	
	if(!strncmp(name,"vi2us",4)){
		*instruction |= 1 << 17;
		*instruction |= 1 << 18;
		*instruction |= 1 << 19;
		*instruction |= 1 << 20;
		*instruction |= 1 << 21;
	}
	
	if(!strncmp(name,"vi2s",4)){
		*instruction |= 1 << 16;
		*instruction |= 1 << 17;
		*instruction |= 1 << 18;
		*instruction |= 1 << 19;
		*instruction |= 1 << 20;
		*instruction |= 1 << 21;
	}
	
	if(!strncmp(name,"vf2h",4)){
		*instruction |= 1 << 17;
		*instruction |= 1 << 20;
		*instruction |= 1 << 21;
	}
	
	if(!strncmp(name,"vs2i",4)){
		*instruction |= 1 << 16;
		*instruction |= 1 << 17;
		*instruction |= 1 << 19;
		*instruction |= 1 << 20;
		*instruction |= 1 << 21;
	}
	
	if(!strncmp(name,"vus2i",5)){
		*instruction |= 1 << 17;
		*instruction |= 1 << 19;
		*instruction |= 1 << 20;
		*instruction |= 1 << 21;
	}
	
	if(!strncmp(name,"vc2i",4)){
		*instruction |= 1 << 16;
		*instruction |= 1 << 19;
		*instruction |= 1 << 20;
		*instruction |= 1 << 21;
	}
	
	if(!strncmp(name,"vh2f",4)){
		*instruction |= 1 << 16;
		*instruction |= 1 << 17;
		*instruction |= 1 << 20;
		*instruction |= 1 << 21;
	}
	
	if(!strncmp(name,"vt4444",6)){
		*instruction |= 1 << 16;
		*instruction |= 1 << 19;
		*instruction |= 1 << 20;
		*instruction |= 1 << 22;
	}
	
	if(!strncmp(name,"vt5551",6)){
		*instruction |= 1 << 17;
		*instruction |= 1 << 19;
		*instruction |= 1 << 20;
		*instruction |= 1 << 22;
	}
	
	if(!strncmp(name,"vt5650",6)){
		*instruction |= 1 << 16;
		*instruction |= 1 << 17;
		*instruction |= 1 << 19;
		*instruction |= 1 << 20;
		*instruction |= 1 << 22;
	}
	
	if(!strncmp(name,"vfim",4)){
		*instruction |= 1 << 23;
	}
}

void VS_PrintErrorString(int error){
	switch(error){
		case 0:{
			printf("Error: Bad syntax\n");
		}break;
		case -1:{
			printf("Syntax Error: The operand of an instruction must begin with a $ symbol in GNU syntax\n");
		}break;
		case -16:{
			printf("Syntax Error: The operand of an instruction must not begin with a $ in ASMPSX syntax\n");
		}break;
		case -2:{
			printf("Syntax Error: Invalid register name\n");
		}break;
		case -3:{
			printf("Syntax Error: Logical/arithematic shift value cannot be negative or greater than 31\n");
		}break;
		case -4:{
			printf("Syntax Error: Label not found\n");
		}break;
		case -5:{
			printf("Syntax Error: Invalid hexadecimal number for immediate value\n");
		}break;
		case -6:{
			printf("Syntax Error: Invalid immediate value\n");
		}break;
		case -7:{
			printf("Syntax Error: Immedidate value too large\n");
		}break;
		case -8:{
			printf("Syntax Error: Floating point instructions must use floating point registers\n");
		}break;
		case -9:{
			printf("Syntax Error: Invalid register for non-floating point instruction\n");
		}break;
		case -11:{
			printf("Syntax Error: Invalid floating-point value\n");
		}break;			
		case -13:{
			printf("Syntax Error: Invalid cop register name\n");
		}break;
		case -14:{
			printf("Syntax Error: Invalid PSP VFPU register name\n");
		}break;
		case -15:{
			printf("Syntax Error: Invalid PSP VFPU comparison condition name\n");
		}break;
		case -17:{
			printf("Syntax Error: Invalid expression\n");
		}break;
		case 3:{
			printf("Warning: Unaligned memory access\n");
		}break;
		case 4:{
			printf("Warning: Divide by zero\n");
		}break;
		default: printf("Unrecognized error code");
	}
}

void VS_WriteInstruction(FILE* file, unsigned long instruction, VS_ENDIAN endian){
	unsigned char msb1, msb2, lsb1, lsb2;
	
	msb1 = (instruction >> 24) & 0xff;
	msb2 = (instruction >> 16) & 0xff;
	lsb1 = (instruction >> 8) & 0xff;
	lsb2 = instruction & 0xff;
	
	if(endian == VS_LITTLE_ENDIAN){
		fwrite(&lsb2,1,1,file);
		fwrite(&lsb1,1,1,file);
		fwrite(&msb2,1,1,file);
		fwrite(&msb1,1,1,file);
	}
	else{
		fwrite(&msb1,1,1,file);
		fwrite(&msb2,1,1,file);
		fwrite(&lsb1,1,1,file);
		fwrite(&lsb2,1,1,file);
	}
}

void VS_WriteNop(FILE* file){
	unsigned long zero = 0;
	fwrite(&zero,4,1,file);
}