#include <vs_psexe.h>
#include <string.h>
#include <stdlib.h>
#include <vs_elf.h>

/********************************************
*   VideoStation Assembler
*
*   Copyright (c) 2025 Ryandracus Chapman
*
*   File: vs_psexe.c
*   Date: 5/3/2025
*   Version: 1.1
*   Updated: 6/7/2025
*   Author: Ryandracus Chapman
*
********************************************/

void VS_WritePSEXEHeader(FILE* file, VS_PSEXE psexe){
	fwrite(&psexe,1,16,file);
	fwrite(&psexe.pc,4,1,file);
	fwrite(&psexe.gp,4,1,file);
	fwrite(&psexe.ram_dest_addr,4,1,file);
	fwrite(&psexe.file_size,4,1,file);
	fwrite(&psexe.unknown_1,4,1,file);
	fwrite(&psexe.unknown_2,4,1,file);
	fwrite(&psexe.mem_fill_start_addr,4,1,file);
	fwrite(&psexe.mem_fill_size,4,1,file);
	fwrite(&psexe.init_sp_base,4,1,file);
	fwrite(&psexe.init_sp_offset,4,1,file);
	fwrite(&psexe.reserved,1,20,file);
}

void VS_WritePSEXE(char* filename, unsigned long org, int output){
	FILE* file, *textsec, *datasec;
	VS_PSEXE psexe;
	unsigned long instruction_arr[VS_MAX_INSTRUCTION_READ], count, num_of_instructions, datasec_size, file_size;
	int ext;
	unsigned char byte_arr[VS_MAX_READ];
	char name[256];
	
	memset(&psexe,0x0,sizeof(VS_PSEXE));
	memset(name,0x0,255);
	
	ext = VS_FilenameHasExtension(filename);
	
	if(ext != -1){
		if(output){
			memcpy(name,filename,strlen(filename) & 0xff);
		}
		else{
			memcpy(name,filename, ext & 0xff);
			strcat(name,".exe");
		}
	}
	else{
		memcpy(name,filename,strlen(filename) & 0xfb);
		strcat(name,".exe");
	}
	
	psexe.psexe_magic_id[0] = 'P';psexe.psexe_magic_id[1] = 'S';psexe.psexe_magic_id[2] = '-';psexe.psexe_magic_id[3] = 'X';psexe.psexe_magic_id[4] = ' ';
	psexe.psexe_magic_id[5] = 'E'; psexe.psexe_magic_id[6] = 'X'; psexe.psexe_magic_id[7] = 'E';
	psexe.pc = org;
	psexe.gp = 0xFFFFFFFF;
	psexe.ram_dest_addr = org;
	psexe.file_size = 0;
	psexe.unknown_1 = 0;
	psexe.unknown_2 = 0;
	psexe.mem_fill_start_addr = 0;
	psexe.mem_fill_size = 0;
	psexe.init_sp_base = 0x801FFFF0;
	psexe.init_sp_offset = 0;
	
	file = fopen(name,"wb");
	
	VS_WritePSEXEHeader(file,psexe);
	
	fseek(file,0x800,SEEK_SET);
	
	textsec = fopen("textsec.dat","rb");
	
	if(textsec != NULL){
		fseek(textsec,0x0,SEEK_END);
		num_of_instructions = ftell(textsec) >> 2;
		fseek(textsec,0x0,SEEK_SET);
	
		unsigned long i;
		for(i = 0; i < num_of_instructions;){
			count = fread(instruction_arr,4,VS_MAX_INSTRUCTION_READ,textsec);
			fwrite(instruction_arr,4,count,file);
			i += count;
		}
		
		fclose(textsec);
	}
	
	datasec = fopen("datasec.dat","rb");
	
	if(datasec != NULL){
		fseek(datasec,0x0,SEEK_END);
		datasec_size = ftell(datasec);
		fseek(datasec,0x0,SEEK_SET);
		
		unsigned long i;
		for(i = 0; i < datasec_size;){
			count = fread(byte_arr,1,VS_MAX_READ,datasec);
			fwrite(byte_arr,1,count,file);
			i += count;
		}
		
		fclose(datasec);
	}
	
	file_size = ftell(file);
	VS_AlignOffset(file,&file_size,0x800);
	fseek(file,28,SEEK_SET);
	psexe.file_size = file_size - 0x800;
	fwrite(&psexe.file_size,4,1,file);
	
	fclose(file);
}
