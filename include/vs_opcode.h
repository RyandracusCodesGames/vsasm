#ifndef VS_OPCODE_H
#define VS_OPCODE_H

/********************************************
*   VideoStation Assembler
*
*   Copyright (c) 2025 Ryandracus Chapman
*
*   File: vs_opcode.h
*   Date: 4/22/2025
*   Version: 1.0
*   Updated: 6/1/2025
*   Author: Ryandracus Chapman
*
********************************************/

#define VS_SINGLE_POINT_PRECISION 0x10
#define VS_COP_1 17
#define VS_VFPU_ONE_ARGUMENT 1
#define VS_VFPU_TWO_ARGUMENTS 2
#define VS_VFPU_THREE_ARGUMENTS 3

typedef enum VS_INSTRUCTION_TYPE{
	VS_R_INSTRUCTION = 0x1,
	VS_I_INSTRUCTION = 0x2,
	VS_J_INSTRUCTION = 0x4,
	VS_B_INSTRUCTION = 0x8,
	VS_ADDR_INSTRUCTION = 16,
	VS_MOVE_INSTRUCTION = 32,
	VS_COP_INSTRUCTION = 64,
	VS_FLOAT_INSTRUCTION = 128,
	VS_VFPU_INSTRUCTION = 256,
	VS_COND_INSTRUCTION = 512,
}VS_INSTRUCTION_TYPE;

typedef enum VS_OPCODE_TYPE{
	VS_DEFAULT_OPCODE = 0x0,
	VS_DIRECTIVE_OPCODE = 0x1,
	VS_PSEUDO_OPCODE = 0x2,
}VS_OPCODE_TYPE;

typedef enum VS_ARCHITECTURE{
	VS_MIPS_I_ARCH  = 0x1,
	VS_MIPS_PSX_ARCH = 0x2,
	VS_MIPS_PSP_ARCH = 0x4,
	VS_MIPS_II_ARCH = 0x0B,
}VS_ARCHITECTURE;

typedef struct VS_R_TYPE{
	unsigned char op;
	signed char rs;
	signed char rt;
	signed char rd;
	unsigned char shamt;
	unsigned char func;
}VS_R_TYPE;

typedef struct VS_I_TYPE{
	signed char op;
	signed char rs;
	signed char rt;
	int imm;
}VS_I_TYPE;

typedef struct VS_J_TYPE{
	unsigned char op;
	unsigned long imm;
}VS_J_TYPE;

typedef struct VS_ADDR_TYPE{
	unsigned char op;
	unsigned char base;
	unsigned char rt;
	unsigned short offset;
}VS_ADDR_TYPE;

typedef struct VS_B_TYPE{
	unsigned char op;
	signed char rt;
	signed char rs;
	unsigned short offset;
}VS_B_TYPE;

typedef struct VS_MOVE_TYPE{
	signed char rd;
	signed char op;
}VS_MOVE_TYPE;

typedef struct VS_COP_TYPE{
	signed char op;
	signed char rt;
	signed char rd;
}VS_COP_TYPE;

typedef struct VS_VFPU_TYPE{
	unsigned short op;
	signed short rs;
	signed short rt;
	signed short rd;
	signed char imm;
}VS_VFPU_TYPE;

typedef struct VS_OPCODE{
	const char* name;
	const char* opcode;
	VS_INSTRUCTION_TYPE itype;
	VS_OPCODE_TYPE optype;
	VS_ARCHITECTURE arch;
}VS_OPCODE;

typedef struct VS_REGISTER{
	const char* reg;
	const unsigned char reg_num;
}VS_REGISTER;

unsigned long VS_Pow2(unsigned long num);
unsigned long VS_Bin2Decimal(const char* bin);
int VS_GetOpcode(const char* instruction);
void VS_GetOpcodeFromIndex(VS_OPCODE* opcode, unsigned short index);
int VS_GetRegisterNumber(const char* reg);
unsigned char VS_IsValidRegister(const char* reg, unsigned long syntax);
unsigned char VS_IsValidFpRegister(const char* reg, unsigned long syntax);
unsigned char VS_LineContainsRegister(const char* reg);
unsigned long VS_GetNumberOfInstructions();

#endif