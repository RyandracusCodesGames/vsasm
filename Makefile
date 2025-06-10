CC = gcc
INCLUDES = -I"$(CURDIR)/include"
CFLAGS = -Wall -Wextra -Wshadow -Wpedantic -O2 $(INCLUDES)
LDFLAGS =
OBJS = source/vs_asm.o source/vs_opcode.o source/vs_preprocessor.o source/vs_symtable.o \
	source/vs_utils.o source/vs_parser.o source/vs_elf.o source/vs_psyqobj.o source/vs_psexe.o \
	source/vs_cpe.o source/vs_exp_parser.o
	
TARGET = vsasm

all: $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)
	
clean:
	rm -f $(OBJS) $(TARGET) *~