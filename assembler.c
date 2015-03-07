#include <stdio.h> /* standard input/output library */
#include <stdlib.h> /* Standard C Library */
#include <string.h> /* String operations library */
#include <ctype.h> /* Library for useful character operations */
#include <limits.h> /* Library for definitions of common variable type characteristics */

#define MAX_LINE_LENGTH 255
#define MAX_LABEL_LEN 20
#define MAX_SYMBOLS 255


typedef struct{
	int address;
	char label[MAX_LABEL_LEN + 1];
}TableEntry;
TableEntry symbolTable[MAX_SYMBOLS];

enum
{
	DONE, OK, EMPTY_LINE
};

/* Opening and closing files*/
FILE* infile = NULL;
FILE* outfile = NULL;

int main( int argc, char* argv[] ) 
{
	int labelnum = 0;
	/* open the source file */
	infile = fopen(argv[1], "r");
	outfile = fopen(argv[2], "w");
		 
	if (!infile) 
	{
		printf("Error: Cannot open file %s\n", argv[1]);
		exit(4);
 	}
	if (!outfile) 
	{
		printf("Error: Cannot open file %s\n", argv[2]);
		exit(4);
	}

	/* Do stuff with files */
	labelnum = pass1( infile );
	rewind( infile );
	pass2( infile, labelnum);
     
	fclose(infile);
	fclose(outfile);
}

/*Convert a string to a number*/
int toNum( char * pStr )
{
   char * t_ptr;
   char * orig_pStr;
   int t_length,k;
   int lNum, lNeg = 0;
   long int lNumLong;

   orig_pStr = pStr;
   if( *pStr == '#' )				/* decimal */
   { 
     pStr++;
     if( *pStr == '-' )				/* dec is negative */
     {
       lNeg = 1;
       pStr++;
     }
     t_ptr = pStr;
     t_length = strlen(t_ptr);
     for(k=0;k < t_length;k++)
     {
       if (!isdigit(*t_ptr))
       {
	 printf("Error: invalid decimal operand, %s\n",orig_pStr);
	 exit(4);
       }
       t_ptr++;
     }
     lNum = atoi(pStr);				/*atoi(): converts the string argument str to an integer*/
     if (lNeg)
       lNum = -lNum;
 
     return lNum;
   }
   else if( *pStr == 'x' )	/* hex     */
   {
     pStr++;
     if( *pStr == '-' )				/* hex is negative */
     {
       lNeg = 1;
       pStr++;
     }
     t_ptr = pStr;
     t_length = strlen(t_ptr);
     for(k=0;k < t_length;k++)
     {
       if (!isxdigit(*t_ptr))
       {
		 printf("Error: invalid hex operand, %s\n",orig_pStr);
		 exit(4);
       }
       t_ptr++;
     }
     lNumLong = strtol(pStr, NULL, 16);    /* convert hex string into integer */
     lNum = (lNumLong > INT_MAX)? INT_MAX : lNumLong;
     if( lNeg )
       lNum = -lNum;
     return lNum;
   }
   else
   {
		printf( "Error: invalid operand, %s\n", orig_pStr);
		exit(4);  /* This has been changed from error code 3 to error code 4, see clarification 12 */
   }
}

/*Parsing assembly language*/
int	readAndParse( FILE * pInfile, char * pLine, char ** pLabel, char ** pOpcode, char ** pArg1, char ** pArg2, char ** pArg3, char ** pArg4 )
{
	char * lRet, * lPtr;
	int i;

	if( !fgets( pLine, MAX_LINE_LENGTH, pInfile ) ) 
		return( DONE );	/*finish reading the input file or >MAX_LINE_LENGTH*/ 
		
	/* convert entire line to lowercase */
	for( i = 0; i < strlen( pLine ); i++ )
		pLine[i] = tolower( pLine[i] );	
	   
  
	*pLabel = *pOpcode = *pArg1 = *pArg2 = *pArg3 = *pArg4 = pLine + strlen(pLine); 

	/* ignore the comments */
	lPtr = pLine;
	while( *lPtr != ';' && *lPtr != '\0' && *lPtr != '\n' ) 
		lPtr++;
	*lPtr = '\0';
	
	if( !(lPtr = strtok( pLine, "\t\n ," ) ) ) 
		return( EMPTY_LINE );

	if( isOpcode( lPtr ) == 0 && lPtr[0] != '.' ) /* found a label */
	{
		*pLabel = lPtr;
		if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );
	}
	   
	*pOpcode = lPtr;

	if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );
	   
	*pArg1 = lPtr;
	   
	if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );

	*pArg2 = lPtr;
	
	if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );

	*pArg3 = lPtr;

	if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );

	*pArg4 = lPtr;

	return( OK );
}
	/* Note: MAX_LINE_LENGTH, OK, EMPTY_LINE, and DONE are defined values */

/*Pass 1*/
int pass1( FILE * lInfile ) 
{
	char lLine[MAX_LINE_LENGTH + 1], *lLabel, *lOpcode, *lArg1, *lArg2, *lArg3, *lArg4;
	int lRet;
	int initialAddress;
	int lineCounter = 1;	/*line number counter*/
	int instructionCounter = 0;	/*instruction number counter*/
	int labelCounter = 0;
	char *labelPtr;
	int i;
	int flagORIG = 0;
	int flagEND = 0;

	do
	{
		lRet = readAndParse( lInfile, lLine, &lLabel, &lOpcode, &lArg1, &lArg2, &lArg3, &lArg4 );
		if( lRet != DONE && lRet != EMPTY_LINE )
		{
			if( strcmp( lOpcode, ".orig") == 0)
			{
				flagORIG++;
				initialAddress = toNum(lArg1);
				if( initialAddress%2 != 0 || initialAddress > 65535 || initialAddress < 0 )
				{
					printf("Error 3 at Line %d: invalid constant!\n", lineCounter);
					exit(3);
				}
				if( strcmp( lArg2, "" ) != 0 )	/*If there are operands after .ORIG, report error.*/
				{
					printf("Error 4 at Line %d: unnecessary operand!\n", lineCounter);
					exit(4);
				}
			}
			
			if( strcmp( lLabel, "" ) != 0 && flagORIG != 0 )
			{
				labelPtr = lLabel;
				if( isOpcode( lOpcode ) == 0 && isPseudoOp( lOpcode ) == 0)
				{
					printf("Error 2 at Line %d: invalid opcode!\n", lineCounter);
					exit(2);
				}
				
				do
				{
					if( isalnum(*labelPtr) == 0 || *lLabel == 'x' || isOpcode(lLabel) == 1 || isPseudoOp(lLabel) == 1 || isalpha(*lLabel) == 0 || strlen( lLabel ) > MAX_LABEL_LEN )
					{
						printf("Error 4 at Line %d: invalid label!\n", lineCounter);
						exit(4);
					}
					labelPtr++;	
				}while(*labelPtr != '\0');
				
				for( i=0; i<labelCounter; i++)
				{
					if(strcmp(lLabel, symbolTable[i].label) == 0)
					{
						printf("Error 4 at Line %d: repeated label!\n", lineCounter);
						exit(4);
					}
				}	
							
				strcpy( symbolTable[labelCounter].label, lLabel);
				symbolTable[labelCounter].address = initialAddress + ( instructionCounter - 1 )*2;
				labelCounter++;
			}
			
			if( strcmp( lOpcode , ".end") == 0 )
			{
				flagEND++;
				break;
			}
			if( flagORIG != 0 ) instructionCounter++;	
		}
		lineCounter++;
	} while( lRet != DONE );
	
	if( flagORIG == 0 )
	{
		printf( "Error 4: no .ORIG in the file!\n" );
		exit(4);
	}
	
	if( flagEND == 0 )
	{
		printf("Error 4: no .END in the file!\n");
		exit(4);
	}
	return (labelCounter);
}	

/*Pass2*/	
pass2( FILE * lInfile, int labelnumber) 
{
	char lLine[MAX_LINE_LENGTH + 1], *lLabel, *lOpcode, *lArg1, *lArg2, *lArg3, *lArg4;
	int lRet;
	int lineCounter = 1;	/*line number counter*/
	int instructionCounter = 0;
	int flagORIG = 0;
	int flagEND = 0;
	int initialAddress = 0;
	
	do
	{
		lRet = readAndParse( lInfile, lLine, &lLabel, &lOpcode, &lArg1, &lArg2, &lArg3, &lArg4 );
		if( lRet != DONE && lRet != EMPTY_LINE )
		{
			if( strcmp( lOpcode, ".orig") == 0) flagORIG++;
			
			if( flagORIG != 0)
			{
				if( isOpcode( lOpcode ) == 0 && isPseudoOp( lOpcode ) == 0 )
				{
					printf("Error 2 at line %d: invalid opcode!\n", lineCounter);
					exit(2);
				}
				
				switch( *lOpcode )
				{
					case 'a':
					switch( *(lOpcode+1) )
					{
						case 'd': functionADD(lArg1, lArg2, lArg3, lArg4, lineCounter); break;
						case 'n': functionAND(lArg1, lArg2, lArg3, lArg4, lineCounter); break;
					}
					break;
					
					case 'b':
					switch( *(lOpcode+2) )
					{
						case '\0': functionBR( lArg1, lArg2, lArg3, lArg4, lineCounter, instructionCounter, labelnumber, initialAddress ); break;
						
						case 'n': 
						switch( *(lOpcode+3) )
						{
      						case '\0': functionBRn(lArg1, lArg2, lArg3, lArg4, lineCounter, instructionCounter, labelnumber, initialAddress); break;
							case 'z':
							switch ( *(lOpcode+4) )
							{
								case '\0': functionBRnz(lArg1, lArg2, lArg3, lArg4, lineCounter, instructionCounter, labelnumber, initialAddress); break;
								case 'p': functionBRnzp(lArg1, lArg2, lArg3, lArg4, lineCounter, instructionCounter, labelnumber, initialAddress); break;
							}	
							break;
							case 'p': functionBRnp(lArg1, lArg2, lArg3, lArg4, lineCounter, instructionCounter, labelnumber, initialAddress); break;
						}
						break;
						
						case 'z':
						switch( *(lOpcode+3) )
						{
							case '\0': functionBRz(lArg1, lArg2, lArg3, lArg4, lineCounter, instructionCounter, labelnumber, initialAddress); break;
							case 'p': functionBRzp(lArg1, lArg2, lArg3, lArg4, lineCounter, instructionCounter, labelnumber, initialAddress); break;
						}		
						break;
						
						case 'p': functionBRp(lArg1, lArg2, lArg3, lArg4, lineCounter, instructionCounter, labelnumber, initialAddress); break;		
					}
					break;
					
					case 'h': functionHALT(lArg1, lArg2, lArg3, lArg4, lineCounter); break;
					
					case 'j':
					switch( *(lOpcode+1) )
					{
				 		case 'm': functionJMP(lArg1, lArg2, lArg3, lArg4, lineCounter); break;
				 		
						case 's': 
						switch( *(lOpcode+3) )
						{
							case '\0': functionJSR(lArg1, lArg2, lArg3, lArg4, lineCounter, instructionCounter, labelnumber, initialAddress); break;
							case 'r': functionJSRR(lArg1, lArg2, lArg3, lArg4, lineCounter); break;	
						}
						break;	
					}	
					break;
					
					case 'l':
					switch( *(lOpcode+1) )
					{
						case 'e': functionLEA(lArg1, lArg2, lArg3, lArg4, lineCounter, instructionCounter, labelnumber, initialAddress); break;
						
						case 's': functionLSHF(lArg1, lArg2, lArg3, lArg4, lineCounter); break;
						
						case 'd':
						switch( *(lOpcode+2) )
						{
							case 'b': functionLDB( lArg1, lArg2, lArg3, lArg4, lineCounter ); break;
							case 'w': functionLDW(lArg1, lArg2, lArg3, lArg4, lineCounter ); break;	
						}	
						break;	
					}
						
					break;
					
					case 'n': 
					switch( *(lOpcode+2) )
					{
						case 't': functionNOT(lArg1, lArg2, lArg3, lArg4, lineCounter); break;
						case 'p': functionNOP(lArg1, lArg2, lArg3, lArg4, lineCounter); break;
					}
					break;
					
					case 'r':
					switch( *(lOpcode+1) )
					{
						case 'e': functionRET(lArg1, lArg2, lArg3, lArg4, lineCounter); break;
						
						case 't': functionRTI(lArg1, lArg2, lArg3, lArg4, lineCounter); break;
						
						case 's':
						switch( *(lOpcode+4) )
						{
							case 'a': functionRSHFA(lArg1, lArg2, lArg3, lArg4, lineCounter); break;
							case 'l': functionRSHFL(lArg1, lArg2, lArg3, lArg4, lineCounter); break;	
						}
						break;	
					}				
					break;
					
					case 's':
					switch( *(lOpcode+2) )
					{
						case 'b': functionSTB(lArg1, lArg2, lArg3, lArg4, lineCounter); break;
						case 'w': functionSTW(lArg1, lArg2, lArg3, lArg4, lineCounter); break;	
					}
					break;
					
					case 't': functionTRAP(lArg1, lArg2, lArg3, lArg4, lineCounter); break;
					
					case 'x': functionXOR(lArg1, lArg2, lArg3, lArg4, lineCounter); break;	
					
					case '.':
						switch( *(lOpcode+1) )
						{
							case 'o': initialAddress = functionORIG( lArg1, lArg2, lArg3, lArg4, lineCounter ); break;
							case 'f': functionFILL(lArg1, lArg2, lArg3, lArg4, lineCounter); break;
							/*
							case 'b': functionBLKW(lArg1, lArg2, lArg3, lArg4, lineCounter); break;
							case 's': functionSTRINGZ(lArg1, lArg2, lArg3, lArg4, lineCounter); break;
							*/
							case 'e': exit(0); 
							default: break;	
						}	
					break;					
				}
				instructionCounter++;	
			}	
		}
		lineCounter++;
	} while( lRet != DONE );
}

/*Judge whether it is an opcode. YES return 1. NO return 0;*/
int isOpcode( char * codeToBeTested)
{
	int i;
	char opcode[28][6] = {"add","and","br","brn","brz","brp","brnz","brzp","brnp","brnzp","halt","jmp","jsr","jsrr","ldb","ldw","lea","not","nop","ret","rti","lshf","rshfl","rshfa","stb","stw","trap","xor"};

	for( i=0 ; i<28 ; i++ )
		if(!strcmp( codeToBeTested , opcode[i])) return(1);
		
	return(0);
}

/*Judge whether it is an Pseudo-Op. YES return 1. NO return 0;*/
int isPseudoOp( char * codeToBeTested)
{
	int i;
	char pseudoOp[5][9] = {".orig",".fill",".blkw",".stringz",".end"};
	
	for( i=0; i<5; i++)
		if(!strcmp( codeToBeTested, pseudoOp[i])) return(1);
		
	return(0);
}

/*Judge whether it is an Pseudo-Op. YES return # of register. NO return -1;*/
int isRegister( char *stringToBeTested)
{
	int i;
	char registerFile[8][3] = {"r0","r1","r2","r3","r4","r5","r6","r7"};
	
	for( i=0; i<8; i++)
		if( strcmp( stringToBeTested, registerFile[i] ) == 0 ) return(i);
		
	return(-1);
	
} 

/*Convert a 4-bits binary string to a 1-bit hex string*/
int binaryToHex( char *binaryCode)
{
	int decimal = 0;
	decimal = (*binaryCode -48)*8 + (*(binaryCode+1)-48)*4 + (*(binaryCode+2)-48)*2 + (*(binaryCode+3)-48)*1;
	if( decimal<0 || decimal>15) return(0);
	else if( decimal>=0 && decimal<=9) return( decimal + 48 );
	else return( decimal + 55 );
}

functionADD(char *operand1, char *operand2, char *operand3, char* operand4, int lctr)
{
	char machineCode[22];
	int temp, i;
	
	machineCode[16] = '\0';
	machineCode[21] = '\0';
	machineCode[0] = '0';
	machineCode[1] = '0';
	machineCode[2] = '0';
	machineCode[3] = '1';
		
	if( strcmp( operand1, "" ) == 0 || strcmp( operand2, "" ) == 0 || strcmp( operand3, "" ) == 0)
	{
		printf("Error 4 at Line %d: missing operand!\n", lctr);
		exit(4);
	}
	if( strcmp( operand4, "") != 0 )
	{
		printf("Error 4 at Line %d: unnecessary operand!\n", lctr);
		exit(4);
	}
	if( isRegister( operand1 ) == -1 || isRegister( operand2 ) == -1 )
	{
		printf("Error 4 at Line %d: unexpected operand!\n", lctr);
		exit(4);	
	}
	
	temp = isRegister( operand1 );
	for(i=2; i >= 0 ; i--)
	{
		machineCode[ 4+i ] = 48 + temp%2;
		temp = temp/2;
	}
	
	temp = isRegister( operand2 );
	for(i=2; i >= 0; i--)
	{
		machineCode[ 7+i ] = 48 + temp%2;
		temp = temp/2;
	}

	if( isRegister( operand3 ) != -1 )
	{
		machineCode[10] = machineCode[11] = machineCode[12] = '0';
		temp = isRegister( operand3 );
		for(i=2; i >= 0; i--)
			{
				machineCode[ 13+i ] = 48 + temp%2;
				temp = temp/2;
			}
	}
	else
	{
		machineCode[10] = '1';
		temp = toNum( operand3 );
		if( temp < -16 || temp > 15)
		{
			printf("Error 3 at Line %d: invalid constant!\n", lctr);
			exit(3);
		}
		if( temp >= -16 && temp < 0 ) temp = temp + 32;
		for(i=4; i >= 0 ; i--)
			{
				machineCode[ 11+i ] = 48 + temp%2;
				temp = temp/2;
			}			
	}
	machineCode[17] = binaryToHex( machineCode );
	machineCode[18] = binaryToHex( machineCode+4 );
	machineCode[19] = binaryToHex( machineCode+8 );
	machineCode[20] = binaryToHex( machineCode+12 );
	fprintf( outfile, "0x%s\n", machineCode+17 );
}

functionAND(char *operand1, char *operand2, char *operand3, char* operand4, int lctr)
{
	char machineCode[22];
	int temp, i;
	
	machineCode[16] = '\0';
	machineCode[21] = '\0';
	machineCode[0] = '0';
	machineCode[1] = '1';
	machineCode[2] = '0';
	machineCode[3] = '1';
		
	if( strcmp( operand1, "" ) == 0 || strcmp( operand2, "" ) == 0 || strcmp( operand3, "" ) == 0)
	{
		printf("Error 4 at Line %d: missing operand!\n", lctr);
		exit(4);
	}
	if( strcmp( operand4, "") != 0 )
	{
		printf("Error 4 at Line %d: unnecessary operand!\n", lctr);
		exit(4);
	}
	if( isRegister( operand1 ) == -1 || isRegister( operand2 ) == -1 )
	{
		printf("Error 4 at Line %d: unexpected operand!\n", lctr);
		exit(4);	
	}
	
	temp = isRegister( operand1 );
	for(i=2; i >= 0 ; i--)
	{
		machineCode[ 4+i ] = 48 + temp%2;
		temp = temp/2;
	}
	
	temp = isRegister( operand2 );
	for(i=2; i >= 0; i--)
	{
		machineCode[ 7+i ] = 48 + temp%2;
		temp = temp/2;
	}

	if( isRegister( operand3 ) != -1 )
	{
		machineCode[10] = machineCode[11] = machineCode[12] = '0';
		temp = isRegister( operand3 );
		for(i=2; i >= 0; i--)
			{
				machineCode[ 13+i ] = 48 + temp%2;
				temp = temp/2;
			}
	}
	else
	{
		machineCode[10] = '1';
		temp = toNum( operand3 );
		if( temp < -16 || temp > 15)
		{
			printf("Error 3 at Line %d: invalid constant!\n", lctr);
			exit(3);
		}
		if( temp >= -16 && temp < 0 ) temp = temp + 32;
		for(i=4; i >= 0 ; i--)
			{
				machineCode[ 11+i ] = 48 + temp%2;
				temp = temp/2;
			}			
	}
	machineCode[17] = binaryToHex( machineCode );
	machineCode[18] = binaryToHex( machineCode+4 );
	machineCode[19] = binaryToHex( machineCode+8 );
	machineCode[20] = binaryToHex( machineCode+12 );
	fprintf( outfile, "0x%s\n", machineCode+17 );
}
functionRET(char *operand1, char *operand2, char *operand3, char* operand4, int lctr)
{
	if( strcmp( operand1, "") != 0 || strcmp( operand2, "") != 0 || strcmp( operand3, "") != 0 || strcmp( operand4, "") != 0 )
	{
		printf("Error 4 at Line %d: unnecessary operand!\n", lctr);
		exit(4);
	}
	fprintf( outfile , "0xc1c0\n");
}

functionJMP(char *operand1, char *operand2, char *operand3, char* operand4, int lctr)
{
	char machineCode[22];
	int temp, i;
	
	machineCode[16] = '\0';
	machineCode[21] = '\0';
	machineCode[0] = '1';
	machineCode[1] = '1';
	machineCode[2] = '0';
	machineCode[3] = '0';
	machineCode[4] = '0';
	machineCode[5] = '0';
	machineCode[6] = '0';
	machineCode[10] = '0';
	machineCode[11] = '0';
	machineCode[12] = '0';
	machineCode[13] = '0';
	machineCode[14] = '0';
	machineCode[15] = '0';
		
	if( strcmp( operand1, "" ) == 0 )
	{
		printf("Error 4 at Line %d: missing operand!\n", lctr);
		exit(4);
	}
	if( strcmp( operand2, "" ) != 0 || strcmp( operand3, "" ) != 0 || strcmp( operand4, "") != 0 )
	{
		printf("Error 4 at Line %d: unnecessary operand!\n", lctr);
		exit(4);
	}
	if( isRegister( operand1 ) == -1 )
	{
		printf("Error 4 at Line %d: unexpected operand!\n", lctr);
		exit(4);	
	}
	
	temp = isRegister( operand1 );
	for(i=2; i >= 0 ; i--)
	{
		machineCode[ 7+i ] = 48 + temp%2;
		temp = temp/2;
	}
	
	machineCode[17] = binaryToHex( machineCode );
	machineCode[18] = binaryToHex( machineCode+4 );
	machineCode[19] = binaryToHex( machineCode+8 );
	machineCode[20] = binaryToHex( machineCode+12 );
	fprintf( outfile, "0x%s\n", machineCode+17 );			
}

functionJSRR(char *operand1, char *operand2, char *operand3, char* operand4, int lctr)
{
	char machineCode[22];
	int temp, i;
	
	machineCode[16] = '\0';
	machineCode[21] = '\0';
	machineCode[0] = '0';
	machineCode[1] = '1';
	machineCode[2] = '0';
	machineCode[3] = '0';
	machineCode[4] = '0';
	machineCode[5] = '0';
	machineCode[6] = '0';
	machineCode[10] = '0';
	machineCode[11] = '0';
	machineCode[12] = '0';
	machineCode[13] = '0';
	machineCode[14] = '0';
	machineCode[15] = '0';
		
	if( strcmp( operand1, "" ) == 0 )
	{
		printf("Error 4 at Line %d: missing operand!\n", lctr);
		exit(4);
	}
	if( strcmp( operand2, "" ) != 0 || strcmp( operand3, "" ) != 0 || strcmp( operand4, "") != 0 )
	{
		printf("Error 4 at Line %d: unnecessary operand!\n", lctr);
		exit(4);
	}
	if( isRegister( operand1 ) == -1 )
	{
		printf("Error 4 at Line %d: unexpected operand!\n", lctr);
		exit(4);	
	}
	
	temp = isRegister( operand1 );
	for(i=2; i >= 0 ; i--)
	{
		machineCode[ 7+i ] = 48 + temp%2;
		temp = temp/2;
	}
	
	machineCode[17] = binaryToHex( machineCode );
	machineCode[18] = binaryToHex( machineCode+4 );
	machineCode[19] = binaryToHex( machineCode+8 );
	machineCode[20] = binaryToHex( machineCode+12 );
	fprintf( outfile, "0x%s\n", machineCode+17 );	
}

functionNOT(char *operand1, char *operand2, char *operand3, char* operand4, int lctr)
{
	char machineCode[22];
	int temp, i;
	
	machineCode[16] = '\0';
	machineCode[21] = '\0';
	machineCode[0] = '1';
	machineCode[1] = '0';
	machineCode[2] = '0';
	machineCode[3] = '1';
	machineCode[10] = '1';
	machineCode[11] = '1';
	machineCode[12] = '1';
	machineCode[13] = '1';
	machineCode[14] = '1';
	machineCode[15] = '1';
		
	if( strcmp( operand1, "" ) == 0 || strcmp( operand2, "" ) == 0 )
	{
		printf("Error 4 at Line %d: missing operand!\n", lctr);
		exit(4);
	}
	if( strcmp( operand3, "" ) != 0 || strcmp( operand4, "") != 0 )
	{
		printf("Error 4 at Line %d: unnecessary operand!\n", lctr);
		exit(4);
	}
	if( isRegister( operand1 ) == -1 || isRegister( operand2 ) == -1 )
	{
		printf("Error 4 at Line %d: unexpected operand!\n", lctr);
		exit(4);	
	}
	
	temp = isRegister( operand1 );
	for(i=2; i >= 0 ; i--)
	{
		machineCode[ 4+i ] = 48 + temp%2;
		temp = temp/2;
	}
	
	temp = isRegister( operand2 );
	for(i=2; i >= 0; i--)
	{
		machineCode[ 7+i ] = 48 + temp%2;
		temp = temp/2;
	}	
	machineCode[17] = binaryToHex( machineCode );
	machineCode[18] = binaryToHex( machineCode+4 );
	machineCode[19] = binaryToHex( machineCode+8 );
	machineCode[20] = binaryToHex( machineCode+12 );
	fprintf( outfile, "0x%s\n", machineCode+17 );
}

functionRTI(char *operand1, char *operand2, char *operand3, char* operand4, int lctr)
{
	if( strcmp( operand1, "") != 0 || strcmp( operand2, "") != 0 || strcmp( operand3, "") != 0 || strcmp( operand4, "") != 0 )
	{
		printf("Error 4 at Line %d: unnecessary operand!\n", lctr);
		exit(4);
	}
	fprintf( outfile , "0x8000\n");
}

functionLSHF(char *operand1, char *operand2, char *operand3, char* operand4, int lctr)
{
	char machineCode[22];
	int temp, i;
	
	machineCode[16] = '\0';
	machineCode[21] = '\0';
	machineCode[0] = '1';
	machineCode[1] = '1';
	machineCode[2] = '0';
	machineCode[3] = '1';
	machineCode[10] = '0';
	machineCode[11] = '0';
		
	if( strcmp( operand1, "" ) == 0 || strcmp( operand2, "" ) == 0 || strcmp( operand3, "" ) == 0)
	{
		printf("Error 4 at Line %d: missing operand!\n", lctr);
		exit(4);
	}
	if( strcmp( operand4, "") != 0 )
	{
		printf("Error 4 at Line %d: unnecessary operand!\n", lctr);
		exit(4);
	}
	if( isRegister( operand1 ) == -1 || isRegister( operand2 ) == -1 )
	{
		printf("Error 4 at Line %d: unexpected operand!\n", lctr);
		exit(4);	
	}
	
	temp = isRegister( operand1 );
	for(i=2; i >= 0 ; i--)
	{
		machineCode[ 4+i ] = 48 + temp%2;
		temp = temp/2;
	}
	
	temp = isRegister( operand2 );
	for(i=2; i >= 0; i--)
	{
		machineCode[ 7+i ] = 48 + temp%2;
		temp = temp/2;
	}

	temp = toNum( operand3 );
	if( temp < 0 || temp >15)
	{
		printf("Error 3 at Line %d: invalid constant!\n", lctr);
		exit(3);
	}
	for(i=3; i >= 0 ; i--)
			{
				machineCode[ 12+i ] = 48 + temp%2;
				temp = temp/2;
			}	
	
	machineCode[17] = binaryToHex( machineCode );
	machineCode[18] = binaryToHex( machineCode+4 );
	machineCode[19] = binaryToHex( machineCode+8 );
	machineCode[20] = binaryToHex( machineCode+12 );
	fprintf( outfile, "0x%s\n", machineCode+17 );		
}

functionRSHFL(char *operand1, char *operand2, char *operand3, char* operand4, int lctr)
{
	char machineCode[22];
	int temp, i;
	
	machineCode[16] = '\0';
	machineCode[21] = '\0';
	machineCode[0] = '1';
	machineCode[1] = '1';
	machineCode[2] = '0';
	machineCode[3] = '1';
	machineCode[10] = '0';
	machineCode[11] = '1';
		
	if( strcmp( operand1, "" ) == 0 || strcmp( operand2, "" ) == 0 || strcmp( operand3, "" ) == 0)
	{
		printf("Error 4 at Line %d: missing operand!\n", lctr);
		exit(4);
	}
	if( strcmp( operand4, "") != 0 )
	{
		printf("Error 4 at Line %d: unnecessary operand!\n", lctr);
		exit(4);
	}
	if( isRegister( operand1 ) == -1 || isRegister( operand2 ) == -1 )
	{
		printf("Error 4 at Line %d: unexpected operand!\n", lctr);
		exit(4);	
	}
	
	temp = isRegister( operand1 );
	for(i=2; i >= 0 ; i--)
	{
		machineCode[ 4+i ] = 48 + temp%2;
		temp = temp/2;
	}
	
	temp = isRegister( operand2 );
	for(i=2; i >= 0; i--)
	{
		machineCode[ 7+i ] = 48 + temp%2;
		temp = temp/2;
	}

	temp = toNum( operand3 );
	if( temp < 0 || temp >15)
	{
		printf("Error 3 at Line %d: invalid constant!\n", lctr);
		exit(3);
	}
	for(i=3; i >= 0 ; i--)
			{
				machineCode[ 12+i ] = 48 + temp%2;
				temp = temp/2;
			}	
	
	machineCode[17] = binaryToHex( machineCode );
	machineCode[18] = binaryToHex( machineCode+4 );
	machineCode[19] = binaryToHex( machineCode+8 );
	machineCode[20] = binaryToHex( machineCode+12 );
	fprintf( outfile, "0x%s\n", machineCode+17 );		
}

functionRSHFA(char *operand1, char *operand2, char *operand3, char* operand4, int lctr)
{
	char machineCode[22];
	int temp, i;
	
	machineCode[16] = '\0';
	machineCode[21] = '\0';
	machineCode[0] = '1';
	machineCode[1] = '1';
	machineCode[2] = '0';
	machineCode[3] = '1';
	machineCode[10] = '1';
	machineCode[11] = '1';
		
	if( strcmp( operand1, "" ) == 0 || strcmp( operand2, "" ) == 0 || strcmp( operand3, "" ) == 0)
	{
		printf("Error 4 at Line %d: missing operand!\n", lctr);
		exit(4);
	}
	if( strcmp( operand4, "") != 0 )
	{
		printf("Error 4 at Line %d: unnecessary operand!\n", lctr);
		exit(4);
	}
	if( isRegister( operand1 ) == -1 || isRegister( operand2 ) == -1 )
	{
		printf("Error 4 at Line %d: unexpected operand!\n", lctr);
		exit(4);	
	}
	
	temp = isRegister( operand1 );
	for(i=2; i >= 0 ; i--)
	{
		machineCode[ 4+i ] = 48 + temp%2;
		temp = temp/2;
	}
	
	temp = isRegister( operand2 );
	for(i=2; i >= 0; i--)
	{
		machineCode[ 7+i ] = 48 + temp%2;
		temp = temp/2;
	}

	temp = toNum( operand3 );
	if( temp < 0 || temp >15)
	{
		printf("Error 3 at Line %d: invalid constant!\n", lctr);
		exit(3);
	}
	for(i=3; i >= 0 ; i--)
			{
				machineCode[ 12+i ] = 48 + temp%2;
				temp = temp/2;
			}	
	
	machineCode[17] = binaryToHex( machineCode );
	machineCode[18] = binaryToHex( machineCode+4 );
	machineCode[19] = binaryToHex( machineCode+8 );
	machineCode[20] = binaryToHex( machineCode+12 );
	fprintf( outfile, "0x%s\n", machineCode+17 );		
}

functionXOR(char *operand1, char *operand2, char *operand3, char* operand4, int lctr)
{
	char machineCode[22];
	int temp, i;
	
	machineCode[16] = '\0';
	machineCode[21] = '\0';
	machineCode[0] = '1';
	machineCode[1] = '0';
	machineCode[2] = '0';
	machineCode[3] = '1';
		
	if( strcmp( operand1, "" ) == 0 || strcmp( operand2, "" ) == 0 || strcmp( operand3, "" ) == 0)
	{
		printf("Error 4 at Line %d: missing operand!\n", lctr);
		exit(4);
	}
	if( strcmp( operand4, "") != 0 )
	{
		printf("Error 4 at Line %d: unnecessary operand!\n", lctr);
		exit(4);
	}
	if( isRegister( operand1 ) == -1 || isRegister( operand2 ) == -1 )
	{
		printf("Error 4 at Line %d: unexpected operand!\n", lctr);
		exit(4);	
	}
	
	temp = isRegister( operand1 );
	for(i=2; i >= 0 ; i--)
	{
		machineCode[ 4+i ] = 48 + temp%2;
		temp = temp/2;
	}
	
	temp = isRegister( operand2 );
	for(i=2; i >= 0; i--)
	{
		machineCode[ 7+i ] = 48 + temp%2;
		temp = temp/2;
	}

	if( isRegister( operand3 ) != -1 )
	{
		machineCode[10] = machineCode[11] = machineCode[12] = '0';
		temp = isRegister( operand3 );
		for(i=2; i >= 0; i--)
			{
				machineCode[ 13+i ] = 48 + temp%2;
				temp = temp/2;
			}
	}
	else
	{
		machineCode[10] = '1';
		temp = toNum( operand3 );
		if( temp < -16 || temp > 15)
		{
			printf("Error 3 at Line %d: invalid constant!\n", lctr);
			exit(3);
		}
		if( temp >= -16 && temp < 0 ) temp = temp + 32;
		for(i=4; i >= 0 ; i--)
			{
				machineCode[ 11+i ] = 48 + temp%2;
				temp = temp/2;
			}			
	}
	machineCode[17] = binaryToHex( machineCode );
	machineCode[18] = binaryToHex( machineCode+4 );
	machineCode[19] = binaryToHex( machineCode+8 );
	machineCode[20] = binaryToHex( machineCode+12 );
	fprintf( outfile, "0x%s\n", machineCode+17 );
}
functionTRAP(char *operand1, char *operand2, char *operand3, char* operand4, int lctr)
{
	char machineCode[22];
	int temp, i;
	
	machineCode[16] = '\0';
	machineCode[21] = '\0';
	machineCode[0] = '1';
	machineCode[1] = '1';
	machineCode[2] = '1';
	machineCode[3] = '1';
	machineCode[4] = '0';
	machineCode[5] = '0';
	machineCode[6] = '0';
	machineCode[7] = '0';
	
	if( strcmp( operand1, "" ) == 0 )
	{
		printf("Error 4 at Line %d: missing operand!\n", lctr);
		exit(4);
	}
	if( strcmp( operand2, "" ) != 0 || strcmp( operand3, "" ) != 0 || strcmp( operand4, "") != 0 )
	{
		printf("Error 4 at Line %d: unnecessary operand!\n", lctr);
		exit(4);
	}

	temp = toNum( operand1 );
	
	if( temp >= 0 && temp <= 255 && *operand1 == 'x')
	{
		for(i=5; i >= 0 ; i--)
		{
			machineCode[ 10+i ] = 48 + temp%2;
			temp = temp/2;
		}
	}
	else
	{
		printf("Error 3 at Line %d: invalid constant!\n", lctr);
		exit(3);
	}
	
	machineCode[8] = '0';
	machineCode[9] = '0';
	machineCode[17] = binaryToHex( machineCode );
	machineCode[18] = binaryToHex( machineCode+4 );
	machineCode[19] = binaryToHex( machineCode+8 );
	machineCode[20] = binaryToHex( machineCode+12 );
	fprintf( outfile, "0x%s\n", machineCode+17 );
}

functionNOP(char *operand1, char *operand2, char *operand3, char* operand4, int lctr)
{
	if( strcmp( operand1, "") != 0 || strcmp( operand2, "") != 0 || strcmp( operand3, "") != 0 || strcmp( operand4, "") != 0 )
	{
		printf("Error 4 at Line %d: unnecessary operand!\n", lctr);
		exit(4);
	}
	fprintf( outfile , "0x0000\n");
}
functionHALT(char *operand1, char *operand2, char *operand3, char* operand4, int lctr)
{
	if( strcmp( operand1, "") != 0 || strcmp( operand2, "") != 0 || strcmp( operand3, "") != 0 || strcmp( operand4, "") != 0 )
	{
		printf("Error 4 at Line %d: unnecessary operand!\n", lctr);
		exit(4);
	}
	functionTRAP( "x25", "", "", "", lctr);
}

functionBR(char *operand1, char *operand2, char *operand3, char* operand4, int lctr, int instructionctr, int labelctr, int initaddr )
{
	char machineCode[22];
	int temp, i, pcOffset9;
	int flag = 0;
	int index = 0;
	
	machineCode[16] = '\0';
	machineCode[21] = '\0';
	machineCode[0] = '0';
	machineCode[1] = '0';
	machineCode[2] = '0';
	machineCode[3] = '0';
	machineCode[4] = '1';
	machineCode[5] = '1';
	machineCode[6] = '1';
	
	if( strcmp( operand1, "" ) == 0 )
	{
		printf("Error 4 at Line %d: missing operand!\n", lctr);
		exit(4);
	}
	if( strcmp( operand2, "" ) != 0 || strcmp( operand3, "" ) != 0 || strcmp( operand4, "") != 0 )
	{
		printf("Error 4 at Line %d: unnecessary operand!\n", lctr);
		exit(4);
	}	
	
	if( (*operand1 == 'x' || *operand1 == '#'))
	{
		for( i=1; *(operand1+i) != '\0'; i++)
			if( *(operand1+i)>'9' || *(operand1+i)<'0') 
			{
				index = 1;
				break;
			} 
		if( index == 0 )
		{
			printf("Error 4 at Line %d: unexpected operand!\n", lctr);
			exit(4);
		}
	}
	
	
	
	for( i = 0; i < labelctr; i++)
	{
		if( strcmp( operand1, symbolTable[i].label ) ==0 )
		{
			flag = 1;
			pcOffset9 = ( symbolTable[i].address - initaddr )/2 - instructionctr;
			if( pcOffset9 < -256 || pcOffset9 > 255) 
			{
				printf("Error 4 at Line %d: too far!\n", lctr);
				exit(4);
			}
			if( pcOffset9<0 ) pcOffset9 = pcOffset9 + 512;	
			break;	
		}
	}
	
	if( flag == 0)
	{
		printf("Error 1 at Line %d: undefined label!\n", lctr);
		exit(1);
	}
	
	for( i=8 ; i >= 0 ; i--)
	{
		machineCode[ 7+i ] = 48 +pcOffset9%2;
		pcOffset9 = pcOffset9/2;
	}
	
	machineCode[17] = binaryToHex( machineCode );
	machineCode[18] = binaryToHex( machineCode+4 );
	machineCode[19] = binaryToHex( machineCode+8 );
	machineCode[20] = binaryToHex( machineCode+12 );
	fprintf( outfile, "0x%s\n", machineCode+17 );		
}

functionBRn(char *operand1, char *operand2, char *operand3, char* operand4, int lctr, int instructionctr, int labelctr, int initaddr )
{
	char machineCode[22];
	int temp, i, pcOffset9;
	int flag = 0;
	int index = 0;
	
	machineCode[16] = '\0';
	machineCode[21] = '\0';
	machineCode[0] = '0';
	machineCode[1] = '0';
	machineCode[2] = '0';
	machineCode[3] = '0';
	machineCode[4] = '1';
	machineCode[5] = '0';
	machineCode[6] = '0';
	
	if( strcmp( operand1, "" ) == 0 )
	{
		printf("Error 4 at Line %d: missing operand!\n", lctr);
		exit(4);
	}
	if( strcmp( operand2, "" ) != 0 || strcmp( operand3, "" ) != 0 || strcmp( operand4, "") != 0 )
	{
		printf("Error 4 at Line %d: unnecessary operand!\n", lctr);
		exit(4);
	}	
	
	if( (*operand1 == 'x' || *operand1 == '#'))
	{
		for( i=1; *(operand1+i) != '\0'; i++)
			if( *(operand1+i)>'9' || *(operand1+i)<'0') 
			{
				index = 1;
				break;
			} 
		if( index == 0 )
		{
			printf("Error 4 at Line %d: unexpected operand!\n", lctr);
			exit(4);
		}
	}
	
	for( i = 0; i < labelctr; i++)
	{
		if( strcmp( operand1, symbolTable[i].label ) ==0 )
		{
			flag = 1;
			pcOffset9 = ( symbolTable[i].address - initaddr )/2 - instructionctr;
			if( pcOffset9 < -256 || pcOffset9 > 255) 
			{
				printf("Error 4 at Line %d: too far!\n", lctr);
				exit(4);
			}
			if( pcOffset9<0 ) pcOffset9 = pcOffset9 + 512;	
			break;	
		}
	}
	
	if( flag == 0)
	{
		printf("Error 1 at Line %d: undefined label!\n", lctr);
		exit(1);
	}
	
	for( i=8 ; i >= 0 ; i--)
	{
		machineCode[ 7+i ] = 48 +pcOffset9%2;
		pcOffset9 = pcOffset9/2;
	}
	
	machineCode[17] = binaryToHex( machineCode );
	machineCode[18] = binaryToHex( machineCode+4 );
	machineCode[19] = binaryToHex( machineCode+8 );
	machineCode[20] = binaryToHex( machineCode+12 );
	fprintf( outfile, "0x%s\n", machineCode+17 );		
}

functionBRz(char *operand1, char *operand2, char *operand3, char* operand4, int lctr, int instructionctr, int labelctr, int initaddr )
{
	char machineCode[22];
	int temp, i, pcOffset9;
	int flag = 0;
	int index = 0;
	
	machineCode[16] = '\0';
	machineCode[21] = '\0';
	machineCode[0] = '0';
	machineCode[1] = '0';
	machineCode[2] = '0';
	machineCode[3] = '0';
	machineCode[4] = '0';
	machineCode[5] = '1';
	machineCode[6] = '0';
	
	if( strcmp( operand1, "" ) == 0 )
	{
		printf("Error 4 at Line %d: missing operand!\n", lctr);
		exit(4);
	}
	if( strcmp( operand2, "" ) != 0 || strcmp( operand3, "" ) != 0 || strcmp( operand4, "") != 0 )
	{
		printf("Error 4 at Line %d: unnecessary operand!\n", lctr);
		exit(4);
	}	
	
	if( (*operand1 == 'x' || *operand1 == '#'))
	{
		for( i=1; *(operand1+i) != '\0'; i++)
			if( *(operand1+i)>'9' || *(operand1+i)<'0') 
			{
				index = 1;
				break;
			} 
		if( index == 0 )
		{
			printf("Error 4 at Line %d: unexpected operand!\n", lctr);
			exit(4);
		}
	}
	
	for( i = 0; i < labelctr; i++)
	{
		if( strcmp( operand1, symbolTable[i].label ) ==0 )
		{
			flag = 1;
			pcOffset9 = ( symbolTable[i].address - initaddr )/2 - instructionctr;
			if( pcOffset9 < -256 || pcOffset9 > 255) 
			{
				printf("Error 4 at Line %d: too far!\n", lctr);
				exit(4);
			}
			if( pcOffset9<0 ) pcOffset9 = pcOffset9 + 512;	
			break;	
		}
	}
	
	if( flag == 0)
	{
		printf("Error 1 at Line %d: undefined label!\n", lctr);
		exit(1);
	}
	
	for( i=8 ; i >= 0 ; i--)
	{
		machineCode[ 7+i ] = 48 +pcOffset9%2;
		pcOffset9 = pcOffset9/2;
	}
	
	machineCode[17] = binaryToHex( machineCode );
	machineCode[18] = binaryToHex( machineCode+4 );
	machineCode[19] = binaryToHex( machineCode+8 );
	machineCode[20] = binaryToHex( machineCode+12 );
	fprintf( outfile, "0x%s\n", machineCode+17 );		
}

functionBRp(char *operand1, char *operand2, char *operand3, char* operand4, int lctr, int instructionctr, int labelctr, int initaddr )
{
	char machineCode[22];
	int temp, i, pcOffset9;
	int flag = 0;
	int index = 0;
	
	machineCode[16] = '\0';
	machineCode[21] = '\0';
	machineCode[0] = '0';
	machineCode[1] = '0';
	machineCode[2] = '0';
	machineCode[3] = '0';
	machineCode[4] = '0';
	machineCode[5] = '0';
	machineCode[6] = '1';
	
	if( strcmp( operand1, "" ) == 0 )
	{
		printf("Error 4 at Line %d: missing operand!\n", lctr);
		exit(4);
	}
	if( strcmp( operand2, "" ) != 0 || strcmp( operand3, "" ) != 0 || strcmp( operand4, "") != 0 )
	{
		printf("Error 4 at Line %d: unnecessary operand!\n", lctr);
		exit(4);
	}
	
	if( (*operand1 == 'x' || *operand1 == '#'))
	{
		for( i=1; *(operand1+i) != '\0'; i++)
			if( *(operand1+i)>'9' || *(operand1+i)<'0') 
			{
				index = 1;
				break;
			} 
		if( index == 0 )
		{
			printf("Error 4 at Line %d: unexpected operand!\n", lctr);
			exit(4);
		}
	}
	
	for( i = 0; i < labelctr; i++)
	{
		if( strcmp( operand1, symbolTable[i].label ) ==0 )
		{
			flag = 1;
			pcOffset9 = ( symbolTable[i].address - initaddr )/2 - instructionctr;
			if( pcOffset9 < -256 || pcOffset9 > 255) 
			{
				printf("Error 4 at Line %d: too far!\n", lctr);
				exit(4);
			}
			if( pcOffset9<0 ) pcOffset9 = pcOffset9 + 512;	
			break;	
		}
	}
	
	if( flag == 0)
	{
		printf("Error 1 at Line %d: undefined label!\n", lctr);
		exit(1);
	}
	
	for( i=8 ; i >= 0 ; i--)
	{
		machineCode[ 7+i ] = 48 +pcOffset9%2;
		pcOffset9 = pcOffset9/2;
	}
	
	machineCode[17] = binaryToHex( machineCode );
	machineCode[18] = binaryToHex( machineCode+4 );
	machineCode[19] = binaryToHex( machineCode+8 );
	machineCode[20] = binaryToHex( machineCode+12 );
	fprintf( outfile, "0x%s\n", machineCode+17 );		
}

functionBRnz(char *operand1, char *operand2, char *operand3, char* operand4, int lctr, int instructionctr, int labelctr, int initaddr )
{
	char machineCode[22];
	int temp, i, pcOffset9;
	int flag = 0;
	int index = 0;
	
	machineCode[16] = '\0';
	machineCode[21] = '\0';
	machineCode[0] = '0';
	machineCode[1] = '0';
	machineCode[2] = '0';
	machineCode[3] = '0';
	machineCode[4] = '1';
	machineCode[5] = '1';
	machineCode[6] = '0';
	
	if( strcmp( operand1, "" ) == 0 )
	{
		printf("Error 4 at Line %d: missing operand!\n", lctr);
		exit(4);
	}
	if( strcmp( operand2, "" ) != 0 || strcmp( operand3, "" ) != 0 || strcmp( operand4, "") != 0 )
	{
		printf("Error 4 at Line %d: unnecessary operand!\n", lctr);
		exit(4);
	}	
	
	if( (*operand1 == 'x' || *operand1 == '#'))
	{
		for( i=1; *(operand1+i) != '\0'; i++)
			if( *(operand1+i)>'9' || *(operand1+i)<'0') 
			{
				index = 1;
				break;
			} 
		if( index == 0 )
		{
			printf("Error 4 at Line %d: unexpected operand!\n", lctr);
			exit(4);
		}
	}
	
	for( i = 0; i < labelctr; i++)
	{
		if( strcmp( operand1, symbolTable[i].label ) ==0 )
		{
			flag = 1;
			pcOffset9 = ( symbolTable[i].address - initaddr )/2 - instructionctr;
			if( pcOffset9 < -256 || pcOffset9 > 255) 
			{
				printf("Error 4 at Line %d: too far!\n", lctr);
				exit(4);
			}
			if( pcOffset9<0 ) pcOffset9 = pcOffset9 + 512;	
			break;	
		}
	}
	
	if( flag == 0)
	{
		printf("Error 1 at Line %d: undefined label!\n", lctr);
		exit(1);
	}
	
	for( i=8 ; i >= 0 ; i--)
	{
		machineCode[ 7+i ] = 48 +pcOffset9%2;
		pcOffset9 = pcOffset9/2;
	}
	
	machineCode[17] = binaryToHex( machineCode );
	machineCode[18] = binaryToHex( machineCode+4 );
	machineCode[19] = binaryToHex( machineCode+8 );
	machineCode[20] = binaryToHex( machineCode+12 );
	fprintf( outfile, "0x%s\n", machineCode+17 );		
}

functionBRnp(char *operand1, char *operand2, char *operand3, char* operand4, int lctr, int instructionctr, int labelctr, int initaddr )
{
	char machineCode[22];
	int temp, i, pcOffset9;
	int flag = 0;
	int index = 0;
	
	machineCode[16] = '\0';
	machineCode[21] = '\0';
	machineCode[0] = '0';
	machineCode[1] = '0';
	machineCode[2] = '0';
	machineCode[3] = '0';
	machineCode[4] = '1';
	machineCode[5] = '0';
	machineCode[6] = '1';
	
	if( strcmp( operand1, "" ) == 0 )
	{
		printf("Error 4 at Line %d: missing operand!\n", lctr);
		exit(4);
	}
	if( strcmp( operand2, "" ) != 0 || strcmp( operand3, "" ) != 0 || strcmp( operand4, "") != 0 )
	{
		printf("Error 4 at Line %d: unnecessary operand!\n", lctr);
		exit(4);
	}	
	
	if( (*operand1 == 'x' || *operand1 == '#'))
	{
		for( i=1; *(operand1+i) != '\0'; i++)
			if( *(operand1+i)>'9' || *(operand1+i)<'0') 
			{
				index = 1;
				break;
			} 
		if( index == 0 )
		{
			printf("Error 4 at Line %d: unexpected operand!\n", lctr);
			exit(4);
		}
	}
	
	for( i = 0; i < labelctr; i++)
	{
		if( strcmp( operand1, symbolTable[i].label ) ==0 )
		{
			flag = 1;
			pcOffset9 = ( symbolTable[i].address - initaddr )/2 - instructionctr;
			if( pcOffset9 < -256 || pcOffset9 > 255) 
			{
				printf("Error 4 at Line %d: too far!\n", lctr);
				exit(4);
			}
			if( pcOffset9<0 ) pcOffset9 = pcOffset9 + 512;	
			break;	
		}
	}
	
	if( flag == 0)
	{
		printf("Error 1 at Line %d: undefined label!\n", lctr);
		exit(1);
	}
	
	for( i=8 ; i >= 0 ; i--)
	{
		machineCode[ 7+i ] = 48 +pcOffset9%2;
		pcOffset9 = pcOffset9/2;
	}
	
	machineCode[17] = binaryToHex( machineCode );
	machineCode[18] = binaryToHex( machineCode+4 );
	machineCode[19] = binaryToHex( machineCode+8 );
	machineCode[20] = binaryToHex( machineCode+12 );
	fprintf( outfile, "0x%s\n", machineCode+17 );		
}

functionBRzp(char *operand1, char *operand2, char *operand3, char* operand4, int lctr, int instructionctr, int labelctr, int initaddr )
{
	char machineCode[22];
	int temp, i, pcOffset9;
	int flag = 0;
	int index = 0;
	
	machineCode[16] = '\0';
	machineCode[21] = '\0';
	machineCode[0] = '0';
	machineCode[1] = '0';
	machineCode[2] = '0';
	machineCode[3] = '0';
	machineCode[4] = '0';
	machineCode[5] = '1';
	machineCode[6] = '1';
	
	if( strcmp( operand1, "" ) == 0 )
	{
		printf("Error 4 at Line %d: missing operand!\n", lctr);
		exit(4);
	}
	if( strcmp( operand2, "" ) != 0 || strcmp( operand3, "" ) != 0 || strcmp( operand4, "") != 0 )
	{
		printf("Error 4 at Line %d: unnecessary operand!\n", lctr);
		exit(4);
	}	
	
	if( (*operand1 == 'x' || *operand1 == '#'))
	{
		for( i=1; *(operand1+i) != '\0'; i++)
			if( *(operand1+i)>'9' || *(operand1+i)<'0') 
			{
				index = 1;
				break;
			} 
		if( index == 0 )
		{
			printf("Error 4 at Line %d: unexpected operand!\n", lctr);
			exit(4);
		}
	}
	
	for( i = 0; i < labelctr; i++)
	{
		if( strcmp( operand1, symbolTable[i].label ) ==0 )
		{
			flag = 1;
			pcOffset9 = ( symbolTable[i].address - initaddr )/2 - instructionctr;
			if( pcOffset9 < -256 || pcOffset9 > 255) 
			{
				printf("Error 4 at Line %d: too far!\n", lctr);
				exit(4);
			}
			if( pcOffset9<0 ) pcOffset9 = pcOffset9 + 512;	
			break;	
		}
	}
	
	if( flag == 0)
	{
		printf("Error 1 at Line %d: undefined label!\n", lctr);
		exit(1);
	}
	
	for( i=8 ; i >= 0 ; i--)
	{
		machineCode[ 7+i ] = 48 +pcOffset9%2;
		pcOffset9 = pcOffset9/2;
	}
	
	machineCode[17] = binaryToHex( machineCode );
	machineCode[18] = binaryToHex( machineCode+4 );
	machineCode[19] = binaryToHex( machineCode+8 );
	machineCode[20] = binaryToHex( machineCode+12 );
	fprintf( outfile, "0x%s\n", machineCode+17 );		
}

functionBRnzp(char *operand1, char *operand2, char *operand3, char* operand4, int lctr, int instructionctr, int labelctr, int initaddr )
{
	char machineCode[22];
	int temp, i, pcOffset9;
	int flag = 0;
	int index = 0;
	
	machineCode[16] = '\0';
	machineCode[21] = '\0';
	machineCode[0] = '0';
	machineCode[1] = '0';
	machineCode[2] = '0';
	machineCode[3] = '0';
	machineCode[4] = '1';
	machineCode[5] = '1';
	machineCode[6] = '1';
	
	if( strcmp( operand1, "" ) == 0 )
	{
		printf("Error 4 at Line %d: missing operand!\n", lctr);
		exit(4);
	}
	if( strcmp( operand2, "" ) != 0 || strcmp( operand3, "" ) != 0 || strcmp( operand4, "") != 0 )
	{
		printf("Error 4 at Line %d: unnecessary operand!\n", lctr);
		exit(4);
	}	
	
	if( (*operand1 == 'x' || *operand1 == '#'))
	{
		for( i=1; *(operand1+i) != '\0'; i++)
			if( *(operand1+i)>'9' || *(operand1+i)<'0') 
			{
				index = 1;
				break;
			} 
		if( index == 0 )
		{
			printf("Error 4 at Line %d: unexpected operand!\n", lctr);
			exit(4);
		}
	}
	
	for( i = 0; i < labelctr; i++)
	{
		if( strcmp( operand1, symbolTable[i].label ) ==0 )
		{
			flag = 1;
			pcOffset9 = ( symbolTable[i].address - initaddr )/2 - instructionctr;
			if( pcOffset9 < -256 || pcOffset9 > 255) 
			{
				printf("Error 4 at Line %d: too far!\n", lctr);
				exit(4);
			}
			if( pcOffset9<0 ) pcOffset9 = pcOffset9 + 512;	
			break;	
		}
	}
	
	if( flag == 0)
	{
		printf("Error 1 at Line %d: undefined label!\n", lctr);
		exit(1);
	}
	
	for( i=8 ; i >= 0 ; i--)
	{
		machineCode[ 7+i ] = 48 +pcOffset9%2;
		pcOffset9 = pcOffset9/2;
	}
	
	machineCode[17] = binaryToHex( machineCode );
	machineCode[18] = binaryToHex( machineCode+4 );
	machineCode[19] = binaryToHex( machineCode+8 );
	machineCode[20] = binaryToHex( machineCode+12 );
	fprintf( outfile, "0x%s\n", machineCode+17 );		
}

functionJSR( char *operand1, char *operand2, char *operand3, char* operand4, int lctr, int instructionctr, int labelctr, int initaddr )
{
	char machineCode[22];
	int temp, i, pcOffset11;
	int flag = 0;
	int index = 0;
	
	machineCode[16] = '\0';
	machineCode[21] = '\0';
	machineCode[0] = '0';
	machineCode[1] = '1';
	machineCode[2] = '0';
	machineCode[3] = '0';
	machineCode[4] = '1';
	
	if( strcmp( operand1, "" ) == 0 )
	{
		printf("Error 4 at Line %d: missing operand!\n", lctr);
		exit(4);
	}
	if( strcmp( operand2, "" ) != 0 || strcmp( operand3, "" ) != 0 || strcmp( operand4, "") != 0 )
	{
		printf("Error 4 at Line %d: unnecessary operand!\n", lctr);
		exit(4);
	}	
	
	if( (*operand1 == 'x' || *operand1 == '#'))
	{
		for( i=1; *(operand1+i) != '\0'; i++)
			if( *(operand1+i)>'9' || *(operand1+i)<'0') 
			{
				index = 1;
				break;
			} 
		if( index == 0 )
		{
			printf("Error 4 at Line %d: unexpected operand!\n", lctr);
			exit(4);
		}
	}
	
	for( i = 0; i < labelctr; i++)
	{
		if( strcmp( operand1, symbolTable[i].label ) ==0 )
		{
			flag = 1;
			pcOffset11 = ( symbolTable[i].address - initaddr )/2 - instructionctr;
			if( pcOffset11 < -1024 || pcOffset11 > 1023)
			{
				printf("Error 4 at Line %d: too far!\n", lctr);
				exit(4);
			}
			if( pcOffset11 < 0 ) pcOffset11 = pcOffset11 + 2048;	
			break;	
		}
	}
		
	if( flag == 0)
	{
		printf("Error 1 at Line %d: undefined label!\n", lctr);
		exit(1);
	}
	

	
	for( i=10 ; i >= 0 ; i--)
	{
		machineCode[ 5+i ] = 48 +pcOffset11%2;
		pcOffset11 = pcOffset11/2;
	}
	
	machineCode[17] = binaryToHex( machineCode );
	machineCode[18] = binaryToHex( machineCode+4 );
	machineCode[19] = binaryToHex( machineCode+8 );
	machineCode[20] = binaryToHex( machineCode+12 );
	fprintf( outfile, "0x%s\n", machineCode+17 );		
}

functionLDB( char *operand1, char *operand2, char *operand3, char* operand4, int lctr )
{
	char machineCode[22];
	int temp, i;
	
	machineCode[16] = '\0';
	machineCode[21] = '\0';
	machineCode[0] = '0';
	machineCode[1] = '0';
	machineCode[2] = '1';
	machineCode[3] = '0';
		
	if( strcmp( operand1, "" ) == 0 || strcmp( operand2, "" ) == 0 || strcmp( operand3, "" ) == 0)
	{
		printf("Error 4 at Line %d: missing operand!\n", lctr);
		exit(4);
	}
	if( strcmp( operand4, "") != 0 )
	{
		printf("Error 4 at Line %d: unnecessary operand!\n", lctr);
		exit(4);
	}
	if( isRegister( operand1 ) == -1 || isRegister( operand2 ) == -1 )
	{
		printf("Error 4 at Line %d: unexpected operand!\n", lctr);
		exit(4);	
	}
	
	temp = isRegister( operand1 );
	for(i=2; i >= 0 ; i--)
	{
		machineCode[ 4+i ] = 48 + temp%2;
		temp = temp/2;
	}
	
	temp = isRegister( operand2 );
	for(i=2; i >= 0; i--)
	{
		machineCode[ 7+i ] = 48 + temp%2;
		temp = temp/2;
	}
	
	temp = toNum( operand3 );
	if( temp < -32 || temp > 31)
	{
		printf("Error 3 at Line %d: invalid constant!\n", lctr);
		exit(3);
	}
	if( temp >= -32 && temp < 0 ) temp = temp + 64;
	for(i=5; i >= 0 ; i--)
	{
		machineCode[ 10+i ] = 48 + temp%2;
		temp = temp/2;
	}
	machineCode[17] = binaryToHex( machineCode );
	machineCode[18] = binaryToHex( machineCode+4 );
	machineCode[19] = binaryToHex( machineCode+8 );
	machineCode[20] = binaryToHex( machineCode+12 );
	fprintf( outfile, "0x%s\n", machineCode+17 );				
}

functionLDW( char *operand1, char *operand2, char *operand3, char* operand4, int lctr)
{
	char machineCode[22];
	int temp, i;
	
	machineCode[16] = '\0';
	machineCode[21] = '\0';
	machineCode[0] = '0';
	machineCode[1] = '1';
	machineCode[2] = '1';
	machineCode[3] = '0';
		
	if( strcmp( operand1, "" ) == 0 || strcmp( operand2, "" ) == 0 || strcmp( operand3, "" ) == 0)
	{
		printf("Error 4 at Line %d: missing operand!\n", lctr);
		exit(4);
	}
	if( strcmp( operand4, "") != 0 )
	{
		printf("Error 4 at Line %d: unnecessary operand!\n", lctr);
		exit(4);
	}
	if( isRegister( operand1 ) == -1 || isRegister( operand2 ) == -1 )
	{
		printf("Error 4 at Line %d: unexpected operand!\n", lctr);
		exit(4);	
	}
	
	temp = isRegister( operand1 );
	for(i=2; i >= 0 ; i--)
	{
		machineCode[ 4+i ] = 48 + temp%2;
		temp = temp/2;
	}
	
	temp = isRegister( operand2 );
	for(i=2; i >= 0; i--)
	{
		machineCode[ 7+i ] = 48 + temp%2;
		temp = temp/2;
	}
	
	temp = toNum( operand3 );
	if( temp < -32 || temp > 31)
	{
		printf("Error 3 at Line %d: invalid constant!\n", lctr);
		exit(3);
	}
	if( temp >= -32 && temp < 0 ) temp = temp + 64;
	for(i=5; i >= 0 ; i--)
	{
		machineCode[ 10+i ] = 48 + temp%2;
		temp = temp/2;
	}			
	machineCode[17] = binaryToHex( machineCode );
	machineCode[18] = binaryToHex( machineCode+4 );
	machineCode[19] = binaryToHex( machineCode+8 );
	machineCode[20] = binaryToHex( machineCode+12 );
	fprintf( outfile, "0x%s\n", machineCode+17 );	
}

functionLEA( char *operand1, char *operand2, char *operand3, char* operand4, int lctr, int instructionctr, int labelctr, int initaddr )
{
	char machineCode[22];
	int temp, i, pcOffset9;
	int flag = 0;
	int index = 0;
	
	machineCode[16] = '\0';
	machineCode[21] = '\0';
	machineCode[0] = '1';
	machineCode[1] = '1';
	machineCode[2] = '1';
	machineCode[3] = '0';
	
	if( strcmp( operand1, "" ) == 0 || strcmp( operand2, "" ) == 0 )
	{
		printf("Error 4 at Line %d: missing operand!\n", lctr);
		exit(4);
	}
	if( strcmp( operand3, "" ) != 0 || strcmp( operand4, "") != 0 )
	{
		printf("Error 4 at Line %d: unnecessary operand!\n", lctr);
		exit(4);
	}	
	if( isRegister( operand1 ) == -1)
	{
		printf("Error 4 at Line %d: unexpected operand!\n", lctr);
		exit(4);	
	}
	
	temp = isRegister( operand1 );
	for(i=2; i >= 0 ; i--)
	{
		machineCode[ 4+i ] = 48 + temp%2;
		temp = temp/2;
	}
	
	if( (*operand2 == 'x' || *operand2 == '#'))
	{
		for( i=1; *(operand2+i) != '\0'; i++)
			if( *(operand2+i)>'9' || *(operand2+i)<'0') 
			{
				index = 1;
				break;
			} 
		if( index == 0 )
		{
			printf("Error 4 at Line %d: unexpected operand!\n", lctr);
			exit(4);
		}
	
	}
	
	
	for( i = 0; i < labelctr; i++)
	{
		if( strcmp( operand2, symbolTable[i].label ) ==0 )
		{
			flag = 1;
			pcOffset9 = ( symbolTable[i].address - initaddr )/2 - instructionctr;
			if( pcOffset9 < -256 || pcOffset9 > 255)
		 	{
			 	printf("Error 4 at Line %d: too far!\n", lctr);
			 	exit(4);
		 	}
			if( pcOffset9<0 ) pcOffset9 = pcOffset9 + 512;	
			break;	
		}
	}
 	
	if( flag == 0)
	{
		printf("Error 1 at Line %d: undefined label!\n", lctr);
		exit(1);
	}
	
	for( i=8 ; i >= 0 ; i--)
	{
		machineCode[ 7+i ] = 48 +pcOffset9%2;
		pcOffset9 = pcOffset9/2;
	}
	
	machineCode[17] = binaryToHex( machineCode );
	machineCode[18] = binaryToHex( machineCode+4 );
	machineCode[19] = binaryToHex( machineCode+8 );
	machineCode[20] = binaryToHex( machineCode+12 );
	fprintf( outfile, "0x%s\n", machineCode+17 );		
}

functionSTB( char *operand1, char *operand2, char *operand3, char* operand4, int lctr )
{
	char machineCode[22];
	int temp, i;
	
	machineCode[16] = '\0';
	machineCode[21] = '\0';
	machineCode[0] = '0';
	machineCode[1] = '0';
	machineCode[2] = '1';
	machineCode[3] = '1';
		
	if( strcmp( operand1, "" ) == 0 || strcmp( operand2, "" ) == 0 || strcmp( operand3, "" ) == 0)
	{
		printf("Error 4 at Line %d: missing operand!\n", lctr);
		exit(4);
	}
	if( strcmp( operand4, "") != 0 )
	{
		printf("Error 4 at Line %d: unnecessary operand!\n", lctr);
		exit(4);
	}
	if( isRegister( operand1 ) == -1 || isRegister( operand2 ) == -1 )
	{
		printf("Error 4 at Line %d: unexpected operand!\n", lctr);
		exit(4);	
	}
	
	temp = isRegister( operand1 );
	for(i=2; i >= 0 ; i--)
	{
		machineCode[ 4+i ] = 48 + temp%2;
		temp = temp/2;
	}
	
	temp = isRegister( operand2 );
	for(i=2; i >= 0; i--)
	{
		machineCode[ 7+i ] = 48 + temp%2;
		temp = temp/2;
	}
	
	temp = toNum( operand3 );
	if( temp < -32 || temp > 31)
	{
		printf("Error 3 at Line %d: invalid constant!\n", lctr);
		exit(3);
	}
	if( temp >= -31 && temp < 0 ) temp = temp + 64;
	for(i=5; i >= 0 ; i--)
	{
		machineCode[ 10+i ] = 48 + temp%2;
		temp = temp/2;
	}
	machineCode[17] = binaryToHex( machineCode );
	machineCode[18] = binaryToHex( machineCode+4 );
	machineCode[19] = binaryToHex( machineCode+8 );
	machineCode[20] = binaryToHex( machineCode+12 );
	fprintf( outfile, "0x%s\n", machineCode+17 );	
}

functionSTW( char *operand1, char *operand2, char *operand3, char* operand4, int lctr )
{
	char machineCode[22];
	int temp, i;
	
	machineCode[16] = '\0';
	machineCode[21] = '\0';
	machineCode[0] = '0';
	machineCode[1] = '1';
	machineCode[2] = '1';
	machineCode[3] = '1';
		
	if( strcmp( operand1, "" ) == 0 || strcmp( operand2, "" ) == 0 || strcmp( operand3, "" ) == 0)
	{
		printf("Error 4 at Line %d: missing operand!\n", lctr);
		exit(4);
	}
	if( strcmp( operand4, "") != 0 )
	{
		printf("Error 4 at Line %d: unnecessary operand!\n", lctr);
		exit(4);
	}
	if( isRegister( operand1 ) == -1 || isRegister( operand2 ) == -1 )
	{
		printf("Error 4 at Line %d: unexpected operand!\n", lctr);
		exit(4);	
	}
	
	temp = isRegister( operand1 );
	for(i=2; i >= 0 ; i--)
	{
		machineCode[ 4+i ] = 48 + temp%2;
		temp = temp/2;
	}
	
	temp = isRegister( operand2 );
	for(i=2; i >= 0; i--)
	{
		machineCode[ 7+i ] = 48 + temp%2;
		temp = temp/2;
	}
	
	temp = toNum( operand3 );
	if( temp < -32 || temp > 31)
	{
		printf("Error 3 at Line %d: invalid constant!\n", lctr);
		exit(3);
	}
	if( temp >= -31 && temp < 0 ) temp = temp + 64;
	for(i=5; i >= 0 ; i--)
	{
		machineCode[ 10+i ] = 48 + temp%2;
		temp = temp/2;
	}
	machineCode[17] = binaryToHex( machineCode );
	machineCode[18] = binaryToHex( machineCode+4 );
	machineCode[19] = binaryToHex( machineCode+8 );
	machineCode[20] = binaryToHex( machineCode+12 );
	fprintf( outfile, "0x%s\n", machineCode+17 );	
}

int functionORIG( char *operand1, char *operand2, char *operand3, char* operand4, int lctr )
{
	char machineCode[22];
	int temp, i;
	
	machineCode[16] = '\0';
	machineCode[21] = '\0';
	
	if( strcmp( operand1, "" ) == 0 )
	{
		printf("Error 4 at Line %d: missing operand!\n", lctr);
		exit(4);
	}
	if( strcmp( operand2, "" ) != 0 || strcmp( operand3, "" ) != 0 || strcmp( operand4, "") != 0 )
	{
		printf("Error 4 at Line %d: unnecessary operand!\n", lctr);
		exit(4);
	}
	
	temp = toNum( operand1 );
	
	if( temp < 0 || temp > 65535 )
	{
		printf("Error 3 at Line %d: invalid constant!\n", lctr);
		exit(3);
	}
	
	for( i = 15; i >= 0 ; i-- )
	{
		machineCode[i] = 48 + temp%2;
		temp = temp/2;
	}
	
	machineCode[17] = binaryToHex( machineCode );
	machineCode[18] = binaryToHex( machineCode+4 );
	machineCode[19] = binaryToHex( machineCode+8 );
	machineCode[20] = binaryToHex( machineCode+12 );
	fprintf( outfile, "0x%s\n", machineCode+17 );
	temp = toNum(operand1);
	return( temp );	
}
functionFILL( char *operand1, char *operand2, char *operand3, char* operand4, int lctr )
{
	char machineCode[22];
	int temp, i;
	
	machineCode[16] = '\0';
	machineCode[21] = '\0';
	
	if( strcmp( operand1, "" ) == 0 )
	{
		printf("Error 4 at Line %d: missing operand!\n", lctr);
		exit(4);
	}
	if( strcmp( operand2, "" ) != 0 || strcmp( operand3, "" ) != 0 || strcmp( operand4, "") != 0 )
	{
		printf("Error 4 at Line %d: unnecessary operand!\n", lctr);
		exit(4);
	}
	
	temp = toNum( operand1 );
	if( temp < -32768 || temp > 32767)
	{
		printf("Error 3 at Line %d: invalid constant!\n", lctr);
		exit(3);
	}

	if( temp<0 ) temp = temp + 65536;
	
	for( i = 15; i >= 0 ; i-- )
	{
		machineCode[i] = 48 + temp%2;
		temp = temp/2;
	}
	
	machineCode[17] = binaryToHex( machineCode );
	machineCode[18] = binaryToHex( machineCode+4 );
	machineCode[19] = binaryToHex( machineCode+8 );
	machineCode[20] = binaryToHex( machineCode+12 );
	fprintf( outfile, "0x%s\n", machineCode+17 );	
}