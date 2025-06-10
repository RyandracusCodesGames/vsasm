#include <vs_cpe.h>
#include <vs_symtable.h>
#include <vs_preprocessor.h>
#include <vs_psexe.h>
#include <vs_elf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

void VS_WriteCPEChunk1(FILE* file, VS_CPE_CHUNK1 chunk1){
	fwrite(&chunk1.id,1,1,file);
	fwrite(&chunk1.addr,4,1,file);
	fwrite(&chunk1.size,4,1,file);
}

void VS_WriteCPEChunk3(FILE* file, VS_CPE_CHUNK3 chunk3){
	fwrite(&chunk3.id,1,1,file);
	fwrite(&chunk3.reg,2,1,file);
	fwrite(&chunk3.value,4,1,file);
}

void VS_WriteCPE(char* filename, unsigned long org, int output){
	FILE* file, *textsec, *datasec;
	VS_CPE_CHUNK1 chunk1;
	VS_CPE_CHUNK3 chunk3;
	VS_SYM sym;
	unsigned long instruction, count, num_of_instructions;
	unsigned long size, i, j;
	int ext, datasec_size;
	unsigned char byte_arr[VS_MAX_READ];
	unsigned char header[4] = {'C','P','E',0x1};
	unsigned char select_unit[2] = {0x8,0x0};
	unsigned char zero = 0;
	char name[256];

	memset(name,0x0,255);
	
	ext = VS_FilenameHasExtension(filename);
	
	if(ext != -1){
		if(output){
			memcpy(name,filename,strlen(filename) & 0xff);
		}
		else{
			memcpy(name,filename, ext & 0xff);
			strcat(name,".cpe");
		}
	}
	else{
		memcpy(name,filename,strlen(filename) & 0xfb);
		strcat(name,".cpe");
	}
	
	file = fopen(name,"wb");
	
	for(i = 0; i < 4; i++){
		fwrite(&header[i],1,1,file);
	}
	
	for(i = 0; i < 2; i++){
		fwrite(&select_unit[i],1,1,file);
	}
	
	chunk3.id = 0x3;
	chunk3.reg = 0x90;
	chunk3.value = org;
	
	VS_WriteCPEChunk3(file,chunk3);
	
	textsec = fopen("textsec.dat","rb");
	
	if(textsec != NULL){
		size = VS_GetSymbolTableSize();
		
		for(i = 0; i < size; i++){
			sym = VS_GetSymbolFromIndex(i);
			
			if(sym.type != VS_SYM_FUNC){
				continue;
			}
			
			chunk1.id = 0x1;
			chunk1.addr = org + sym.addr;
			chunk1.size = sym.num_of_instructions << 2;
			
			VS_WriteCPEChunk1(file,chunk1);
			
			num_of_instructions = sym.num_of_instructions;

			for(j = 0; j < num_of_instructions; j++){
				fread(&instruction,4,1,textsec);
				fwrite(&instruction,4,1,file);
			}	
		}
		
		fclose(textsec);
	}
	
	datasec = fopen("datasec.dat","rb");
	
	if(datasec != NULL){
		size = VS_GetSymbolTableSize();
		
		for(i = 0; i < size; i++){
			sym = VS_GetSymbolFromIndex(i);
			
			if(sym.type != VS_SYM_OBJ){
				continue;
			}
			
			chunk1.id = 0x1;
			chunk1.addr = sym.addr;
			chunk1.size = sym.size;
			
			VS_WriteCPEChunk1(file,chunk1);
			
			datasec_size = sym.size;
			
			while(datasec_size > 0){
				int read_size;
				
				if(datasec_size < VS_MAX_READ){
					read_size = datasec_size;
				}
				else{
					read_size = VS_MAX_READ;
				}
				
				count = fread(byte_arr,1,read_size,datasec);
				fwrite(byte_arr,1,count,file);
				datasec_size -= read_size;
			}
		}
		
		fclose(datasec);
	}
	
	fwrite(&zero,1,1,file);
	
	fclose(file);
}

unsigned long VS_FindRamDestAddr(FILE* in){
	VS_CPE_CHUNK1 chunk1;
	unsigned long addr = 0, pos = ftell(in);
	unsigned char byte;
	
	do{
		fread(&byte,1,1,in);
		
		switch(byte){
			case 0:{ break; }
			case 1:{
				fread(&chunk1.addr,4,1,in);
				fread(&chunk1.size,4,1,in);
				
				if(addr == 0 || chunk1.addr < addr){
                    addr = chunk1.addr;
                }
				
				fseek(in,chunk1.size,SEEK_CUR);
				
			}break;
			case 2:
			case 7:{
				fseek(in,0x4,SEEK_CUR);
			}break;
			case 8:{
				fseek(in,0x1,SEEK_CUR);
			}break;
		}
		
	}while(byte != 0);
	
	fseek(in,pos,SEEK_SET);
	
	return addr;
}

void VS_CPE2EXE(char* filein, char* fileout){
	FILE* in, *out;
	VS_PSEXE psexe;
	VS_CPE_CHUNK1 chunk1;
	VS_CPE_CHUNK3 chunk3;
	unsigned long count, offset, file_size;
	int ext, datasec_size;
	unsigned char byte_arr[VS_MAX_READ];
	unsigned char header[4] = {'C','P','E',0x1};
	unsigned char arr[4];
	unsigned char select_unit[2] = {0x8,0x0};
	unsigned char byte;
	char name[256];

	memset(name,0x0,255);
	
	ext = VS_FilenameHasExtension(filein);
	
	if(ext != -1){
		if(!strcmp(filein,fileout)){
			memcpy(name,fileout, ext & 0xff);
			strcat(name,".exe");
		}
		else{
			memcpy(name,fileout,strlen(fileout) & 0xff);
		}
	}
	else{
		if(!strcmp(filein,fileout)){
			memcpy(name,filein,strlen(filein) & 0xfb);
			strcat(name,".exe");
		}
		else{
			memcpy(name,fileout,strlen(fileout) & 0xff);
		}
	}
	
	in = fopen(filein,"rb");
	
	if(in == NULL){
		printf("Error: Aborting CPE to EXE conversion. Input file not found - %s\n",filein);
		return;
	}

	fread(arr,1,4,in);
	
	if((header[0] != arr[0]) || (header[1] != arr[1]) || (header[2] != arr[2]) || (header[3] != arr[3])){
		printf("Error: Aborting CPE to EXE conversion. Invalid CPE header - %s\n",filein);
		fclose(in);
		return;
	}
	
	fread(arr,1,2,in);
	
	if((select_unit[0] != arr[0])){
		printf("Error: Aborting CPE to EXE conversion. First chunk must be select unit chunk  - %s\n",filein);
		fclose(in);
		return;
	}
	
	fread(&byte,1,1,in);
	
	if(byte != 3){
		printf("Error: Aborting CPE to EXE conversion. Second chunk must be entrypoint chunk 3 - %s\n",filein);
		fclose(in);
		return;
	}
	
	fread(&chunk3.reg,2,1,in);
	fread(&chunk3.value,4,1,in);
	
	psexe.psexe_magic_id[0] = 'P';psexe.psexe_magic_id[1] = 'S';psexe.psexe_magic_id[2] = '-';psexe.psexe_magic_id[3] = 'X';psexe.psexe_magic_id[4] = ' ';
	psexe.psexe_magic_id[5] = 'E'; psexe.psexe_magic_id[6] = 'X'; psexe.psexe_magic_id[7] = 'E';
	psexe.pc = chunk3.value;
	psexe.gp = 0;
	psexe.ram_dest_addr = VS_FindRamDestAddr(in);
	psexe.file_size = 0;
	psexe.unknown_1 = 0;
	psexe.unknown_2 = 0;
	psexe.mem_fill_start_addr = 0;
	psexe.mem_fill_size = 0;
	psexe.init_sp_base = 0x801FFFF0;
	psexe.init_sp_offset = 0;
	
	out = fopen(name,"wb");
	
	VS_WritePSEXEHeader(out,psexe);
	fseek(out,0x800,SEEK_SET);
	
	do{
		fread(&byte,1,1,in);
		
		switch(byte){
			case 0:{ break; }
			case 1:{
				fread(&chunk1.addr,4,1,in);
				fread(&chunk1.size,4,1,in);

				offset = (chunk1.addr - psexe.ram_dest_addr) + 0x800;
				
				fseek(out,offset,SEEK_SET);
				
				datasec_size = chunk1.size;
				
				while(datasec_size > 0){
					int read_size;
					
					if(datasec_size < VS_MAX_READ){
						read_size = datasec_size;
					}
					else{
						read_size = VS_MAX_READ;
					}
					
					count = fread(byte_arr,1,read_size,in);
					fwrite(byte_arr,1,count,out);
					datasec_size -= read_size;
				}
			}break;
			case 2:
			case 7:{
				fseek(in,0x4,SEEK_CUR);
			}break;
			case 8:{
				fseek(in,0x1,SEEK_CUR);
			}break;
		}
		
	}while(byte != 0);
	
	fseek(out,0x0,SEEK_END);
	file_size = ftell(out);
	VS_AlignOffset(out,&file_size,0x800);
	fseek(out,0,SEEK_SET);
	psexe.file_size = file_size - 0x800;
	VS_WritePSEXEHeader(out,psexe);
	
	fclose(in);
	fclose(out);
}