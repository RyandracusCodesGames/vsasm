#include <vs_parser.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <vs_preprocessor.h>
#include <vs_symtable.h>
#include <vs_utils.h>
#include <vs_elf.h>
#include <vs_psyqobj.h>

/********************************************
*   VideoStation Assembler
*
*   Copyright (c) 2025 Ryandracus Chapman
*
*   File: vs_parser.c
*   Date: 4/29/2025
*   Version: 1.0
*   Updated: 6/1/2025
*   Author: Ryandracus Chapman
*
********************************************/

unsigned long org;

int sym_index, symbol_index, label_instruction_count, instruction_count, safe_load_delay, OEXE;
int syntax;

VS_REGISTER vs_cop_registers[32] = {
	{"$0", 0},
	{"$1", 1},
	{"$2", 2},
	{"$3", 3},
	{"$4", 4},
	{"$5", 5},
	{"$6", 6},
	{"$7", 7},
	{"$8", 8},
	{"$9", 9},
	{"$10", 10},
	{"$11", 11},
	{"$12", 12},
	{"$13", 13},
	{"$14", 14},
	{"$15", 15},
	{"$16", 16},
	{"$17", 17},
	{"$18", 18},
	{"$19", 19},
	{"$20", 20},
	{"$21", 21},
	{"$22", 22},
	{"$23", 23},
	{"$24", 24},
	{"$25", 25},
	{"$26", 26},
	{"$27", 27},
	{"$28", 28},
	{"$29", 29},
	{"$30", 30},
	{"$31", 31},
};

VS_REGISTER vs_asmpsx_cop_registers[32] = {
	{"r0", 0},
	{"r1", 1},
	{"r2", 2},
	{"r3", 3},
	{"r4", 4},
	{"r5", 5},
	{"r6", 6},
	{"r7", 7},
	{"r8", 8},
	{"r9", 9},
	{"r10", 10},
	{"r11", 11},
	{"r12", 12},
	{"r13", 13},
	{"r14", 14},
	{"r15", 15},
	{"r16", 16},
	{"r17", 17},
	{"r18", 18},
	{"r19", 19},
	{"r20", 20},
	{"r21", 21},
	{"r22", 22},
	{"r23", 23},
	{"r24", 24},
	{"r25", 25},
	{"r26", 26},
	{"r27", 27},
	{"r28", 28},
	{"r29", 29},
	{"r30", 30},
	{"r31", 31},
};

unsigned char VS_HexDFA(char* line){
	int i, len = strlen(line);
	for(i = 0; i < len; i++){
		if(line[i] != '\n' && !isxdigit(line[i]) && line[i] != 'x'){
			return 0;
		}
	}
	
	return 1;
}

unsigned char VS_IntDFA(char* line){
	int i, len = strlen(line);
	for(i = 0; i < len; i++){
		if(line[i] != '\n' && !isdigit(line[i])){
			return 0;
		}
	}
	
	return 1;
}

unsigned char VS_FloatDFA(char* line){
	int i, decimals = 0, len = strlen(line);
	for(i = 0; i < len; i++){
		if(line[i] != '\n' && !isdigit(line[i])){
			if(decimals == 0){
				decimals++;
			}
			else return 0;
		}
	}
	
	return 1;
}

int VS_GetRegister(char* line, unsigned long* size_out){
	int i;
	unsigned long size = 0;
	char reg_str[8];

	for(i = 0; i < 7; i++, size++){
		if(line[i] == ',' || line[i] == '\n' || line[i] == ')'){
			break;
		}
		
		reg_str[i] = line[i];
	}
	
	*size_out = size;
	reg_str[size] = '\0';
	
	if(!VS_IsValidRegister(reg_str,syntax)){
		return -2;
	}
	else return VS_GetRegisterNumber(reg_str);
}

int VS_GetFpRegister(char* line, unsigned long* size_out){
	int i;
	unsigned long size = 0;
	char reg_str[7];

	for(i = 0; i < 7; i++, size++){
		if(line[i] == ',' || line[i] == '\n' || line[i] == ')'){
			break;
		}
		
		reg_str[i] = line[i];
	}
	
	*size_out = size;
	reg_str[size] = '\0';
	
	if(!VS_IsValidFpRegister(reg_str,syntax)){
		if(!VS_IsValidRegister(reg_str,syntax)){
			return -1;
		}
		else return -2;
	}
	else return VS_GetRegisterNumber(reg_str);
}


int VS_GetCopRegister(char* line, unsigned long* size_out){
	int i, reg;
	unsigned long size = 0;
	char reg_str[6];

	for(i = 0; i < 5; i++, size++){
		if(line[i] == ',' || line[i] == '\n' || line[i] == ')'){
			break;
		}
		
		reg_str[i] = line[i];
	}
	
	*size_out = size;
	reg_str[size] = '\0';

	for(i = 0; i < 32; i++){
		if(!strcmp(reg_str,vs_cop_registers[i].reg)){
			reg = VS_GetRegisterNumber(reg_str);
			return reg;
		}
		
		if(!strcmp(reg_str,vs_asmpsx_cop_registers[i].reg)){
			return i;
		}
	}
	
	return -1;
}

int VS_GetVFPURegister(char* line, int size){
	int matrix, column, row, vfpureg;
	char c;
	
	c = line[0];
	vfpureg = 0;
	
	if(size < 0 || size > 3){
		return -1;
	}
	
	if(strlen(line) != 4 || (c != 's' && c != 'm' && c != 'e' && c != 'c' && c != 'r')){
		return -1;
	}
	
	if(!VS_IntDFA(line+1)){
		return -1;
	}
	
	matrix = VS_ToDigit(line[1]);
	column = VS_ToDigit(line[2]);
	row = VS_ToDigit(line[3]);
	
	if(matrix > 7 || column > 3 || row > 3){
	    return -1;
	}
	
	switch(c){
	    case 'r':{
	        vfpureg |= 1 << 5;
	        int temp = column;
	        column = row;
	        row = temp;
	    }
		case 'c':{
			switch (size)
			{
				case 1:
				case 3:
					if(row & 1)
						return -1;
					break;
				case 2:
					if (row & 2)
						return -1;
					row <<= 1;
					break;
				default: return -1;
			}
		}break;	
		case 's':{
			if(size != 0)
				return -1;
		}break;
		case 'e':	
			vfpureg |= (1 << 5);
		case 'm':{
			switch (size)
			{
				case 1:
				case 3:
					if(row & 1)
						return -1;
					break;
				case 2:
					if(row & ~1)
						return -1;
					row <<= 1;
					break;
				default: return -1;
			}
		}break;
	}
	
	vfpureg |= matrix << 2;
	vfpureg |= column;
	vfpureg |= row << 5;
	
	return vfpureg;
}

void VS_CopyLabelName(char* dest, char* src, int offset){
	int i;
	for(i = 0; i < VS_MAX_LINE; i++){
		if(src[offset + i] != '\n'){
			dest[i] = src[offset + i];
		}
		else{
			dest[i] = '\0';
			break;
		}
	}
}

int VS_IsValidImmediate(char* line){
	int is_valid_hex, is_valid_int;
	
	if(line[0] == '-'){
		line++;
	}
	
	if(line[0] == '0' && line[1] == 'x'){
		is_valid_hex = VS_HexDFA(line);
		
		if(is_valid_hex){
			return 1;
		}
		else return -1;
	}
	else{
		is_valid_int = VS_IntDFA(line);
		
		if(is_valid_int){
			return 2;
		}
		else return -2;
	}
}

int VS_ParseRType(VS_OPCODE op, unsigned long* instruction, char* line, FILE* file){
	VS_R_TYPE rtype;
	VS_OPCODE opcode;
	unsigned long size1, size2, size3, encode;
	int len, imm, is_valid_imm, neg;
	
	memset(&rtype,0x0,sizeof(VS_R_TYPE));
	
	len = strlen(op.name);
	neg = 0;
	
	if(!strcmp(op.name,"syscall") || !strcmp(op.name,"break")){
		*instruction = VS_Bin2Decimal(op.opcode);
		return 1;
	}
	
	if(line[len] != '$' && syntax == VS_GNU_SYNTAX){
		return -1;
	}

	rtype.op = VS_Bin2Decimal(op.opcode);
	rtype.rd = VS_GetRegister(line + len, &size1);

	if(rtype.rd == -1){
		return -2;
	}
	
	if(rtype.rd == -2){
		if(VS_GetNumberOfCommas(line) == 0){
			return 0;
		}
		else return -9;
	}

	if(!strcmp(op.name,"jr")){
		*instruction = rtype.rd << 21 | rtype.op;
			
		if(safe_load_delay){
			encode = rtype.rd << 21 | rtype.op;
			fwrite(&encode,4,1,file);
			encode = 0;
			fwrite(&encode,4,1,file);
			instruction_count++;
			return 2;
		}
		
		return 1;
	}
	else if(!strcmp(op.name,"jalr")){
		*instruction = rtype.rd << 21 | 31 << 11 | rtype.op;
		
		if(safe_load_delay){
			encode = rtype.rd << 21 | 31 << 11 | rtype.op;
			fwrite(&encode,4,1,file);
			encode = 0;
			fwrite(&encode,4,1,file);
			instruction_count++;
			return 2;
		}
		
		return 1;
	}
	else if(!strcmp(op.name,"neg")){
		*instruction = VS_GetRegisterNumber("$0") << 21 | rtype.rd << 16 | rtype.rd << 11 | rtype.op;
		return 1;
	}
	
	rtype.rs = VS_GetRegister(line + len + size1 + 1, &size2);
	
	if(line[len + size1 + 1] != '$' && rtype.rs < 0){
		
		int num_commas = VS_GetNumberOfCommas(line);
		if(num_commas >= 1 && (!strcmp(op.name,"add") || !strcmp(op.name,"addu") || !strcmp(op.name,"and") || !strcmp(op.name,"sub") || !strcmp(op.name,"subu") 
			|| !strcmp(op.name,"or") || !strcmp(op.name,"xor"))){
				
			char* operands = line + len + size1 + 1;
			
			if(VS_StrictIsStringBlank(operands)){
				return 0;
			}
			
			if(line[len + size1 + 1] == '-'){
				neg = 1;
			}
			
			is_valid_imm = VS_IsValidImmediate(line + len + size1 + 1);
		
			if(is_valid_imm == -1){
				return -5;
			}
			else if(is_valid_imm == -2){
			   return -6;	
			}
			
			if(neg){
				imm = (signed int)strtol(line + len + size1 + 1, NULL, 0);
				
				if(imm <= -32768 || imm >= 32768){
					return -7;
				}
			}
			else{
				imm = (unsigned int)strtoul(line + len + size1 + 1, NULL, 0);
				
				if(imm >= 65536){
					return -7;
				}
			}
			
			if(!strcmp(op.name,"addu")){
				VS_GetOpcodeFromIndex(&opcode,VS_GetOpcode("addiu"));
				*instruction = VS_Bin2Decimal(opcode.opcode) << 26 | rtype.rd << 21 | rtype.rd << 16 | (imm & 0xFFFF);
			}
			else{
				char itype_instruction[VS_MAX_LINE];
				
				if(syntax == VS_GNU_SYNTAX){
					strcpy(itype_instruction,op.name);
					strcat(itype_instruction,"i");
					VS_GetOpcodeFromIndex(&opcode,VS_GetOpcode(itype_instruction));
					sprintf(line,"%s$%d,$%d,%d",opcode.name,rtype.rd,rtype.rd,imm & 0xFFFF);
					return VS_ParseIType(opcode,instruction,line,file);
				}
				else{
					strcpy(itype_instruction,op.name);
					strcat(itype_instruction,"i");
					VS_GetOpcodeFromIndex(&opcode,VS_GetOpcode(itype_instruction));
					sprintf(line,"%sr%d,r%d,%d",opcode.name,rtype.rd,rtype.rd,imm & 0xFFFF);
					return VS_ParseIType(opcode,instruction,line,file);
				}
			}
			
			return 1;
		}
		else if(num_commas == 0){
			return 0;
		}
		else return -1;
	}

	if(rtype.rs == -1){
		return -2;
	}
	
	if(rtype.rs == -2){
		return -9;
	}
	
	if(!strcmp(op.name,"move")){
		*instruction = rtype.rs << 21 | VS_GetRegisterNumber("$0") << 16 | rtype.rd << 11 | rtype.op;
		return 1;
	}
	else if(!strcmp(op.name,"neg")){
		*instruction = VS_GetRegisterNumber("$0") << 21 | rtype.rs << 16 | rtype.rd << 11 | rtype.op;
		return 1;
	}
	else if(!strcmp(op.name,"div") || !strcmp(op.name,"divu") || !strcmp(op.name,"mult") || !strcmp(op.name,"multu")){
		*instruction = rtype.rd << 21 | rtype.rs << 16 | rtype.op;
		
		if(rtype.rd == 0 || rtype.rs == 0){
			return 4;
		}
		
		return 1;
	}
	else if(!strcmp(op.name,"teq") || !strcmp(op.name,"tge") || !strcmp(op.name,"tgeu") || !strcmp(op.name,"tlt") || !strcmp(op.name,"tne")){
		*instruction = rtype.rd << 21 | rtype.rs << 16 | rtype.op;
		return 1;
	}
	
	rtype.rt = VS_GetRegister(line + len + size1 + size2 + 2, &size3);

	if(line[len + size1 + size2 + 2] != '$' && rtype.rt < 0){
		int num_commas = VS_GetNumberOfCommas(line);

		if(num_commas == 1 && (!strcmp(op.name,"add") || !strcmp(op.name,"addu") || !strcmp(op.name,"and") || !strcmp(op.name,"sub") || !strcmp(op.name,"subu") 
			|| !strcmp(op.name,"or") || !strcmp(op.name,"xor") || !strcmp(op.name,"nor") || !strcmp(op.name,"slt") || !strcmp(op.name,"sltu"))){
			*instruction = rtype.rd << 21 | rtype.rs << 16 | rtype.rd << 11 | rtype.op;
			return 1;
		}
		else if(num_commas == 1 && (!strcmp(op.name,"sllv") || !strcmp(op.name,"srlv") || !strcmp(op.name,"srav"))){
			*instruction = rtype.rs << 21 | rtype.rd << 16 | rtype.rd << 11 | rtype.op;
			return 1;
		}
		else if(num_commas == 2 && syntax == VS_ASMPSX_SYNTAX && (!strcmp(op.name,"add") || !strcmp(op.name,"addu") || !strcmp(op.name,"and") || !strcmp(op.name,"sub") || !strcmp(op.name,"subu") 
			|| !strcmp(op.name,"or") || !strcmp(op.name,"xor"))){
				
			char* operands = line + len + size1 + size2 + 2;
			char reg[10];
			
			if(VS_StrictIsStringBlank(operands)){
				return 0;
			}
			
			VS_TrimStrictLine(reg,operands);
			
			if(VS_IsValidFpRegister(reg,VS_ASMPSX_SYNTAX)){
				return -9;
			}

			if(line[len + size1 + size2 + 2] == '-'){
				neg = 1;
			}
			
			is_valid_imm = VS_IsValidImmediate(line + len + size1 + size2 + 2);
		
			if(is_valid_imm == -1){
				return -5;
			}
			else if(is_valid_imm == -2){
			   return -6;	
			}
			
			if(neg){
				imm = (signed int)strtol(line + len + size1 + size2 + 2, NULL, 0);
				
				if(imm <= -32768 || imm >= 32768){
					return -7;
				}
			}
			else{
				imm = (unsigned int)strtoul(line + len + size1 + size2 + 2, NULL, 0);
				
				if(imm >= 65536){
					return -7;
				}
			}
			
			char itype_instruction[VS_MAX_LINE];
			
			if(!strcmp(op.name,"addu")){
				strcpy(itype_instruction,"addiu");
				VS_GetOpcodeFromIndex(&opcode,VS_GetOpcode(itype_instruction));
				sprintf(line,"%sr%d,r%d,%d",opcode.name,rtype.rd,rtype.rs,imm);
				return VS_ParseIType(opcode,instruction,line,file);
			}
			else{				
				strcpy(itype_instruction,op.name);
				strcat(itype_instruction,"i");
				VS_GetOpcodeFromIndex(&opcode,VS_GetOpcode(itype_instruction));
				sprintf(line,"%sr%d,r%d,%d",opcode.name,rtype.rd,rtype.rs,imm);
				return VS_ParseIType(opcode,instruction,line,file);
			}
			
			return 1;
		}
		else return -1;
	}
	
	if(rtype.rt == -1){
		return -2;
	}
	
	if(rtype.rt == -2){
		return -9;
	}
	
	if(!strcmp(op.name,"mul") || !strcmp(op.name,"umul")){
		*instruction = rtype.rs << 21 | rtype.rt << 16 | rtype.op;
		fwrite(instruction,4,1,file);
		*instruction = rtype.rd << 11 | 18; /* mflo */
		instruction_count++;
		return 1;
	}
	else if(!strcmp(op.name,"sllv") || !strcmp(op.name,"srav") || !strcmp(op.name,"srlv")){
		*instruction = rtype.rt << 21 | rtype.rs << 16 | rtype.rd << 11 | rtype.op;
	}
	else{
		*instruction = rtype.rs << 21 | rtype.rt << 16 | rtype.rd << 11 | rtype.op;
	}
	
	return 1;
}

int VS_ParseIType(VS_OPCODE op, unsigned long* instruction, char* line, FILE* file){
	VS_I_TYPE itype;
	VS_OPCODE opcode;
	unsigned long size1, size2, size3, instr;
	int is_valid_imm, len, neg;
	
	memset(&itype,0x0,sizeof(VS_I_TYPE));
	
	len = strlen(op.name);
	neg = 0;
	is_valid_imm = 0;
	instr = 0;
	
	if(!strcmp(op.name,"nop")){
		*instruction = itype.op << 26 | itype.rs << 25 | itype.rt << 16 | itype.imm;
		return 1;
	}
	
	if(line[len] != '$' && syntax == VS_GNU_SYNTAX){
		return -1;
	}
	
	itype.op = VS_Bin2Decimal(op.opcode);
	itype.rt = VS_GetRegister(line + len, &size1);
	
	if(itype.rt == -1){
		return -2;
	}
	
	if(itype.rt == -2){
		if(VS_GetNumberOfCommas(line) == 0){
			return 0;
		}
		else return -9;
	}

	if(!strcmp(op.name,"lui")){
		itype.rs = itype.rt;
		itype.rt = 0;
		
		is_valid_imm = VS_IsValidImmediate(line + len + size1 + 1);
		
		if(is_valid_imm == -1){
			return -5;
		}
		else if(is_valid_imm == -2){
		   return -6;	
		}
		 
		itype.imm = (signed int)strtoul(line + len + size1 + 1, NULL, 0);
		
		if(itype.imm > 65535){
			return -7;
		}
		
		*instruction = itype.op << 26 | itype.rt << 21 | itype.rs << 16 | (itype.imm & 0xFFFF);
	}
	else if(!strcmp(op.name,"teqi") || !strcmp(op.name,"tgei") || !strcmp(op.name,"tgeiu") || !strcmp(op.name,"tlti") || !strcmp(op.name,"tltiu") 
		|| !strcmp(op.name,"tnei")){

		if(line[len + size1 + 1] == '-'){
			neg = 1;
		}
			
		is_valid_imm = VS_IsValidImmediate(line + len + size1 + 1 + neg);

		if(is_valid_imm == -1){
			return -5;
		}
		else if(is_valid_imm == -2){
		   return -6;	
		}
		
		itype.imm = (signed short)strtoul(line + len + size1 + 1, NULL, 0);
		
		*instruction = 1 << 26 | itype.rt << 21 | itype.op << 16 | (itype.imm & 0xFFFF);
		return 1;
	}
	else if(!strcmp(op.name,"sll") || !strcmp(op.name,"sra") || !strcmp(op.name,"srl")){
		int num_commas = VS_GetNumberOfCommas(line);
		
		itype.rs = VS_GetRegister(line + len + size1 + 1, &size2);

		if(line[len + size1 + 1] != '$' && itype.rs < 0){
			
			char* operands = line + len + size1 + 1;
			
			if(VS_StrictIsStringBlank(operands)){
				return 0;
			}
	
			if(num_commas == 1){
				
				if(line[len + size1 + 1] == '-'){
					return -3;
				}
					
				is_valid_imm = VS_IsValidImmediate(line + len + size1 + 1);
		
				if(is_valid_imm == -1){
					return -5;
				}
				else if(is_valid_imm == -2){
				   return -6;	
				}
				
				itype.imm = (unsigned int)strtoul(line + len + size1 + 1, NULL, 0);
		
				if(itype.imm < 0 || itype.imm > 31){
					return -3;
				}
				
				*instruction = itype.rt << 16 | itype.rt << 11 | (itype.imm & 0x1f) << 6 | itype.op;
				
				return 1;
			}else return -1;
		}
		else if(num_commas == 1){
			itype.rs = VS_GetRegister(line + len + size1 + 1, &size2);
		
			if(itype.rs == -1){
				return -2;
			}
			
			if(itype.rs == -2){
				return -9;
			}

			if(!strcmp(op.name,"sll")){
				*instruction = itype.rs << 21 | itype.rt << 16 | (itype.rt & 0x1f) << 11 | VS_Bin2Decimal("000100"); /* sllv */
			}
			else if(!strcmp(op.name,"sra")){
				*instruction = itype.rs << 21 | itype.rt << 16 | (itype.rt & 0x1f) << 11 | VS_Bin2Decimal("000111"); /* srav */
			}
			else{
				*instruction = itype.rs << 21 | itype.rt << 16 | (itype.rt & 0x1f) << 11 | VS_Bin2Decimal("000110"); /* srlv */
			}
			
			return 1;
		}
	
		if(itype.rs == -1){
			return -2;
		}
		
		if(itype.rs == -2){
			return -9;
		}
		
		char* operands = line + len + size1 + size2 + 2;
			
		if(VS_StrictIsStringBlank(operands)){
			return 0;
		}
		
		itype.imm = VS_GetRegister(line + len + size1 + size2 + 2, &size3);
	
		if(itype.imm == -2){
			is_valid_imm = VS_IsValidImmediate(line + len + size1 + 2 + size2);
		
			if(is_valid_imm == -1){
				return -5;
			}
			else if(is_valid_imm == -2){
			   return -6;	
			}
			
			itype.imm = (unsigned int)strtoul(line + len + size1 + 2 + size2, NULL, 0);
			
			if(itype.imm < 0 || itype.imm > 31){
				return -3;
			}

			*instruction = itype.rs << 16 | itype.rt << 11 | (itype.imm & 0x1f) << 6 | itype.op;
		}
		else{
			if(!strcmp(op.name,"sll")){
				*instruction = itype.imm << 21 | itype.rs << 16 | (itype.rt & 0x1f) << 11 | VS_Bin2Decimal("000100"); /* sllv */
			}
			else if(!strcmp(op.name,"sra")){
				*instruction = itype.imm << 21 | itype.rs << 16 | (itype.rt & 0x1f) << 11 | VS_Bin2Decimal("000111"); /* srav */
			}
			else{
				*instruction = itype.imm << 21 | itype.rs << 16 | (itype.rt & 0x1f) << 11 | VS_Bin2Decimal("000110"); /* srlv */
			}
		}
	}
	else{
		itype.rs = VS_GetRegister(line + len + size1 + 1, &size2);
		
		if(line[len + size1 + 1] != '$' && itype.rs < 0){
			
			char* operands = line + len + size1 + 1;
			
			if(VS_StrictIsStringBlank(operands)){
				return 0;
			}
	
			int num_commas = VS_GetNumberOfCommas(line);
			
			if(line[len + size1 + 1] == '-'){
				neg = 1;
			}
			
			is_valid_imm = VS_IsValidImmediate(line + len + size1 + 1);
			
			if(neg){
				itype.imm = (signed int)strtol(line + len + size1 + 1, NULL, 0);
			}
			else{
				itype.imm = (unsigned int)strtoul(line + len + size1 + 1, NULL, 0);
			}
			
			if(!strcmp(op.name,"subi") || !strcmp(op.name,"subiu")){
				neg = 1;
				itype.imm = -itype.imm;
			}
			
			if((neg && (itype.imm <= -32678 || itype.imm >= 32767)) || itype.imm >= 65536){
				char itype_instruction[10];
				
				instr = 15 << 26 | 1 << 16 | ((itype.imm >> 16) & 0xFFFF); /* lui */
				fwrite(&instr,4,1,file);
				instr = 13 << 26 | 1 << 21 | 1 << 16 | (itype.imm & 0xFFFF); /* ori */
				fwrite(&instr,4,1,file);
				memset(itype_instruction,'\0',10);
				
				unsigned long i, count;
				for(i = 0, count = 0; i < strlen(op.name); i++){
					if(op.name[i] != 'i'){
						itype_instruction[count++] = op.name[i];
					}
				}

				if(syntax == VS_GNU_SYNTAX){
					VS_GetOpcodeFromIndex(&opcode,VS_GetOpcode(itype_instruction));
					sprintf(line,"%s$%d,$%d,$%d",opcode.name,itype.rt,itype.rt,1);
					VS_ParseRType(opcode,instruction,line,file);
				}
				else{
					VS_GetOpcodeFromIndex(&opcode,VS_GetOpcode(itype_instruction));
					sprintf(line,"%sr%d,r%d,r%d",opcode.name,itype.rt,itype.rt,1);
					VS_ParseRType(opcode,instruction,line,file);
				}
					
				instruction_count += 2;
				
				return 1;
			}

			*instruction = itype.op << 26 | itype.rt << 21 | itype.rt << 16 | (itype.imm & 0xFFFF);
			
			if((is_valid_imm == 1 || is_valid_imm == 2) && num_commas == 1){
				return 1;
			}
			else if(!(is_valid_imm == 1 || is_valid_imm == 2) && num_commas == 1){
				return -6;
			}else return -2;
		}
		
		if(itype.rs == -1){
			return -2;
		}
		
		if(itype.rs == -2){
			return -9;
		}
		
		if(line[len + size1 + 2 + size2] == '-'){
			neg = 1;
		}
		
		if(VS_GetNumberOfCommas(line + len + size1 + 1) == 0){
			return 0;
		}
		
		is_valid_imm = VS_IsValidImmediate(line + len + size1 + 2 + size2);
		
		if(is_valid_imm == -1){
			return -5;
		}
		else if(is_valid_imm == -2){
		   return -6;	
		}
		
		if(neg){
			itype.imm = (signed int)strtol(line + len + size1 + 2 + size2, NULL, 0);
		}
		else{
			itype.imm = (unsigned long)strtoul(line + len + size1 + 2 + size2, NULL, 0);
		}
		
		if(!strcmp(op.name,"subi") || !strcmp(op.name,"subiu")){
			neg = 1;
			itype.imm = -itype.imm;
		}
		
		if((neg && (itype.imm <= -32678 || itype.imm >= 32767)) || itype.imm >= 65536){
			char itype_instruction[10];
			
			instr = 15 << 26 | 1 << 16 | ((itype.imm >> 16) & 0xFFFF); /* lui */
			fwrite(&instr,4,1,file);
			instr = 13 << 26 | 1 << 21 | 1 << 16 | (itype.imm & 0xFFFF); /* ori */
			fwrite(&instr,4,1,file);
			
			memset(itype_instruction,'\0',10);
				
			unsigned long i, count;
			for(i = 0, count = 0; i < strlen(op.name); i++){
				if(op.name[i] != 'i'){
					itype_instruction[count++] = op.name[i];
				}
			}

			if(syntax == VS_GNU_SYNTAX){
				VS_GetOpcodeFromIndex(&opcode,VS_GetOpcode(itype_instruction));
				sprintf(line,"%s$%d,$%d,$%d",opcode.name,itype.rt,itype.rt,1);
				VS_ParseRType(opcode,instruction,line,file);
			}
			else{
				VS_GetOpcodeFromIndex(&opcode,VS_GetOpcode(itype_instruction));
				sprintf(line,"%sr%d,r%d,r%d",opcode.name,itype.rt,itype.rt,1);
				VS_ParseRType(opcode,instruction,line,file);
			}
				
			instruction_count += 2;
			
			return 1;
		}

		*instruction = itype.op << 26 | itype.rs << 21 | itype.rt << 16 | (itype.imm & 0xFFFF);
	}
	
	return 1;
}

int VS_ParseJType(VS_OPCODE op, unsigned long* instruction, char* line, FILE* file){
	VS_SYM sym;
	unsigned long encode, addr;
	int len, is_valid_imm;
	char new_line[VS_MAX_LINE+1];
	
	len = strlen(op.name);

	VS_CopyLabelName(new_line, line, len);
	
	is_valid_imm = VS_IsValidImmediate(new_line);
	
	if(VS_FindSymbol(new_line)){
		sym = VS_GetSymbol(new_line);
		addr = sym.addr;
		
		if(OEXE){
			addr += org;
		}
		
		if(!strcmp(op.name,"j") || !strcmp(op.name,"jal")){
			VS_SetRelocTrue();
			VS_SetPSYQRelocTrue();
			VS_AddRelocEntry((instruction_count << 2), VS_MIPS_26, 1);
			VS_AddPSYQRelocEntry(symbol_index, VS_PSYQ_26, ((instruction_count - label_instruction_count) << 2), 2, addr);
		}
		
		*instruction = VS_Bin2Decimal(op.opcode) << 26 | (((addr >> 2) & 0x3FFFFFF));
		
		if(safe_load_delay){
			encode = VS_Bin2Decimal(op.opcode) << 26 | (((addr >> 2) & 0x3FFFFFF));
			fwrite(&encode,4,1,file);
			encode = 0;
			fwrite(&encode,4,1,file);
			instruction_count++;
			return 2;
		}
		
		return 1;
	}
	else if(is_valid_imm != -1 && is_valid_imm != -2){
		addr = (unsigned long)strtoul(new_line, NULL, 0);
		
		*instruction = VS_Bin2Decimal(op.opcode) << 26 | (((addr >> 2) & 0x3FFFFFF));

		if(safe_load_delay){
			encode = VS_Bin2Decimal(op.opcode) << 26 | (((addr >> 2) & 0x3FFFFFF));
			fwrite(&encode,4,1,file);
			encode = 0;
			fwrite(&encode,4,1,file);
			instruction_count++;
			return 2;
		}
		
		return 1;
	}
	else if(new_line[0] == '0' && new_line[1] == 'x'){
		return -5;
	}
	else return -4;
}

int VS_ParseBType(VS_OPCODE op, unsigned long* instruction, char* line, FILE* file){
	VS_B_TYPE btype;
	VS_SYM sym;
	int len;
	char new_line[VS_MAX_LINE+1];
	unsigned long size1, size2, encode;
	int offset, cop, mips2;
	unsigned char pseudo_op;
	
	len = strlen(op.name);

	if(!strcmp(op.name,"b")){
		VS_CopyLabelName(new_line, line, len);
		
		if(VS_FindSymbol(new_line)){
			sym = VS_GetSymbol(new_line);
			offset = sym.instruction_count - instruction_count;
			offset--;
			
			*instruction = VS_Bin2Decimal(op.opcode) << 26 | (offset & 0xFFFF);
			
			if(safe_load_delay){
				encode = VS_Bin2Decimal(op.opcode) << 26 | (offset & 0xFFFF);
				fwrite(&encode,4,1,file);
				encode = 0;
				fwrite(&encode,4,1,file);
				instruction_count++;
				return 2;
			}
		}
		else{
			return -4;
		}
	}
	else if(!strcmp(op.name,"bc2f") || !strcmp(op.name,"bc1f") || !strcmp(op.name,"bc2fl") || !strcmp(op.name,"bc1fl")){
		VS_CopyLabelName(new_line, line, len);
		
		if(!strcmp(op.name,"bc2f") || !strcmp(op.name,"bc2fl")){
			cop = 18;
		}
		else{
			cop = 17;
		}
		
		if(!strcmp(op.name,"bc2fl") || !strcmp(op.name,"bc1fl")){
			mips2 = 1;
		}
		else{
			mips2 = 0;
		}
		
		if(VS_FindSymbol(new_line)){
			sym = VS_GetSymbol(new_line);
			offset = sym.instruction_count - instruction_count;
			offset--;
			
			*instruction = cop << 26 | VS_Bin2Decimal(op.opcode) << 21 | mips2 << 17 | (offset & 0xFFFF);
			
			if(safe_load_delay){
				encode = cop << 26 | VS_Bin2Decimal(op.opcode) << 21 | mips2 << 17 | (offset & 0xFFFF);
				fwrite(&encode,4,1,file);
				encode = 0;
				fwrite(&encode,4,1,file);
				instruction_count++;
				return 2;
			}
		}
	}
	else if(!strcmp(op.name,"bc2t") || !strcmp(op.name,"bc1t") || !strcmp(op.name,"bc2tl") || !strcmp(op.name,"bc1tl")){
		VS_CopyLabelName(new_line, line, len);
		
		if(!strcmp(op.name,"bc2t") || !strcmp(op.name,"bc2tl")){
			cop = 18;
		}
		else{
			cop = 17;
		}
		
		if(!strcmp(op.name,"bc2tl") || !strcmp(op.name,"bc1tl")){
			mips2 = 1;
		}
		else{
			mips2 = 0;
		}
		
		if(VS_FindSymbol(new_line)){
			sym = VS_GetSymbol(new_line);
			offset = sym.instruction_count - instruction_count;
			offset--;
			
			*instruction = cop << 26 | VS_Bin2Decimal(op.opcode) << 21 | mips2 << 17 | 1 << 16 | (offset & 0xFFFF);
			
			if(safe_load_delay){
				encode = cop << 26 | VS_Bin2Decimal(op.opcode) << 21 | mips2 << 17 | 1 << 16  | (offset & 0xFFFF);
				fwrite(&encode,4,1,file);
				encode = 0;
				fwrite(&encode,4,1,file);
				instruction_count++;
				return 2;
			}
		}
	}
	else if(!strcmp(op.name,"bal")){
		VS_CopyLabelName(new_line, line, len);
		
		if(VS_FindSymbol(new_line)){
			sym = VS_GetSymbol(new_line);
			offset = sym.instruction_count - instruction_count;
			offset--;
			
			*instruction = VS_Bin2Decimal(op.opcode) << 26 | 17 << 16 | (offset & 0xFFFF);
			
			if(safe_load_delay){
				encode = VS_Bin2Decimal(op.opcode) << 26 | 17 << 16 | (offset & 0xFFFF);
				fwrite(&encode,4,1,file);
				encode = 0;
				fwrite(&encode,4,1,file);
				instruction_count++;
				return 2;
			}
		}
		else{
			return -4;
		}
	}
	else{
		if(line[len] != '$' && syntax == VS_GNU_SYNTAX){
			return -1;
		}
		
		btype.op = VS_Bin2Decimal(op.opcode);
		btype.rs = VS_GetRegister(line + len, &size1);
		
		if(btype.rs == -1){
			return -2;
		}
		
		if(btype.rs == -2){
			return -9;
		}
		
		if(!strcmp(op.name,"bgezal") || !strcmp(op.name,"bltzal") || !strcmp(op.name,"bgezall") || !strcmp(op.name,"bltzall") || !strcmp(op.name,"bgezl")
			|| !strcmp(op.name,"bltzl")){
			VS_CopyLabelName(new_line, line, len + size1 + 1);
			
			if(VS_FindSymbol(new_line)){
				sym = VS_GetSymbol(new_line);
				offset = sym.instruction_count - instruction_count;
				offset--;
				
				*instruction = 1 << 26 | btype.rs << 21 | VS_Bin2Decimal(op.opcode) << 16 | (offset & 0xFFFF);
				
				if(safe_load_delay){
					encode = *instruction;
					fwrite(&encode,4,1,file);
					encode = 0;
					fwrite(&encode,4,1,file);
					instruction_count++;
					return 2;
				}
			}
			else{
				return -4;
			}
		}
		else if(!strcmp(op.name,"bgtz") || !strcmp(op.name,"blez") || !strcmp(op.name,"bltz") || !strcmp(op.name,"beqz") || !strcmp(op.name,"bnez")
			|| !strcmp(op.name,"bgez") || !strcmp(op.name,"bgtzl") || !strcmp(op.name,"blezl")){
				
			if(!strcmp(op.name,"bgez")){
				btype.rt = 1;
			}
			else{
				btype.rt = 0;
			}
			
			VS_CopyLabelName(new_line, line, len + size1 + 1);
			
			if(VS_FindSymbol(new_line)){
				sym = VS_GetSymbol(new_line);
				offset = sym.instruction_count - instruction_count;
				offset--;
				
				*instruction = VS_Bin2Decimal(op.opcode) << 26 | btype.rs << 21 | btype.rt << 16 | (offset & 0xFFFF);
				
				if(safe_load_delay){
					encode = VS_Bin2Decimal(op.opcode) << 26 | btype.rs << 21 | btype.rt << 16 | (offset & 0xFFFF);
					fwrite(&encode,4,1,file);
					encode = 0;
					fwrite(&encode,4,1,file);
					instruction_count++;
					return 2;
				}
			}
			else{
				return -4;
			}
		}
		else if(!strcmp(op.name,"blt") || !strcmp(op.name,"bltu") || !strcmp(op.name,"bgt") || !strcmp(op.name,"bgtu") || !strcmp(op.name,"bge")  || !strcmp(op.name,"bgeu") || !strcmp(op.name,"ble")){
			btype.rt = VS_GetRegister(line + len + size1 + 1, &size2);
			
			if(line[len + size1 + 1] != '$' && btype.rt < 0){
				return -1;
			}
			
			if(btype.rt == -1){
				return -2;
			}
			
			if(btype.rt == -2){
				return -9;
			}
			
			//rs rt

			VS_CopyLabelName(new_line, line, len + size1 + size2 + 2);
			
			if(VS_FindSymbol(new_line)){
				sym = VS_GetSymbol(new_line);
				offset = sym.instruction_count - instruction_count;
				offset -= 2;
				
				unsigned long instruction1;
				
				if(!strcmp(op.name,"bge") || !strcmp(op.name,"bgeu")){
					
					if(!strcmp(op.name,"bge")){
						pseudo_op = 42;
					}
					else{
						pseudo_op = 43;
					}
					
					instruction1 = btype.rs << 21 | btype.rt << 16 | VS_GetRegisterNumber("$at") << 11 | pseudo_op;
					
					fwrite(&instruction1,4,1,file);
					instruction1 = 4 << 26 | VS_GetRegisterNumber("$at") << 21 | 0 << 16 | (offset & 0xFFFF);
					fwrite(&instruction1,4,1,file);
					
					if(safe_load_delay){
						encode = 0;
						fwrite(&encode,4,1,file);
						instruction_count += 2;
						return 2;
					}
				}
				else if(!strcmp(op.name,"ble") || !strcmp(op.name,"bleu")){
					
					if(!strcmp(op.name,"ble")){
						pseudo_op = 42;
					}
					else{
						pseudo_op = 43;
					}
					
					instruction1 = btype.rt << 21 | btype.rs << 16 | VS_GetRegisterNumber("$at") << 11 | pseudo_op;
					
					fwrite(&instruction1,4,1,file);
					instruction1 = 4 << 26 | VS_GetRegisterNumber("$at") << 21 | 0 << 16 | (offset & 0xFFFF);
					fwrite(&instruction1,4,1,file);
					
					if(safe_load_delay){
						encode = 0;
						fwrite(&encode,4,1,file);
						instruction_count += 2;
						return 2;
					}
				}
				else{
					if(!strcmp(op.name,"blt")){
						instruction1 = btype.rs << 21 | btype.rt << 16 | VS_GetRegisterNumber("$at") << 11 | 42;
					}
					else if(!strcmp(op.name,"bltu")){
						instruction1 = btype.rs << 21 | btype.rt << 16 | VS_GetRegisterNumber("$at") << 11 | 43;
					}
					else if(!strcmp(op.name,"bgt")){
						instruction1 = btype.rt << 21 | btype.rs << 16 | VS_GetRegisterNumber("$at") << 11 | 42;
					}
					else{
						instruction1 = btype.rt << 21 | btype.rs << 16 | VS_GetRegisterNumber("$at") << 11 | 43;
					}
					
					fwrite(&instruction1,4,1,file);
					instruction1 = 5 << 26 | VS_GetRegisterNumber("$at") << 21 | 0 << 16 | (offset & 0xFFFF);
					fwrite(&instruction1,4,1,file);
					
					if(safe_load_delay){
						encode = 0;
						fwrite(&encode,4,1,file);
						instruction_count += 2;
						return 2;
					}
				}
				
				instruction_count++;
			}
			else{
				return -4;
			}
			
			return 2;
		}
		else{
			btype.rt = VS_GetRegister(line + len + size1 + 1, &size2);
			
			if(line[len + size1 + 1] != '$' && btype.rt < 0){
				return -1;
			}

			if(btype.rt == -1){
				return -2;
			}
			
			if(btype.rt == -2){
				return -9;
			}
			
			VS_CopyLabelName(new_line, line, len + size1 + size2 + 2);
			
			if(VS_FindSymbol(new_line)){
				sym = VS_GetSymbol(new_line);
				offset = sym.instruction_count - instruction_count;
				offset--;
				
				*instruction = VS_Bin2Decimal(op.opcode) << 26 | btype.rs << 21 | btype.rt << 16 | (offset & 0xFFFF);
				
				if(safe_load_delay){
					encode = VS_Bin2Decimal(op.opcode) << 26 | btype.rs << 21 | btype.rt << 16 | (offset & 0xFFFF);
					fwrite(&encode,4,1,file);
					encode = 0;
					fwrite(&encode,4,1,file);
					instruction_count++;
					return 2;
				}
			}
			else{
				return -4;
			}
		}
		
	}		
	
	return 1;
}

int VS_ParseAddrType(VS_OPCODE op, unsigned long* instruction, char* line, FILE* file){
	VS_ADDR_TYPE atype;
	VS_SYM sym;
	int len, is_valid_imm, reg;
	char new_line[VS_MAX_LINE+1];
	unsigned long end, size1;
	unsigned long start;
	unsigned char neg;
	
	len = strlen(op.name);
	atype.op = VS_Bin2Decimal(op.opcode);
	neg = 0;
	
	if(line[len] != '$' && syntax == VS_GNU_SYNTAX){
		return -1;
	}
	
	if(!strcmp(op.name,"li")){
		if(VS_GetNumberOfCommas(line) != 1){
			return -10;
		}
		
		reg = VS_GetRegister(line + len, &size1);
		
		if(reg == -1){
			return -2;
		}
		
		if(reg == -2){
			return -9;
		}
		
		VS_CopyLabelName(new_line, line + len - 1, VS_GetFirstOccurenceIndex(line,','));

		if(new_line[0] == '-'){
			neg = 1;
		}
		
		is_valid_imm = VS_IsValidImmediate(new_line);
		
		if(is_valid_imm == -1){
			return -5;
		}
		else if(is_valid_imm == -2){
		   return -6;	
		}
		
		start = (unsigned long)strtoul(new_line+neg, NULL, 0);
		
		if(neg){
			signed int negative = -start;
			
			if(negative <= -32678 || negative >= 32767){
				*instruction = 15 << 26 | reg << 16 | ((negative >> 16) & 0xFFFF);
				start = 13 << 26 | reg << 21 | reg << 16 | (negative & 0xFFFF);
				
				fwrite(instruction,4,1,file);
				fwrite(&start,4,1,file);
				
				instruction_count++;
				return 2;
			}
			else{
				*instruction = 9 << 26 | reg << 16 | (negative & 0xFFFF);
				return 1;
			}
		}
		else{
			
			if(start >= 65536){
				*instruction = 15 << 26 | reg << 16 | ((start >> 16) & 0xFFFF);
				start = 13 << 26 | reg << 21 | reg << 16 | (start & 0xFFFF);
				
				fwrite(instruction,4,1,file);
				fwrite(&start,4,1,file);
				
				instruction_count++;
			}
			else if(start <= 32767){
				*instruction = 9 << 26 | reg << 16 | (start & 0xFFFF);
				return 1;
			}
			else{
				*instruction = 13 << 26 | reg << 16 | (start & 0xFFFF);
				return 1;
			}
		}
		
		return 2;
	}
	else if(!strcmp(op.name,"la")){		
		reg = VS_GetRegister(line + len, &size1);
		
		if(VS_GetNumberOfCommas(line) != 1){
			return -10;
		}
		
		if(reg == -1){
			return -2;
		}
		
		if(reg == -2){
			if(VS_GetNumberOfCommas(line) == 0){
				return 0;
			}
			return -9;
		}
		
		VS_CopyLabelName(new_line, line + len - 1, VS_GetFirstOccurenceIndex(line,','));
		
		if(VS_FindSymbol(new_line)){
			sym = VS_GetSymbol(new_line);
			
			start = sym.addr;
			
			if(OEXE && sym.type == VS_SYM_FUNC){
				start += org;
			}
			
			VS_SetRelocTrue();
			VS_SetPSYQRelocTrue();
			
			if(sym.type == VS_SYM_OBJ){
				VS_AddRelocEntry((instruction_count << 2), VS_MIPS_HI_16, 0);
				VS_AddRelocEntry((instruction_count + 1) << 2, VS_MIPS_LO_16, 0);
				VS_AddPSYQRelocEntry(symbol_index, VS_PSYQ_HI_16, ((instruction_count - label_instruction_count) << 2), 3, start);
				VS_AddPSYQRelocEntry(symbol_index, VS_PSYQ_LO_16, (((instruction_count + 1) - label_instruction_count) << 2), 3, start);
			}
			else{
				VS_AddRelocEntry((instruction_count << 2), VS_MIPS_HI_16, 1);
				VS_AddRelocEntry(((instruction_count + 1) << 2), VS_MIPS_LO_16, 1);
				VS_AddPSYQRelocEntry(symbol_index, VS_PSYQ_HI_16, ((instruction_count - label_instruction_count) << 2), 2, start);
				VS_AddPSYQRelocEntry(symbol_index, VS_PSYQ_LO_16, (((instruction_count + 1) - label_instruction_count) << 2), 2, start);
			}
			
			end = 15 << 26 | reg << 16 | ((start >> 16) & 0xFFFF);
			start = 13 << 26 | reg << 21 | reg << 16 | (start & 0xFFFF);
			
			fwrite(&end,4,1,file);
			fwrite(&start,4,1,file);
			
			instruction_count++;
		}
		else{
			if(new_line[0] == '-'){
				neg = 1;
			}
			
			is_valid_imm = VS_IsValidImmediate(new_line);
		
			if(is_valid_imm == -1){
				return -5;
			}
			else if(is_valid_imm == -2){
			   return -6;	
			}
			
			start = (unsigned long)strtoul(new_line+neg, NULL, 0);
			
			if(neg){
				signed int negative = -start;
				
				*instruction = 15 << 26 | reg << 16 | ((negative >> 16) & 0xFFFF);
				start = 9 << 26 | reg << 21 | reg << 16 | (negative & 0xFFFF);
				
				fwrite(instruction,4,1,file);
				fwrite(&start,4,1,file);
			}
			else{
			
				*instruction = 15 << 26 | reg << 16 | ((start >> 16) & 0xFFFF);
				start = 9 << 26 | reg << 21 | reg << 16 | (start & 0xFFFF);
				
				fwrite(instruction,4,1,file);
				fwrite(&start,4,1,file);
			}
			
			instruction_count++;
		}
		
		return 2;
	}
	else{
		if(!strcmp(op.name,"lwc2") || !strcmp(op.name,"swc2")){
			reg = VS_GetCopRegister(line + len,&size1);
		}
		else{
			reg = VS_GetRegister(line + len, &size1);
		}

		if(reg == -1){
			return -2;
		}
		
		if(reg == -2){
			return -9;
		}
		
		atype.rt = reg;
		
		char* rs = strstr(line,"(");
		
		if(rs == NULL){
			return 0;
		}
		
		if(strstr(line,")") == NULL){
			return 0;
		}
		
		reg = VS_GetRegister(rs + 1,&size1);
		
		if(reg == -1){
			return -2;
		}
		
		if(reg == -2){
			return -9;
		}
		
		start = VS_GetFirstOccurenceIndex(line,',') + 1;
		end = VS_GetFirstOccurenceIndex(line,'(');
		memcpy(new_line, line + start,end-start);
		new_line[end-start] = '\0';
		
		if(end-start == 0){
			if(syntax != VS_ASMPSX_SYNTAX && VS_GetNumberOfCommas(line) < 1){
				return 0;
			}
		}
		
		if(new_line[0] == '-'){
			neg = 1;
		}
		
		is_valid_imm = VS_IsValidImmediate(new_line);
		
		if(is_valid_imm == -1){
			return -5;
		}
		else if(is_valid_imm == -2){
		   return -6;	
		}
			
		if(neg){
			atype.offset = (signed short)strtoul(new_line, NULL, 0);
		}
		else{
			atype.offset = (unsigned short)strtoul(new_line, NULL, 0);
		}
	
		atype.base = reg;
		
		if(safe_load_delay){
			if(!strcmp(op.name,"lw") || !strcmp(op.name,"lh") || !strcmp(op.name,"lhu") || !strcmp(op.name,"lbu") | !strcmp(op.name,"lb")){
				start = atype.op << 26 | atype.base << 21 | atype.rt << 16 | (atype.offset & 0xFFFF);
				fwrite(&start,4,1,file);
				start = 0;
				fwrite(&start,4,1,file);
				instruction_count++;
				return 2;
			}
		}
		
		if((!strcmp(op.name,"lw") || !strcmp(op.name,"sw")) && (atype.offset % 4)){
			*instruction = atype.op << 26 | atype.base << 21 | atype.rt << 16 | (atype.offset & 0xFFFF);
			return 3;
		}
		else if((!strcmp(op.name,"lh") || !strcmp(op.name,"lhu") || !strcmp(op.name,"sh")) && (atype.offset % 2)){
			*instruction = atype.op << 26 | atype.base << 21 | atype.rt << 16 | (atype.offset & 0xFFFF);
			return 3;
		}
	}
	
	*instruction = atype.op << 26 | atype.base << 21 | atype.rt << 16 | (atype.offset & 0xFFFF);
	
	return 1;
}

int VS_ParseMoveType(VS_OPCODE op, unsigned long* instruction, char* line){
	VS_MOVE_TYPE mtype;
	unsigned long size1, num_commas;
	int len;
	
	len = strlen(op.name);
	mtype.op = VS_Bin2Decimal(op.opcode);
	
	if(line[len] != '$' && syntax == VS_GNU_SYNTAX){
		return -1;
	}
	
	num_commas = VS_GetNumberOfCommas(line);

	if(num_commas >= 1){
		return -12;
	}
	
	mtype.rd = VS_GetRegister(line + len, &size1);
	
	if(mtype.rd == -1){
		return -2;
	}
	
	if(mtype.rd == -2){
		return -9;
	}
	
	if(!strcmp(op.name,"mthi") || !strcmp(op.name,"mtlo")){
		*instruction = mtype.rd << 21 | mtype.op;
	}
	else{
		*instruction = mtype.rd << 11 | mtype.op;
	}
	
	return 1;
}

int VS_ParseCopType(VS_OPCODE op, unsigned long* instruction, char* line){
	VS_COP_TYPE ctype;
	unsigned long size1, size2, offset;
	int len, is_valid_imm;
	
	len = strlen(op.name);
	ctype.op = VS_Bin2Decimal(op.opcode);
	
	if(!strcmp(op.name,"cop2")){
		char* cofunc = line + strlen("cop2");
		
		is_valid_imm = VS_IsValidImmediate(cofunc);
		
		if(is_valid_imm == -1){
			return -5;
		}
		else if(is_valid_imm == -2){
		   return -6;	
		}
		
		offset = (unsigned long)strtoul(cofunc, NULL, 0);
		
		*instruction = VS_Bin2Decimal(op.opcode) << 26 | 1 << 25 | (offset & 0x1ffffff);
		
		return 1;
	}
	
	if(VS_GetNumberOfCommas(line) != 1){
		return 0;
	}
	
	if(line[len] != '$' && syntax == VS_GNU_SYNTAX){
		return -1;
	}
	
	ctype.rt = VS_GetRegister(line + len, &size1);
	
	if(ctype.rt == -1){
		return -2;
	}
	
	if(ctype.rt == -2){
		return -9;
	}
	
	ctype.rd = VS_GetCopRegister(line + len + size1 + 1, &size2);
	
	if(line[len + size1 + 1] != '$' && syntax == VS_GNU_SYNTAX){
		return -1;
	}
	
	if(ctype.rd == -1){
		return -13;
	}
	
	if(ctype.rd == -2){
		return -9;
	}
	
	if(!strcmp(op.name,"cfc2")){
		*instruction = 18 << 26 | 2 << 21 | ctype.rt << 16 | ctype.rd << 11;
	}
	else if(!strcmp(op.name,"ctc2")){
		*instruction = 18 << 26 | 6 << 21 | ctype.rt << 16 | ctype.rd << 11;
	}
	else if(!strcmp(op.name,"mfc0") || !strcmp(op.name,"mfc2")){
		*instruction = VS_Bin2Decimal(op.opcode) << 26 | 0 << 21 | ctype.rt << 16 | ctype.rd << 11;
	}
	else if(!strcmp(op.name,"mtc0") || !strcmp(op.name,"mtc2")){
		*instruction = VS_Bin2Decimal(op.opcode) << 26 | 4 << 21 | ctype.rt << 16 | ctype.rd << 11;
	}
	else{
		*instruction = 18 << 26 | 4 << 21 | ctype.rt << 16 | ctype.rd << 11;
	}
	
	return 1;
}

int VS_ParseFloatType(VS_OPCODE op, unsigned long* instruction, char* line, FILE* file){
	VS_R_TYPE rtype;
	unsigned long instr, size1, size2, size3;
	int len, reg, short_hand, is_valid_imm, is_valid_float, neg, offset;
	char new_line[VS_MAX_LINE+1];
	float fvalue;
	
	memset(&rtype,0x0,sizeof(VS_R_TYPE));
	
	len = strlen(op.name);
	rtype.op = VS_Bin2Decimal(op.opcode);
	short_hand = 0;
	neg = 0;
	
	if(!strcmp(op.name,"sync")){
		*instruction = 15;
		return 1;
	}
	
	if(line[len] != '$' && syntax == VS_GNU_SYNTAX){
		return -1;
	}
	
	rtype.rd = VS_GetFpRegister(line + len, &size1);
	
	if(rtype.rd == -1){
		return -2;
	}
	
	if(rtype.rd == -2){
		if(!strcmp(op.name,"cfc1")){
			rtype.rd = VS_GetRegister(line + len, &size1);
		}
		else if(!strcmp(op.name,"ctc1")){
			rtype.rd = VS_GetRegister(line + len, &size1);
		}
		else if(!strcmp(op.name,"mfc1")){
			rtype.rd = VS_GetRegister(line + len, &size1);
		}
		else if(!strcmp(op.name,"mtc1")){
			rtype.rd = VS_GetRegister(line + len, &size1);
		}
		else return -8;
	}
	
	if(!strcmp(op.name,"li.s")){
		if(VS_GetNumberOfCommas(line) != 1){
			return -10;
		}
		
		char* float_line = line + len + size1 + 1;

		if(float_line[0] == '-'){
			neg = 1;
		}
		
		is_valid_float = VS_FloatDFA(float_line + neg);
		
		if(!is_valid_float){
			return -11;
		}
		
		fvalue = (float)atof(float_line);
		
		memcpy(&size1,&fvalue,sizeof(fvalue));
		
		instr = 15 << 26 | 1 << 16 | ((size1 >> 16) & 0xFFFF); /* lui */
		fwrite(&instr,4,1,file);
		instr = 13 << 26 | 1 << 21 | 1 << 16 | (size1 & 0xFFFF); /* ori */
		fwrite(&instr,4,1,file);
		instr = VS_COP_1 << 26 | 4 << 21 | 1 << 16 | rtype.rd << 11; /* mtc1 */
		fwrite(&instr,4,1,file);
		instr = 0;                 /* nop */
		fwrite(&instr,4,1,file);
		
		instruction += 3;
		
		return 2;
	}
	
	rtype.rs = VS_GetFpRegister(line + len + size1 + 1, &size2);
	
	if(line[len + size1 + 1] != '$' && rtype.rs < 0){
		if((!strcmp(op.name,"abs.s") || !strcmp(op.name,"sqrt.s") || !strcmp(op.name,"neg.s")) && VS_GetNumberOfCommas(line) == 0){
			*instruction = VS_COP_1 << 26 | VS_SINGLE_POINT_PRECISION << 21 | rtype.rd << 11 | rtype.rd << 6 | rtype.op;
			return 1;
		}
		else if(!strcmp(op.name,"lwc1") || !strcmp(op.name,"swc1")){
			int start, end;
			char* rs = strstr(line,"(");
		
			if(rs == NULL){
				return 0;
			}
			
			if(strstr(line,")") == NULL){
				return 0;
			}
			
			reg = VS_GetRegister(rs + 1,&size1);
			
			if(reg == -1){
				return -2;
			}
			
			if(reg == -2){
				return -9;
			}
			
			start = VS_GetFirstOccurenceIndex(line,',') + 1;
			end = VS_GetFirstOccurenceIndex(line,'(');
			memcpy(new_line, line + start,end-start);
			new_line[end-start] = '\0';
			
			if(end-start == 0){
				if(syntax != VS_ASMPSX_SYNTAX && VS_GetNumberOfCommas(line) < 1){
					return 0;
				}
			}
			
			if(new_line[0] == '-'){
				neg = 1;
			}
		
			is_valid_imm = VS_IsValidImmediate(new_line);
			
			if(is_valid_imm == -1){
				return -5;
			}
			else if(is_valid_imm == -2){
			   return -6;	
			}
				
			if(neg){
				offset = (signed short)strtoul(new_line, NULL, 0);
			}
			else{
				offset = (unsigned short)strtoul(new_line, NULL, 0);
			}
			
			*instruction = rtype.op << 26 | reg << 21 | rtype.rd << 16 | (offset & 0xFFFF);
			
			return 1;
		}
		else return -1;
	}
	
	if(rtype.rs == -1){
		return -2;
	}
	
	if(rtype.rs == -2){
		return -8;
	}
	
	if(!strcmp(op.name,"abs.s") || !strcmp(op.name,"sqrt.s") || !strcmp(op.name,"cvt.w.s")  || !strcmp(op.name,"ceil.w.s") || !strcmp(op.name,"floor.w.s") || 
	!strcmp(op.name,"round.w.s") || !strcmp(op.name,"mov.s") || !strcmp(op.name,"neg.s") || !strcmp(op.name,"trunc.w.s")){
		*instruction = VS_COP_1 << 26 | VS_SINGLE_POINT_PRECISION << 21 | rtype.rs << 11 | rtype.rd << 6 | rtype.op;
		return 1;
	}
	
	if(!strcmp(op.name,"cfc1") || !strcmp(op.name,"ctc1") || !strcmp(op.name,"mfc1") || !strcmp(op.name,"mtc1")){
		*instruction = VS_COP_1 << 26 | rtype.op << 21 | rtype.rd << 16 | rtype.rs << 11;
		return 1;
	}
	
	if(VS_IsFloatingPointComparison(op.name)){
		*instruction = VS_COP_1 << 26 | VS_SINGLE_POINT_PRECISION << 21 | rtype.rs << 16 | rtype.rd << 11 | 3 << 4 | VS_GetConditionField(op.name);
		return 1;
	}
	
	rtype.rt = VS_GetFpRegister(line + len + size1 + 2 + size2, &size3);
	
	if(line[len + size1 + 2 + size2] != '$' && rtype.rt < 0){
		if((!strcmp(op.name,"add.s") || !strcmp(op.name,"div.s") || !strcmp(op.name,"mul.s") || !strcmp(op.name,"sub.s")) && VS_GetNumberOfCommas(line) == 1){
			short_hand = 1;
			rtype.rt = rtype.rs;
			rtype.rs = rtype.rd;
		}
		else return -1;
	}
	
	if(!short_hand){
		if(rtype.rt == -1){
			return -2;
		}
		
		if(rtype.rt == -2){
			return -8;
		}
	}
	
	*instruction = VS_COP_1 << 26 | VS_SINGLE_POINT_PRECISION << 21 | rtype.rt << 16 | rtype.rs << 11 | rtype.rd << 6 | rtype.op;
	
	return 1;
}

int VS_ParseVFPUType(VS_OPCODE op, unsigned long* instruction, char* line){
	VS_VFPU_TYPE vtype;
	unsigned long size1;
	int len, size, is_valid_imm, neg, offset, num_commas;
	char reg_str[5], cond_str[3], new_line[VS_MAX_LINE+1];
	unsigned char cond;
	
	len = strlen(op.name);
	neg = 0;
	vtype.op = VS_Bin2Decimal(op.opcode);
	size = VS_GetVFPUSize(op.name);
	
	if(!strcmp(op.name,"vnop")){
		*instruction = 0xFFFF0000;
		return 1;
	}
	
	if(!strcmp(op.name,"vflush")){
		*instruction = 0xFFFF040D;
		return 1;
	}
	
	if(!strcmp(op.name,"vsync")){
		*instruction = 0xFFFF0320;
		return 1;
	}
	
	if(op.itype & VS_COND_INSTRUCTION){
		memcpy(cond_str,line+len,2);
		cond_str[2] = '\0';
		
		cond = VS_GetVFPUComparison(cond_str);
		
		if(cond == 16){
			return -15;
		}
		
		memcpy(reg_str,line+len+3,4);
		reg_str[4] = '\0';
		
		VS_UnderlineLine(reg_str);
		
		vtype.rd = VS_GetVFPURegister(reg_str, size);
		
		if(vtype.rd == -1){
			return -14;
		}
		
		memcpy(reg_str,line+len+8,4);
		reg_str[4] = '\0';
		
		VS_UnderlineLine(reg_str);
		
		vtype.rs = VS_GetVFPURegister(reg_str, size);
		
		if(vtype.rs == -1){
			return -14;
		}
		
		switch(size){
			case 0:{
				*instruction = vtype.op << 23 | vtype.rs << 16 | vtype.rd << 8 | cond;
			}break;
			case 1:{
				*instruction = vtype.op << 23 | vtype.rs << 16 | vtype.rd << 8 | 1 << 7 | cond;
			}break;
			case 2:{
				*instruction = vtype.op << 23 | vtype.rs << 16 | 1 << 15 | vtype.rd << 8 | cond;
			}break;
			case 3:{
				*instruction = vtype.op << 23 | vtype.rs << 16 | 1 << 15 | vtype.rd << 8 | 1 << 7 | cond;
			}break;
		}
		
		return 1;
	}
	
	memcpy(reg_str,line+len,4);
	reg_str[4] = '\0';
	
	VS_UnderlineLine(reg_str);
	
	vtype.rd = VS_GetVFPURegister(reg_str, size);
	
	if(vtype.rd == -1){
		return -14;
	}
	
	if(op.itype & VS_ADDR_INSTRUCTION){
		int start, end;
		char* rs = strstr(line,"(");
	
		if(rs == NULL){
			return 0;
		}
		
		if(strstr(line,")") == NULL){
			return 0;
		}
		
		vtype.rs = VS_GetRegister(rs + 1,&size1);
		
		if(vtype.rs == -1){
			return -2;
		}
		
		if(vtype.rs == -2){
			return -9;
		}
		
		start = VS_GetFirstOccurenceIndex(line,',') + 1;
		end = VS_GetFirstOccurenceIndex(line,'(');
		memcpy(new_line, line + start,end-start);
		new_line[end-start] = '\0';
		
		if(end-start == 0){
			if(syntax != VS_ASMPSX_SYNTAX && VS_GetNumberOfCommas(line) < 1){
				return 0;
			}
		}
		
		if(new_line[0] == '-'){
			neg = 1;
		}
	
		is_valid_imm = VS_IsValidImmediate(new_line);
		
		if(is_valid_imm == -1){
			return -5;
		}
		else if(is_valid_imm == -2){
		   return -6;	
		}
			
		if(neg){
			offset = (signed short)strtoul(new_line, NULL, 0);
		}
		else{
			offset = (unsigned short)strtoul(new_line, NULL, 0);
		}
		
		if(!strcmp(op.name,"lvr.q") || !strcmp(op.name,"svr.q")){
			*instruction = vtype.op << 26 | vtype.rs << 21 | (vtype.rd & 0x1f) << 16 | ((offset >> 2) & 0x3FFF) << 2 | (1 << 1) | ((vtype.rd >> 5) & 3);
		}
		else{
			*instruction = vtype.op << 26 | vtype.rs << 21 | (vtype.rd & 0x1f) << 16 | ((offset >> 2) & 0x3FFF) << 2 | ((vtype.rd >> 5) & 3);
		}
		
		if(offset % 4){
			return 3;
		}
	}
	
	if(op.itype & VS_R_INSTRUCTION){
		int args;
		memcpy(reg_str,line+len+5,4);
		reg_str[4] = '\0';
		
		num_commas = VS_GetNumberOfCommas(line);
		args = op.itype >> 29;
		
		if((num_commas+1) != args){
			return 60 + args;
		}
		
		VS_UnderlineLine(reg_str);
		
		if(args > 1){
			vtype.rs = VS_GetVFPURegister(reg_str, size);

			if(vtype.rs == -1){
				return -14;
			}
		}
		
		if(num_commas == 0){
			vtype.rt = 0;
			vtype.rs = 0;
			
			if(!strcmp(op.name,"vrand.s")){
				vtype.rs = vtype.rd;
				vtype.rd = 0;
			}
		}
		else if(num_commas == 1){
			vtype.rt = 0;
		}
		else{
			memcpy(reg_str,line+len+10,4);
			reg_str[4] = '\0';
			
			VS_UnderlineLine(reg_str);
		
			vtype.rt = VS_GetVFPURegister(reg_str, size);

			if(vtype.rt == -1){
				return -14;
			}
		}
		
		switch(size){
			case 0:{
				*instruction = vtype.op << 23 | vtype.rt << 16 | vtype.rs << 8 | vtype.rd;
			}break;
			case 1:{
				*instruction = vtype.op << 23 | vtype.rt << 16 | vtype.rs << 8 | 1 << 7 | vtype.rd;
			}break;
			case 2:{
				*instruction = vtype.op << 23 | vtype.rt << 16 | 1 << 15 | vtype.rs << 8 | vtype.rd;
			}break;
			case 3:{
				*instruction = vtype.op << 23 | vtype.rt << 16 | 1 << 15 | vtype.rs << 8 | 1 << 7 | vtype.rd;
			}break;
		}
		
		VS_AppendVFPUEncoding(op.name,instruction);
	}
	
	if(op.itype & VS_I_INSTRUCTION){
		int args;
		memcpy(reg_str,line+len+5,4);
		reg_str[4] = '\0';
		
		num_commas = VS_GetNumberOfCommas(line);
		args = op.itype >> 29;
		
		if((num_commas+1) != args){
			return 60 + args;
		}
		
		VS_UnderlineLine(reg_str);
		
		if(args > 2){
			vtype.rs = VS_GetVFPURegister(reg_str, size);

			if(vtype.rs == -1){
				return -14;
			}
			
			if(line[len+10] == '-'){
				return -6;
			}
			
			is_valid_imm = VS_IsValidImmediate(line + len + 10);
	
			if(is_valid_imm == -1){
				return -5;
			}
			else if(is_valid_imm == -2){
			   return -6;	
			}
			
			vtype.imm = (unsigned long)strtoul(line + len + 10, NULL, 0);
			
			if(vtype.imm > 31){
				return -6;
			}
			
			switch(size){
				case 0:{
					*instruction = vtype.op << 23 | (vtype.imm & 0x1f) << 16 | vtype.rs << 8 | vtype.rd;
				}break;
				case 1:{
					*instruction = vtype.op << 23 | (vtype.imm & 0x1f) << 16 | vtype.rs << 8 | 1 << 7 | vtype.rd;
				}break;
				case 2:{
					*instruction = vtype.op << 23 | (vtype.imm & 0x1f) << 16 | 1 << 15 | vtype.rs << 8 | vtype.rd;
				}break;
				case 3:{
					*instruction = vtype.op << 23 | (vtype.imm & 0x1f) << 16 | 1 << 15 | vtype.rs << 8 | 1 << 7 | vtype.rd;
				}break;
			}
		}
		else{
			if(line[len + 5] == '-'){
				neg = 1;
			}
			
			is_valid_imm = VS_IsValidImmediate(line + len + 5);
		
			if(is_valid_imm == -1){
				return -5;
			}
			else if(is_valid_imm == -2){
			   return -6;	
			}
				
			if(neg){
				offset = (signed long)strtoul(line + len + 5, NULL, 0);
				
				if(offset <= -32768 || offset >= 32678){
					return -7;
				}
			}
			else{
				offset = (unsigned long)strtoul(line + len + 5, NULL, 0);
				
				if(offset >= 32768){
					return -7;
				}
			}

			*instruction = vtype.op << 24 | vtype.rd << 16 | (offset & 0x7fff);
		}

		VS_AppendVFPUEncoding(op.name,instruction);
	}
	
	if(op.itype & VS_MOVE_INSTRUCTION){
		
	}
	
	return 1;
}

int VS_ParseAssemblyFile(FILE* in, FILE* parse, VS_ASM_PARAMS* params){
	FILE* text;
	VS_OPCODE op;
	unsigned long instruction;
	int instr_index, dir, error, err, first_occurence, line_count = 0;
	char line[VS_MAX_LINE+1];
	char dest[VS_MAX_LINE+1];
	char* arr[2];
	
	arr[0] = "jal";
	arr[1] = "bal";
	
	OEXE = params->oexe;
	org = params->org;
	
	text = fopen("textsec.dat","wb");
	
	instruction_count = 0;
	safe_load_delay = 0;
	symbol_index = -1;
	label_instruction_count = 0;
	syntax = params->syntax;
	
	while(VS_ReadLine(parse, line)){
		line_count++;
		
		instr_index = VS_LineContainsInstruction(line,params);
		dir = VS_LineContainsDirective(line);
		
		if(instr_index == -1 && strstr(line,":") == NULL && dir == -1 && strlen(line) > 1){
			err = 1;
			char* string;
			
			int i;
			for(i = 0; i < 2; i++){
				string = strstr(line,arr[i]);
				
				if(string != NULL){
					string += strlen(arr[i]);
					
					VS_TrimStrictLine(dest,string);
					
					if(!VS_FindSymbol(dest)){
						printf("Syntax Error: Label not found\n");
						
						int incerr = VS_ErrorOccuredInIncludeEntry(line_count);
					
						if(incerr != -1){
							VS_PrintErrorFromIncludeEntry(incerr,line_count);
						}
						else{
							VS_PrintError(in,line,line_count);
						}
						
						fclose(text);
						return -1;
					}
					else{
						instr_index = VS_GetOpcode(arr[i]);
						err = 0;
					}
				}
			}
			
			if((line[0] == 'b' || line[0] == 'j') && err == 1){
				if(line[0] == 'b'){
					string = strstr(line,"b") + 1;
				}
				else{
					string = strstr(line,"j") + 1;
				}
				
				VS_TrimStrictLine(dest,string);
				
				if(!VS_FindSymbol(dest)){
					printf("Syntax Error: Label not found\n");
					
					int incerr = VS_ErrorOccuredInIncludeEntry(line_count);
				
					if(incerr != -1){
						VS_PrintErrorFromIncludeEntry(incerr,line_count);
					}
					else{
						VS_PrintError(in,line,line_count);
					}
					
					fclose(text);
					return -1;
				}
				else{
					err = 0;
					
					if(line[0] == 'b'){
						instr_index = VS_GetOpcode("b");
					}
					else{
						instr_index = VS_GetOpcode("j");
					}
				}
			}
			
			if(err){
				printf("Error: Unrecognized assembler instruction\n");
				
				int incerr = VS_ErrorOccuredInIncludeEntry(line_count);
				
				if(incerr != -1){
					VS_PrintErrorFromIncludeEntry(incerr,line_count);
				}
				else{
					VS_PrintError(in,line,line_count);
				}
				
				fclose(text);
				
				return -1;
			}
		}
		
		char* label = strstr(line,":");
		
		if(label != NULL){
			first_occurence = VS_GetFirstOccurenceIndex(line,':');
			memcpy(dest,line,first_occurence);
			dest[first_occurence] = '\0';
			
			if(VS_FindSymbol(dest)){
				symbol_index++;
				label_instruction_count = instruction_count;
			}
		}
		
		if(strstr(line,".safeloaddelay") != NULL){
			safe_load_delay = 1;
		}
		
		if(strstr(line,".safeloadoff") != NULL){
			safe_load_delay = 0;
		}
		
		if(instr_index != -1 && VS_LineContainsDirective(line) == -1 && strstr(line,":") == NULL){
			VS_GetOpcodeFromIndex(&op,instr_index);
			instruction = 0;
			error = 0;
			
			//printf("name = %s\n",op.name);
			//printf("line = %s\n",line);
			
			if(!(op.arch & params->architecture)){
				printf("Error: %s architecture does not support the %s instruction\n",VS_GetArchitectureName(params->architecture), op.name);
				
				int incerr = VS_ErrorOccuredInIncludeEntry(line_count);
				
				if(incerr != -1){
					VS_PrintErrorFromIncludeEntry(incerr,line_count);
				}
				else{
					VS_PrintError(in,line,line_count);
				}
				
				fclose(text);
				return -1;
			}

			switch(op.itype){
				case VS_R_INSTRUCTION:{
					error = VS_ParseRType(op,&instruction,line,text);
				}break;
				case VS_I_INSTRUCTION:{
					error = VS_ParseIType(op,&instruction,line,text);
				}break;
				case VS_J_INSTRUCTION:{
					error = VS_ParseJType(op,&instruction,line,text);
				}break;
				case VS_B_INSTRUCTION:{
					error = VS_ParseBType(op,&instruction,line,text);
				}break;
				case VS_ADDR_INSTRUCTION:{
					error = VS_ParseAddrType(op,&instruction,line,text);
				}break;
				case VS_MOVE_INSTRUCTION:{
					error = VS_ParseMoveType(op,&instruction,line);
				}break;
				case VS_COP_INSTRUCTION:{
					error = VS_ParseCopType(op,&instruction,line);
				}break;
				case VS_FLOAT_INSTRUCTION:{
					error = VS_ParseFloatType(op,&instruction,line,text);
				}break;
				case VS_VFPU_INSTRUCTION: break;
			}
			
			if(op.itype & VS_VFPU_INSTRUCTION){
				error = VS_ParseVFPUType(op,&instruction,line);
			}
			
			instruction_count++;
			
		
			if(error == -10){
				printf("Syntax Error: instruction '%s' requires one argument in the form '%s reg1, arg1'\n",op.name,op.name);
			}
			else if(error == -12){
				printf("Syntax Error: instruction '%s' requires one destination register in the form '%s reg1'\n",op.name,op.name);
			}

			else if(error >= 60){
				printf("Syntax Error: instruction '%s' requires %d registers\n",op.name,error-60);
			}
			else if(error != 1 && error != 2){
				VS_PrintErrorString(error);
			}
			
			if(error <= 0 || error >= 60){
				int incerr = VS_ErrorOccuredInIncludeEntry(line_count);
				
				if(incerr != -1){
					VS_PrintErrorFromIncludeEntry(incerr,line_count);
				}
				else{
					VS_PrintError(in,line,line_count);
				}
				
				fclose(text);
				return -1;
			}
			
			//printf("instruction = %d\n",instruction_count);
			//printf("name = %s\n",op.name);
			//printf("line = %s\n",line);
			
			if(error != 2)
				fwrite(&instruction,4,1,text);
		}
	}
	
	fclose(text);

	return 1;
}