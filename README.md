# comparch
The LC-3b supports a rich, but lean, instruction set. Each 16-bit instruction consists of an opcode (bits[15:12]) plus 12 additional bits to specify the other information which is needed to carry out the work of that instruc- tion. Figure A summarizes the 14 different opcodes in the LC-3b and the specification of the remaining bits of each instruction. The 15th and 16th 4-bit opcodes are not specified, but are reserved for future use. Figure B shows the entire LC-3b instruction set.

![image](https://github.com/sparkfiresprairie/comparch/blob/master/16_lc3b_opcodes.png)

![image](https://github.com/sparkfiresprairie/comparch/blob/master/entire_lc3b_ia.png)

******************************************************************************************
Lab1 Write an assembler for the LC­3b Assembly Language
******************************************************************************************
The task of the assembler is that of line­by­line translation. The input is an assembly language file, and the output is an object (ISA) file (consisting of hexadecimal digits). To make it a little more concrete, here is a sample assembly language program:
;This program counts from 10 to 0 .ORIG x3000
LEA R0, TEN
LDW R1, R0, #0 START ADDR1,R1,#­1
BRZ DONE BR START
DONE TRAP x25 TEN .FILL x000A
.END
;This instruction will be loaded into memory location x3000
;blank line
;The last executable instruction
;This is 10 in 2's comp, hexadecimal
;The pseudo­op, delimiting the source program

And its corresponding ISA program:
0x3000
0xE005
0x6200
0x127F 
0x0401 
0x0FFD 
0xF025 
0x000A