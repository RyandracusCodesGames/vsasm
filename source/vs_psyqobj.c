#include <vs_psyqobj.h>
#include <stdlib.h>
#include <string.h>
#include <vs_symtable.h>
#include <vs_utils.h>
#include <vs_elf.h>

/********************************************
*   VideoStation Assembler
*
*   Copyright (c) 2025 Ryandracus Chapman
*
*   File: vs_psyqobj.c
*   Date: 5/25/2025
*   Version: 1.1
*   Updated: 6/6/2025
*   Author: Ryandracus Chapman
*
********************************************/

unsigned char end_marker = 0x2E;
unsigned char switch_cmd = 6;
unsigned char code_cmd = 2;
unsigned char sld_info_cmd = 60;
unsigned char reloc_cmd = 0x0A;
unsigned char jump_reloc_cmd = 74;
unsigned char hi_reloc_cmd = 82;
unsigned char lo_reloc_cmd = 84;
unsigned char pc16_reloc_cmd = 30;
unsigned char reloc_unknown_cmd1 = 0x2C;
unsigned char reloc_unknown_cmd2 = 0x4;
unsigned char xdef_symbol_cmd = 0x0C;
unsigned char local_symbol_cmd = 0x12;
unsigned char xref_symbol_cmd = 0x0E; /* unknown/unresolved symbol */
unsigned char xbss_symbol_cmd = 48;

unsigned long psyq_reloc_size;
unsigned char psyq_contains_reloc;

VS_PSYQ_RELOC psyq_reloc_table[VS_MAX_RELOCS];

void VS_InitPSYQRelocTable(){
	psyq_reloc_size = 0;
	psyq_contains_reloc = 0;
}

void VS_SetPSYQRelocTrue(){
	psyq_contains_reloc = 1;
}

void VS_AddPSYQRelocEntry(unsigned long index, unsigned char reloc_type, unsigned short offset, unsigned short section_number, unsigned short dest_symbol_addr){
	if(psyq_reloc_size < VS_MAX_RELOCS){
		
		psyq_reloc_table[psyq_reloc_size].index = index;
		psyq_reloc_table[psyq_reloc_size].reloc_type = reloc_type;
		psyq_reloc_table[psyq_reloc_size].reloc_offset = offset;
		psyq_reloc_table[psyq_reloc_size].section_num = section_number;
		psyq_reloc_table[psyq_reloc_size].dest_symbol_addr = dest_symbol_addr;
		psyq_reloc_table[psyq_reloc_size].undefined = 0;
		
		psyq_reloc_size++;
	}
}

void VS_AddUndefinedPSYQRelocEntry(unsigned long index, unsigned char reloc_type, unsigned short offset, unsigned short section_number, unsigned short dest_symbol_addr){
	if(psyq_reloc_size < VS_MAX_RELOCS){
		
		psyq_reloc_table[psyq_reloc_size].index = index;
		psyq_reloc_table[psyq_reloc_size].reloc_type = reloc_type;
		psyq_reloc_table[psyq_reloc_size].reloc_offset = offset;
		psyq_reloc_table[psyq_reloc_size].section_num = section_number;
		psyq_reloc_table[psyq_reloc_size].dest_symbol_addr = dest_symbol_addr;
		psyq_reloc_table[psyq_reloc_size].undefined = 1;
		
		psyq_reloc_size++;
	}
}

void VS_WritePSYQHeader(FILE* file, VS_PSYQ_HEADER header){
	fwrite(&header.magic,4,1,file);
	fwrite(&end_marker,1,1,file);
	fwrite(&header.processor_type,1,1,file);
}

void VS_WritePSYQSectionHeader(FILE* file, VS_PSYQ_SECTION_HEADER header){
	fwrite(&header.magic,1,1,file);
	fwrite(&header.section_number,1,1,file);
	fwrite(&header.group,2,1,file);
	fwrite(&header.alignment,2,1,file);
	fwrite(&header.symbol_strlen,1,1,file);
	fwrite(&header.symbol_name,1,header.symbol_strlen,file);
}

void VS_SwitchSection(FILE* file, unsigned short section){
	fwrite(&switch_cmd,1,1,file);
	fwrite(&section,2,1,file);
}

void VS_WriteCodeCmd(FILE* file, unsigned short size){
	fwrite(&code_cmd,1,1,file);
	fwrite(&size,2,1,file);
}

void VS_WriteSLDInfoCmd(FILE* file, unsigned short offset){
	fwrite(&sld_info_cmd,1,1,file);
	fwrite(&offset,2,1,file);
}

void VS_WritePSYQSymbol(FILE* file, VS_PSYQ_SYMBOL symbol, int und){	
	fwrite(&symbol.cmd,1,1,file);
	fwrite(&symbol.symbol_num,2,1,file);
	
	if(und != VS_SYM_UND){
		fwrite(&symbol.section_number,2,1,file);
		fwrite(&symbol.offset,4,1,file);
	}
	
	fwrite(&symbol.symbol_strlen,1,1,file);
}

void VS_WritePSYQRelocEntry(FILE* file, VS_PSYQ_RELOC reloc){
	unsigned char zero = 0;
	
	reloc.dest_symbol_addr = VS_SwapShort(reloc.dest_symbol_addr);
	
	fwrite(&reloc_cmd,1,1,file);
	fwrite(&reloc.reloc_type,1,1,file);
	fwrite(&reloc.reloc_offset,2,1,file);
	fwrite(&reloc_unknown_cmd1,1,1,file);
	fwrite(&reloc_unknown_cmd2,1,1,file);
	fwrite(&reloc.section_num,2,1,file);
	fwrite(&reloc.dest_symbol_addr,2,1,file);
	fwrite(&zero,1,1,file);
	fwrite(&zero,1,1,file);
	fwrite(&zero,1,1,file);

}

void VS_WriteUndefinedPSYQRelocEntry(FILE* file, VS_PSYQ_RELOC reloc, unsigned short index){
	if(reloc.reloc_type != VS_PSYQ_PC16){
		fwrite(&reloc_cmd,1,1,file);
		fwrite(&reloc.reloc_type,1,1,file);
		fwrite(&reloc.reloc_offset,2,1,file);
		fwrite(&reloc.section_num,1,1,file);
		fwrite(&index,2,1,file);
	}
	else{
		unsigned short cmd1 = 0x32;
		unsigned long cmd2 = 0x4;
		unsigned char cmd3[6] = {0x2E,0x2C,0x04,0x02,0x0,0x0};
		unsigned long offset;
		
		fwrite(&reloc_cmd,1,1,file);
		fwrite(&reloc.reloc_type,1,1,file);
		fwrite(&reloc.reloc_offset,2,1,file);
		fwrite(&cmd1,2,1,file);
		fwrite(&cmd2,4,1,file);
		
		int i;
		for(i = 0; i < 6; i++){
			fwrite(&cmd3[i],1,1,file);
		}
		
		offset = reloc.reloc_offset + 4;
		
		fwrite(&offset,4,1,file);
		fwrite(&reloc.section_num,1,1,file);
		fwrite(&index,2,1,file);
	}
}


void VS_PrintPSYQRelocTable(){
	printf("PSYQ OBJ Relocation Table:\n\n");
	
	unsigned long i;
	for(i = 0; i < psyq_reloc_size; i++){
		printf("symbol index = %ld\n",psyq_reloc_table[i].index);
		printf("reloc type = %d\n",psyq_reloc_table[i].reloc_type);
		printf("reloc_ offset = 0x%x\n",psyq_reloc_table[i].reloc_offset);
		printf("reloc section number = %d\n",psyq_reloc_table[i].section_num);
		printf("reloc dest symbol address = %ld\n\n",psyq_reloc_table[i].dest_symbol_addr);
	}
}

void VS_WritePSYQObj(char* filename){
	FILE* file, *textsec, *datasec;
	VS_PSYQ_HEADER header;
	VS_PSYQ_SECTION_HEADER sheader[8];
	VS_PSYQ_SYMBOL symbol;
	VS_SYM sym;
	unsigned long i, size, instruction, num_of_instructions;
	int data_size, count;
	unsigned char zero = 0, byte_arr[VS_MAX_READ];
	
	header.magic[0] = 'L'; header.magic[1] = 'N'; header.magic[2] = 'K'; header.magic[3] = 2;
	header.processor_type = 7;
	
	file = fopen(filename,"wb");
	
	VS_WritePSYQHeader(file,header);
	
	sheader[0].magic = 0x10;
	sheader[0].section_number = 1;
	sheader[0].group = 0;
	sheader[0].alignment = VS_SwapShort(8);
	sheader[0].symbol_strlen = 6;
	memcpy(sheader[0].symbol_name,".rdata",sheader[0].symbol_strlen);
	
	sheader[1].magic = 0x10;
	sheader[1].section_number = 2;
	sheader[1].group = 0;
	sheader[1].alignment = VS_SwapShort(8);
	sheader[1].symbol_strlen = 5;
	memcpy(sheader[1].symbol_name,".text",sheader[1].symbol_strlen);
	
	sheader[2].magic = 0x10;
	sheader[2].section_number = 3;
	sheader[2].group = 0;
	sheader[2].alignment = VS_SwapShort(8);
	sheader[2].symbol_strlen = 5;
	memcpy(sheader[2].symbol_name,".data",sheader[2].symbol_strlen);
	
	sheader[3].magic = 0x10;
	sheader[3].section_number = 4;
	sheader[3].group = 0;
	sheader[3].alignment = VS_SwapShort(8);
	sheader[3].symbol_strlen = 6;
	memcpy(sheader[3].symbol_name,".sdata",sheader[3].symbol_strlen);
	
	sheader[4].magic = 0x10;
	sheader[4].section_number = 5;
	sheader[4].group = 0;
	sheader[4].alignment = VS_SwapShort(8);
	sheader[4].symbol_strlen = 5;
	memcpy(sheader[4].symbol_name,".sbss",sheader[4].symbol_strlen);
	
	sheader[5].magic = 0x10;
	sheader[5].section_number = 6;
	sheader[5].group = 0;
	sheader[5].alignment = VS_SwapShort(8);
	sheader[5].symbol_strlen = 4;
	memcpy(sheader[5].symbol_name,".bss",sheader[5].symbol_strlen);
	
	sheader[6].magic = 0x10;
	sheader[6].section_number = 7;
	sheader[6].group = 0;
	sheader[6].alignment = VS_SwapShort(8);
	sheader[6].symbol_strlen = 6;
	memcpy(sheader[6].symbol_name,".ctors",sheader[6].symbol_strlen);
	
	sheader[7].magic = 0x10;
	sheader[7].section_number = 8;
	sheader[7].group = 0;
	sheader[7].alignment = VS_SwapShort(8);
	sheader[7].symbol_strlen = 6;
	memcpy(sheader[7].symbol_name,".dtors",sheader[7].symbol_strlen);
	
	for(i = 0; i < 8; i++){
		VS_WritePSYQSectionHeader(file,sheader[i]);
	}
	
	size = VS_GetSymbolTableSize();
	
	VS_SwitchSection(file,2);
	
	textsec = fopen("textsec.dat","rb");
	
	if(textsec != NULL){
		for(i = 0; i < size; i++){
			sym = VS_GetSymbolFromIndex(i);
			
			if(sym.type != VS_SYM_FUNC && sym.type != VS_SYM_UND){
				continue;
			}

			num_of_instructions = sym.num_of_instructions;
			
			if(sym.type == VS_SYM_FUNC){
				VS_SwitchSection(file,2);
				VS_WriteCodeCmd(file,sym.num_of_instructions << 2);
			}
			
			unsigned long j;
			for(j = 0; j < num_of_instructions; j++){
				fread(&instruction,4,1,textsec);
				fwrite(&instruction,4,1,file);
			}

			if(psyq_contains_reloc){
				for(j = 0; j < psyq_reloc_size; j++){
					if(psyq_reloc_table[j].index == i){
						if(!psyq_reloc_table[j].undefined){
							VS_WritePSYQRelocEntry(file,psyq_reloc_table[j]);
						}
						else{
							VS_WriteUndefinedPSYQRelocEntry(file,psyq_reloc_table[j],psyq_reloc_table[j].dest_symbol_addr+1);
						}
					}
				}
			}
		}
		
		fclose(textsec);
	}
	
	datasec = fopen("datasec.dat","rb");
	
	if(datasec != NULL){
		for(i = 0; i < size; i++){
			sym = VS_GetSymbolFromIndex(i);
			
			if(sym.type != VS_SYM_OBJ){
				continue;
			}
			
			VS_SwitchSection(file,3);
			VS_WriteCodeCmd(file,sym.size);
			
			data_size = sym.size;
			
			while(data_size > 0){
				int read_size;
				
				if(data_size < VS_MAX_READ){
					read_size = data_size;
				}
				else{
					read_size = VS_MAX_READ;
				}
				
				count = fread(byte_arr,1,read_size,datasec);
				fwrite(byte_arr,1,count,file);
				data_size -= read_size;
			}
		}
		
		fclose(datasec);
	}
	
	VS_SwitchSection(file,2);
	VS_WriteSLDInfoCmd(file,0);

	for(i = 0; i < size; i++){
		sym = VS_GetSymbolFromIndex(i);
		
		if(sym.type == VS_SYM_UND){
			symbol.cmd = xref_symbol_cmd;
		}
		else{
			symbol.cmd = xdef_symbol_cmd;
		}
		
		symbol.symbol_num = i + 9;
		
		if(sym.type == VS_SYM_UND){
			symbol.section_number = sym.string_table_index;
		}
		else if(sym.type == VS_SYM_FUNC){
			symbol.section_number = 2;
		}
		else{
			symbol.section_number = 3;
		}
		
		symbol.offset = sym.addr;
		symbol.symbol_strlen = strlen(sym.name);
		
		VS_WritePSYQSymbol(file,symbol,sym.type);
		
		fwrite(sym.name,1,strlen(sym.name),file);
	}
	
	fwrite(&zero,1,1,file);
	
	fclose(file);
}