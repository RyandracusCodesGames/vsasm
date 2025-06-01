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
*   Version: 1.0
*   Updated: 6/1/2025
*   Author: Ryandracus Chapman
*
********************************************/

int VS_GetRegister(char* line, unsigned long* size_out);
int VS_GetFpRegister(char* line, unsigned long* size_out);
int VS_GetCopRegister(char* line, unsigned long* size_out);
unsigned char VS_HexDFA(char* line);
unsigned char VS_IntDFA(char* line);
unsigned char VS_FloatDFA(char* line);
int VS_IsValidImmediate(char* line);
int VS_ParseRType(VS_OPCODE op, unsigned long* instruction, char* line, FILE* file);
int VS_ParseIType(VS_OPCODE op, unsigned long* instruction, char* line, FILE* file);
int VS_ParseJType(VS_OPCODE op, unsigned long* instruction, char* line, FILE* file);
int VS_ParseBType(VS_OPCODE op, unsigned long* instruction, char* line, FILE* file);
int VS_ParseAddrType(VS_OPCODE op, unsigned long* instruction, char* line, FILE* file);
int VS_ParseMoveType(VS_OPCODE op, unsigned long* instruction, char* line);
int VS_ParseCopType(VS_OPCODE op, unsigned long* instruction, char* line);
int VS_ParseFloatType(VS_OPCODE op, unsigned long* instruction, char* line, FILE* file);
int VS_ParseVFPUType(VS_OPCODE op, unsigned long* instruction, char* line);
int VS_ParseAssemblyFile(FILE* in, FILE* parse, VS_ASM_PARAMS* params);

#endif