#include <vs_elf.h>
#include <stdlib.h>
#include <string.h>
#include <vs_symtable.h>
#include <vs_opcode.h>

/********************************************
*   VideoStation Assembler
*
*   Copyright (c) 2025 Ryandracus Chapman
*
*   File: vs_elf.c
*   Date: 4/30/2025
*   Version: 1.1
*   Updated: 6/10/2025
*   Author: Ryandracus Chapman
*
********************************************/

unsigned long reloc_size;
unsigned char elf_contains_reloc;
VS_ELF_RELOC reloc_table[VS_MAX_RELOCS];

void VS_InitRelocTable(){
	reloc_size = 0;
	elf_contains_reloc = 0;
}

void VS_SetRelocTrue(){
	elf_contains_reloc = 1;
}

void VS_AddRelocEntry(unsigned long offset, unsigned long reloc_type, int is_jump){
	if(reloc_size < VS_MAX_RELOCS){
		
		reloc_table[reloc_size].r_offset = offset;
		
		if(is_jump){
			reloc_table[reloc_size].r_info = VS_ELF32_R_INFO(1,reloc_type);
		}
		else{
			reloc_table[reloc_size].r_info = VS_ELF32_R_INFO(2,reloc_type);
		}
		
		reloc_size++;
	}
}

void VS_AddUndefinedRelocEntry(unsigned long offset, unsigned long reloc_type, int index){
	if(reloc_size < VS_MAX_RELOCS){
		reloc_table[reloc_size].r_offset = offset;
		reloc_table[reloc_size].r_info = VS_ELF32_R_INFO(index,reloc_type);
		reloc_size++;
	}
}

void VS_PrintRelocTable(){
	printf("ELF Relocation Table:\n\n");
	
	unsigned long i;
	for(i = 0; i < reloc_size; i++){
		printf("r_offset = 0x%lx\n",reloc_table[i].r_offset);
		printf("r_info = 0x%lx\n\n",reloc_table[i].r_info);
	}
}

void VS_AlignOffset(FILE* file, unsigned long* offset, unsigned long align){
	unsigned long off = *offset;
	unsigned char zero = 0;
	
	while(off % align){
		off++;
		fwrite(&zero,1,1,file);
	}
	
	*offset = off;
}

int VS_FilenameHasExtension(char* filename){
	int i, len = strlen(filename);
	for(i = 0; i < len; i++){
		if(filename[i] == '.'){
			return i;
		}
	}
	
	return -1;
}

int VS_WriteELF(char* filename, VS_ASM_PARAMS params){
	FILE* file, *textsec, *datasec;
	VS_ELF elf;
	VS_SECTION_HEADER section_header;
	VS_SYM sym;
	unsigned long size, num_of_instructions, textsec_offset, datasec_offset;
	unsigned long datasec_size, symtab_offset, symtab_string_offset, symtab_string_size, string_offset;
	unsigned long bsssec_size, bsssec_offset, mipssec_offset, gnu_offset, reginfo_offset;
	unsigned long symtab_size, strtab_offset, offset, reloc_strlen, reloc_offset;
	unsigned char byte, byte_arr[VS_MAX_READ], zero = 0;
	unsigned long instruction_arr[VS_MAX_INSTRUCTION_READ], count;
	char section[16384];
	VS_ELF_SYMBOL elf_symbols[7];
	
	unsigned char mips_attribute_arr[25] = {0,0,1,0,1,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	unsigned char reginfo_arr[25] = {0,3,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	unsigned char gnu_attributes_arr[17] = {0x41,0xF,0,0,0,0x67,0x6E,0x75,0,1,7,0,0,0,4,1};

	int start, end;
	
	reloc_strlen = 0;

	memset(&elf,0x0,sizeof(VS_ELF));
	memset(section,0x0,16384);

	elf.magic[0] = 0x7F; elf.magic[1] = 'E'; elf.magic[2] = 'L'; elf.magic[3] = 'F'; 
	elf.bits = 1;
	elf.endian = 1;
	elf.version = 1;
	elf.os = 0;
	elf.abi_version = 0;
	elf.obj_type = 0x1;
	elf.architecture = 0x8;
	elf.elf_version = 1;
	elf.entry = 0;
	elf.program_header_offset = 0;
	elf.section_header_table = 0;
	elf.flags = 0x1000;
	elf.size = 52;
	elf.program_header_size = 0;
	elf.program_header_num = 0;
	elf.section_header_size = 40;
	
	if(params.architecture == VS_MIPS_II_ARCH || params.architecture == VS_MIPS_PSP_ARCH){
		mips_attribute_arr[2] = 2, mips_attribute_arr[7] = 2;
		gnu_attributes_arr[15] = 2;
		elf.flags = 0x10a23000;
	}
	
	if(elf_contains_reloc){
		elf.section_header_num = 12;
		elf.shstrndx = 11;
	}
	else{
		elf.section_header_num = 11;
		elf.shstrndx = 10;
	}
	
	textsec_offset = 0; datasec_offset = 0; bsssec_offset = 0;
	datasec_size = 0; bsssec_size = 0; num_of_instructions = 0;
	
	file = fopen(filename,"wb");
	
	VS_WriteELFHeader(file,elf);
	
	textsec = fopen("textsec.dat","rb");
	
	if(textsec != NULL){
		textsec_offset = 0x34;
		
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
	datasec_offset = 0x34 + (num_of_instructions << 2);
	
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
	
	reginfo_offset = ftell(file);
	
	VS_AlignOffset(file,&reginfo_offset,4);
	
	unsigned long i;
	for(i = 0; i < 24; i++)
		fwrite(&reginfo_arr[i],1,1,file);
	
	mipssec_offset = ftell(file);
	
	VS_AlignOffset(file,&mipssec_offset,8);
	
	for(i = 0; i < 24; i++)
		fwrite(&mips_attribute_arr[i],1,1,file);
	
	gnu_offset = ftell(file);
	
	VS_AlignOffset(file,&gnu_offset,4);
	
	for(i = 0; i < 16; i++)
		fwrite(&gnu_attributes_arr[i],1,1,file);
	
	bsssec_offset = reginfo_offset;
	symtab_offset = ftell(file);
	
	VS_AlignOffset(file,&symtab_offset,4);

	size = VS_GetSymbolTableSize();
		
	start = ftell(file);
	
	for(i = 0; i < 16; i++)
		fwrite(&zero,1,1,file);
	
	if(elf_contains_reloc){
		/* TEXT SYMBOL */
		elf_symbols[0].st_name = 1;
		elf_symbols[0].st_value = 0;
		elf_symbols[0].st_size = 0;
		elf_symbols[0].st_info = 3;
		elf_symbols[0].st_other = 0;
		elf_symbols[0].st_shndx = 1;
		
		/* DATA SYMBOL */
		elf_symbols[1].st_name = 7;
		elf_symbols[1].st_value = 0;
		elf_symbols[1].st_size = 0;
		elf_symbols[1].st_info = 3;
		elf_symbols[1].st_other = 0;
		elf_symbols[1].st_shndx = 2;
		
		/* BSS SYMBOL */
		elf_symbols[2].st_name = 13;
		elf_symbols[2].st_value = 0;
		elf_symbols[2].st_size = 0;
		elf_symbols[2].st_info = 3;
		elf_symbols[2].st_other = 0;
		elf_symbols[2].st_shndx = 3;
		
		/* REGINFO SYMBOL */
		elf_symbols[3].st_name = 0x12;
		elf_symbols[3].st_value = 0;
		elf_symbols[3].st_size = 0;
		elf_symbols[3].st_info = 3;
		elf_symbols[3].st_other = 0;
		elf_symbols[3].st_shndx = 4;
		
		/* MIPS ATTRIBUTES SYMBOL */
		elf_symbols[4].st_name = 27;
		elf_symbols[4].st_value = 0;
		elf_symbols[4].st_size = 0;
		elf_symbols[4].st_info = 3;
		elf_symbols[4].st_other = 0;
		elf_symbols[4].st_shndx = 5;
		
		/* PDR SYMBOL */
		elf_symbols[5].st_name = 42;
		elf_symbols[5].st_value = 0;
		elf_symbols[5].st_size = 0;
		elf_symbols[5].st_info = 3;
		elf_symbols[5].st_other = 0;
		elf_symbols[5].st_shndx = 6;
		
		/* GNU ATTRIBUTES SYMBOL */
		elf_symbols[6].st_name = 47;
		elf_symbols[6].st_value = 0;
		elf_symbols[6].st_size = 0;
		elf_symbols[6].st_info = 3;
		elf_symbols[6].st_other = 0;
		elf_symbols[6].st_shndx = 7;
	}
	else{
		/* TEXT SYMBOL */
		elf_symbols[0].st_name = 1;
		elf_symbols[0].st_value = 0;
		elf_symbols[0].st_size = 0;
		elf_symbols[0].st_info = 3;
		elf_symbols[0].st_other = 0;
		elf_symbols[0].st_shndx = 1;
		
		/* DATA SYMBOL */
		elf_symbols[1].st_name = 7;
		elf_symbols[1].st_value = 0;
		elf_symbols[1].st_size = 0;
		elf_symbols[1].st_info = 3;
		elf_symbols[1].st_other = 0;
		elf_symbols[1].st_shndx = 2;
		
		/* BSS SYMBOL */
		elf_symbols[2].st_name = 13;
		elf_symbols[2].st_value = 0;
		elf_symbols[2].st_size = 0;
		elf_symbols[2].st_info = 3;
		elf_symbols[2].st_other = 0;
		elf_symbols[2].st_shndx = 3;
		
		/* REGINFO SYMBOL */
		elf_symbols[3].st_name = 0x12;
		elf_symbols[3].st_value = 0;
		elf_symbols[3].st_size = 0;
		elf_symbols[3].st_info = 3;
		elf_symbols[3].st_other = 0;
		elf_symbols[3].st_shndx = 4;
		
		/* MIPS ATTRIBUTES SYMBOL */
		elf_symbols[4].st_name = 27;
		elf_symbols[4].st_value = 0;
		elf_symbols[4].st_size = 0;
		elf_symbols[4].st_info = 3;
		elf_symbols[4].st_other = 0;
		elf_symbols[4].st_shndx = 5;
		
		/* PDR SYMBOL */
		elf_symbols[5].st_name = 42;
		elf_symbols[5].st_value = 0;
		elf_symbols[5].st_size = 0;
		elf_symbols[5].st_info = 3;
		elf_symbols[5].st_other = 0;
		elf_symbols[5].st_shndx = 6;
		
		/* GNU ATTRIBUTES SYMBOL */
		elf_symbols[6].st_name = 47;
		elf_symbols[6].st_value = 0;
		elf_symbols[6].st_size = 0;
		elf_symbols[6].st_info = 3;
		elf_symbols[6].st_other = 0;
		elf_symbols[6].st_shndx = 7;
	}
	
	for(i = 0; i < 7; i++){
		VS_WriteELFSymbol(file,elf_symbols[i]);
	}
	
	string_offset = 63;
	
	for(i = 0; i < size; i++){
		sym = VS_GetSymbolFromIndex(i);

		offset = string_offset;

		fwrite(&offset,4,1,file);
		
		if(sym.type == VS_SYM_UND){
			sym.addr = 0;
			fwrite(&sym.addr,4,1,file);
		}
		else{
			fwrite(&sym.addr,4,1,file);
		}
		
		fwrite(&sym.size,4,1,file);
		
		string_offset += strlen(sym.name) + 1;
		
		byte =  VS_ELF32_ST_INFO(sym.scope,sym.type);
		
		fwrite(&byte,1,1,file);
		fwrite(&zero,1,1,file);
		
		unsigned long hword = 0;
		
		if(sym.type == VS_SYM_FUNC){
			hword = 1;
		}
		else if(sym.type == VS_SYM_UND){
			hword = 0;
		}
		else{
			if(elf_contains_reloc){
				hword = 3;
			}
			else{
				hword = 2;
			}
		}
		
		fwrite(&hword,2,1,file);
	}
	
	fwrite(&zero,1,1,file);
	
	end = ftell(file);
	
	VS_WriteELFSymbol(file,elf_symbols[0]);
	
	string_offset = 0;
	
	symtab_string_offset = ftell(file);
	symtab_size = end - start;
	
	strcpy(section + string_offset, ".text"); 
	string_offset += strlen(".text") + 1;
	strcpy(section + string_offset, ".data");
	string_offset += strlen(".data") + 1;
	strcpy(section + string_offset, ".bss");
	string_offset += strlen(".bss") + 1;
	strcpy(section + string_offset, ".reginfo");
	string_offset += strlen(".reginfo") + 1;
	strcpy(section + string_offset, ".MIPS.abiflags");
	string_offset += strlen(".MIPS.abiflags") + 1;
	strcpy(section + string_offset, ".pdr");
	string_offset += strlen(".pdr") + 1;
	strcpy(section + string_offset, ".gnu.attributes");
	string_offset += strlen(".gnu.attributes") + 1;
	
	for(i = 0; i < size; i++){
		sym = VS_GetSymbolFromIndex(i);
		strcpy(section + string_offset, sym.name);
		string_offset += strlen(sym.name) + 1;
	}
	
	fwrite(section,1,string_offset,file);
	
	symtab_string_size = string_offset;
	
	string_offset = 0;
	strtab_offset = ftell(file);
	
	memset(section,0x0,16384);
	
	strcpy(section + string_offset, ".symtab"); 
	string_offset += strlen(".symtab") + 1;
	strcpy(section + string_offset, ".strtab");
	string_offset += strlen(".strtab") + 1;
	strcpy(section + string_offset, ".shstrtab");
	string_offset += strlen(".shstrtab") + 1;
	strcpy(section + string_offset, ".text"); 
	string_offset += strlen(".text") + 1;
	
	if(elf_contains_reloc){
		reloc_strlen = 10;
		strcpy(section + string_offset, ".rel.text"); 
		string_offset += strlen(".rel.text") + 1;
	}
	
	strcpy(section + string_offset, ".data");
	string_offset += strlen(".data") + 1;
	strcpy(section + string_offset, ".bss");
	string_offset += strlen(".bss") + 1;
	strcpy(section + string_offset, ".reginfo");
	string_offset += strlen(".reginfo") + 1;
	strcpy(section + string_offset, ".MIPS.abiflags");
	string_offset += strlen(".MIPS.abiflags") + 1;
	strcpy(section + string_offset, ".pdr");
	string_offset += strlen(".pdr") + 1;
	strcpy(section + string_offset, ".gnu.attributes");
	string_offset += strlen(".gnu.attributes") + 1;
	
	fwrite(section,1,string_offset,file);
	
	offset = ftell(file);
	
	VS_AlignOffset(file,&offset,4);
	
	fseek(file,32,SEEK_SET);
	fwrite(&offset,4,1,file);
	fseek(file,offset,SEEK_SET);
	
	for(i = 0; i < 40; i++)
		fwrite(&zero,1,1,file);
	
	/* WRITE .TEXT SECTION HEADER */
	section_header.sh_name = 27;
	section_header.sh_type = 1;
	section_header.sh_flags = 6;
	section_header.sh_addr = 0;
	section_header.sh_offset = textsec_offset;
	section_header.sh_size = num_of_instructions << 2;
	section_header.sh_link = 0;
	section_header.sh_info = 0;
	section_header.sh_addralign = 4;
	section_header.sh_entsize = 0;
	
	VS_WriteSectionHeader(file,section_header);
	
	if(elf_contains_reloc){
		offset = ftell(file);
		VS_WriteSectionHeader(file,section_header);
	}
	
	/* WRITE .DATA SECTION HEADER */
	section_header.sh_name = 33 + reloc_strlen;
	section_header.sh_type = 1;
	section_header.sh_flags = 3;
	section_header.sh_addr = 0;
	section_header.sh_offset = datasec_offset;
	section_header.sh_size = datasec_size;
	section_header.sh_link = 0;
	section_header.sh_info = 0;
	section_header.sh_addralign = 1;
	section_header.sh_entsize = 0;
	
	VS_WriteSectionHeader(file,section_header);
	
	/* WRITE .BSS SECTION HEADER */
	section_header.sh_name = 39 + reloc_strlen;
	section_header.sh_type = 8;
	section_header.sh_flags = 3;
	section_header.sh_addr = 0;
	section_header.sh_offset = bsssec_offset;
	section_header.sh_size = bsssec_size;
	section_header.sh_link = 0;
	section_header.sh_info = 0;
	section_header.sh_addralign = 1;
	section_header.sh_entsize = 0;
	
	VS_WriteSectionHeader(file,section_header);
	
	/* WRITE .REGINFO SECTION HEADER */
	section_header.sh_name = 44 + reloc_strlen;
	section_header.sh_type = 1879048198;
	section_header.sh_flags = 0;
	section_header.sh_addr = 0;
	section_header.sh_offset = reginfo_offset;
	section_header.sh_size = 0x18;
	section_header.sh_link = 0;
	section_header.sh_info = 0;
	section_header.sh_addralign = 4;
	section_header.sh_entsize = 1;
	
	VS_WriteSectionHeader(file,section_header);
	
	/* WRITE .MIPS.ATTRIBUTES SECTION HEADER */
	section_header.sh_name = 53 + reloc_strlen;
	section_header.sh_type = 1879048234;
	section_header.sh_flags = 2;
	section_header.sh_addr = 0;
	section_header.sh_offset = mipssec_offset;
	section_header.sh_size = 0x18;
	section_header.sh_link = 0;
	section_header.sh_info = 0;
	section_header.sh_addralign = 8;
	section_header.sh_entsize = 0x18;
	
	VS_WriteSectionHeader(file,section_header);
	
	/* WRITE .PDR SECTION HEADER */
	section_header.sh_name = 68 + reloc_strlen;
	section_header.sh_type = 1;
	section_header.sh_flags = 0;
	section_header.sh_addr = 0;
	section_header.sh_offset = gnu_offset;
	section_header.sh_size = 0;
	section_header.sh_link = 0;
	section_header.sh_info = 0;
	section_header.sh_addralign = 4;
	section_header.sh_entsize = 0;
	
	VS_WriteSectionHeader(file,section_header);
	
	/* WRITE .GNU.ATTRIBUTES SECTION HEADER */
	section_header.sh_name = 73 + reloc_strlen;
	section_header.sh_type = 1879048181;
	section_header.sh_flags = 0;
	section_header.sh_addr = 0;
	section_header.sh_offset = gnu_offset;
	section_header.sh_size = 0x10;
	section_header.sh_link = 0;
	section_header.sh_info = 0;
	section_header.sh_addralign = 1;
	section_header.sh_entsize = 0;
	
	VS_WriteSectionHeader(file,section_header);
	
	/* WRITE .SYMTAB SECTION */
	section_header.sh_name = 1;
	section_header.sh_type = 2;
	section_header.sh_flags = 0;
	section_header.sh_addr = 0;
	section_header.sh_offset = symtab_offset;
	section_header.sh_size = symtab_size - 1;
	
	if(elf_contains_reloc){
		section_header.sh_link = 10;
	}
	else{
		section_header.sh_link = 9;
	}
	
	section_header.sh_info = 8;
	section_header.sh_addralign = 4;
	section_header.sh_entsize = 16;
	
	VS_WriteSectionHeader(file,section_header);
	
	section_header.sh_name = 9;
	section_header.sh_type = 3;
	section_header.sh_flags = 0;
	section_header.sh_addr = 0;
	section_header.sh_offset = symtab_string_offset - 1;
	section_header.sh_size = symtab_string_size + 1;
	section_header.sh_link = 0;
	section_header.sh_info = 0;
	section_header.sh_addralign = 1;
	section_header.sh_entsize = 0;
	
	VS_WriteSectionHeader(file,section_header);
	
	section_header.sh_name = 17;
	section_header.sh_type = 3;
	section_header.sh_flags = 0;
	section_header.sh_addr = 0;
	section_header.sh_offset = strtab_offset - 1;
	section_header.sh_size = string_offset + 1;
	section_header.sh_link = 0;
	section_header.sh_info = 0;
	section_header.sh_addralign = 1;
	section_header.sh_entsize = 0;
	
	VS_WriteSectionHeader(file,section_header);
	
	if(elf_contains_reloc){
		
		reloc_offset = ftell(file);
		
		for(i = 0; i < reloc_size; i++){
			fwrite(&reloc_table[i].r_offset,4,1,file);
			fwrite(&reloc_table[i].r_info,4,1,file);
		}
		
		/* WRITE .REL.TEXT SECTION HEADER */
		section_header.sh_name = 33;
		section_header.sh_type = 0x9;
		section_header.sh_flags = 0x40;
		section_header.sh_addr = 0;
		section_header.sh_offset = reloc_offset;
		section_header.sh_size = reloc_size << 3;
		section_header.sh_link = 9;
		section_header.sh_info = 1;
		section_header.sh_addralign = 4;
		section_header.sh_entsize = 8;
		
		fseek(file,offset,SEEK_SET);
		
		VS_WriteSectionHeader(file,section_header);
	}
	
	fclose(file);
	
	return 1;
}

void VS_WriteELFHeader(FILE* file, VS_ELF elf){
	fwrite(&elf.magic,4,1,file);
	fwrite(&elf.bits,1,1,file);
	fwrite(&elf.endian,1,1,file);
	fwrite(&elf.version,1,1,file);
	fwrite(&elf.os,1,1,file);
	fwrite(&elf.abi_version,1,1,file);
	fwrite(&elf.pad,7,1,file);
	fwrite(&elf.obj_type,2,1,file);
	fwrite(&elf.architecture,2,1,file);
	fwrite(&elf.elf_version,4,1,file);
	fwrite(&elf.entry,4,1,file);
	fwrite(&elf.program_header_offset,4,1,file);
	fwrite(&elf.section_header_table,4,1,file);
	fwrite(&elf.flags,4,1,file);
	fwrite(&elf.size,2,1,file);
	fwrite(&elf.program_header_size,2,1,file);
	fwrite(&elf.program_header_num,2,1,file);
	fwrite(&elf.section_header_size,2,1,file);
	fwrite(&elf.section_header_num,2,1,file);
	fwrite(&elf.shstrndx,2,1,file);
}

void VS_WriteELFSymbol(FILE* file, VS_ELF_SYMBOL symbol){
	fwrite(&symbol.st_name,4,1,file);
	fwrite(&symbol.st_value,4,1,file);
	fwrite(&symbol.st_size,4,1,file);
	fwrite(&symbol.st_info,1,1,file);
	fwrite(&symbol.st_other,1,1,file);
	fwrite(&symbol.st_shndx,2,1,file);
}

void VS_WriteSectionHeader(FILE* file, VS_SECTION_HEADER sh){
	fwrite(&sh.sh_name,4,1,file);
	fwrite(&sh.sh_type,4,1,file);
	fwrite(&sh.sh_flags,4,1,file);
	fwrite(&sh.sh_addr,4,1,file);
	fwrite(&sh.sh_offset,4,1,file);
	fwrite(&sh.sh_size,4,1,file);
	fwrite(&sh.sh_link,4,1,file);
	fwrite(&sh.sh_info,4,1,file);
	fwrite(&sh.sh_addralign,4,1,file);
	fwrite(&sh.sh_entsize,4,1,file);
}