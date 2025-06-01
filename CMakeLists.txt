cmake_minimum_required(VERSION 3.25.1)
project(vsasm LANGUAGES C VERSION 1.0.0)

include_directories(vsasm PUBLIC include)

set(SOURCES
		source/vs_opcode.c 
		source/vs_symtable.c 
        source/vs_preprocessor.c 
		source/vs_parser.c 
		source/vs_elf.c
		source/vs_psyqobj.c
		source/vs_psexe.c 
		source/vs_utils.c
		source/vs_asm.c
)

add_executable(vsasm ${SOURCES})
