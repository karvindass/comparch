/*
 Name 1: Xingyuan Wang
 UTEID 1: xw3893
 */

/***************************************************************/
/*                                                             */
/*   LC-3b Simulator                                           */
/*                                                             */
/*   EE 460N                                                   */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/***************************************************************/
/*                                                             */
/* Files:  ucode        Microprogram file                      */
/*         isaprogram   LC-3b machine language program file    */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void eval_micro_sequencer();
void cycle_memory();
void eval_bus_drivers();
void drive_bus();
void latch_datapath_values();

/***************************************************************/
/* A couple of useful definitions.                             */
/***************************************************************/
#define FALSE 0
#define TRUE  1

/***************************************************************/
/* Use this to avoid overflowing 16 bits on the bus.           */
/***************************************************************/
#define Low16bits(x) ((x) & 0xFFFF)

/***************************************************************/
/* Definition of the control store layout.                     */
/***************************************************************/
#define CONTROL_STORE_ROWS 64
#define INITIAL_STATE_NUMBER 18

/***************************************************************/
/* Definition of bit order in control store word.              */
/***************************************************************/
enum CS_BITS {
    IRD,
    COND1, COND0,
    J5, J4, J3, J2, J1, J0,
    LD_MAR,
    LD_MDR,
    LD_IR,
    LD_BEN,
    LD_REG,
    LD_CC,
    LD_PC,
    GATE_PC,
    GATE_MDR,
    GATE_ALU,
    GATE_MARMUX,
    GATE_SHF,
    PCMUX1, PCMUX0,
    DRMUX,
    SR1MUX,
    ADDR1MUX,
    ADDR2MUX1, ADDR2MUX0,
    MARMUX,
    ALUK1, ALUK0,
    MIO_EN,
    R_W,
    DATA_SIZE,
    LSHF1,
    CONTROL_STORE_BITS	/* Here CONTROL_STORE_BITS is automatically assigned the value of 35*/
} CS_BITS;

/***************************************************************/
/* Functions to get at the control bits.                       */
/***************************************************************/
int GetIRD(int *x)           { return(x[IRD]); }
int GetCOND(int *x)          { return((x[COND1] << 1) + x[COND0]); }
int GetJ(int *x)             { return((x[J5] << 5) + (x[J4] << 4) +
                                      (x[J3] << 3) + (x[J2] << 2) +
                                      (x[J1] << 1) + x[J0]); }
int GetLD_MAR(int *x)        { return(x[LD_MAR]); }
int GetLD_MDR(int *x)        { return(x[LD_MDR]); }
int GetLD_IR(int *x)         { return(x[LD_IR]); }
int GetLD_BEN(int *x)        { return(x[LD_BEN]); }
int GetLD_REG(int *x)        { return(x[LD_REG]); }
int GetLD_CC(int *x)         { return(x[LD_CC]); }
int GetLD_PC(int *x)         { return(x[LD_PC]); }
int GetGATE_PC(int *x)       { return(x[GATE_PC]); }
int GetGATE_MDR(int *x)      { return(x[GATE_MDR]); }
int GetGATE_ALU(int *x)      { return(x[GATE_ALU]); }
int GetGATE_MARMUX(int *x)   { return(x[GATE_MARMUX]); }
int GetGATE_SHF(int *x)      { return(x[GATE_SHF]); }
int GetPCMUX(int *x)         { return((x[PCMUX1] << 1) + x[PCMUX0]); }
int GetDRMUX(int *x)         { return(x[DRMUX]); }
int GetSR1MUX(int *x)        { return(x[SR1MUX]); }
int GetADDR1MUX(int *x)      { return(x[ADDR1MUX]); }
int GetADDR2MUX(int *x)      { return((x[ADDR2MUX1] << 1) + x[ADDR2MUX0]); }
int GetMARMUX(int *x)        { return(x[MARMUX]); }
int GetALUK(int *x)          { return((x[ALUK1] << 1) + x[ALUK0]); }
int GetMIO_EN(int *x)        { return(x[MIO_EN]); }
int GetR_W(int *x)           { return(x[R_W]); }
int GetDATA_SIZE(int *x)     { return(x[DATA_SIZE]); }
int GetLSHF1(int *x)         { return(x[LSHF1]); }

/***************************************************************/
/* The control store rom.                                      */
/***************************************************************/
int CONTROL_STORE[CONTROL_STORE_ROWS][CONTROL_STORE_BITS]; 	/* i.e. CONTROL_STORE[64][35]*/

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
 MEMORY[A][1] stores the most significant byte of word at word address A
 There are two write enable signals, one for each byte. WE0 is used for
 the least significant byte of a word. WE1 is used for the most significant
 byte of a word. */

#define WORDS_IN_MEM    0x08000
#define MEM_CYCLES      5
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT;	/* run bit */
int BUS;	/* value of the bus */

typedef struct System_Latches_Struct{
    
    int PC,		/* program counter */
    MDR,	/* memory data register */
    MAR,	/* memory address register */
    IR,		/* instruction register */
    N,		/* n condition bit */
    Z,		/* z condition bit */
    P,		/* p condition bit */
    BEN;        /* ben register */
    
    int READY;	/* ready bit */
    /* The ready bit is also latched as you don't want the memory system to assert it
     at a bad point in the cycle*/
    
    int REGS[LC_3b_REGS]; /* register file. */
    
    int MICROINSTRUCTION[CONTROL_STORE_BITS]; /* The microinstruction */
    
    int STATE_NUMBER; /* Current State Number - Provided for debugging */
} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int CYCLE_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands.                   */
/*                                                             */
/***************************************************************/
void help() {
    printf("----------------LC-3bSIM Help-------------------------\n");
    printf("go               -  run program to completion       \n");
    printf("run n            -  execute program for n cycles    \n");
    printf("mdump low high   -  dump memory from low to high    \n");
    printf("rdump            -  dump the register & bus values  \n");
    printf("?                -  display this help menu          \n");
    printf("quit             -  exit the program                \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {
    
    eval_micro_sequencer();
    cycle_memory();
    eval_bus_drivers();
    drive_bus();
    latch_datapath_values();
    
    CURRENT_LATCHES = NEXT_LATCHES;
    
    CYCLE_COUNT++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3b for n cycles.                 */
/*                                                             */
/***************************************************************/
void run(int num_cycles) {
    int i;
    
    if (RUN_BIT == FALSE) {
        printf("Can't simulate, Simulator is halted\n\n");
        return;
    }
    
    printf("Simulating for %d cycles...\n\n", num_cycles);
    for (i = 0; i < num_cycles; i++) {
        if (CURRENT_LATCHES.PC == 0x0000) {
            RUN_BIT = FALSE;
            printf("Simulator halted\n\n");
            break;
        }
        cycle();
    }
}

/***************************************************************/
/*                                                             */
/* Procedure : go                                              */
/*                                                             */
/* Purpose   : Simulate the LC-3b until HALTed.                 */
/*                                                             */
/***************************************************************/
void go() {
    if (RUN_BIT == FALSE) {
        printf("Can't simulate, Simulator is halted\n\n");
        return;
    }
    
    printf("Simulating...\n\n");
    while (CURRENT_LATCHES.PC != 0x0000)
        cycle();
    RUN_BIT = FALSE;
    printf("Simulator halted\n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : mdump                                           */
/*                                                             */
/* Purpose   : Dump a word-aligned region of memory to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void mdump(FILE * dumpsim_file, int start, int stop) {
    int address; /* this is a byte address */
    
    printf("\nMemory content [0x%0.4x..0x%0.4x] :\n", start, stop);
    printf("-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
        printf("  0x%0.4x (%d) : 0x%0.2x%0.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
    printf("\n");
    
    /* dump the memory contents into the dumpsim file */
    fprintf(dumpsim_file, "\nMemory content [0x%0.4x..0x%0.4x] :\n", start, stop);
    fprintf(dumpsim_file, "-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
        fprintf(dumpsim_file, " 0x%0.4x (%d) : 0x%0.2x%0.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
    fprintf(dumpsim_file, "\n");
    fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : rdump                                           */
/*                                                             */
/* Purpose   : Dump current register and bus values to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void rdump(FILE * dumpsim_file) {
    int k;
    
    printf("\nCurrent register/bus values :\n");
    printf("-------------------------------------\n");
    printf("Cycle Count  : %d\n", CYCLE_COUNT);
    printf("PC           : 0x%0.4x\n", CURRENT_LATCHES.PC);
    printf("IR           : 0x%0.4x\n", CURRENT_LATCHES.IR);
    printf("STATE_NUMBER : 0x%0.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    printf("BUS          : 0x%0.4x\n", BUS);
    printf("MDR          : 0x%0.4x\n", CURRENT_LATCHES.MDR);
    printf("MAR          : 0x%0.4x\n", CURRENT_LATCHES.MAR);
    printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    printf("Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
        printf("%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
    printf("\n");
    
    /* dump the state information into the dumpsim file */
    fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
    fprintf(dumpsim_file, "-------------------------------------\n");
    fprintf(dumpsim_file, "Cycle Count  : %d\n", CYCLE_COUNT);
    fprintf(dumpsim_file, "PC           : 0x%0.4x\n", CURRENT_LATCHES.PC);
    fprintf(dumpsim_file, "IR           : 0x%0.4x\n", CURRENT_LATCHES.IR);
    fprintf(dumpsim_file, "STATE_NUMBER : 0x%0.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    fprintf(dumpsim_file, "BUS          : 0x%0.4x\n", BUS);
    fprintf(dumpsim_file, "MDR          : 0x%0.4x\n", CURRENT_LATCHES.MDR);
    fprintf(dumpsim_file, "MAR          : 0x%0.4x\n", CURRENT_LATCHES.MAR);
    fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    fprintf(dumpsim_file, "Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
        fprintf(dumpsim_file, "%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
    fprintf(dumpsim_file, "\n");
    fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : get_command                                     */
/*                                                             */
/* Purpose   : Read a command from standard input.             */
/*                                                             */
/***************************************************************/
void get_command(FILE * dumpsim_file) {
    char buffer[20];
    int start, stop, cycles;
    
    printf("LC-3b-SIM> ");
    
    scanf("%s", buffer);
    printf("\n");
    
    switch(buffer[0]) {
        case 'G':
        case 'g':
            go();
            break;
            
        case 'M':
        case 'm':
            scanf("%i %i", &start, &stop);
            mdump(dumpsim_file, start, stop);
            break;
            
        case '?':
            help();
            break;
        case 'Q':
        case 'q':
            printf("Bye.\n");
            exit(0);
            
        case 'R':
        case 'r':
            if (buffer[1] == 'd' || buffer[1] == 'D')
                rdump(dumpsim_file);
            else {
                scanf("%d", &cycles);
                run(cycles);
            }
            break;
            
        default:
            printf("Invalid Command\n");
            break;
    }
}

/***************************************************************/
/*                                                             */
/* Procedure : init_control_store                              */
/*                                                             */
/* Purpose   : Load microprogram into control store ROM        */
/*                                                             */
/***************************************************************/
void init_control_store(char *ucode_filename) {
    FILE *ucode;
    int i, j, index;
    char line[200];
    
    printf("Loading Control Store from file: %s\n", ucode_filename);
    
    /* Open the micro-code file. */
    if ((ucode = fopen(ucode_filename, "r")) == NULL) {
        printf("Error: Can't open micro-code file %s\n", ucode_filename);
        exit(-1);
    }
    
    /* Read a line for each row in the control store. */
    for(i = 0; i < CONTROL_STORE_ROWS; i++) {
        if (fscanf(ucode, "%[^\n]\n", line) == EOF) {
            printf("Error: Too few lines (%d) in micro-code file: %s\n",
                   i, ucode_filename);
            exit(-1);
        }
        
        /* Put in bits one at a time. */
        index = 0;
        
        for (j = 0; j < CONTROL_STORE_BITS; j++) {
            /* Needs to find enough bits in line. */
            if (line[index] == '\0') {
                printf("Error: Too few control bits in micro-code file: %s\nLine: %d\n",
                       ucode_filename, i);
                exit(-1);
            }
            if (line[index] != '0' && line[index] != '1') {
                printf("Error: Unknown value in micro-code file: %s\nLine: %d, Bit: %d\n",
                       ucode_filename, i, j);
                exit(-1);
            }
            
            /* Set the bit in the Control Store. */
            CONTROL_STORE[i][j] = (line[index] == '0') ? 0:1;
            index++;
        }
        
        /* Warn about extra bits in line. */
        if (line[index] != '\0')
            printf("Warning: Extra bit(s) in control store file %s. Line: %d\n",
                   ucode_filename, i);
    }
    printf("\n");
}

/************************************************************/
/*                                                          */
/* Procedure : init_memory                                  */
/*                                                          */
/* Purpose   : Zero out the memory array                    */
/*                                                          */
/************************************************************/
void init_memory() {
    int i;
    
    for (i=0; i < WORDS_IN_MEM; i++) {
        MEMORY[i][0] = 0;
        MEMORY[i][1] = 0;
    }
}

/**************************************************************/
/*                                                            */
/* Procedure : load_program                                   */
/*                                                            */
/* Purpose   : Load program and service routines into mem.    */
/*                                                            */
/**************************************************************/
void load_program(char *program_filename) {
    FILE * prog;
    int ii, word, program_base;
    
    /* Open program file. */
    prog = fopen(program_filename, "r");
    if (prog == NULL) {
        printf("Error: Can't open program file %s\n", program_filename);
        exit(-1);
    }
    
    /* Read in the program. */
    if (fscanf(prog, "%x\n", &word) != EOF)
        program_base = word >> 1;
    else {
        printf("Error: Program file is empty\n");
        exit(-1);
    }
    
    ii = 0;
    while (fscanf(prog, "%x\n", &word) != EOF) {
        /* Make sure it fits. */
        if (program_base + ii >= WORDS_IN_MEM) {
            printf("Error: Program file %s is too long to fit in memory. %x\n",
                   program_filename, ii);
            exit(-1);
        }
        
        /* Write the word to memory array. */
        MEMORY[program_base + ii][0] = word & 0x00FF;
        MEMORY[program_base + ii][1] = (word >> 8) & 0x00FF;
        ii++;
    }
    
    if (CURRENT_LATCHES.PC == 0) CURRENT_LATCHES.PC = (program_base << 1);
    
    printf("Read %d words from program into memory.\n\n", ii);
}

/***************************************************************/
/*                                                             */
/* Procedure : initialize                                      */
/*                                                             */
/* Purpose   : Load microprogram and machine language program  */
/*             and set up initial state of the machine.        */
/*                                                             */
/***************************************************************/
void initialize(char *ucode_filename, char *program_filename, int num_prog_files) {
    int i;
    init_control_store(ucode_filename);
    
    init_memory();
    for ( i = 0; i < num_prog_files; i++ ) {
        load_program(program_filename);
        while(*program_filename++ != '\0');
    }
    CURRENT_LATCHES.Z = 1;
    CURRENT_LATCHES.STATE_NUMBER = INITIAL_STATE_NUMBER;
    memcpy(CURRENT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[INITIAL_STATE_NUMBER], sizeof(int)*CONTROL_STORE_BITS);
    
    NEXT_LATCHES = CURRENT_LATCHES;
    
    RUN_BIT = TRUE;
}

/***************************************************************/
/*                                                             */
/* Procedure : main                                            */
/*                                                             */
/***************************************************************/
int main(int argc, char *argv[]) {
    FILE * dumpsim_file;
    
    /* Error Checking */
    if (argc < 3) {
        printf("Error: usage: %s <micro_code_file> <program_file_1> <program_file_2> ...\n",
               argv[0]);
        exit(1);
    }
    
    printf("LC-3b Simulator\n\n");
    
    initialize(argv[1], argv[2], argc - 2);
    
    if ( (dumpsim_file = fopen( "dumpsim", "w" )) == NULL ) {
        printf("Error: Can't open dumpsim file\n");
        exit(-1);
    }
    
    while (1)
        get_command(dumpsim_file);
    
}

/***************************************************************/
/* Do not modify the above code.
 You are allowed to use the following global variables in your
 code. These are defined above.
 
 CONTROL_STORE
 MEMORY
 BUS
 
 CURRENT_LATCHES
 NEXT_LATCHES
 
 You may define your own local/global variables and functions.
 You may use the functions to get at the control bits defined
 above.
 
 Begin your code here 	  			       */
/***************************************************************/


void eval_micro_sequencer() {
    
    /*
     * Evaluate the address of the next state according to the
     * micro sequencer logic. Latch the next microinstruction.
     */
    int myIRD, myCOND1, myCOND0, myBEN, myReady, myIR11, myJ5, myJ4, myJ3, myJ2, myJ1, myJ0;
    
    myIRD = CURRENT_LATCHES.MICROINSTRUCTION[IRD];
    myCOND1 = CURRENT_LATCHES.MICROINSTRUCTION[COND1];
    myCOND0 = CURRENT_LATCHES.MICROINSTRUCTION[COND0];
    myBEN = CURRENT_LATCHES.BEN;
    myReady = CURRENT_LATCHES.READY;
    myIR11 = ( CURRENT_LATCHES.IR>>11 ) % 2;	/*myIR11 =( CURRENT_LATCHES.IR<<( sizeof(int) - 12 ) ) >> ( sizeof(int)-1 )*/
    myJ5 = CURRENT_LATCHES.MICROINSTRUCTION[J5];
    myJ4 = CURRENT_LATCHES.MICROINSTRUCTION[J4];
    myJ3 = CURRENT_LATCHES.MICROINSTRUCTION[J3];
    myJ2 = CURRENT_LATCHES.MICROINSTRUCTION[J2];
    myJ1 = CURRENT_LATCHES.MICROINSTRUCTION[J1];
    myJ0 = CURRENT_LATCHES.MICROINSTRUCTION[J0];
    
    if( myIRD == 1 ) NEXT_LATCHES.STATE_NUMBER = ( CURRENT_LATCHES.IR>>12 );
    else
    {
        myJ2 = ( myCOND1 && ( !myCOND0 ) && myBEN ) || myJ2;
        myJ1 = ( ( !myCOND1 ) && myCOND0 && myReady ) || myJ1;
        myJ0 = ( myCOND1 && myCOND0 && myIR11 ) || myJ0;
        NEXT_LATCHES.STATE_NUMBER = ( myJ5<<5 ) + ( myJ4<<4 ) + ( myJ3<<3 ) + ( myJ2<<2 ) + ( myJ1<<1 ) + myJ0;
    }
    
    memcpy(NEXT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[NEXT_LATCHES.STATE_NUMBER], sizeof(int)*CONTROL_STORE_BITS);
}

int memoryCycleCounter;

void cycle_memory() {
    
    /*
     * This function emulates memory and the WE logic.
     * Keep track of which cycle of MEMEN we are dealing with.
     * If fourth, we need to latch Ready bit at the end of
     * cycle to prepare microsequencer for the fifth cycle.
     */
    int myMIO_EN, myR_W, myDATA_SIZE, myMAR, myMDR;
    
    NEXT_LATCHES.READY = 0;
    
    myMIO_EN = CURRENT_LATCHES.MICROINSTRUCTION[MIO_EN];
    myR_W = CURRENT_LATCHES.MICROINSTRUCTION[R_W];
    myDATA_SIZE = CURRENT_LATCHES.MICROINSTRUCTION[DATA_SIZE];
    myMAR = CURRENT_LATCHES.MAR;
    myMDR = CURRENT_LATCHES.MDR;
    
    if( myMIO_EN == 0 ) { memoryCycleCounter = 0; return; }
    else
    {
        memoryCycleCounter++;
        if( memoryCycleCounter < 4 ) return;
        else
        {
            NEXT_LATCHES.READY = 1;
            if( myR_W == 0 )	/*Read*/
                CURRENT_LATCHES.MDR = Low16bits( MEMORY[myMAR/2][0] + ( MEMORY[myMAR/2][1]<<8 ) );
            else	/*Write*/
            {
                if( myDATA_SIZE == 0 )	/*Byte*/
                {
                    if( myMAR%2 == 0 ) MEMORY[myMAR/2][0] = Low16bits( myMDR & 0x00FF );
                    else MEMORY[myMAR/2][1] = Low16bits( myMDR>>8 );
                }
                else
                {
                    MEMORY[myMAR/2][0] = Low16bits( myMDR & 0x00FF );
                    MEMORY[myMAR/2][1] = Low16bits( myMDR>>8 );
                }
            }
        }
    }
}


int GATE_PC_INPUT, GATE_MDR_INPUT, GATE_ALU_INPUT, GATE_MARMUX_INPUT, GATE_SHF_INPUT;

void eval_bus_drivers() {
    
    /*
     * Datapath routine emulating operations before driving the bus.
     * Evaluate the input of tristate drivers
     *             Gate_MARMUX,
     *		 Gate_PC,
     *		 Gate_ALU,
     *		 Gate_SHF,
     *		 Gate_MDR.
     */
    int myMAR, myMDR, myDATA_SIZE, myALUK, myIR, myIR5, myMARMUX, myADDR1MUX, myADDR2MUX, myLSHF1, mySR1MUX, myAMOUNT4;
    int A, B, SR1, SR2, ADDR1MUX_OUTPUT, ADDR2MUX_OUTPUT, LEFT_OR_RIGHT, i, sum = 0 ;
	   
    GATE_PC_INPUT = 0;
    GATE_MDR_INPUT = 0;
    GATE_ALU_INPUT = 0;
    GATE_MARMUX_INPUT = 0;
    GATE_SHF_INPUT = 0;
    
    myMAR = CURRENT_LATCHES.MAR;
    myMDR = CURRENT_LATCHES.MDR;
    myDATA_SIZE = CURRENT_LATCHES.MICROINSTRUCTION[DATA_SIZE];
    myALUK = GetALUK( CURRENT_LATCHES.MICROINSTRUCTION );
    myIR =CURRENT_LATCHES.IR;
    myIR5 = (myIR>>5) % 2; /*myIR5 = ( myIR<<(sizeof(int)-6) )>>(sizeof(int)-1);*/
    myMARMUX = CURRENT_LATCHES.MICROINSTRUCTION[MARMUX];
    myADDR1MUX = CURRENT_LATCHES.MICROINSTRUCTION[ADDR1MUX];
    myADDR2MUX = GetADDR2MUX( CURRENT_LATCHES.MICROINSTRUCTION );
    myLSHF1 = CURRENT_LATCHES.MICROINSTRUCTION[LSHF1];
    mySR1MUX = CURRENT_LATCHES.MICROINSTRUCTION[SR1MUX];
    myAMOUNT4 = myIR%16 ;
    LEFT_OR_RIGHT = (myIR>>4)%4;
    
    /* Calculate Gate_PC's input */
    GATE_PC_INPUT  = CURRENT_LATCHES.PC;
    
    /* Calculate Gate_MDR's input */
    if( myDATA_SIZE == 0 )	/*Byte*/
    {
        /*Here should be Sign-extended
        if( myMAR%2 == 0 ) GATE_MDR_INPUT = myMDR & 0x00FF;
        else GATE_MDR_INPUT = myMDR>>8;
         */
        if( myMAR%2 == 0 ) GATE_MDR_INPUT = myMDR & 0x00FF;
        else GATE_MDR_INPUT = myMDR>>8;
        
        if ( (GATE_MDR_INPUT>>7)==1 )
            GATE_MDR_INPUT = GATE_MDR_INPUT | 0xFFFFFF00;
        
    }
    else	/*Word*/
    {
        GATE_MDR_INPUT = myMDR;
    }
    
    /* Calculate Gate_ALU's input */
    SR2 = myIR % 8;
    if( mySR1MUX == 1 )	SR1 = ( myIR>>6 ) % 8;
    else SR1 = ( myIR>>9 ) % 8;
    
    A = CURRENT_LATCHES.REGS[SR1];
    if( myIR5 == 0 ) B = CURRENT_LATCHES.REGS[SR2];
    else
    {
        B = myIR%32;
        if( B>15 ) B = B | 0xFFFFFFE0; /* sign-extended if it it negative*/
    }
    
    switch( myALUK )
    {
        case 0: { GATE_ALU_INPUT = A + B; break; }
        case 1:	{ GATE_ALU_INPUT = A & B; break; }
        case 2: { GATE_ALU_INPUT = A ^ B; break; }
        case 3: { GATE_ALU_INPUT = A; break; }
        default: break;
    }
    
    /* Calculate Gate_MARMUX's input */
    if( myMARMUX == 0 ) GATE_MARMUX_INPUT = ( myIR & 0x000000FF )<<1;
    else
    {
        if( myADDR1MUX == 0 ) ADDR1MUX_OUTPUT = CURRENT_LATCHES.PC;
        else ADDR1MUX_OUTPUT = CURRENT_LATCHES.REGS[SR1];
        
        switch( myADDR2MUX )
        {
            case 0: { ADDR2MUX_OUTPUT = 0; break; }
            case 1:
            {
                ADDR2MUX_OUTPUT = myIR%64;
                if( ADDR2MUX_OUTPUT > 31 ) ADDR2MUX_OUTPUT = ADDR2MUX_OUTPUT | 0xFFFFFFC0;
                break;
            }
            case 2:
            {
                ADDR2MUX_OUTPUT = myIR%512;
                if( ADDR2MUX_OUTPUT > 255 ) ADDR2MUX_OUTPUT = ADDR2MUX_OUTPUT | 0xFFFFFE00;
                break;
            }
            case 3:
            {
                ADDR2MUX_OUTPUT = myIR%2048;
                if( ADDR2MUX_OUTPUT > 1023 ) ADDR2MUX_OUTPUT = ADDR2MUX_OUTPUT | 0xFFFFF800;
                break;
            }
            default: break;
        }
        
        if( myLSHF1 == 1 ) ADDR2MUX_OUTPUT = ADDR2MUX_OUTPUT<<1;
        
        GATE_MARMUX_INPUT = ADDR1MUX_OUTPUT + ADDR2MUX_OUTPUT;
    }
    
    /* Calculate Gate_SHF's input */
    switch( LEFT_OR_RIGHT )
    {
        case 0:
        {
            GATE_SHF_INPUT = CURRENT_LATCHES.REGS[SR1] << myAMOUNT4;
            break;
        }
        case 1:
        {
            GATE_SHF_INPUT = CURRENT_LATCHES.REGS[SR1] >> myAMOUNT4;
            break;
        }
        case 3:
        {
            GATE_SHF_INPUT = CURRENT_LATCHES.REGS[SR1] >> myAMOUNT4;
            if( CURRENT_LATCHES.REGS[SR1] > 32767 )
            {
                for( i=0; i<myAMOUNT4; i++ ) sum = sum + ( 1<<(15-i));
                GATE_SHF_INPUT = GATE_SHF_INPUT + sum;
            }
            break;
        }
        default: break;
    }
}


void drive_bus() {
    
    /*
     * Datapath routine for driving the bus from one of the 5 possible
     * tristate drivers.
     */
    int myGATE_PC, myGATE_MDR, myGATE_ALU, myGATE_MARMUX, myGATE_SHF;
    
    myGATE_PC = CURRENT_LATCHES.MICROINSTRUCTION[GATE_PC];
    myGATE_MDR = CURRENT_LATCHES.MICROINSTRUCTION[GATE_MDR];
    myGATE_ALU = CURRENT_LATCHES.MICROINSTRUCTION[GATE_ALU];
    myGATE_MARMUX = CURRENT_LATCHES.MICROINSTRUCTION[GATE_MARMUX];
    myGATE_SHF = CURRENT_LATCHES.MICROINSTRUCTION[GATE_SHF];
    
    if( myGATE_PC == 1 ) BUS = Low16bits( GATE_PC_INPUT );
    else if( myGATE_MDR == 1 ) BUS = Low16bits( GATE_MDR_INPUT );
    else if( myGATE_ALU == 1 ) BUS = Low16bits( GATE_ALU_INPUT );
    else if( myGATE_MARMUX == 1 ) BUS = Low16bits( GATE_MARMUX_INPUT );
    else if( myGATE_SHF == 1 ) BUS = Low16bits( GATE_SHF_INPUT );
    else BUS = 0;
}


void latch_datapath_values() {
    
    /*
     * Datapath routine for computing all functions that need to latch
     * values in the data path at the end of this cycle.  Some values
     * require sourcing the bus; therefore, this routine has to come
     * after drive_bus.
     */
    int  myLD_MAR, myLD_MDR, myLD_IR, myLD_BEN, myLD_REG, myLD_CC, myLD_PC,
    myMIO_EN, myDATA_SIZE, myMAR, myIR, myIR11, myIR10, myIR9,
    myN, myZ, myP, myDRMUX, myPCMUX, myADDR1MUX, myADDR2MUX, myLSHF1, mySR1MUX;
    int DR, SR1, ADDR1MUX_OUTPUT, ADDR2MUX_OUTPUT, PCMUX_INPUT;
   	myLD_MAR = CURRENT_LATCHES.MICROINSTRUCTION[LD_MAR];
   	myLD_MDR = CURRENT_LATCHES.MICROINSTRUCTION[LD_MDR];
   	myLD_IR = CURRENT_LATCHES.MICROINSTRUCTION[LD_IR];
   	myLD_BEN = CURRENT_LATCHES.MICROINSTRUCTION[LD_BEN];
   	myLD_REG = CURRENT_LATCHES.MICROINSTRUCTION[LD_REG];
   	myLD_CC = CURRENT_LATCHES.MICROINSTRUCTION[LD_CC];
   	myLD_PC = CURRENT_LATCHES.MICROINSTRUCTION[LD_PC];
   	myMIO_EN = CURRENT_LATCHES.MICROINSTRUCTION[MIO_EN];
   	myDATA_SIZE = CURRENT_LATCHES.MICROINSTRUCTION[DATA_SIZE];
   	myMAR = CURRENT_LATCHES.MAR;
   	myIR = CURRENT_LATCHES.IR;
   	myIR11 = (myIR>>11)%2;
   	myIR10 = (myIR>>10)%2;
   	myIR9 = (myIR>>9)%2;
   	myN = CURRENT_LATCHES.N;
   	myZ = CURRENT_LATCHES.Z;
   	myP = CURRENT_LATCHES.P;
   	myDRMUX = CURRENT_LATCHES.MICROINSTRUCTION[DRMUX];
   	mySR1MUX = CURRENT_LATCHES.MICROINSTRUCTION[SR1MUX];
   	if( myDRMUX == 1 ) DR = 7;
   	else DR = ( myIR>>9 ) % 8;
   	if( mySR1MUX == 1 )	SR1 = ( myIR>>6 ) % 8;
    else SR1 = ( myIR>>9 ) % 8;
   	myPCMUX = GetPCMUX(CURRENT_LATCHES.MICROINSTRUCTION);
   	myADDR1MUX = CURRENT_LATCHES.MICROINSTRUCTION[ADDR1MUX];
    myADDR2MUX = GetADDR2MUX( CURRENT_LATCHES.MICROINSTRUCTION );
    myLSHF1 = CURRENT_LATCHES.MICROINSTRUCTION[LSHF1];
    mySR1MUX = CURRENT_LATCHES.MICROINSTRUCTION[SR1MUX];
    
    if( myLD_MAR == 1 ) NEXT_LATCHES.MAR = BUS;
    
   	if( myLD_MDR == 1 )
   	{
        if( myMIO_EN == 0 )
        {
            if( myDATA_SIZE == 1 ) /*Word*/
                NEXT_LATCHES.MDR = BUS;			
            else	/*Byte*/
            {
                /* Here should be SR'[7:0]SR[7:0], duplicate SR!!!
                if( myMAR%2 == 0 )
                    NEXT_LATCHES.MDR = BUS;
                else
                    NEXT_LATCHES.MDR = BUS<<8;
                 */
                NEXT_LATCHES.MDR =Low16bits((BUS&0x00FF) | (BUS<<8));
            }	
        }
        else
            NEXT_LATCHES.MDR = MEMORY[myMAR/2][0] + ( MEMORY[myMAR/2][1]<<8 );
   	}
    
   	if( myLD_IR == 1 ) NEXT_LATCHES.IR = BUS;
  	 
   	if( myLD_BEN == 1 ) NEXT_LATCHES.BEN = ( myIR11&&myN ) || ( myIR10&&myZ ) || ( myIR9&&myP );
    
   	if( myLD_REG == 1 ) NEXT_LATCHES.REGS[DR] = BUS;
    
   	if( myLD_CC == 1 ) 
   	{
        if( BUS == 0 ) { NEXT_LATCHES.N = 0; NEXT_LATCHES.Z = 1; NEXT_LATCHES.P = 0;}
        else if( BUS > 32767 ) { NEXT_LATCHES.N = 1; NEXT_LATCHES.Z = 0; NEXT_LATCHES.P = 0;}
        else { NEXT_LATCHES.N = 0; NEXT_LATCHES.Z = 0; NEXT_LATCHES.P = 1;}
  	 }
    
   	if( myLD_PC == 1 )
   	{
        if( myADDR1MUX == 0 ) ADDR1MUX_OUTPUT = CURRENT_LATCHES.PC;
        else ADDR1MUX_OUTPUT = CURRENT_LATCHES.REGS[SR1];
        
        switch( myADDR2MUX )
        {
            case 0: { ADDR2MUX_OUTPUT = 0; break; }
            case 1: 
            {
                ADDR2MUX_OUTPUT = myIR%64;
                if( ADDR2MUX_OUTPUT > 31 ) ADDR2MUX_OUTPUT = ADDR2MUX_OUTPUT | 0xFFFFFFC0;
                break;	
            }
            case 2:
            {
                ADDR2MUX_OUTPUT = myIR%512;
                if( ADDR2MUX_OUTPUT > 255 ) ADDR2MUX_OUTPUT = ADDR2MUX_OUTPUT | 0xFFFFFE00;
                break;
            }
            case 3:
            {
                ADDR2MUX_OUTPUT = myIR%2048;
                if( ADDR2MUX_OUTPUT > 1023 ) ADDR2MUX_OUTPUT = ADDR2MUX_OUTPUT | 0xFFFFF800;
                break;
            }
            default: break;
        }
        
        if( myLSHF1 == 1 ) ADDR2MUX_OUTPUT = ADDR2MUX_OUTPUT<<1;
        
        PCMUX_INPUT = ADDR1MUX_OUTPUT + ADDR2MUX_OUTPUT;
        switch( myPCMUX )
        {
            case 0: { NEXT_LATCHES.PC = CURRENT_LATCHES.PC + 2; break;}
            case 1: { NEXT_LATCHES.PC = BUS; break;}
            case 2:	{ NEXT_LATCHES.PC = PCMUX_INPUT; break;}
            default: break;	
        }	
    }  
    
}
