#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***************************************************************/
/*                                                             */
/* Files: isaprogram   LC-3b machine language program file     */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void process_instruction();

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
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A 
*/

#define WORDS_IN_MEM    0x08000 
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT;	/* run bit */


typedef struct System_Latches_Struct{

  int PC,		/* program counter */
    N,		/* n condition bit */
    Z,		/* z condition bit */
    P;		/* p condition bit */
  int REGS[LC_3b_REGS]; /* register file. */
} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int INSTRUCTION_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands                    */
/*                                                             */
/***************************************************************/
void help() {                                                    
  printf("----------------LC-3b ISIM Help-----------------------\n");
  printf("go               -  run program to completion         \n");
  printf("run n            -  execute program for n instructions\n");
  printf("mdump low high   -  dump memory from low to high      \n");
  printf("rdump            -  dump the register & bus values    \n");
  printf("?                -  display this help menu            \n");
  printf("quit             -  exit the program                  \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {                                                

  process_instruction();
  CURRENT_LATCHES = NEXT_LATCHES;
  INSTRUCTION_COUNT++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3b for n cycles                 */
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
/* Purpose   : Simulate the LC-3b until HALTed                 */
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
  printf("Instruction Count : %d\n", INSTRUCTION_COUNT);
  printf("PC                : 0x%0.4x\n", CURRENT_LATCHES.PC);
  printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
  printf("Registers:\n");
  for (k = 0; k < LC_3b_REGS; k++)
    printf("%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
  printf("\n");

  /* dump the state information into the dumpsim file */
  fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
  fprintf(dumpsim_file, "-------------------------------------\n");
  fprintf(dumpsim_file, "Instruction Count : %d\n", INSTRUCTION_COUNT);
  fprintf(dumpsim_file, "PC                : 0x%0.4x\n", CURRENT_LATCHES.PC);
  fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
  fprintf(dumpsim_file, "Registers:\n");
  for (k = 0; k < LC_3b_REGS; k++)
    fprintf(dumpsim_file, "%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
  fprintf(dumpsim_file, "\n");
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
/* Procedure : init_memory                                     */
/*                                                             */
/* Purpose   : Zero out the memory array                       */
/*                                                             */
/***************************************************************/
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

/************************************************************/
/*                                                          */
/* Procedure : initialize                                   */
/*                                                          */
/* Purpose   : Load machine language program                */ 
/*             and set up initial state of the machine.     */
/*                                                          */
/************************************************************/
void initialize(char *program_filename, int num_prog_files) { 
  int i;

  init_memory();
  for ( i = 0; i < num_prog_files; i++ ) {
    load_program(program_filename);
    while(*program_filename++ != '\0');
  }
  CURRENT_LATCHES.Z = 1;  
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
  if (argc < 2) {
    printf("Error: usage: %s <program_file_1> <program_file_2> ...\n",
           argv[0]);
    exit(1);
  }

  printf("LC-3b Simulator\n\n");

  initialize(argv[1], argc - 1);

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

   MEMORY

   CURRENT_LATCHES
   NEXT_LATCHES

   You may define your own local/global variables and functions.
   You may use the functions to get at the control bits defined
   above.

   Begin your code here 	  			       */

/***************************************************************/

char instructionRegister[17];

void process_instruction(){
  /*  function: process_instruction
   *  
   *    Process one instruction at a time  
   *       -Fetch one instruction
   *       -Decode 
   *       -Execute
   *       -Update NEXT_LATCHES
   */     

	int i;
	int leastSignificantByte = MEMORY[CURRENT_LATCHES.PC/2][0];
	int mostSignificantByte = MEMORY[CURRENT_LATCHES.PC/2][1];
	int opcodenumber;
	
	instructionRegister[16] = '\0';
	
	for( i=15; i>=8; i-- )
	{
		instructionRegister[i] = leastSignificantByte%2 + 48;
		leastSignificantByte = leastSignificantByte/2;
	}
	
	for( i=7; i>=0; i--)
	{
		instructionRegister[i] = mostSignificantByte%2 + 48;
		mostSignificantByte = mostSignificantByte/2;
	}
	
	opcodenumber = 8*(instructionRegister[0]-48) + 4*(instructionRegister[1]-48) + 2*(instructionRegister[2]-48) + 1*(instructionRegister[3]-48);
	
	switch(opcodenumber)
	{
		case 0: functionBR(); break;
		case 1: functionADD(); break;
		case 2: functionLDB(); break;
		case 3: functionSTB(); break;
		case 4: functionJSR(); break;
		case 5: functionAND(); break;
		case 6: functionLDW(); break;
		case 7: functionSTW(); break;
/*		case 8: functionRTI(); break;	*/
		case 9: functionXOR(); break;
		case 12: functionJMP(); break;
		case 13: functionSHF(); break;
		case 14: functionLEA(); break;
		case 15: functionTRAP(); break;
		default: break;	
	}
}

int signExtend(int length)
{
	int i;
	int sum = 0;
	
	for(i = 0;i < length-1;i++)
		sum = sum + ((instructionRegister[15-i] - 48 ) * (1<<i));
	if(instructionRegister[15-length+1] == '1' ) sum = sum - (1<<(length-1));
/*	sum = Low16bits(sum);	*/
	return sum;
}

/* Return the value that strings between instructionRegister[low] and instructionRegister[high] stand for*/
int whichRegister(int low, int high)
{
	int i;
	int j = 0;
	int sum = 0;
	for(i=high; i>=low; i--)
	{
		sum = sum + (instructionRegister[i]-48)*(1<<j);
		j++;
	}
	return sum;
}

functionBR()
{
	if( ((instructionRegister[4]-48)&&CURRENT_LATCHES.N) || ((instructionRegister[5]-48)&&CURRENT_LATCHES.Z) || ((instructionRegister[6]-48)&&CURRENT_LATCHES.P))
		NEXT_LATCHES.PC = Low16bits( CURRENT_LATCHES.PC+2 + (signExtend(9)<<1) );
	else
		NEXT_LATCHES.PC = Low16bits( CURRENT_LATCHES.PC+2 );	
}

functionADD()
{
	int DR,SR1,SR2;
	int imm5;
	
	DR = whichRegister(4,6);
	SR1 = whichRegister(7,9);
	
	if(instructionRegister[10] == '0' )
	{
		SR2 = whichRegister(13,15);
		NEXT_LATCHES.REGS[DR] = Low16bits( CURRENT_LATCHES.REGS[SR1]+CURRENT_LATCHES.REGS[SR2] );
	}	
	else
	{
		imm5 = signExtend(5);
		NEXT_LATCHES.REGS[DR] = Low16bits( CURRENT_LATCHES.REGS[SR1]+imm5 );	
	}
	
	if(NEXT_LATCHES.REGS[DR] == 0) 
	{ NEXT_LATCHES.Z = 1; NEXT_LATCHES.N = 0; NEXT_LATCHES.P= 0;}
	else if( (NEXT_LATCHES.REGS[DR]>>15) == 0 ) { NEXT_LATCHES.P = 1; NEXT_LATCHES.N = 0; NEXT_LATCHES.Z = 0;}
	else { NEXT_LATCHES.N = 1; NEXT_LATCHES.Z = 0; NEXT_LATCHES.P = 0;}
	NEXT_LATCHES.PC = Low16bits( CURRENT_LATCHES.PC+2 );
}

functionLDB()
{
	int DR, BaseR, boffset6;
	int temp;
	
	DR = whichRegister(4,6);
	BaseR = whichRegister(7,9);
	boffset6 = signExtend(6);
	
	temp = Low16bits( CURRENT_LATCHES.REGS[BaseR] + boffset6 );
	
	temp = MEMORY[temp/2][temp%2];
	
	if( (temp>>7) == 0) NEXT_LATCHES.REGS[DR] = Low16bits( temp );
	else NEXT_LATCHES.REGS[DR] = Low16bits( temp | 0xFFFFFF00 );
	
	if(NEXT_LATCHES.REGS[DR] == 0) { NEXT_LATCHES.Z = 1; NEXT_LATCHES.N = 0; NEXT_LATCHES.P= 0;}
	else if( (NEXT_LATCHES.REGS[DR]>>15) == 0 ) { NEXT_LATCHES.P = 1; NEXT_LATCHES.N = 0; NEXT_LATCHES.Z = 0;}
	else { NEXT_LATCHES.N = 1; NEXT_LATCHES.Z = 0; NEXT_LATCHES.P = 0;}
	
	NEXT_LATCHES.PC = Low16bits( CURRENT_LATCHES.PC+2 );
}

functionSTB()
{
	int SR, BaseR, boffset6;
	int temp1,temp2;
	
	SR = whichRegister(4,6);
	BaseR = whichRegister(7,9);
	boffset6 = signExtend(6);
	
	temp1 = CURRENT_LATCHES.REGS[SR] & 0x000000FF;
	temp2 = Low16bits( CURRENT_LATCHES.REGS[BaseR] + boffset6 );
	MEMORY[temp2/2][temp2%2] = temp1;
	
	NEXT_LATCHES.PC = Low16bits( CURRENT_LATCHES.PC+2 );
	
}

functionJSR()
{
	int BaseR, pcOffset11, temp;
	temp = Low16bits( CURRENT_LATCHES.PC+2 );
	if( instructionRegister[4] == '0' )
	{
		BaseR = whichRegister(7,9);
		NEXT_LATCHES.PC = Low16bits( CURRENT_LATCHES.REGS[BaseR] );
	} 	
	else
	{
		pcOffset11 = signExtend(11);
		NEXT_LATCHES.PC = Low16bits( CURRENT_LATCHES.PC+2 + (pcOffset11<<1) );
	}
	NEXT_LATCHES.REGS[7] = temp;
}

functionAND()
{
	int DR,SR1,SR2;
	int imm5;
	
	DR = whichRegister(4,6);
	SR1 = whichRegister(7,9);
	
	if(instructionRegister[10] == '0')
	{
		SR2 = whichRegister(13,15);
		NEXT_LATCHES.REGS[DR] = Low16bits( CURRENT_LATCHES.REGS[SR1] & CURRENT_LATCHES.REGS[SR2] );
	}	
	else
	{
		imm5 = signExtend(5);
		NEXT_LATCHES.REGS[DR] = Low16bits( CURRENT_LATCHES.REGS[SR1] & imm5 );	
	}
	
	if(NEXT_LATCHES.REGS[DR] == 0) { NEXT_LATCHES.Z = 1; NEXT_LATCHES.N = 0; NEXT_LATCHES.P= 0;}
	else if( (NEXT_LATCHES.REGS[DR]>>15) == 0 ) { NEXT_LATCHES.P = 1; NEXT_LATCHES.N = 0; NEXT_LATCHES.Z = 0;}
	else { NEXT_LATCHES.N = 1; NEXT_LATCHES.Z = 0; NEXT_LATCHES.P = 0;}
	NEXT_LATCHES.PC = Low16bits( CURRENT_LATCHES.PC+2 );
}

functionLDW()
{
	int DR, BaseR, offset6;
	int temp;
	
	DR = whichRegister(4,6);
	BaseR = whichRegister(7,9);
	offset6 = signExtend(6);
	
	temp = Low16bits( CURRENT_LATCHES.REGS[BaseR] + (offset6<<1) );
	NEXT_LATCHES.REGS[DR] = MEMORY[temp/2][0];
	NEXT_LATCHES.REGS[DR] =Low16bits( NEXT_LATCHES.REGS[DR] + ( MEMORY[temp/2][1]<<8 ) ) ;
	
	
	if(NEXT_LATCHES.REGS[DR] == 0) { NEXT_LATCHES.Z = 1; NEXT_LATCHES.N = 0; NEXT_LATCHES.P= 0;}
	else if( (NEXT_LATCHES.REGS[DR]>>15) == 0 ) { NEXT_LATCHES.P = 1; NEXT_LATCHES.N = 0; NEXT_LATCHES.Z = 0;}
	else { NEXT_LATCHES.N = 1; NEXT_LATCHES.Z = 0; NEXT_LATCHES.P = 0;}
	NEXT_LATCHES.PC = Low16bits( CURRENT_LATCHES.PC+2 );
}

functionSTW()
{
	int SR, BaseR, offset6,temp;
	
	SR = whichRegister(4,6);
	BaseR = whichRegister(7,9);
	offset6 = signExtend(6);
	
	temp = Low16bits( CURRENT_LATCHES.REGS[BaseR] + (offset6<<1) );
	MEMORY[temp/2][0] = CURRENT_LATCHES.REGS[SR]&0x000000FF;
	MEMORY[temp/2][1] = CURRENT_LATCHES.REGS[SR]>>8;
	NEXT_LATCHES.PC = Low16bits( CURRENT_LATCHES.PC+2 );
}

functionXOR()
{
	int DR,SR1,SR2;
	int imm5;
	
	DR = whichRegister(4,6);
	SR1 = whichRegister(7,9);
	
	if(instructionRegister[10] == '0')
	{
		SR2 = whichRegister(13,15);
		NEXT_LATCHES.REGS[DR] = Low16bits( CURRENT_LATCHES.REGS[SR1]^CURRENT_LATCHES.REGS[SR2] );
	}	
	else
	{
		imm5 = signExtend(5);
		NEXT_LATCHES.REGS[DR] = Low16bits( CURRENT_LATCHES.REGS[SR1]^imm5 );	
	}
	
	if(NEXT_LATCHES.REGS[DR] == 0) { NEXT_LATCHES.Z = 1; NEXT_LATCHES.N = 0; NEXT_LATCHES.P= 0;}
	else if( (NEXT_LATCHES.REGS[DR]>>15) == 0 ) { NEXT_LATCHES.P = 1; NEXT_LATCHES.N = 0; NEXT_LATCHES.Z = 0;}
	else { NEXT_LATCHES.N = 1; NEXT_LATCHES.Z = 0; NEXT_LATCHES.P = 0;}
	NEXT_LATCHES.PC = Low16bits( CURRENT_LATCHES.PC+2 );
}

functionJMP()
{
	int BaseR;
	
	BaseR = whichRegister(7,9);
	
	NEXT_LATCHES.PC = CURRENT_LATCHES.REGS[BaseR];
}

functionSHF()
{
	int DR,SR,amount4;
	int sum = 0;
	int i;
	
	DR = whichRegister(4,6);
	SR = whichRegister(7,9);
	
	amount4 = (instructionRegister[12]-48)*8+(instructionRegister[13]-48)*4+(instructionRegister[14]-48)*2+(instructionRegister[15]-48);
	
	if( instructionRegister[11] == '0')	NEXT_LATCHES.REGS[DR] = Low16bits( CURRENT_LATCHES.REGS[SR]<<amount4 );
	else
		if( instructionRegister[10] == '0' ) NEXT_LATCHES.REGS[DR] = Low16bits( CURRENT_LATCHES.REGS[SR]>>amount4 );
		else
			if( (CURRENT_LATCHES.REGS[SR]>>15) == 0 ) NEXT_LATCHES.REGS[DR] = Low16bits( CURRENT_LATCHES.REGS[SR]>>amount4 );
			else
			{
				for(i=0;i<amount4;i++)	sum = sum + (1<<(15-i));
				NEXT_LATCHES.REGS[DR] = Low16bits( (CURRENT_LATCHES.REGS[SR]>>amount4) + sum );		
			}
	NEXT_LATCHES.PC = Low16bits( CURRENT_LATCHES.PC+2 );
			
}
functionLEA()
{
	int DR, pcOffset9;
	
	DR = whichRegister(4,6);
	pcOffset9 = signExtend(9);
	
	NEXT_LATCHES.REGS[DR] =  Low16bits( CURRENT_LATCHES.PC+2 + (pcOffset9<<1) );
	NEXT_LATCHES.PC = Low16bits( CURRENT_LATCHES.PC+2 );
}
functionTRAP()
{
	int trapvector8,temp;
	NEXT_LATCHES.REGS[7] = Low16bits( CURRENT_LATCHES.PC+2 );
	trapvector8 = (instructionRegister[8]-48)*128+(instructionRegister[9]-48)*64+(instructionRegister[10]-48)*32+(instructionRegister[11]-48)*16+
	(instructionRegister[12]-48)*8+(instructionRegister[13]-48)*4+(instructionRegister[14]-48)*2+(instructionRegister[15]-48)*1;
	
	temp = Low16bits(trapvector8<<1);
	NEXT_LATCHES.PC = MEMORY[temp/2][0];
	NEXT_LATCHES.PC = Low16bits(NEXT_LATCHES.PC+(MEMORY[temp/2][1]<<8));
}