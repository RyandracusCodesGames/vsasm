#ifndef VS_CPE_H
#define VS_CPE_H

/********************************************
*   VideoStation Assembler
*
*   Copyright (c) 2025 Ryandracus Chapman
*
*   File: vs_cpe.h
*   Date: 6/7/2025
*   Version: 1.1
*   Updated: 6/7/2025
*   Author: Ryandracus Chapman
*
********************************************/

#include <stdio.h>

typedef struct VS_CPE_CHUNK1{
	unsigned char id;
	unsigned long addr;
	unsigned long size;
}VS_CPE_CHUNK1;

typedef struct VS_CPE_CHUNK3{
	unsigned char id;
	unsigned short reg;
	unsigned long value;
}VS_CPE_CHUNK3;

void VS_WriteCPEChunk1(FILE* file, VS_CPE_CHUNK1 chunk1);
void VS_WriteCPEChunk3(FILE* file, VS_CPE_CHUNK3 chunk3);
void VS_WriteCPE(char* filename, unsigned long org, int output);
void VS_CPE2EXE(char* filein, char* fileout);

#endif