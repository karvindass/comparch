Computer Architecture Lab Description 
=====================================
Instructor: Prof. Yale Patt

The course mainly talks about characteristics of instruction set architecture and microarchitecture; physical and virtual memory; caches and cache design; interrupts and exceptions; integer and floating-point arithmetic; I/O processing; buses; pipelining, out-of-order execution, branch prediction, and other performance enhancements; design trade-offs; case studies of commercial microprocessors. 

Laboratory work includes completing the behavioral-level design of a microarchitecture (LC-3b). The following is how these labs work.

Lab1. Write an Assembler for the LC-3b Assembly Language
-------------------------------------------------------
The LC-3b supports a rich, but lean, instruction set. Each 16-bit instruction consists of an opcode (bits[15:12]) plus 12 additional bits to specify the other information which is needed to carry out the work of that instruction. Figure A summarizes the 14 different opcodes in the LC-3b and the specification of the remaining bits of each instruction. The 15th and 16th 4-bit opcodes are not specified, but are reserved for future use. Figure B shows the entire LC-3b instruction set.

![image](https://github.com/sparkfiresprairie/comparch/blob/master/16_lc3b_opcodes.png)

![image](https://github.com/sparkfiresprairie/comparch/blob/master/entire_lc3b_ia.png)

The task of the assembler is that of line-by-line translation. The input is an assembly language file, and the output is an object (ISA) file (consisting of hexadecimal digits). To make it a little more concrete, here is a sample assembly language program:

    ;This program counts from 10 to 0 .ORIG x3000
          LEA R0, TEN       ;This instruction will be loaded into memory location x3000
          LDW R1, R0, #0
    START ADDR1,R1,#­1
          BRZ DONE 
          BR START
                            ;blank line
    DONE  TRAP x25          ;The last executable instruction
    TEN   .FILL x000A       ;This is 10 in 2's comp, hexadecimal
          .END              ;The pseudo­op, delimiting the source program

And its corresponding ISA program:

    0x3000
    0xE005
    0x6200
    0x127F 
    0x0401 
    0x0FFD 
    0xF025 
    0x000A

Here is how I implement this.

Lab2. write an Instruction-Level Simulator for the LC-3b
--------------------------------------------------------
The simulator will take one input file entitled isaprogram, which is an assembled LC-3b program.And The simulator will execute the input LC-3b program, one instruction at a time, modifying the architectural state of the LC-3b after each instruction.

The simulator is partitioned into two main sections: the shell (provided) and the simulation routines.

The purpose of the shell is to provide the user with commands to control the execution of the simulator. The shell accepts one or more ISA programs as arguments and loads them into the memory image. In order to extract information from the simulator, a file named dumpsim will be created to hold information requested from the simulator. The shell supports the following commands:
    
    1. go – simulate the program until a HALT instruction is executed.
    2. run <n> – simulate the execution of the machine for n instructions
    3. mdump <low> <high> – dump the contents of memory, from location low to location
    high to the screen and the dump file
    4. rdump – dump the current instruction count, the contents of R0–R7, PC, and condition
    codes to the screen and the dump file.
    5. ? – print out a list of all shell commands.
    6. quit – quit the shell
