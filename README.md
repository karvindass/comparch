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

Lab2. Write an Instruction-Level Simulator for the LC-3b
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

The simulation routines carry out the instruction­level simulation of the input LC-3b program. During the execution of an instruction, the simulator should take the current architectural state and modify it according to the ISA description of the instruction. To be more specific, the dumpsim file could be something like the following:

    Current register/bus values :
    -------------------------------------
    Instruction Count : 2
    PC                : 0x3054
    CCs: N = 0  Z = 1  P = 0
    Registers:
    0: 0x0000
    1: 0x3060
    2: 0x0000
    3: 0x0000
    4: 0x0000
    5: 0x0000
    6: 0x0000
    7: 0x0000
    
    Memory content [0x3bf4..0x3bf6] :
    -------------------------------------
     0x3bf4 (15348) : 0x0000
     0x3bf6 (15350) : 0x0000

Here is how I implement this.

Lab3. Write a Cycle-Level Simulator for the LC-3b.
-------------------------------------------------------
The following pictures illustrate one example of a microarchitecture that implements the base machine of the LC-3b ISA. We have not included exception handling, interrupt processing, or virtual memory. We have used a very straightforward non-pipelined version. Interrupts, exceptions, virtual memory, pipelining, they will all come later - in lab 4,5,6.

Figure C shows the skeleton of the microarchitecture of LC-3b.

![image](https://github.com/sparkfiresprairie/comparch/blob/master/uarch.png)

Figure D gives a state machine for LC-3b.

![image](https://github.com/sparkfiresprairie/comparch/blob/master/state_machine.png)

Figure E illustates the data path of LC-3b.

![image](https://github.com/sparkfiresprairie/comparch/blob/master/data_path.png)

Figure F shows the structure of microsequencer of LC-3b base machine.

![image](https://github.com/sparkfiresprairie/comparch/blob/master/usequencer.png)

The simulator will take two input files:
  1. A file entitled ucode3 (fill it on our own, excel version) which holds the control store.
  2. A file entitled isaprogram which is an assembled LC-3b program.

The simulator will execute the input LC-3b program, using the microcode to direct the simulation of the microsequencer, datapath, and memory components of the LC-3b. To be specific, the dumpsim file is something like this:

    Current register/bus values :
    -------------------------------------
    Cycle Count  : 57
    PC           : 0x300a
    IR           : 0x3f81
    STATE_NUMBER : 0x0018
    
    BUS          : 0x300b
    MDR          : 0x3f81
    MAR          : 0x300b
    CCs: N = 1  Z = 0  P = 0
    Registers:
    0: 0xfffe
    1: 0x0000
    2: 0x0000
    3: 0x0000
    4: 0x0000
    5: 0x0000
    6: 0x300a
    7: 0xfedc

Here is how I implement this.

Lab4 Augment the Existing LC-3b Microarchitecture to Support Detection and Handling of Interruptions and Exceptions
-----------------------------------------------------------------------------------------------------------------
We are required to augment the existing LC-3b microarchitecture to support detection and handling of one type of interrupts (timer) and three types of exceptions (protection, unaligned access, and unknown opcode). We have to provide microarchitectural support for handling interruptions and exceptions as well as code for their service routine.

  1. The changes made to state machine, data path, microsequencer.
  2. The assembly code for the interrupt service routine, the interrupt/exception vector table, the protection exception handler, the unaligned access exception handler, the unknown opcode exception handler, the user program, and the data for locations xC000– xC013, called int.asm, vector_table.asm, except_prot.asm, except_unaligned.asm, except_unknown.asm, add.asm, and data.asm, respectively.
  3. The new microcode called ucode4.
 
Here is how I implement this.

Lab5 Augment the Existing LC-3b Microarchitecture to Support Virtual to Physical Address Translation
-----------------------------------------------------------------------------------------------------
