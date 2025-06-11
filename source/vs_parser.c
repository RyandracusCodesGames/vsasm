#include <vs_parser.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <vs_preprocessor.h>
#include <vs_symtable.h>
#include <vs_utils.h>
#include <vs_elf.h>
#include <vs_psyqobj.h>
#include <vs_exp_parser.h>

/********************************************
*   VideoStation Assembler
*
*   Copyright (c) 2025 Ryandracus Chapman
*
*   File: vs_parser.c
*   Date: 4/29/2025
*   Version: 1.1
*   Updated: 6/11/2025
*   Author: Ryandracus Chapman
*
********************************************/

int sym_index, symbol_index, label_instruction_count, instruction_count, safe_load_delay, OEXE;

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
	if(line[0] == '0' && line[1] == 'x'){
		line += 2;
	}
	
	int i, len = strlen(line);
	for(i = 0; i < len; i++){
		if(line[i] != '\n' && !isxdigit(line[i])){
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

int VS_GetRegister(char* line, unsigned long* size_out, VS_ASM_PARAMS* params){
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
	
	if(!VS_IsValidRegister(reg_str,params->syntax)){
		return -2;
	}
	else return VS_GetRegisterNumber(reg_str);
}

int VS_GetFpRegister(char* line, unsigned long* size_out, VS_ASM_PARAMS* params){
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
	
	if(!VS_IsValidFpRegister(reg_str,params->syntax)){
		if(!VS_IsValidRegister(reg_str,params->syntax)){
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

int VS_IsValidImmediate(char* line, VS_ASM_PARAMS* params){
	int is_valid_hex, is_valid_int;
	
	if(line[0] == '-'){
		line++;
	}
	
	if(params->syntax == VS_ASMPSX_SYNTAX && line[0] == '$'){
		line++;
		
		is_valid_hex = VS_HexDFA(line);
		
		if(is_valid_hex){
			return 1;
		}
		else return -1;
	}
	else if(line[0] == '0' && line[1] == 'x'){
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

int VS_IsValidReinterpretableRType(const char* name){
	 return (!strcmp(name,"add") || !strcmp(name,"addu") || !strcmp(name,"and") || !strcmp(name,"sub") || !strcmp(name,"subu") 
			|| !strcmp(name,"or") || !strcmp(name,"xor") || !strcmp(name,"nor") || !strcmp(name,"slt") || !strcmp(name,"sltu"));
}

int VS_IsValidRegImmType(const char* name){
	return (!strcmp(name,"bgezal") || !strcmp(name,"bltzal") || !strcmp(name,"bgezall") || !strcmp(name,"bltzall") || !strcmp(name,"bgezl")
			|| !strcmp(name,"bltzl"));
}

int VS_IsValidBranchOnZeroType(const char* name){
	return (!strcmp(name,"bgtz") || !strcmp(name,"blez") || !strcmp(name,"bltz") || !strcmp(name,"beqz") ||  !strcmp(name,"beqz") || !strcmp(name,"bnez")
			|| !strcmp(name,"bgez") || !strcmp(name,"bgtzl") || !strcmp(name,"blezl") || !strcmp(name,"beqzl") || !strcmp(name,"bnezl"));
}

int VS_IsValidSymbolAddrOperator(char c){
	return (c == '>' || c == '<');
}

int VS_ParseImmediateValue(char* immediate, VS_ASM_PARAMS* params){
	int imm, neg;
	char imm_str[256];
	
	neg = 0;
	
	if(immediate[0] == '-'){
		neg = 1;
	}
	
	if(neg){
		if(params->syntax == VS_ASMPSX_SYNTAX && immediate[1] == '$'){
			memset(imm_str,'\0',256);
			imm_str[0] = '-'; imm_str[1] = '0'; imm_str[2] = 'x';
			imm = (signed int)strtol(strcat(imm_str,immediate+2), NULL, 0);
		}
		else{
			imm = (signed int)strtol(immediate, NULL, 0);
		}
	}
	else{
		if(params->syntax == VS_ASMPSX_SYNTAX && immediate[0] == '$'){
			memset(imm_str,'\0',256);
			imm_str[0] = '0'; imm_str[1] = 'x';
			imm = (unsigned long)strtoul(strcat(imm_str,immediate+1), NULL, 0);
		}
		else{
			imm = (unsigned long)strtoul(immediate, NULL, 0);
		}
	}
	
	return imm;
}

int VS_IsValidRegisterPrefix(char* line, int len, VS_SYNTAX syntax){
	if(line[len] != '$' && syntax == VS_GNU_SYNTAX){
		return -1;
	}
	
	if(line[len] == '$' && syntax == VS_ASMPSX_SYNTAX){
		return -16;
	}
	
	return 1;
}

int VS_WriteLoadDelay(FILE* file, unsigned long* instruction, VS_ENDIAN endian){
	if(safe_load_delay){
		VS_WriteInstruction(file,*instruction,endian);
		VS_WriteNop(file);
		instruction_count++;
		return 2;
	}
	
	return 1;
}

int VS_ReinterpretRTypeAsIType(FILE* file, const char* name, int rd, int rt, char* operands, unsigned long* instruction, VS_ASM_PARAMS* params){
	VS_OPCODE opcode;
	long expr;
	int neg, imm, is_valid_imm;
	char line[VS_MAX_LINE];
	
	VS_InitExprParser();
	
	neg = 0;
	
	if(VS_StrictIsStringBlank(operands)){
		return 0;
	}
	
	if(operands[0] == '-'){
		neg = 1;
	}
	
	if(VS_LineContainsOperator(operands+neg)){		
		expr = VS_IsValidExpression(operands, params->syntax);
		
		if(!expr){
			return -17;
		}
		
		expr = VS_EvaluateExpr(operands, params->syntax);
		sprintf(operands,"%ld",expr);
		is_valid_imm = VS_IsValidImmediate(operands, params);
	}
	else{
		is_valid_imm = VS_IsValidImmediate(operands, params);
	}

	if(is_valid_imm == -1){
		return -5;
	}
	else if(is_valid_imm == -2){
	   return -6;	
	}
	
	imm = VS_ParseImmediateValue(operands, params);
	
	if(neg){
		if(imm <= -32768 || imm >= 32768){
			return -7;
		}
	}
	else{
		if(imm >= 65536){
			return -7;
		}
	}
	
	if(!strcmp(name,"addu")){
		VS_GetOpcodeFromIndex(&opcode,VS_GetOpcode("addiu"));
		*instruction = VS_Bin2Decimal(opcode.opcode) << 26 | rd << 21 | rt << 16 | (imm & 0xFFFF);
	}
	else{
		char itype_instruction[VS_MAX_LINE];
		
		if(params->syntax == VS_GNU_SYNTAX){
			strcpy(itype_instruction,name);
			strcat(itype_instruction,"i");
			VS_GetOpcodeFromIndex(&opcode,VS_GetOpcode(itype_instruction));
			sprintf(line,"%s$%d,$%d,%d",opcode.name,rd,rt,imm & 0xFFFF);
			return VS_ParseIType(opcode,instruction,line,file,params);
		}
		else{
			strcpy(itype_instruction,name);
			strcat(itype_instruction,"i");
			VS_GetOpcodeFromIndex(&opcode,VS_GetOpcode(itype_instruction));
			sprintf(line,"%sr%d,r%d,%d",opcode.name,rd,rt,imm & 0xFFFF);
			return VS_ParseIType(opcode,instruction,line,file,params);
		}
	}
		
	return 1;
}

int VS_ReinterpretITypeAsRType(FILE* file, const char* name, int rt, int rs, int imm, unsigned long* instruction, VS_ASM_PARAMS* params){
	VS_OPCODE opcode;
	unsigned long instr;
	unsigned char len;
	char itype_instruction[10];
	char line[VS_MAX_LINE];
	
	len = strlen(name);
	
	instr = 15 << 26 | 1 << 16 | ((imm >> 16) & 0xFFFF); /* lui */
	VS_WriteInstruction(file,instr,params->endian);
	instr = 13 << 26 | 1 << 21 | 1 << 16 | (imm & 0xFFFF); /* ori */
	VS_WriteInstruction(file,instr,params->endian);
	memset(itype_instruction,'\0',10);
	
	unsigned long i, count;
	for(i = 0, count = 0; i < len; i++){
		if(name[i] != 'i'){
			itype_instruction[count++] = name[i];
		}
	}

	if(params->syntax == VS_GNU_SYNTAX){
		VS_GetOpcodeFromIndex(&opcode,VS_GetOpcode(itype_instruction));
		sprintf(line,"%s$%d,$%d,$%d",opcode.name,rt,rs,1);
		VS_ParseRType(opcode,instruction,line,file,params);
	}
	else{
		VS_GetOpcodeFromIndex(&opcode,VS_GetOpcode(itype_instruction));
		sprintf(line,"%sr%d,r%d,r%d",opcode.name,rt,rs,1);
		VS_ParseRType(opcode,instruction,line,file,params);
	}
		
	instruction_count += 2;
	
	return 1;
}

int VS_ParseRType(VS_OPCODE op, unsigned long* instruction, char* line, FILE* file, VS_ASM_PARAMS* params){
	VS_R_TYPE rtype;
	unsigned long size1, size2, size3;
	int len, is_valid_prefix, num_commas;
	
	VS_InitExprParser();
	
	memset(&rtype,0x0,sizeof(VS_R_TYPE));
	
	len = strlen(op.name);
	
	if(!strcmp(op.name,"syscall") || !strcmp(op.name,"break")){
		*instruction = VS_Bin2Decimal(op.opcode);
		return 1;
	}
	
	is_valid_prefix = VS_IsValidRegisterPrefix(line,len,params->syntax);
	
	if(is_valid_prefix != 1){
		return is_valid_prefix;
	}

	rtype.op = VS_Bin2Decimal(op.opcode);
	rtype.rd = VS_GetRegister(line + len, &size1, params);

	if(rtype.rd == -1){
		return -2;
	}
	
	num_commas = VS_GetNumberOfCommas(line);
	
	if(rtype.rd == -2){
		if(num_commas == 0){
			return 0;
		}
		else return -9;
	}

	if(!strcmp(op.name,"jr") || !strcmp(op.name,"jalr")){
		*instruction = rtype.rd << 21 | rtype.op;
		
		if(!strcmp(op.name,"jalr")){
			*instruction |= 31 << 11;
		}
		
		return VS_WriteLoadDelay(file,instruction,params->endian);
	}
	else if(!strcmp(op.name,"neg") && num_commas == 1){
		*instruction = VS_GetRegisterNumber("$0") << 21 | rtype.rd << 16 | rtype.rd << 11 | rtype.op;
		return 1;
	}
	
	rtype.rs = VS_GetRegister(line + len + size1 + 1, &size2, params);
	
	if(rtype.rs < 0){
		if(num_commas >= 1 && (!strcmp(op.name,"add") || !strcmp(op.name,"addu") || !strcmp(op.name,"and") || !strcmp(op.name,"sub") || !strcmp(op.name,"subu") 
			|| !strcmp(op.name,"or") || !strcmp(op.name,"xor"))){
			char* operands = line + len + size1 + 1;
			return VS_ReinterpretRTypeAsIType(file,op.name,rtype.rd,rtype.rd,operands,instruction,params);
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
	else if(!strcmp(op.name,"div") || !strcmp(op.name,"divu") || !strcmp(op.name,"mult") || !strcmp(op.name,"multu") ||
	!strcmp(op.name,"teq") || !strcmp(op.name,"tge") || !strcmp(op.name,"tgeu") || !strcmp(op.name,"tlt") || !strcmp(op.name,"tne")){
		*instruction = rtype.rd << 21 | rtype.rs << 16 | rtype.op;
		
		if(!strcmp(op.name,"div") || !strcmp(op.name,"divu")){
			if(rtype.rd == 0 || rtype.rs == 0){
				return 4;
			}
		}
		
		return 1;
	}
	
	rtype.rt = VS_GetRegister(line + len + size1 + size2 + 2, &size3, params);

	if(rtype.rt < 0){
		if(num_commas == 1 && VS_IsValidReinterpretableRType(op.name)){
			*instruction = rtype.rd << 21 | rtype.rs << 16 | rtype.rd << 11 | rtype.op;
			return 1;
		}
		else if(num_commas == 1 && (!strcmp(op.name,"sllv") || !strcmp(op.name,"srlv") || !strcmp(op.name,"srav"))){
			*instruction = rtype.rs << 21 | rtype.rd << 16 | rtype.rd << 11 | rtype.op;
			return 1;
		}
		else if(num_commas == 2 && params->syntax == VS_ASMPSX_SYNTAX && VS_IsValidReinterpretableRType(op.name)){
				
			char* operands = line + len + size1 + size2 + 2;
			char reg[10];
			
			if(VS_StrictIsStringBlank(operands)){
				return 0;
			}
			
			VS_TrimStrictLine(reg,operands);
			
			if(VS_IsValidFpRegister(reg,VS_ASMPSX_SYNTAX)){
				return -9;
			}

			return VS_ReinterpretRTypeAsIType(file,op.name,rtype.rd,rtype.rs,operands,instruction,params);
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
		VS_WriteInstruction(file,rtype.rs << 21 | rtype.rt << 16 | rtype.op,params->endian);
		*instruction = rtype.rd << 11 | 18; /* mflo */
		instruction_count++;
	}
	else if(!strcmp(op.name,"sllv") || !strcmp(op.name,"srav") || !strcmp(op.name,"srlv")){
		*instruction = rtype.rt << 21 | rtype.rs << 16 | rtype.rd << 11 | rtype.op;
	}
	else{
		*instruction = rtype.rs << 21 | rtype.rt << 16 | rtype.rd << 11 | rtype.op;
	}
	
	return 1;
}

int VS_ParseIType(VS_OPCODE op, unsigned long* instruction, char* line, FILE* file, VS_ASM_PARAMS* params){
	VS_I_TYPE itype;
	VS_SYM sym;
	unsigned long size1, size2, size3;
	long expr;
	int is_valid_imm, is_valid_prefix, len, neg;
	char* new_line, imm[256];
	char trim[VS_MAX_LINE];
	
	VS_InitExprParser();
	
	memset(&itype,0x0,sizeof(VS_I_TYPE));
	memset(imm,'\0',256);
	memset(trim,'\0',VS_MAX_LINE);
	
	len = strlen(op.name);
	imm[0] = '0'; imm[1] = 'x';
	neg = 0;
	is_valid_imm = 0;
	
	if(!strcmp(op.name,"nop")){
		return 1;
	}
	
	is_valid_prefix = VS_IsValidRegisterPrefix(line,len,params->syntax);
	
	if(is_valid_prefix != 1){
		return is_valid_prefix;
	}
	
	itype.op = VS_Bin2Decimal(op.opcode);
	itype.rt = VS_GetRegister(line + len, &size1, params);
	
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
		
		new_line = line + len + size1 + 1;
		
		VS_TrimStrictLine(trim,new_line);
		
		if(trim[0] == '-'){
			neg = 1;
		}

		is_valid_imm = VS_IsValidImmediate(trim, params);

		if(VS_FindSymbol(trim) && is_valid_imm < 0){
			sym = VS_GetSymbol(new_line);
			itype.imm = sym.addr & 0xFFFF;
		}
		else if(VS_IsValidSymbolAddrOperator(trim[0]) && VS_FindSymbol(trim+1) && is_valid_imm < 0){
			sym = VS_GetSymbol(trim+1);
			
			if(trim[0] == '>'){
				itype.imm = (sym.addr >> 16) & 0xFFFF;
			}
			else{
				itype.imm = sym.addr & 0xFFFF;
			}
		}
		else if(VS_LineContainsOperator(trim+neg) && is_valid_imm < 0){
			expr = VS_IsValidExpression(trim, params->syntax);
			
			if(!expr){
				return -17;
			}
			
			expr = VS_EvaluateExpr(trim, params->syntax);
			sprintf(trim,"%ld",expr);
			is_valid_imm = VS_IsValidImmediate(trim, params);
			
			if(is_valid_imm == -1){
				return -5;
			}
			else if(is_valid_imm == -2){
			   return -6;	
			}
			
			itype.imm = VS_ParseImmediateValue(trim, params);
		}
		else{
			if(is_valid_imm == -1){
				return -5;
			}
			else if(is_valid_imm == -2){
			   return -6;	
			}
			
			itype.imm = VS_ParseImmediateValue(trim, params);
		}
		
		*instruction = itype.op << 26 | itype.rt << 21 | itype.rs << 16 | (itype.imm & 0xFFFF);
	}
	else if(!strcmp(op.name,"teqi") || !strcmp(op.name,"tgei") || !strcmp(op.name,"tgeiu") || !strcmp(op.name,"tlti") || !strcmp(op.name,"tltiu") 
		|| !strcmp(op.name,"tnei")){

		if(line[len + size1 + 1] == '-'){
			neg = 1;
		}
			
		is_valid_imm = VS_IsValidImmediate(line + len + size1 + 1 + neg, params);

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
		
		itype.rs = VS_GetRegister(line + len + size1 + 1, &size2, params);

		if((line[len + size1 + 1] != '$' && params->syntax == VS_GNU_SYNTAX) || itype.rs < 0){
			
			char* operands = line + len + size1 + 1;
			
			if(VS_StrictIsStringBlank(operands)){
				return 0;
			}
	
			if(num_commas == 1){

				if(operands[0] == '-'){
					return -3;
				}
				
				if(VS_LineContainsOperator(operands)){
					expr = VS_IsValidExpression(operands, params->syntax);
					
					if(!expr){
						return -17;
					}
					
					expr = VS_EvaluateExpr(operands, params->syntax);
					sprintf(operands,"%ld",expr);
					is_valid_imm = VS_IsValidImmediate(operands, params);
				}
				else{
					is_valid_imm = VS_IsValidImmediate(operands, params);
				}
		
				if(is_valid_imm == -1){
					return -5;
				}
				else if(is_valid_imm == -2){
				   return -6;	
				}
				
				itype.imm = VS_ParseImmediateValue(operands, params);
		
				if(itype.imm < 0 || itype.imm > 31){
					return -3;
				}
				
				*instruction = itype.rt << 16 | itype.rt << 11 | (itype.imm & 0x1f) << 6 | itype.op;
				
				return 1;
			}else return -1;
		}
		else if(num_commas == 1){
			itype.rs = VS_GetRegister(line + len + size1 + 1, &size2, params);
		
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
		
		itype.imm = VS_GetRegister(operands, &size3, params);
	
		if(itype.imm == -2){
			if(operands[0] == '-'){
				neg = 1;
			}
			
			if(VS_LineContainsOperator(operands+neg)){
				expr = VS_IsValidExpression(operands, params->syntax);
				
				if(!expr){
					return -17;
				}
				
				expr = VS_EvaluateExpr(operands, params->syntax);
				sprintf(operands,"%ld",expr);
				is_valid_imm = VS_IsValidImmediate(operands, params);
			}
			else{
				is_valid_imm = VS_IsValidImmediate(operands, params);
			}
		
			if(is_valid_imm == -1){
				return -5;
			}
			else if(is_valid_imm == -2){
			   return -6;	
			}
			
			itype.imm = VS_ParseImmediateValue(operands, params);
			
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
		itype.rs = VS_GetRegister(line + len + size1 + 1, &size2, params);
		
		if((line[len + size1 + 1] != '$' && params->syntax == VS_GNU_SYNTAX) || itype.rs < 0){
			
			char* operands = line + len + size1 + 1;
			
			if(VS_StrictIsStringBlank(operands)){
				return 0;
			}
	
			int num_commas = VS_GetNumberOfCommas(line);
			
			if(operands[0] == '-'){
				neg = 1;
			}
			
			if(VS_LineContainsOperator(operands+neg)){		
				expr = VS_IsValidExpression(operands, params->syntax);
				
				if(!expr){
					return -17;
				}
				
				expr = VS_EvaluateExpr(operands, params->syntax);
				sprintf(operands,"%ld",expr);
				is_valid_imm = VS_IsValidImmediate(operands, params);
			}
			else{
				is_valid_imm = VS_IsValidImmediate(operands, params);
			}
			
			if(operands[0] == '-'){
				neg = 1;
			}
			
			itype.imm = VS_ParseImmediateValue(operands, params);
			
			if(!strcmp(op.name,"subi") || !strcmp(op.name,"subiu")){
				neg = 1;
				itype.imm = -itype.imm;
			}
			
			if((neg && (itype.imm <= -32678 || itype.imm >= 32767)) || itype.imm >= 65536){
				return VS_ReinterpretITypeAsRType(file,op.name,itype.rt,itype.rt,itype.imm,instruction,params);
			}

			*instruction = itype.op << 26 | itype.rt << 21 | itype.rt << 16 | (itype.imm & 0xFFFF);
			
			if((is_valid_imm == 1 || is_valid_imm == 2) && num_commas == 1){
				return 1;
			}
			else if(!(is_valid_imm == 1 || is_valid_imm == 2) && num_commas == 1){
				return -6;
			}else return -2;
		}
		
		new_line = line + len + size1 + 2 + size2;
		
		if(itype.rs == -1){
			return -2;
		}
		
		if(itype.rs == -2){
			return -9;
		}
		
		if(new_line[0] == '-'){
			neg = 1;
		}
		
		if(VS_GetNumberOfCommas(line + len + size1 + 1) == 0){
			return 0;
		}
		
		if(VS_LineContainsOperator(new_line+neg)){		
			expr = VS_IsValidExpression(new_line, params->syntax);
			
			if(!expr){
				return -17;
			}
			
			expr = VS_EvaluateExpr(new_line, params->syntax);
			sprintf(new_line,"%ld",expr);
			is_valid_imm = VS_IsValidImmediate(new_line, params);
		}
		else{
			is_valid_imm = VS_IsValidImmediate(new_line, params);
		}
		
		if(is_valid_imm == -1){
			return -5;
		}
		else if(is_valid_imm == -2){
		   return -6;	
		}
		
		itype.imm = VS_ParseImmediateValue(new_line, params);
		
		if(!strcmp(op.name,"subi") || !strcmp(op.name,"subiu")){
			neg = 1;
			itype.imm = -itype.imm;
		}
		
		if((neg && (itype.imm <= -32678 || itype.imm >= 32767)) || itype.imm >= 65536){
			return VS_ReinterpretITypeAsRType(file,op.name,itype.rt,itype.rs,itype.imm,instruction,params);
		}

		*instruction = itype.op << 26 | itype.rs << 21 | itype.rt << 16 | (itype.imm & 0xFFFF);
	}
	
	return 1;
}

int VS_ParseJType(VS_OPCODE op, unsigned long* instruction, char* line, FILE* file, VS_ASM_PARAMS* params){
	VS_SYM sym;
	unsigned long addr, symbol_ind;
	int len, is_valid_imm;
	char new_line[VS_MAX_LINE+1];
	
	len = strlen(op.name);

	VS_CopyLabelName(new_line, line, len);
	
	is_valid_imm = VS_IsValidImmediate(new_line, params);
	
	if(VS_FindSymbol(new_line)){
		sym = VS_GetSymbol(new_line);
		addr = sym.addr;
		
		if(OEXE){
			addr += params->org;
		}
		
		if(!strcmp(op.name,"j") || !strcmp(op.name,"jal")){
			VS_SetRelocTrue();
			VS_SetPSYQRelocTrue();
			
			if(sym.type != VS_SYM_UND){
				VS_AddRelocEntry((instruction_count << 2), VS_MIPS_26, 1);
				VS_AddPSYQRelocEntry(symbol_index, VS_PSYQ_26, ((instruction_count - label_instruction_count) << 2), 2, addr);
			}
			else{
				VS_AddUndefinedRelocEntry((instruction_count << 2), VS_MIPS_26, sym.string_table_index + 8);
				VS_AddUndefinedPSYQRelocEntry(symbol_index, VS_PSYQ_26, ((instruction_count - label_instruction_count) << 2), 2, sym.string_table_index + 8);
			}
		}
		
		*instruction = VS_Bin2Decimal(op.opcode) << 26 | (((addr >> 2) & 0x3FFFFFF));
		
		return VS_WriteLoadDelay(file,instruction,params->endian);
	}
	else if(is_valid_imm != -1 && is_valid_imm != -2){
		addr = (unsigned long)strtoul(new_line, NULL, 0);
		
		*instruction = VS_Bin2Decimal(op.opcode) << 26 | (((addr >> 2) & 0x3FFFFFF));
		
		return VS_WriteLoadDelay(file,instruction,params->endian);
	}
	else if((new_line[0] == '0' && new_line[1] == 'x') || (new_line[0] == '$' && params->syntax == VS_ASMPSX_SYNTAX)){
		return -5;
	}
	else if(params->undefsym){
		VS_TrimStrictLine(new_line,line + strlen(op.name));
		symbol_ind = VS_AddSymbol(new_line,instruction_count,0,VS_SYM_UND,VS_SCOPE_GLOBAL);
		VS_SetRelocTrue();
		VS_SetPSYQRelocTrue();
		VS_AddUndefinedRelocEntry((instruction_count << 2), VS_MIPS_26, symbol_ind + 8);
		VS_AddUndefinedPSYQRelocEntry(symbol_index, VS_PSYQ_26, ((instruction_count - label_instruction_count) << 2), 2, symbol_ind + 8);
		*instruction = VS_Bin2Decimal(op.opcode) << 26;
		return VS_WriteLoadDelay(file,instruction,params->endian);
	}
	else return -4;
}

int VS_ParseBType(VS_OPCODE op, unsigned long* instruction, char* line, FILE* file, VS_ASM_PARAMS* params){
	VS_B_TYPE btype;
	VS_SYM sym;
	int len;
	char new_line[VS_MAX_LINE+1];
	unsigned long size1, size2, encode, symbol_ind;
	int offset, cop, nd, tf, sym_found;
	unsigned char pseudo_op;
	
	len = strlen(op.name);

	if(!strcmp(op.name,"b") || !strcmp(op.name,"bal")){
		VS_CopyLabelName(new_line, line, len);
		
		if(VS_FindSymbol(new_line)){
			sym = VS_GetSymbol(new_line);
			offset = sym.instruction_count - instruction_count;
			offset--;
			
			if(sym.type == VS_SYM_UND){
				VS_SetRelocTrue();
				VS_SetPSYQRelocTrue();
				VS_AddUndefinedRelocEntry((instruction_count << 2), VS_MIPS_PC16, sym.string_table_index + 8);
				VS_AddUndefinedPSYQRelocEntry(symbol_index, VS_PSYQ_26, ((instruction_count - label_instruction_count) << 2), 2, sym.string_table_index + 8);
			}
			
			*instruction = VS_Bin2Decimal(op.opcode) << 26 | (offset & 0xFFFF);
			
			if(!strcmp(op.name,"bal")){
				*instruction |= 17 << 16;
			}
			
			return VS_WriteLoadDelay(file,instruction,params->endian);
		}
		else if(params->undefsym){
			VS_TrimStrictLine(new_line,line + strlen(op.name));
			symbol_ind = VS_AddSymbol(new_line,instruction_count,0,VS_SYM_UND,VS_SCOPE_GLOBAL);
			VS_SetRelocTrue();
			VS_SetPSYQRelocTrue();
			VS_AddUndefinedRelocEntry((instruction_count << 2), VS_MIPS_PC16, symbol_ind + 8);
			VS_AddUndefinedPSYQRelocEntry(symbol_index, VS_PSYQ_26, ((instruction_count - label_instruction_count) << 2), 2, symbol_ind + 8);
			*instruction = VS_Bin2Decimal(op.opcode) << 26;
			
			if(!strcmp(op.name,"bal")){
				*instruction |= 17 << 16;
			}
			
			return VS_WriteLoadDelay(file,instruction,params->endian);
		}
		else return -4;
	}
	else if(!strcmp(op.name,"bc2f") || !strcmp(op.name,"bc1f") || !strcmp(op.name,"bc2fl") || !strcmp(op.name,"bc1fl") 
		|| !strcmp(op.name,"bc2t") || !strcmp(op.name,"bc1t") || !strcmp(op.name,"bc2tl") || !strcmp(op.name,"bc1tl")){
		VS_CopyLabelName(new_line, line, len);
		
		if(!strcmp(op.name,"bc1f") || !strcmp(op.name,"bc2f")){
			nd = 0;
			tf = 0;
		}
		else if(!strcmp(op.name,"bc1fl") || !strcmp(op.name,"bc2fl")){
			nd = 1;
			tf = 0;
		}
		else if(!strcmp(op.name,"bc1t") || !strcmp(op.name,"bc2t")){
			nd = 0;
			tf = 1;
		}
		else{
			nd = 1;
			tf = 1;
		}
		
		if(!strncmp(op.name,"bc1",3)){
			cop = 17;
		}
		else{
			cop = 18;
		}
		
		if(VS_FindSymbol(new_line)){
			sym = VS_GetSymbol(new_line);
			offset = sym.instruction_count - instruction_count;
			offset--;
			
			if(sym.type == VS_SYM_UND){
				VS_SetRelocTrue();
				VS_SetPSYQRelocTrue();
				VS_AddUndefinedRelocEntry((instruction_count << 2), VS_MIPS_PC16, sym.string_table_index + 8);
				VS_AddUndefinedPSYQRelocEntry(symbol_index, VS_PSYQ_26, ((instruction_count - label_instruction_count) << 2), 2, sym.string_table_index + 8);
			}
			
			*instruction = cop << 26 | VS_Bin2Decimal(op.opcode) << 21 | nd << 17 | tf << 16 | (offset & 0xFFFF);
			
			return VS_WriteLoadDelay(file,instruction,params->endian);
		}
		else if(params->undefsym){
			VS_TrimStrictLine(new_line,line + strlen(op.name));
			symbol_ind = VS_AddSymbol(new_line,instruction_count,0,VS_SYM_UND,VS_SCOPE_GLOBAL);
			VS_SetRelocTrue();
			VS_SetPSYQRelocTrue();
			VS_AddUndefinedRelocEntry((instruction_count << 2), VS_MIPS_PC16, symbol_ind + 8);
			VS_AddUndefinedPSYQRelocEntry(symbol_index, VS_PSYQ_26, ((instruction_count - label_instruction_count) << 2), 2, symbol_ind + 8);
			*instruction = cop << 26 | VS_Bin2Decimal(op.opcode) << 21 | nd << 17 | tf << 16;
			
			return VS_WriteLoadDelay(file,instruction,params->endian);
		}
		else return -4;
	}
	else{
		if(line[len] != '$' && params->syntax == VS_GNU_SYNTAX){
			return -1;
		}
		
		btype.op = VS_Bin2Decimal(op.opcode);
		btype.rs = VS_GetRegister(line + len, &size1, params);
		
		if(btype.rs == -1){
			return -2;
		}
		
		if(btype.rs == -2){
			return -9;
		}
		
		if(VS_IsValidRegImmType(op.name) || VS_IsValidBranchOnZeroType(op.name)){
			int regimm = 0;
			if(VS_IsValidRegImmType(op.name)){
				regimm = 1;
			}
			
			if(!regimm){
				if(!strcmp(op.name,"bgez")){
					btype.rt = 1;
				}
				else{
					btype.rt = 0;
				}
			}
			
			VS_CopyLabelName(new_line, line, len + size1 + 1);
			
			if(VS_FindSymbol(new_line)){
				sym = VS_GetSymbol(new_line);
				offset = sym.instruction_count - instruction_count;
				offset--;
				
				if(sym.type == VS_SYM_UND){
					VS_SetRelocTrue();
					VS_SetPSYQRelocTrue();
					VS_AddUndefinedRelocEntry((instruction_count << 2), VS_MIPS_PC16, sym.string_table_index + 8);
					VS_AddUndefinedPSYQRelocEntry(symbol_index, VS_PSYQ_26, ((instruction_count - label_instruction_count) << 2), 2, sym.string_table_index + 8);
				}
				
				if(!regimm){
					*instruction = VS_Bin2Decimal(op.opcode) << 26 | btype.rs << 21 | btype.rt << 16 | (offset & 0xFFFF);
				}
				else{
					*instruction = 1 << 26 | btype.rs << 21 | VS_Bin2Decimal(op.opcode) << 16 | (offset & 0xFFFF);
				}
				
				if(safe_load_delay){
					encode = *instruction;
					VS_WriteInstruction(file,encode,params->endian);
					VS_WriteNop(file);
					instruction_count++;
					return 2;
				}
			}
			else if(params->undefsym){
				VS_TrimStrictLine(new_line,line + len + size1 + 1);
				symbol_ind = VS_AddSymbol(new_line,instruction_count,0,VS_SYM_UND,VS_SCOPE_GLOBAL);
				VS_SetRelocTrue();
				VS_SetPSYQRelocTrue();
				VS_AddUndefinedRelocEntry((instruction_count << 2), VS_MIPS_PC16, symbol_ind + 8);
				VS_AddUndefinedPSYQRelocEntry(symbol_index, VS_PSYQ_26, ((instruction_count - label_instruction_count) << 2), 2, symbol_ind + 8);
				
				if(!regimm){
					*instruction = VS_Bin2Decimal(op.opcode) << 26 | btype.rs << 21 | btype.rt << 16;
				}
				else{
					*instruction = 1 << 26 | btype.rs << 21 | VS_Bin2Decimal(op.opcode) << 16;
				}
				
				if(safe_load_delay){
					encode = *instruction;
					VS_WriteInstruction(file,encode,params->endian);
					VS_WriteNop(file);
					instruction_count++;
					return 2;
				}
			}
			else{
				return -4;
			}
		}
		else if(!strcmp(op.name,"blt") || !strcmp(op.name,"bltu") || !strcmp(op.name,"bgt") || !strcmp(op.name,"bgtu") || !strcmp(op.name,"bge") || !strcmp(op.name,"bgeu") || !strcmp(op.name,"ble")){
			btype.rt = VS_GetRegister(line + len + size1 + 1, &size2, params);
			
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
			
			sym_found = VS_FindSymbol(new_line);
			
			if(sym_found || params->undefsym){
				sym = VS_GetSymbol(new_line);
				offset = sym.instruction_count - instruction_count;
				offset -= 2;
				
				if(sym.type == VS_SYM_UND && sym_found){
					VS_SetRelocTrue();
					VS_SetPSYQRelocTrue();
					VS_AddUndefinedRelocEntry((instruction_count << 2), VS_MIPS_PC16, sym.string_table_index + 8);
					VS_AddUndefinedPSYQRelocEntry(symbol_index, VS_PSYQ_26, ((instruction_count - label_instruction_count) << 2), 2, sym.string_table_index + 8);
				}
				
				unsigned long instruction1;
				
				if(!strcmp(op.name,"bge") || !strcmp(op.name,"bgeu")){
					
					if(!strcmp(op.name,"bge")){
						pseudo_op = 42;
					}
					else{
						pseudo_op = 43;
					}
					
					instruction1 = btype.rs << 21 | btype.rt << 16 | VS_GetRegisterNumber("$at") << 11 | pseudo_op;
					
					VS_WriteInstruction(file,instruction1,params->endian);
					instruction1 = 4 << 26 | VS_GetRegisterNumber("$at") << 21 | 0 << 16 | (offset & 0xFFFF);
					VS_WriteInstruction(file,instruction1,params->endian);
					
					if(safe_load_delay){
						VS_WriteNop(file);
						instruction_count += 2;
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
					
					VS_WriteInstruction(file,instruction1,params->endian);
					instruction1 = 4 << 26 | VS_GetRegisterNumber("$at") << 21 | 0 << 16 | (offset & 0xFFFF);
					VS_WriteInstruction(file,instruction1,params->endian);
					
					if(safe_load_delay){
						VS_WriteNop(file);
						instruction_count += 2;
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
					
					VS_WriteInstruction(file,instruction1,params->endian);
					instruction1 = 5 << 26 | VS_GetRegisterNumber("$at") << 21 | 0 << 16 | (offset & 0xFFFF);
					VS_WriteInstruction(file,instruction1,params->endian);
					
					if(safe_load_delay){
						VS_WriteNop(file);
						instruction_count += 2;
					}
				}
				
				if(!sym_found && params->undefsym){
					VS_TrimStrictLine(new_line,line + len + size1 + size2 + 2);
					symbol_ind = VS_AddSymbol(new_line,instruction_count,0,VS_SYM_UND,VS_SCOPE_GLOBAL);
					VS_SetRelocTrue();
					VS_SetPSYQRelocTrue();
					VS_AddUndefinedRelocEntry((instruction_count << 2), VS_MIPS_PC16, symbol_ind + 8);
					VS_AddUndefinedPSYQRelocEntry(symbol_index, VS_PSYQ_26, ((instruction_count - label_instruction_count) << 2), 2, symbol_ind + 8);
				}
				
				instruction_count++;
			}
			else return -4;
			
			return 2;
		}
		else{
			btype.rt = VS_GetRegister(line + len + size1 + 1, &size2, params);
			
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
				
				if(sym.type == VS_SYM_UND){
					VS_SetRelocTrue();
					VS_SetPSYQRelocTrue();
					VS_AddUndefinedRelocEntry((instruction_count << 2), VS_MIPS_PC16, sym.string_table_index + 8);
					VS_AddUndefinedPSYQRelocEntry(symbol_index, VS_PSYQ_26, ((instruction_count - label_instruction_count) << 2), 2, sym.string_table_index + 8);
				}
				
				*instruction = VS_Bin2Decimal(op.opcode) << 26 | btype.rs << 21 | btype.rt << 16 | (offset & 0xFFFF);
				
				return VS_WriteLoadDelay(file,instruction,params->endian);
			}
			else if(params->undefsym){
				VS_TrimStrictLine(new_line,line + len + size1 + size2 + 2);
				symbol_ind = VS_AddSymbol(new_line,instruction_count,0,VS_SYM_UND,VS_SCOPE_GLOBAL);
				VS_SetRelocTrue();
				VS_SetPSYQRelocTrue();
				VS_AddUndefinedRelocEntry((instruction_count << 2), VS_MIPS_PC16, symbol_ind + 8);
				VS_AddUndefinedPSYQRelocEntry(symbol_index, VS_PSYQ_26, ((instruction_count - label_instruction_count) << 2), 2, symbol_ind + 8);
				
				*instruction = VS_Bin2Decimal(op.opcode) << 26 | btype.rs << 21 | btype.rt << 16;
				
				return VS_WriteLoadDelay(file,instruction,params->endian);
			}
			else return -4;
		}
		
	}		
	
	return 1;
}

int VS_ParseAddrType(VS_OPCODE op, unsigned long* instruction, char* line, FILE* file, VS_ASM_PARAMS* params){
	VS_ADDR_TYPE atype;
	VS_SYM sym;
	long expr;
	int len, is_valid_imm, is_valid_prefix, reg;
	char new_line[VS_MAX_LINE+1];
	unsigned long end, size1, symbol_ind;
	unsigned long start;
	unsigned char neg;
	
	VS_InitExprParser();
	
	len = strlen(op.name);
	atype.op = VS_Bin2Decimal(op.opcode);
	neg = 0;
	
	is_valid_prefix = VS_IsValidRegisterPrefix(line,len,params->syntax);
	
	if(is_valid_prefix != 1){
		return is_valid_prefix;
	}
	
	if(!strcmp(op.name,"li")){
		if(VS_GetNumberOfCommas(line) != 1){
			return -10;
		}
		
		reg = VS_GetRegister(line + len, &size1, params);
		
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
		
		if(VS_LineContainsOperator(new_line+neg)){		
			expr = VS_IsValidExpression(new_line, params->syntax);
			
			if(!expr){
				return -17;
			}
			
			expr = VS_EvaluateExpr(new_line, params->syntax);
			sprintf(new_line,"%ld",expr);
			is_valid_imm = VS_IsValidImmediate(new_line, params);
		}
		else{
			is_valid_imm = VS_IsValidImmediate(new_line, params);
		}
		
		if(is_valid_imm == -1){
			return -5;
		}
		else if(is_valid_imm == -2){
		   return -6;	
		}
		
		start = VS_ParseImmediateValue(new_line, params);
		
		if(neg){
			signed int negative = start;
			
			if(negative <= -32678 || negative >= 32767){
				*instruction = 15 << 26 | reg << 16 | ((negative >> 16) & 0xFFFF);
				start = 13 << 26 | reg << 21 | reg << 16 | (negative & 0xFFFF);
				
				VS_WriteInstruction(file,*instruction,params->endian);
				VS_WriteInstruction(file,start,params->endian);
				
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
				
				VS_WriteInstruction(file,*instruction,params->endian);
				VS_WriteInstruction(file,start,params->endian);
				
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
		reg = VS_GetRegister(line + len, &size1, params);
		
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
				start += params->org;
			}
			
			VS_SetRelocTrue();
			VS_SetPSYQRelocTrue();
			
			if(sym.type == VS_SYM_UND){
				VS_SetRelocTrue();
				VS_SetPSYQRelocTrue();
				VS_AddUndefinedRelocEntry((instruction_count << 2), VS_MIPS_HI_16, sym.string_table_index + 8);
				VS_AddUndefinedRelocEntry((instruction_count + 1) << 2, VS_MIPS_LO_16, sym.string_table_index + 8);
				VS_AddUndefinedPSYQRelocEntry(sym.string_table_index + 8, VS_PSYQ_HI_16, ((instruction_count - label_instruction_count) << 2), 3, 0);
				VS_AddUndefinedPSYQRelocEntry(sym.string_table_index + 8, VS_PSYQ_LO_16, (((instruction_count + 1) - label_instruction_count) << 2), 3, 0);
			}
			else if(sym.type == VS_SYM_OBJ){
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
			
			VS_WriteInstruction(file,end,params->endian);
			VS_WriteInstruction(file,start,params->endian);
			
			instruction_count++;
		}
		else if(params->undefsym){
			VS_TrimStrictLine(new_line,line + len + size1 + 1);
			symbol_ind = VS_AddSymbol(new_line,instruction_count,0,VS_SYM_UND,VS_SCOPE_GLOBAL);
			VS_SetRelocTrue();
			VS_SetPSYQRelocTrue();
			VS_AddUndefinedRelocEntry((instruction_count << 2), VS_MIPS_HI_16, symbol_ind + 8);
			VS_AddUndefinedRelocEntry(((instruction_count + 1) << 2), VS_MIPS_LO_16, symbol_ind + 8);
			VS_AddUndefinedPSYQRelocEntry(symbol_ind + 8, VS_PSYQ_HI_16, ((instruction_count - label_instruction_count) << 2), 2, 0);
			VS_AddUndefinedPSYQRelocEntry(symbol_ind + 8, VS_PSYQ_LO_16, (((instruction_count + 1) - label_instruction_count) << 2), 2, 0);
			
			start = 0;
			
			end = 15 << 26 | reg << 16 | ((start >> 16) & 0xFFFF);
			start = 13 << 26 | reg << 21 | reg << 16 | (start & 0xFFFF);
			
			VS_WriteInstruction(file,end,params->endian);
			VS_WriteInstruction(file,start,params->endian);
			
			instruction_count++;
		}
		else{
			if(new_line[0] == '-'){
				neg = 1;
			}
			
			if(VS_LineContainsOperator(new_line+neg)){		
				expr = VS_IsValidExpression(new_line, params->syntax);
				
				if(!expr){
					return -17;
				}
				
				expr = VS_EvaluateExpr(new_line, params->syntax);
				sprintf(new_line,"%ld",expr);
				is_valid_imm = VS_IsValidImmediate(new_line, params);
			}
			else{
				is_valid_imm = VS_IsValidImmediate(new_line, params);
			}
		
			if(is_valid_imm == -1){
				return -5;
			}
			else if(is_valid_imm == -2){
			   return -6;	
			}
			
			start = VS_ParseImmediateValue(new_line, params);
			
			if(neg){
				signed int negative = start;
				
				*instruction = 15 << 26 | reg << 16 | ((negative >> 16) & 0xFFFF);
				start = 9 << 26 | reg << 21 | reg << 16 | (negative & 0xFFFF);
				
				VS_WriteInstruction(file,*instruction,params->endian);
				VS_WriteInstruction(file,start,params->endian);
			}
			else{
			
				*instruction = 15 << 26 | reg << 16 | ((start >> 16) & 0xFFFF);
				start = 9 << 26 | reg << 21 | reg << 16 | (start & 0xFFFF);
				
				VS_WriteInstruction(file,*instruction,params->endian);
				VS_WriteInstruction(file,start,params->endian);
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
			reg = VS_GetRegister(line + len, &size1, params);
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
			VS_TrimStrictLine(new_line, line + len + size1 + 1);
			
			if(VS_FindSymbol(new_line) && strcmp(op.name,"lwc2") != 0 && strcmp(op.name,"swc2") != 0 && strcmp(op.name,"lwc0") != 0 && strcmp(op.name,"swc0") != 0){
				sym = VS_GetSymbol(new_line);
				
				start = sym.addr;
			
				if(OEXE && sym.type == VS_SYM_FUNC){
					start += params->org;
				}
				
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
				
				end = 15 << 26 | 1 << 16 | ((start >> 16) & 0xFFFF);       /* lui */
				
				VS_WriteInstruction(file,end,params->endian);
				
				instruction_count++;
				
				atype.rt = 1;
				atype.base = reg;
				atype.offset = start;
		
				if(safe_load_delay && params->architecture != VS_MIPS_II_ARCH && params->architecture != VS_MIPS_PSP_ARCH){
					if(!strcmp(op.name,"lw") || !strcmp(op.name,"lh") || !strcmp(op.name,"lhu") || !strcmp(op.name,"lbu") | !strcmp(op.name,"lb")
						|| !strcmp(op.name,"lwc2")){
						start = atype.op << 26 | atype.rt << 21 | atype.base << 16 | (atype.offset & 0xFFFF);
						VS_WriteInstruction(file,start,params->endian);
						VS_WriteNop(file);
						instruction_count++;
						return 2;
					}
				}
				
				*instruction = atype.op << 26 | atype.rt << 21 | atype.base << 16 | (atype.offset & 0xFFFF);
				
				return 1;
			}
			else if(VS_IsValidImmediate(new_line, params)){
				start = VS_ParseImmediateValue(new_line, params);

				end = 15 << 26 | 1 << 16 | ((start >> 16) & 0xFFFF);       /* lui */
				
				instruction_count++;
				
				VS_WriteInstruction(file,end,params->endian);
				
				atype.rt = 1;
				atype.base = reg;
				atype.offset = start;
				
				if(safe_load_delay && params->architecture != VS_MIPS_II_ARCH && params->architecture != VS_MIPS_PSP_ARCH){
					if(!strcmp(op.name,"lw") || !strcmp(op.name,"lh") || !strcmp(op.name,"lhu") || !strcmp(op.name,"lbu") | !strcmp(op.name,"lb")
						|| !strcmp(op.name,"lwc2")){
						start = atype.op << 26 | atype.rt << 21 | atype.base << 16 | (atype.offset & 0xFFFF);
						VS_WriteInstruction(file,start,params->endian);
						VS_WriteNop(file);
						instruction_count++;
						return 2;
					}
				}
				
				*instruction = atype.op << 26 | atype.rt << 21 | atype.base << 16 | (atype.offset & 0xFFFF);
				
				return 1;
			}
			else return 0;
		}
		
		if(strstr(line,")") == NULL){
			return 0;
		}
		
		reg = VS_GetRegister(rs + 1,&size1, params);
		
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
			if(params->syntax != VS_ASMPSX_SYNTAX && VS_GetNumberOfCommas(line) < 1){
				return 0;
			}
		}
		
		if(new_line[0] == '-'){
			neg = 1;
		}
		
		is_valid_imm = VS_IsValidImmediate(new_line, params);
		
		atype.offset = VS_ParseImmediateValue(new_line, params);
		atype.base = reg;
		
		if(VS_FindSymbol(new_line) && is_valid_imm < 0){
			sym = VS_GetSymbol(new_line);
			atype.offset = sym.addr & 0xFFFF;
		}
		else if(VS_IsValidSymbolAddrOperator(new_line[0]) && VS_FindSymbol(new_line+1) && is_valid_imm < 0){
			sym = VS_GetSymbol(new_line+1);
			
			if(new_line[0] == '>'){
				atype.offset = (sym.addr >> 16) & 0xFFFF;
			}
			else{
				atype.offset = sym.addr & 0xFFFF;
			}
		}
		else if(VS_LineContainsOperator(new_line+neg)){		
			expr = VS_IsValidExpression(new_line, params->syntax);
			
			if(!expr){
				return -17;
			}
			
			expr = VS_EvaluateExpr(new_line, params->syntax);
			sprintf(new_line,"%ld",expr);
			is_valid_imm = VS_IsValidImmediate(new_line, params);
		}
		else{
			if(is_valid_imm == -1){
				return -5;
			}
			else if(is_valid_imm == -2){
			   return -6;	
			}
		}
		
		if(safe_load_delay && params->architecture != VS_MIPS_II_ARCH && params->architecture != VS_MIPS_PSP_ARCH){
			if(!strcmp(op.name,"lw") || !strcmp(op.name,"lh") || !strcmp(op.name,"lhu") || !strcmp(op.name,"lbu") | !strcmp(op.name,"lb")
				|| !strcmp(op.name,"lwc2")){
				start = atype.op << 26 | atype.base << 21 | atype.rt << 16 | (atype.offset & 0xFFFF);
				VS_WriteInstruction(file,start,params->endian);
				VS_WriteNop(file);
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

int VS_ParseMoveType(VS_OPCODE op, unsigned long* instruction, char* line, VS_ASM_PARAMS* params){
	VS_MOVE_TYPE mtype;
	unsigned long size1, num_commas;
	int len, is_valid_prefix;
	
	len = strlen(op.name);
	mtype.op = VS_Bin2Decimal(op.opcode);
	
	is_valid_prefix = VS_IsValidRegisterPrefix(line,len,params->syntax);
	
	if(is_valid_prefix != 1){
		return is_valid_prefix;
	}
	
	num_commas = VS_GetNumberOfCommas(line);

	if(num_commas){
		return -12;
	}
	
	mtype.rd = VS_GetRegister(line + len, &size1, params);
	
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

int VS_ParseCopType(VS_OPCODE op, unsigned long* instruction, char* line, VS_ASM_PARAMS* params){
	VS_COP_TYPE ctype;
	unsigned long size1, size2, offset;
	int len, is_valid_imm;
	
	len = strlen(op.name);
	ctype.op = VS_Bin2Decimal(op.opcode);
	
	if(!strcmp(op.name,"cop2")){
		char* cofunc = line + strlen("cop2");
		
		is_valid_imm = VS_IsValidImmediate(cofunc, params);
		
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
	
	if(line[len] != '$' && params->syntax == VS_GNU_SYNTAX){
		return -1;
	}
	
	ctype.rt = VS_GetRegister(line + len, &size1, params);
	
	if(ctype.rt == -1){
		return -2;
	}
	
	if(ctype.rt == -2){
		return -9;
	}
	
	ctype.rd = VS_GetCopRegister(line + len + size1 + 1, &size2);
	
	if(line[len + size1 + 1] != '$' && params->syntax == VS_GNU_SYNTAX){
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

int VS_ParseFloatType(VS_OPCODE op, unsigned long* instruction, char* line, FILE* file, VS_ASM_PARAMS* params){
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
	
	if(line[len] != '$' && params->syntax == VS_GNU_SYNTAX){
		return -1;
	}
	
	rtype.rd = VS_GetFpRegister(line + len, &size1, params);
	
	if(rtype.rd == -1){
		return -2;
	}
	
	if(rtype.rd == -2){
		if(!strcmp(op.name,"cfc1")){
			rtype.rd = VS_GetRegister(line + len, &size1, params);
		}
		else if(!strcmp(op.name,"ctc1")){
			rtype.rd = VS_GetRegister(line + len, &size1, params);
		}
		else if(!strcmp(op.name,"mfc1")){
			rtype.rd = VS_GetRegister(line + len, &size1, params);
		}
		else if(!strcmp(op.name,"mtc1")){
			rtype.rd = VS_GetRegister(line + len, &size1, params);
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
		VS_WriteInstruction(file,instr,params->endian);
		instr = 13 << 26 | 1 << 21 | 1 << 16 | (size1 & 0xFFFF); /* ori */
		VS_WriteInstruction(file,instr,params->endian);
		instr = VS_COP_1 << 26 | 4 << 21 | 1 << 16 | rtype.rd << 11; /* mtc1 */
		VS_WriteInstruction(file,instr,params->endian);
		instr = 0;                 /* nop */
		VS_WriteInstruction(file,instr,params->endian);
		
		instruction += 3;
		
		return 2;
	}
	
	rtype.rs = VS_GetFpRegister(line + len + size1 + 1, &size2, params);
	
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
			
			reg = VS_GetRegister(rs + 1, &size1, params);
			
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
				if(params->syntax != VS_ASMPSX_SYNTAX && VS_GetNumberOfCommas(line) < 1){
					return 0;
				}
			}
			
			if(new_line[0] == '-'){
				neg = 1;
			}
		
			is_valid_imm = VS_IsValidImmediate(new_line, params);
			
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
	
	rtype.rt = VS_GetFpRegister(line + len + size1 + 2 + size2, &size3, params);
	
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

int VS_ParseVFPUType(VS_OPCODE op, unsigned long* instruction, char* line, VS_ASM_PARAMS* params){
	VS_VFPU_TYPE vtype;
	unsigned long size1;
	int len, size, is_valid_imm, neg, offset, num_commas;
	char reg_str[5], cond_str[3], new_line[VS_MAX_LINE+1];
	unsigned char cond;
	
	memset(&vtype,0x0,sizeof(VS_VFPU_TYPE));
	
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
		
		VS_LowercaseLine(reg_str);
		
		vtype.rd = VS_GetVFPURegister(reg_str, size);
		
		if(vtype.rd == -1){
			return -14;
		}
		
		memcpy(reg_str,line+len+8,4);
		reg_str[4] = '\0';
		
		VS_LowercaseLine(reg_str);
		
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
	
	VS_LowercaseLine(reg_str);
	
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
		
		vtype.rs = VS_GetRegister(rs + 1, &size1, params);
		
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
			if(params->syntax != VS_ASMPSX_SYNTAX && VS_GetNumberOfCommas(line) < 1){
				return 0;
			}
		}
		
		if(new_line[0] == '-'){
			neg = 1;
		}
	
		is_valid_imm = VS_IsValidImmediate(new_line, params);
		
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
		
		VS_LowercaseLine(reg_str);
		
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
			
			VS_LowercaseLine(reg_str);
		
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
		
		VS_LowercaseLine(reg_str);
		
		if(args > 2){
			vtype.rs = VS_GetVFPURegister(reg_str, size);

			if(vtype.rs == -1){
				return -14;
			}
			
			if(line[len+10] == '-'){
				return -6;
			}
			
			is_valid_imm = VS_IsValidImmediate(line + len + 10, params);
	
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
			
			is_valid_imm = VS_IsValidImmediate(line + len + 5, params);
		
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
	unsigned long instruction, size;
	unsigned long instruction_arr[VS_MAX_INSTRUCTION_READ], count;
	int instr_index, dir, error, err, first_occurence, line_count = 0;
	char line[VS_MAX_LINE+1];
	char dest[VS_MAX_LINE+1];
	char* arr[2];
	
	arr[0] = "jal";
	arr[1] = "bal";
	
	OEXE = params->oexe;
	
	text = fopen("textsec.dat","wb");
	
	instruction_count = 0;
	safe_load_delay = 0;
	symbol_index = -1;
	label_instruction_count = 0;
	
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
							VS_PrintError(in,line,VS_GetActualLineCount(line_count));
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
						VS_PrintError(in,line,VS_GetActualLineCount(line_count));
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
					VS_PrintError(in,line,VS_GetActualLineCount(line_count));
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
		
		if(strstr(line,".inject") != NULL){
			char* path =  strstr(line,".inject") + strlen(".inject");
			char* end_quotes;
			
			if(!VS_VerifyPathSyntax(path,".inject",line_count)){
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
			
			FILE* inc = fopen(dest,"rb");
			
			if(inc == NULL){
				printf("Warning: The file path of the inject directive could not be found!\n");
				
				err = VS_ErrorOccuredInIncludeEntry(line_count);
			
				if(err != -1){
					VS_PrintErrorFromIncludeEntry(err,line_count);
				}
				else{
					printf("Line %d: %s\n",VS_GetActualLineCount(line_count),path);
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
					size >>= 2;
					
					unsigned long i;
					for(i = 0; i < size;){
						count = fread(instruction_arr,4,VS_MAX_INSTRUCTION_READ,inc);
						fwrite(instruction_arr,4,count,text);
						i += count;
					}
					instruction_count += size;
				}
				
				fclose(inc);
			}
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
					VS_PrintError(in,line,VS_GetActualLineCount(line_count));
				}
				
				fclose(text);
				return -1;
			}

			switch(op.itype){
				case VS_R_INSTRUCTION:{
					error = VS_ParseRType(op,&instruction,line,text,params);
				}break;
				case VS_I_INSTRUCTION:{
					error = VS_ParseIType(op,&instruction,line,text,params);
				}break;
				case VS_J_INSTRUCTION:{
					error = VS_ParseJType(op,&instruction,line,text,params);
				}break;
				case VS_B_INSTRUCTION:{
					error = VS_ParseBType(op,&instruction,line,text,params);
				}break;
				case VS_ADDR_INSTRUCTION:{
					error = VS_ParseAddrType(op,&instruction,line,text,params);
				}break;
				case VS_MOVE_INSTRUCTION:{
					error = VS_ParseMoveType(op,&instruction,line,params);
				}break;
				case VS_COP_INSTRUCTION:{
					error = VS_ParseCopType(op,&instruction,line,params);
				}break;
				case VS_FLOAT_INSTRUCTION:{
					error = VS_ParseFloatType(op,&instruction,line,text,params);
				}break;
				case VS_VFPU_INSTRUCTION: break;
				case VS_COND_INSTRUCTION: break;
			}
			
			if(op.itype & VS_VFPU_INSTRUCTION){
				error = VS_ParseVFPUType(op,&instruction,line,params);
			}
			
			instruction_count++;
		
			if(error == -10){
				printf("Syntax Error: instruction '%s' requires one argument in the form '%s reg1, arg1'\n",op.name,op.name);
			}
			else if(error == -12){
				printf("Syntax Error: instruction '%s' requires one destination register in the form '%s reg1'\n",op.name,op.name);
			}
			else if(error == 5){
				if(!params->nowarnings){
					printf("Warning: instruction '%s' handles 16-bit immediate values. Truncating value to 16-bits.\n",op.name);
				}
			}
			else if(error >= 60){
				printf("Syntax Error: instruction '%s' requires %d registers\n",op.name,error-60);
			}
			else if(error != 1 && error != 2){
				if(params->nowarnings){
					if(error != 3 && error != 4){
						VS_PrintErrorString(error);
					}
				}
				else{
					VS_PrintErrorString(error);
				}
			}
			
			if(error <= 0 || error >= 60){
				int incerr = VS_ErrorOccuredInIncludeEntry(line_count);
				
				if(params->nowarnings){
					if(error != 3 && error != 4){
						if(incerr != -1){
							VS_PrintErrorFromIncludeEntry(incerr,line_count);
						}
						else{
							VS_PrintError(in,line,VS_GetActualLineCount(line_count));
						}
					}
				}
				else{
					if(incerr != -1){
						VS_PrintErrorFromIncludeEntry(incerr,line_count);
					}
					else{
						VS_PrintError(in,line,VS_GetActualLineCount(line_count));
					}
				}
				
				fclose(text);
				return -1;
			}
			
			//printf("instruction = %d\n",instruction_count);
			//printf("name = %s\n",op.name);
			//printf("line = %s\n",line);
			
			if(error != 2)
				VS_WriteInstruction(text,instruction,params->endian);
		}
	}
	
	fclose(text);

	return 1;
}