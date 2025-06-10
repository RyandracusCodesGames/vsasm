#ifndef VS_PARSER_H
#define VS_PARSER_H

#include <stdio.h>
#include <vs_opcode.h>
#include <vs_utils.h>

/********************************************
*   VideoStation Assembler
*
*   Copyright (c) 2025 Ryandracus Chapman
*
*   File: vs_parser.h
*   Date: 4/29/2025
*   Version: 1.1
*   Updated: 6/10/2025
*   Author: Ryandracus Chapman
*
********************************************/

int VS_GetRegister(char* line, unsigned long* size_out, VS_ASM_PARAMS* params);
int VS_GetFpRegister(char* line, unsigned long* size_out, VS_ASM_PARAMS* params);
int VS_GetCopRegister(char* line, unsigned long* size_out);
unsigned char VS_HexDFA(char* line);
unsigned char VS_IntDFA(char* line);
unsigned char VS_FloatDFA(char* line);
int VS_IsValidImmediate(char* line, VS_ASM_PARAMS* params);
int VS_ParseImmediateValue(char* immediate, VS_ASM_PARAMS* params);
int VS_IsValidRegisterPrefix(char* line, int len, VS_SYNTAX syntax);
int VS_WriteLoadDelay(FILE* file, unsigned long* instruction, VS_ENDIAN endian);
int VS_IsValidReinterpretableRType(const char* name);
int VS_IsValidRegImmType(const char* name);
int VS_IsValidBranchOnZeroType(const char* name);
int VS_ReinterpretRTypeAsIType(FILE* file, const char* name, int rd, int rt, char* operands, unsigned long* instruction, VS_ASM_PARAMS* params);
int VS_ReinterpretITypeAsRType(FILE* file, const char* name, int rt, int rs, int imm, unsigned long* instruction, VS_ASM_PARAMS* params);
int VS_ParseRType(VS_OPCODE op, unsigned long* instruction, char* line, FILE* file, VS_ASM_PARAMS* params);
int VS_ParseIType(VS_OPCODE op, unsigned long* instruction, char* line, FILE* file, VS_ASM_PARAMS* params);
int VS_ParseJType(VS_OPCODE op, unsigned long* instruction, char* line, FILE* file, VS_ASM_PARAMS* params);
int VS_ParseBType(VS_OPCODE op, unsigned long* instruction, char* line, FILE* file, VS_ASM_PARAMS* params);
int VS_ParseAddrType(VS_OPCODE op, unsigned long* instruction, char* line, FILE* file, VS_ASM_PARAMS* params);
int VS_ParseMoveType(VS_OPCODE op, unsigned long* instruction, char* line, VS_ASM_PARAMS* params);
int VS_ParseCopType(VS_OPCODE op, unsigned long* instruction, char* line, VS_ASM_PARAMS* params);
int VS_ParseFloatType(VS_OPCODE op, unsigned long* instruction, char* line, FILE* file, VS_ASM_PARAMS* params);
int VS_ParseVFPUType(VS_OPCODE op, unsigned long* instruction, char* line, VS_ASM_PARAMS* params);
int VS_ParseAssemblyFile(FILE* in, FILE* parse, VS_ASM_PARAMS* params);

#endif