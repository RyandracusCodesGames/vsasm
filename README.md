# vsasm - VideoStation Assembler
A MIPS assembler that converts mnemonic instructions into binary object code for the original Sony PlayStation, PlayStation Portable, and other MIPS platforms.

## About
vsasm is a custom MIPS assembler that fully implements the MIPS I and II ISA's with a primary focus on bare metal homebrew and reverse engineering development 
for the original Sony PlayStation and PlayStation Portable, but vsasm is capable of assembling source code for any 32-bit little-endian MIPS processor.

## Features
* Outputs ELF(Executable and Linkable Format) binary object files compatible with the Mipsel GCC toolchain, allowing direct linkage with C/C++ code in modern PSX/PSP development.
* Outputs SN Systems PsyQ binary object files to facilitate compatibility with the legacy PsyQ toolchain.
* Can generate complete PS-EXE executables for bare metal PlayStation programs that can be ran on real hardware and in emulators.
* Can generate complete CPE debug executables that the original PsyQ SDK toolchain could produce for use in official development kits or emulators today.
* Can convert CPE debug executables to PS-EXE executables.
* Supports distinct syntax modes for compatibility with the GNU toolchain or the old PsyQ SDK's ASMPSX assembler.
* Includes a rudimentary C-like expression parser for immediate values including hexadecimal numbers in GNU and ASMPSX syntax.
* Includes an assembler directive that specifies the architecture of the source file to enable platform-specific instruction selection and behavior. 
* Implements nearly the entire custom instruction set architecture of the PSP's VFPU.
* A core set of common and custom assembler directives to control certain attributes of the source file, functions, macros, and data objects.
* Provides a rich set of error and warning messages printed to the console

## Bare-Metal PSX Programs Assembled Using VSASM

https://github.com/user-attachments/assets/9e8de95f-223e-4b10-8ad3-25d5a82a7519

## Syntax
As mentioned previously in the features section, vsasm supports two distinct syntaxes, that being the standard MIPS GNU syntax and PsyQ's ASMPSX assembler's syntax. 

#### Comments
GNU's `#` pound symbol and the classic `;` semi-colon comment styles are both supported by the assembler.
#### Registers
In GNU's syntax, every register is preceded by a `$` symbol irrespective of the instruction, while in ASMPSX's syntax, the `$` symbol indicates a hexadecimal immediate value and ***NOT*** a register.

GNU                 | ASMPSX       | 
--------------------| -------------| 
$0-$31              | r0-r31       |    
$0, $zero           | zero         |
$2, $3, $v0, $v1    | v0, v1       |
$4-$7, $a0-$a3      | a0-a3        |
$8-$15, $t0-$t7     | t0-t7        |
$16-$23, $s0-$s7    | s0-s7        |
$24, $25, $t8, $t9  | t8, t9       |
$26, $27, $k0, $k1  | k0, k1       |
$28, $gp            | gp           |
$29, $sp            | sp           |
$30, $fp            | fp           |
$31, $ra            | ra           |
$f0-$f31            | f0 - f31     |
#### Data Types
Hexadecimal integers begin with the prefix `0x` such as **0xFFFF** and can contain the digits `0-9` and the letters `A-F` in lower or uppercase format.
In ASMPSX syntax, hexadecimal integers can either begin with the prefix `0x` or `$` such as `0xFFFF` or `$FFFF` since the `$` symbol isn't the prefix of a register.
Floating-point values can either be normal integer values or can have ***ONE*** decimal point such as ***1.07*** or ***0.0075***.
#### Assembler Directives
 Directive          | Description                                       | 
--------------------|---------------------------------------------------| 
.align, n           | Aligns the data section along the n byte boundary |
.arch               | Sets the architecture of the source file(set to mips1, mips1, psx, or psp)         |
.ascii "\<string\>" | Creates a null-terminated string in the data section |
.byte "\<byte\>",...| Puts a single or array of bytes in the data section |
.data               | Switches program to place symbols as data objects |
.db "\<byte\>",...  | Equivalent to .byte directive above |
.dh "\<half\>",...| Puts a single or array of 16-bit half word(s) in the data section |
.dw "\<word\>",...| Puts a single or array of 32-bit word(s) in the data section |
name equ value    | A macro-esque directive that replaces each instance of name with value  and value is immutable once set|
name equr reg    | A macro-esque directive that replaces each instance of name with a register value |
name set value | A macro-esque directive that replaces each instance of name with value and value is mutable once set |
.empty, n         | Fills the data section with n zeros |
.float "\<float\>",...| Puts a single or array of 32-bit floats in the data section |
.globl, .global | Sets the scope of a symbol to be global |
.half "\<half\>",...| Puts a single or array of 16-bit half word(s) in the data section |
.include "\<file>\" | Opens an external file and pastes its contents into the source file |
.incbin "\<file>\" | Opens an external file and pastes its contents as an array of bytes in the data section |
.org, n | Sets the starting memory address for a PlayStation's PS-EXE executable |
.safeloaddelay | Injects nops after instructions that invokes the delay slot such as load, removed after MIPS I, or branch instructions |
.safeloadoff | Turns off nop injection caused by .safeloaddelay |
.syntax      | Sets the syntax of the source file(set to either gnu or asmpsx) |
.text | Switches program to place symbols as function objects in the text section of the program |
.type | Sets the symbol to either being a function or data object
.word "\<word\>",...| Puts a single or array of 32-bit word(s) in the data section |
.undefsym | Enables and adds undefined symbol references in jump, branch, and load calls into the symbol table with relocation entries for each instance |
.inject "\<file>\" | Opens an external file and pastes its contents into the current instruction position in the text section |
#### Pseudo-Instructions
Instruction | Syntax           | Description           | Actual Number of Instructions |
------------|------------------|----------------------------------------|----| 
beqz       | beaz reg, label  | Branch to label if zero                | 1 |
bgeu       | bgeu reg1, reg2, label | Branch to label if greater than or equal to unsigned | 2|
bge        | bge reg1, reg2, label | Branch to label if greater than or equal to | 2|
bgt        | bgt reg1, reg2, label | Branch to label if greater than | 2|
bnez       | bnez reg, label | Branch to label if not zero | 1|
bleu        | bleu reg1, reg2, label | Branch to label if less than or equal to unsigned | 2|
ble       | ble reg1, reg2, label | Branch to label if less than or equal to| 2|
blt        | blt reg1, reg2, label | Branch to label if less than | 2|
bltu       | bltu reg1, reg2, label | Branch to label if less than unsigned | 2|
la        | la reg, imm or label  | Loads the address of a label or immediate value | 2| 
li        | li reg, imm                | Loads an immediate value to the target register | 1 or 2 |
li.s      | li.s fpuReg, float           | Loads a floating-point value to the target register | 4 |
umul      | umul reg1, reg2, reg3        | Performs unsigned multplication on two registers | 2 | 
mul       | mul reg1, reg2, reg3         | Performs multplication on two registers | 2 | 
move      | move reg1, reg2              | Copies the value of reg2 into reg1 | 1| 
nop       | nop                          | No operation | 1 |
subi      | subi reg1, reg2, imm         | Subtracts an immediate value from reg2 | 1 |
subiu      | subiu reg1, reg2, imm         | Unsigned subtractraction on an immediate value from reg2 | 1 |
b          | b label                    | Unconditinal branch to label | 1 |
#### Short-handed Instructions
Instruction                  |  Real Instruction(s)           |
-----------------------------|-----------------------------|
addi reg, imm                | addi reg, reg, imm 
add reg, imm                 | addi reg, reg, imm 
add reg1, reg2               | add reg1, reg1, reg2 
addiu reg, imm               | addiu reg, reg, imm 
addu reg1, reg2              | addu reg1, reg1, reg2 
andi reg, imm                | andi reg, reg, imm 
and reg1, reg2               | and reg1, reg1, reg2 
nor reg1, reg2               | nor reg1, reg1, reg2 
sll reg, imm                 | sll reg, reg, imm
sra reg, imm                 | sra reg, reg, imm
srl reg, imm                 | srl reg, reg, imm
sub reg1, reg2               | sub reg1, reg1, reg2 
ori reg, imm                 | ori reg, reg, imm 
or reg1, reg2                | or reg1, reg1, reg2 
xori reg, imm                | xori reg, reg, imm 
xor reg1, reg2               | xor reg1, reg1, reg2 
load/store reg, label     	 | lui at, (label >> 16) & 0xFFFF
|							 | load/store reg, label & 0xffff(at)
lb, label 					 | Same as above
lbu, label					 | Same as above
lc, label                    | Same as above
lh, label 					 | Same as above
lhu, label					 | Same as above
lw, label                    | Same as above
sb, label					 | Same as above
sc, label                    | Same as above
sh, label					 | Same as above
sw, label                    | Same as above
etc.						 | Same as above
## Expression Parser
VSASM comes packaged with a run-of-the-mill C-like expression parser for any instruction that operates with an immediate value which 
allows for more complex programs that handle these intricate expressions at the assembler level instead of in real-time program execution.
It supports the +, -, x, /, <<, >>, &, and | operators as of now.
* Example Program 
```c 
Work:
	li $a0, VS_IO
	addi $a1, 0x80<<24
	li $t0, 12 + ((4*5) << 2)
	addi $t0, 4 / 2
	addi $t0, 4 + 0x40
```
## Usage
* Basic usage
```c
vsasm example.s
```
* Output the text section of the program to a file
```c
vsasm -text example.s
```
* Print the symbol table to the console
```c
vsasm -sym example.s
```
* Specify output filename
```c
vsasm example.s -o myfile.o
```
* Output a PS-EXE executable
```c
vsasm -oexe example.s -o main.exe
```
* Set output object file format
```c
vsasm -fmt elf example.s -o elf.o
vsasm -fmt psyq example.s -o psyq.o
```
* Sets the format of the PlayStation Executable
```c
vsasm -oexe psexe example.s -o main.exe 
vsasm -oexe cpe example.s -o main.cpe 
```
* Converts a PlayStation Debug Executable(CPE) to a normal PlayStation Executable(PS-EXE)
```c 
vsasm -cpe2exe main.cpe main.exe 
```
## Error Handling
vsasm produces a rich set of error messages and warnings for various syntax and semantic errors that are printed 
the line number of the error, the error message, and the original line that caused the error, such as 

```c 
	Syntax Error: Invalid hexadecimal number for immediate value
	Line 5: addi $t0, $t2, 0x1FFuy
```

In the case that you include other assembler files in the main source file that contains an error, the error handler 
will even print out what line in the included file caused an error, such as 

```c 
	Syntax Error: Invalid hexadecimal number for immediate value
	From file example.inc:
	Line 5: addi $t0, $t2, 0x1FFuy
```

Errors cause the program to abort and will not produce any object code or executables. Warnings will ***CONTINUE*** 
program flow such as unaligned memory accesses from load or store instructions, so be wary of warning messages as well
since code that could potentially crash a system or cause other unforeseen bugs can be passed through and assembled.

```c 
	Warning: Unaligned memory access
	Line 5: lw $t0, 5($a0)
```
## Linking VSASM Object Code with a modern GCC toolchain and the legacy PsyQ toolchain

https://github.com/user-attachments/assets/0b843edf-4d13-41a3-aa5a-3afde9ff0f83

