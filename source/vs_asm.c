#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <vs_opcode.h>
#include <vs_symtable.h>
#include <vs_preprocessor.h>
#include <vs_parser.h>
#include <vs_elf.h>
#include <vs_psexe.h>
#include <vs_utils.h>
#include <vs_psyqobj.h>

/********************************************
*   VideoStation Assembler
*
*   Copyright (c) 2025 Ryandracus Chapman
*
*   File: vs_asm.c
*   Date: 4/24/2025
*   Version: 1.0
*   Updated: 6/1/2025
*   Author: Ryandracus Chapman
*
********************************************/

typedef int Bool;

#define VS_TRUE  1 
#define VS_FALSE 0

unsigned long org = 0x80010000;

const char* usage = "Copyright (c) 2025 Ryandracus Chapman\n"
"vsasm - Assembles source code containing mnemonic instructions of the MIPS I and II architectures into binary object code for the original Sony PlayStation and PlayStation Portable\n"
"usage: vsasm <options> <file1> <options> <file2>...\n"
"-text - Outputs the assembly file's text section, binary encoded machine instructions, to an output file\n"
"-sym - Outputs the assembly file's symbol table to the console\n"
"-fmt - Specifies the format of the object file (elf or psyq as arguments)\n"
"-oexe - Outputs the assembly file's object code into a PlayStation Executable(PS-EXE)\n"
"-o - Specifies the name of the output object code file or PlayStation executable\n";

int main(int argc, char** argv){
	
	FILE* in, *preprocessor, *parser, *textsec, *text_out;
	VS_ObjectFormat fmt;
	VS_ASM_PARAMS params;
	unsigned long num_of_instructions, count;
	unsigned long instruction_arr[VS_MAX_INSTRUCTION_READ]; 
	int ext;
	Bool text, sym;
	char name[256] = {0};
	
	text = VS_FALSE; sym = VS_FALSE;
	fmt = VS_ELF_OBJ;
	
	params.org = org;
	params.architecture = VS_MIPS_PSX_ARCH;
	params.endian = 0;
	params.syntax = VS_GNU_SYNTAX;
	params.oexe = VS_FALSE;

	if(argc == 1){
		printf("%s\n",usage);
		return -1;
	}

	int i;
	for(i = 1; i < argc; i++){
		
		if(!strcmp(argv[i],"-sym")){
			sym = VS_TRUE;
			continue;
		}
		
		if(!strcmp(argv[i],"-text")){
			text = VS_TRUE;
			memset(name,0x0,255);
			continue;
		}
		
		if(!strcmp(argv[i],"-oexe")){
			params.oexe = VS_TRUE;
			continue;
		}
		
		if(!strcmp(argv[i],"-fmt")){
			
			if(!strcmp(argv[i+1],"elf")){
				fmt = VS_ELF_OBJ;
				i++;
			}
			if(!strcmp(argv[i+1],"psyq")){
				fmt = VS_PSYQ_OBJ;
				i++;
			}
			
			continue;
		}
		
		VS_InitRelocTable();
		VS_InitPSYQRelocTable();
	
		in = fopen(argv[i], "rb");
		
		if(in == NULL){
			printf("Error: FILE NOT FOUND - %s!\n",argv[i]);
			return -1;
		}
		
		preprocessor = fopen("preprocessor_init.i","wb+");

		if(VS_PreproccessAssemblyFile(in, preprocessor, &params) == -1){
			VS_DestroyMacroTable();
			VS_DestroySymbolTable();
			VS_DestroyFileIncludeTable();
			fclose(in);
			fclose(preprocessor);
			
			remove("preprocessor_init.i");
			remove("preprocessor.i");
			remove("datasec.dat");
			remove("textsec.dat");
			return -1;
		}
		
		parser = fopen("preprocessor.i", "rb");
		
		if(VS_ParseAssemblyFile(in, parser, &params) == -1){
			VS_DestroyMacroTable();
			VS_DestroySymbolTable();
			VS_DestroyFileIncludeTable();
			fclose(in);
			fclose(preprocessor);
			fclose(parser);
			remove("preprocessor_init.i");
			remove("preprocessor.i");
			remove("datasec.dat");
			remove("textsec.dat");
			return -1;
		}
		
		fclose(in);
		fclose(preprocessor);
		fclose(parser);
		
		ext = VS_FilenameHasExtension(argv[i]);
		
		if(ext != -1){
			memcpy(name,argv[i],ext & 0xff);
			strcat(name,".o");
		}
		else{
			memcpy(name,argv[i],strlen(argv[i]) & 0xff);
			strcat(name,".o");
		}
		
		if(fmt == VS_ELF_OBJ){
			if(argc > i + 2){
				if(!strcmp(argv[i+1],"-o")){
					argv[i] = argv[i+2];
					VS_WriteELF(argv[i],params);
					i += 2;
				}
				else{
					VS_WriteELF(name,params);
				}
			}
			else{
				VS_WriteELF(name,params);
			}
		}
		else{
			if(argc > i + 2){
				if(!strcmp(argv[i+1],"-o")){
					argv[i] = argv[i+2];
					VS_WritePSYQObj(argv[i]);
					i += 2;
				}
				else{
					VS_WritePSYQObj(name);
				}
			}
			else{
				VS_WritePSYQObj(name);
			}
		}
		
		if(params.oexe){
			if(argc > i + 2){
				if(!strcmp(argv[i+1],"-o")){
					argv[i] = argv[i+2];
					VS_WritePSEXE(argv[i],org,1);
					i += 2;
				}
				else{
					VS_WritePSEXE(argv[i],org,0);
				}
			}
			else{
				VS_WritePSEXE(argv[i],org,0);
			}
		}
		
		if(text){
			memset(name,0x0,255);
			
			ext = VS_FilenameHasExtension(argv[i]);
	
			if(ext != -1){
				memcpy(name,argv[i],ext & 0xff);
				strcat(name,".bin");
			}
			else{
				memcpy(name,argv[i],strlen(argv[i]) & 0xff);
				strcat(name,".bin");
			}
			
			textsec = fopen("textsec.dat","rb");
	
			if(textsec != NULL){
				text_out = fopen(name,"wb");
				
				fseek(textsec,0x0,SEEK_END);
				num_of_instructions = ftell(textsec) >> 2;
				fseek(textsec,0x0,SEEK_SET);
				
				unsigned long j;
				for(j = 0; j < num_of_instructions;){
					count = fread(instruction_arr,4,VS_MAX_INSTRUCTION_READ,textsec);
					fwrite(instruction_arr,4,count,text_out);
					j += count;
				}
				
				fclose(textsec);
				fclose(text_out);
			}
		}

		if(sym){
			printf("Symbol Table: %s\n",argv[i]);
			VS_PrintSymbolTable();
		}
		
		VS_DestroyMacroTable();
		VS_DestroySymbolTable();
		VS_DestroyFileIncludeTable();
		
		remove("preprocessor_init.i");
		remove("preprocessor.i");
		remove("datasec.dat");
		remove("textsec.dat");
		
		sym = VS_FALSE;
		text = VS_FALSE;
		fmt = VS_ELF_OBJ;
		
		params.org = org;
		params.architecture = VS_MIPS_PSX_ARCH;
		params.endian = 0;
		params.syntax = VS_GNU_SYNTAX;
		params.oexe = VS_FALSE;
	}

	return 0;
}