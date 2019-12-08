/*
    Name 1: Bilal Quraishi
    UTEID 1: buq57
*/

/***************************************************************/
/*                                                             */
/*   LC-3b Simulator                                           */
/*                                                             */
/*   EE 460N - Lab 5                                           */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***************************************************************/
/*                                                             */
/* Files:  ucode        Microprogram file                      */
/*         pagetable    page table in LC-3b machine language   */
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
void evaluate();
void math(int iR);
void regFile(int iR);
void evalStatusAndVector(int iR);
void other(int iR);
int SEXT(int immediateLength, int num);
int isReadyInstruction(int n);
void latchRegs();
void latchNext(int state);
int decipherState(int j5, int j4, int j3, int j2, int j1, int j0);
void setCC(char cc);
int mask1(int bin);
int mask2(int bin);
int mask3(int bin);
int mask4(int bin);
int mask5(int bin);
int mask6(int bin);
int mask7(int bin);
int mask8(int bin);
int mask9(int bin);
int mask11(int bin);

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
    CHECKT,
    MARMUX2,
    IE,
    EXCSEL1, EXCSEL0,
    R6DR,
    PSRMUX1, PSRMUX0,
    GATE_PSR,
    PSRUPDATE,
    LD_PSR,
    GATE_R6,
    REGSELECT1, REGSELECT0,
    LD_TEMP,
    LD_SSP,
    SSPMUX1, SSPMUX0,
    GATE_SSP,
    VECTORPC,
    LD_VOUT,
    JSELECT3, JSELECT2, JSELECT1, JSELECT0,
    RETURN,
    LD_VA,
    LD_JTEMP,
    LD_M,
    GATE_MAR,
    GATE_ADDR,
    GATE_VA,
    TRANSLATE,
    MDRSEL,
    INPTE,
    INTRAP,
    SETM,
    LD_MDRB,
    MEMMUX,
/* MODIFY: you have to add all your new control signals */
    CONTROL_STORE_BITS
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
/* MODIFY: you can add more Get functions for your new control signals */
int GetEXCSEL(int *x)         { return((x[EXCSEL1] << 1) + x[EXCSEL0]); }
int GetPSRMUX(int *x)         { return((x[PSRMUX1] << 1) + x[PSRMUX0]); }
int GetREGSELECT(int *x)      { return((x[REGSELECT1] << 1) + x[REGSELECT0]); }
int GetSSPMUX(int *x)         { return((x[SSPMUX1] << 1) + x[SSPMUX0]); }
int GetGATE_SSP(int *x)      { return(x[GATE_SSP]); }
int GetGATE_PSR(int *x)      { return(x[GATE_PSR]); }
int GetGATE_R6(int *x)      { return(x[GATE_R6]); }

int GetRETURN(int *x)      { return(x[RETURN]); }
int GetLD_VA(int *x)      { return(x[LD_VA]); }
int GetLD_JTEMP(int *x)      { return(x[LD_JTEMP]); }
int GetLD_M(int *x)      { return(x[LD_M]); }
int GetGATE_MAR(int *x)      { return(x[GATE_MAR]); }
int GetGATE_ADDR(int *x)      { return(x[GATE_ADDR]); }
int GetGATE_VA(int *x)      { return(x[GATE_VA]); }
int GetTRANSLATE(int *x)      { return(x[TRANSLATE]); }
int GetMDRSEL(int *x)      { return(x[MDRSEL]); }
int GetInPTE(int *x)      { return(x[INPTE]); }
int GetInTRAP(int *x)      { return(x[INTRAP]); }
int GetJSELECT(int *x)     {
					     return((x[JSELECT3] << 3) +
						 	(x[JSELECT2] << 2) +
				      	 	(x[JSELECT1] << 1) +
						 	(x[JSELECT0]));
				    }

/***************************************************************/
/* The control store rom.                                      */
/***************************************************************/
int CONTROL_STORE[CONTROL_STORE_ROWS][CONTROL_STORE_BITS];

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A
   There are two write enable signals, one for each byte. WE0 is used for
   the least significant byte of a word. WE1 is used for the most significant
   byte of a word. */

#define WORDS_IN_MEM    0x2000 /* 32 frames */
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
  /* The ready bit is also latched as you dont want the memory system to assert it
     at a bad point in the cycle*/

int REGS[LC_3b_REGS]; /* register file. */

int MICROINSTRUCTION[CONTROL_STORE_BITS]; /* The microintruction */

int STATE_NUMBER; /* Current State Number - Provided for debugging */

/* For lab 4 */
int INTV; /* Interrupt vector register */
int EXCV; /* Exception vector register */
int VTBR; /* Vector table base register */
int SSP; /* Initial value of system stack pointer */
int TEMP; /* Temporary R6 storage */
int PSR; /* Program status register */
int VOUT; /* Vector out */
/* MODIFY: you should add here any other registers you need to implement interrupts and exceptions */

/* For lab 5 */
int PTBR; /* This is initialized when we load the page table */
int VA;   /* Temporary VA register */
int M;
int JTEMP;
int MDRB;
/* MODIFY: you should add here any other registers you need to implement virtual memory */

} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/* For lab 5 */
#define PAGE_NUM_BITS 9
#define PTE_PFN_MASK 0x3E00
#define PTE_VALID_MASK 0x0004
#define PAGE_OFFSET_MASK 0x1FF

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
    printf("MDRB         : 0x%0.4x\n", CURRENT_LATCHES.MDRB);
    printf("MAR          : 0x%0.4x\n", CURRENT_LATCHES.MAR);
    printf("VA           : 0x%0.4x\n", CURRENT_LATCHES.VA);
    printf("JTEMP        : 0x%0.4x\n", CURRENT_LATCHES.JTEMP);
    printf("M            : 0x%0.4x\n", CURRENT_LATCHES.M);
    printf("VOUT         : 0x%0.4x\n", CURRENT_LATCHES.VOUT);

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
void load_program(char *program_filename, int is_virtual_base) {
    FILE * prog;
    int ii, word, program_base, pte, virtual_pc;

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

    if (is_virtual_base) {
      if (CURRENT_LATCHES.PTBR == 0) {
	printf("Error: Page table base not loaded %s\n", program_filename);
	exit(-1);
      }

      /* convert virtual_base to physical_base */
      virtual_pc = program_base << 1;
      pte = (MEMORY[(CURRENT_LATCHES.PTBR + (((program_base << 1) >> PAGE_NUM_BITS) << 1)) >> 1][1] << 8) |
	     MEMORY[(CURRENT_LATCHES.PTBR + (((program_base << 1) >> PAGE_NUM_BITS) << 1)) >> 1][0];

      printf("virtual base of program: %04x\npte: %04x\n", program_base << 1, pte);
		if ((pte & PTE_VALID_MASK) == PTE_VALID_MASK) {
	      program_base = (pte & PTE_PFN_MASK) | ((program_base << 1) & PAGE_OFFSET_MASK);
   	   printf("physical base of program: %x\n\n", program_base);
	      program_base = program_base >> 1;
		} else {
   	   printf("attempting to load a program into an invalid (non-resident) page\n\n");
			exit(-1);
		}
    }
    else {
      /* is page table */
     CURRENT_LATCHES.PTBR = program_base << 1;
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
	MEMORY[program_base + ii][1] = (word >> 8) & 0x00FF;;
	ii++;
    }

    if (CURRENT_LATCHES.PC == 0 && is_virtual_base)
      CURRENT_LATCHES.PC = virtual_pc;

    printf("Read %d words from program into memory.\n\n", ii);
}

/***************************************************************/
/*                                                             */
/* Procedure : initialize                                      */
/*                                                             */
/* Purpose   : Load microprogram and machine language program  */
/*             and set up initial state of the machine         */
/*                                                             */
/***************************************************************/
void initialize(char *argv[], int num_prog_files) {
    int i;
    init_control_store(argv[1]);

    init_memory();
    load_program(argv[2],0);
    for ( i = 0; i < num_prog_files; i++ ) {
	load_program(argv[i + 3],1);
    }
    CURRENT_LATCHES.Z = 1;
    CURRENT_LATCHES.STATE_NUMBER = INITIAL_STATE_NUMBER;
    memcpy(CURRENT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[INITIAL_STATE_NUMBER], sizeof(int)*CONTROL_STORE_BITS);
    CURRENT_LATCHES.SSP = 0x3000; /* Initial value of system stack pointer */
    CURRENT_LATCHES.PSR |= 0x8002;
    CURRENT_LATCHES.MAR = 0x3000;
    CURRENT_LATCHES.VTBR = 0x0200;

/* MODIFY: you can add more initialization code HERE */

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
    if (argc < 4) {
	printf("Error: usage: %s <micro_code_file> <page table file> <program_file_1> <program_file_2> ...\n",
	       argv[0]);
	exit(1);
    }

    printf("LC-3b Simulator\n\n");

    initialize(argv, argc - 3);

    if ( (dumpsim_file = fopen( "dumpsim", "w" )) == NULL ) {
	printf("Error: Can't open dumpsim file\n");
	exit(-1);
    }

    while (1)
	get_command(dumpsim_file);

}

/***************************************************************/
/* Do not modify the above code, except for the places indicated
   with a "MODIFY:" comment.
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


int nextInstruction = 0;
int SR1Out = 0;
int SR2Out = 0;
int SR2MuxOut = 0;
int SSPMuxOut = 0;
int REGSELECTMuxOut = 0;
int PSRUPDATEMuxOut = 0;
int PSRMuxOut = 0;
int R6DRMuxOut = 0;
int EXCSELMuxOut = 0;
int DRMuxOut = 0;
int IEMuxOut = 0;
int VECTORPCMuxOut = 0;
int MARMUX2Out = 0;
int SHFOut = 0;
int ALUOut = 0;
int ADDR1MuxOut = 0;
int ADDR2MuxOut = 0;
int PCOut = 0;
int PlusOut = 0;
int flag = 0;
int vPlusOut = 0;
int INTVOrOut = 0;
int PCMuxOut = 0;
int MARMuxOut = 0;
int MDRLogicOut = 0;
int MEMOut = 0;
int memCycle = 1;

//Lab 5 New Outs
int MDRMask = 0;
int MDRSelOut = 0;
int TranslateOut = 0;
int GateADDR = 0;
int MOut = 0xF;
int Select_53 = 0;
int JMuxOut = 0;
int SELEX = 0;

void eval_micro_sequencer()
{
      /* Exception Determination Logic */
      int U = 0; /*  Unaligned  */
      int P = 0; /*  Protection */
      int K = 0; /*  Unknown    */
	int F = 0; /*  Pagefault  */

      int dS = CURRENT_LATCHES.MICROINSTRUCTION[DATA_SIZE];
      int cMAR = CURRENT_LATCHES.MAR;
      int restriction = 0x2FFF;
      int iR_15_12 = mask4(CURRENT_LATCHES.IR >> 12);
      int pStR = CURRENT_LATCHES.PSR;
      int psr15 = mask1(CURRENT_LATCHES.PSR >> 15);

	int cMDR = CURRENT_LATCHES.MDR;
	int inPTE = CURRENT_LATCHES.MICROINSTRUCTION[INPTE];
	int inTRA = CURRENT_LATCHES.MICROINSTRUCTION[INTRAP];

	int JMuxSel = GetJSELECT(CURRENT_LATCHES.MICROINSTRUCTION);
	switch (JMuxSel)
	{

		case 0: JMuxOut = 54;
			  break;
		case 1: JMuxOut = 24;
			  break;
		case 2: JMuxOut = 23;
			  break;
		case 3: JMuxOut = 25;
			  break;
		case 4: JMuxOut = 29;
			  break;
		case 5: JMuxOut = 28;
			  break;
		case 6: JMuxOut = 49;
			  break;
		case 7: JMuxOut = 45;
			  break;
		case 8: JMuxOut = 41;
			  break;
		case 9: JMuxOut = 53;
			  break;
		case 10: JMuxOut = 57;
			  break;
		default: 0;
	}
	if (CURRENT_LATCHES.MICROINSTRUCTION[LD_JTEMP] == 1) NEXT_LATCHES.JTEMP = JMuxOut;
	else NEXT_LATCHES.JTEMP = CURRENT_LATCHES.JTEMP;

      U = dS & mask1(cMAR) & psr15;

      if (((iR_15_12 == 0xA) || (iR_15_12 == 0xB)) && psr15) K = 1;
      else K = 0;

	if (K == 1) SELEX = 1;
	if (U == 1) SELEX = 0;

	U |= K; 		//Combined control

	if ((inPTE == 1) && (mask1(cMDR >> 3) == 0) && (psr15 == 1)) P = 1;
	else P = 0;

	if ((inPTE == 1) && (mask1(cMDR >> 2) == 0)) F = 1;
	else F = 0;

	F = (F & (~(F & P)));

      int S = U | P | F;

      int IRDMuxOut = (S == 0) ? CURRENT_LATCHES.MICROINSTRUCTION[IRD] : 0;
      INTVOrOut = (CURRENT_LATCHES.INTV != 0) ? 1 : 0;

      /* For interrupts */
      int checkTMuxOut = (CURRENT_LATCHES.MICROINSTRUCTION[CHECKT] == 0) ? 0 : INTVOrOut;

	if (IRDMuxOut == 1)
	{
		nextInstruction = mask4(CURRENT_LATCHES.IR >> 12);
		latchNext(nextInstruction);
	}
	else
	{
		int c0 = CURRENT_LATCHES.MICROINSTRUCTION[COND0];
		int c1 = CURRENT_LATCHES.MICROINSTRUCTION[COND1];
		int bE = CURRENT_LATCHES.BEN;
		int rB = CURRENT_LATCHES.READY;
		int aM = mask1(CURRENT_LATCHES.IR >> 11);
		int j5 = CURRENT_LATCHES.MICROINSTRUCTION[J5];
            j5 = j5 | U | P | F;
		int j4 = CURRENT_LATCHES.MICROINSTRUCTION[J4];
            j4 = ((U & ~P) | (j4 & ~P) | F);
		int j3 = CURRENT_LATCHES.MICROINSTRUCTION[J3];
            j3 = U | P | (~F & j3);
		int j2 = (S == 0) ? CURRENT_LATCHES.MICROINSTRUCTION[J2] : 0;
            j2 = j2 | checkTMuxOut;
		int j1 = (S == 0) ? CURRENT_LATCHES.MICROINSTRUCTION[J1] : 0;
		int j0 = (S == 0) ? CURRENT_LATCHES.MICROINSTRUCTION[J0] : 0;
		int branch = ((~c0) & c1 & bE);
		int ready = (c0 & (~c1) & rB);
		int addressMode = (c0 & c1 & aM);
		j2 = j2 | branch;
		j1 = j1 | ready;
		j0 = j0 | addressMode;
		if (GetRETURN(CURRENT_LATCHES.MICROINSTRUCTION) == 1)
		{
			j5 = mask1(CURRENT_LATCHES.JTEMP >> 5);
			j4 = mask1(CURRENT_LATCHES.JTEMP >> 4);
			j3 = mask1(CURRENT_LATCHES.JTEMP >> 3);
			j2 = mask1(CURRENT_LATCHES.JTEMP >> 2);
			j1 = mask1(CURRENT_LATCHES.JTEMP >> 1);
			j0 = mask1(CURRENT_LATCHES.JTEMP);
		}
		nextInstruction = decipherState(j5, j4, j3, j2, j1, j0);
		latchNext(nextInstruction);
	}
      int iR = CURRENT_LATCHES.IR;
      int n = CURRENT_LATCHES.N;
      int z = CURRENT_LATCHES.Z;
      int p = CURRENT_LATCHES.P;
      NEXT_LATCHES.BEN = ((mask1(iR >> 11) & n) | (mask1(iR >> 10) & z) | (mask1(iR >> 9) & p));
}


void cycle_memory()
{
      if (CURRENT_LATCHES.MICROINSTRUCTION[MIO_EN] == 1)
      {
	     if (memCycle == MEM_CYCLES - 1)
	     {
                 NEXT_LATCHES.READY = 1;
                 memCycle++;
	     }
           else if (memCycle == MEM_CYCLES)
           {
                 NEXT_LATCHES.READY = 0;
                 memCycle = 1;
          }
		else
		{
			NEXT_LATCHES.READY = 0;
			memCycle++;
		}
	}
      if (CURRENT_LATCHES.READY == 1)
      {
            if (CURRENT_LATCHES.MICROINSTRUCTION[R_W] == 0)
            {
                  MEMOut = Low16bits(MEMORY[CURRENT_LATCHES.MAR/2][0]);
                  MEMOut |= Low16bits(MEMORY[CURRENT_LATCHES.MAR/2][1] << 8);
            }
            else
            {
                  if (CURRENT_LATCHES.MICROINSTRUCTION[DATA_SIZE] == 0)
                  {
                        if (mask1(CURRENT_LATCHES.MAR) == 0)
                        {
                              MEMORY[CURRENT_LATCHES.MAR/2][0] = mask8(CURRENT_LATCHES.MDR);
                        }
                        else MEMORY[CURRENT_LATCHES.MAR/2][1] = mask8(CURRENT_LATCHES.MDR >> 8);
                  }
                  else
                  {
				if (CURRENT_LATCHES.MICROINSTRUCTION[MEMMUX] == 1)
				{
					MEMORY[CURRENT_LATCHES.MAR/2][0] = mask8(CURRENT_LATCHES.MDRB);
	                        MEMORY[CURRENT_LATCHES.MAR/2][1] = mask8(CURRENT_LATCHES.MDRB >> 8);
				}
				else
				{
					MEMORY[CURRENT_LATCHES.MAR/2][0] = mask8(CURRENT_LATCHES.MDR);
	                        MEMORY[CURRENT_LATCHES.MAR/2][1] = mask8(CURRENT_LATCHES.MDR >> 8);
				}
                  }
            }
      }
  /*
   * This function emulates memory and the WE logic.
   * Keep track of which cycle of MEMEN we are dealing with.
   * If fourth, we need to latch Ready bit at the end of
   * cycle to prepare microsequencer for the fifth cycle.
   */
}



void eval_bus_drivers()
{
	regFile(CURRENT_LATCHES.IR);
      evalStatusAndVector(CURRENT_LATCHES.IR);
	math(CURRENT_LATCHES.IR);
	other(CURRENT_LATCHES.IR);
  /*
   * Datapath routine emulating operations before driving the bus.
   * Evaluate the input of tristate drivers
   *     Gate_MARMUX,
   *		 Gate_PC,
   *		 Gate_ALU,
   *		 Gate_SHF,
   *		 Gate_MDR.
   */
}
void regFile(int iR)
{
      int SSPMuxSel = GetSSPMUX(CURRENT_LATCHES.MICROINSTRUCTION);
      if (SSPMuxSel == 0) SSPMuxOut = Low16bits(CURRENT_LATCHES.SSP - 2);
      else if (SSPMuxSel == 1) SSPMuxOut = Low16bits(CURRENT_LATCHES.SSP + 2);
      else if (SSPMuxSel == 2) SSPMuxOut = Low16bits(CURRENT_LATCHES.REGS[6]);

	int SR1MUXSelect = CURRENT_LATCHES.MICROINSTRUCTION[SR1MUX];
	if (SR1MUXSelect == 0)
	{
		SR1Out = CURRENT_LATCHES.REGS[mask3(iR >> 9)];
	}
	else if (SR1MUXSelect == 1)
	{
		SR1Out = CURRENT_LATCHES.REGS[mask3(iR >> 6)];
	}
	SR2Out = CURRENT_LATCHES.REGS[mask3(iR)];

      int DRMuxSelect = CURRENT_LATCHES.MICROINSTRUCTION[DRMUX];
      if (DRMuxSelect == 0) DRMuxOut = mask3(CURRENT_LATCHES.IR >> 9);
      else if (DRMuxSelect == 1) DRMuxOut = 7;

      int R6MuxSelect = CURRENT_LATCHES.MICROINSTRUCTION[R6DR];
      if (R6MuxSelect == 0) R6DRMuxOut = 6;
      else if (R6MuxSelect == 1) R6DRMuxOut = DRMuxOut;
}

void evalStatusAndVector(int iR)
{
      int PSRMuxSel = GetPSRMUX(CURRENT_LATCHES.MICROINSTRUCTION);
      if (PSRMuxSel == 0) PSRMuxOut = Low16bits(CURRENT_LATCHES.PSR | 0x8000);
      else if (PSRMuxSel == 1) PSRMuxOut = Low16bits(CURRENT_LATCHES.PSR);
      else if (PSRMuxSel == 2) PSRMuxOut = Low16bits(CURRENT_LATCHES.PSR & 0x7FFF);

      int EXCSel = GetEXCSEL(CURRENT_LATCHES.MICROINSTRUCTION);
      if (EXCSel == 0) EXCSELMuxOut = 0x04 << 1;
      else if (EXCSel == 1)
	{
		if (SELEX == 1)
		{
			EXCSELMuxOut = 0x05 << 1;
		}
		else EXCSELMuxOut = 0x03 << 1;
	}
      else if (EXCSel == 2) EXCSELMuxOut = 0x02 << 1;

      int INTVOut = CURRENT_LATCHES.INTV << 1;

      int IESel = CURRENT_LATCHES.MICROINSTRUCTION[IE];
      if (IESel == 0) IEMuxOut = EXCSELMuxOut;
      else if (IESel == 1) IEMuxOut = INTVOut;

      vPlusOut = CURRENT_LATCHES.VTBR + IEMuxOut;

	GateADDR = (CURRENT_LATCHES.MDR & 0x3E00) + mask9(CURRENT_LATCHES.VA);
}

void math(int iR)
{
	if (mask1(iR >> 5) == 1) SR2MuxOut = Low16bits(SEXT(5, mask5(iR)));
	else if (mask1(iR >> 5) == 0) SR2MuxOut = SR2Out;

	int ALUSelect = GetALUK(CURRENT_LATCHES.MICROINSTRUCTION);
	if (ALUSelect == 0) ALUOut = Low16bits(SR2MuxOut + SR1Out);
	else if (ALUSelect == 1) ALUOut = SR2MuxOut & SR1Out;
	else if (ALUSelect == 2) ALUOut = SR2MuxOut ^ SR1Out;
	else if (ALUSelect == 3) ALUOut = SR1Out;

	if (mask2(iR >> 4) == 0) SHFOut = SR1Out << mask4(iR);
	else if (mask2(iR >> 4) == 1) SHFOut = SR1Out >> mask4(iR);
	else if (mask2(iR >> 4) == 3)
	{
		SHFOut = SR1Out;
		int amountToShift = mask4(iR);
		int signBit = (SR1Out & 0x8000);
          	while (amountToShift > 0)
          	{
            	SHFOut = SHFOut >> 1;
            	SHFOut = SHFOut | signBit;
            	amountToShift--;
          	}
	}
}

void other(int iR)
{
	int ADDR1MuxSelect = GetADDR1MUX(CURRENT_LATCHES.MICROINSTRUCTION);
	if (ADDR1MuxSelect == 0) ADDR1MuxOut = CURRENT_LATCHES.PC;
	else if (ADDR1MuxSelect == 1) ADDR1MuxOut = SR1Out;

	int ADDR2MuxSelect = GetADDR2MUX(CURRENT_LATCHES.MICROINSTRUCTION);
	if (ADDR2MuxSelect == 0) ADDR2MuxOut = 0;
	else if (ADDR2MuxSelect == 1) ADDR2MuxOut = Low16bits(SEXT(6, mask6(iR)));
	else if (ADDR2MuxSelect == 2) ADDR2MuxOut = Low16bits(SEXT(9, mask9(iR)));
	else if (ADDR2MuxSelect == 3) ADDR2MuxOut = Low16bits(SEXT(11, mask11(iR)));

	int LSHFSelect = GetLSHF1(CURRENT_LATCHES.MICROINSTRUCTION);
	if (LSHFSelect == 0) PlusOut = ADDR1MuxOut + ADDR2MuxOut;
	else if (LSHFSelect == 1) PlusOut = ADDR1MuxOut + (ADDR2MuxOut << 1);

	int MARMUXSelect = GetMARMUX(CURRENT_LATCHES.MICROINSTRUCTION);
	if (MARMUXSelect == 0) MARMuxOut = Low16bits(mask8(iR) << 1);
	else if (MARMUXSelect == 1) MARMuxOut = PlusOut;

	if (GetDATA_SIZE(CURRENT_LATCHES.MICROINSTRUCTION) == 0)
	{
		int MARZero = CURRENT_LATCHES.MAR % 2;
		if (MARZero == 0) MDRLogicOut = SEXT(8, mask8(CURRENT_LATCHES.MDR));
		else if (MARZero == 1) MDRLogicOut = SEXT(8, mask8(CURRENT_LATCHES.MDR >> 8));
	}
	else if (GetDATA_SIZE(CURRENT_LATCHES.MICROINSTRUCTION) == 1) MDRLogicOut = CURRENT_LATCHES.MDR;
	PCOut = CURRENT_LATCHES.PC;
}


void drive_bus()
{
	if (GetGATE_MARMUX(CURRENT_LATCHES.MICROINSTRUCTION) == 1) BUS = Low16bits(MARMuxOut);
	else if (GetGATE_PC(CURRENT_LATCHES.MICROINSTRUCTION) == 1) BUS = Low16bits(PCOut);
	else if (GetGATE_ALU(CURRENT_LATCHES.MICROINSTRUCTION) == 1) BUS = Low16bits(ALUOut);
      else if (GetGATE_MDR(CURRENT_LATCHES.MICROINSTRUCTION) == 1) BUS = Low16bits(MDRLogicOut);
	else if (GetGATE_SHF(CURRENT_LATCHES.MICROINSTRUCTION) == 1) BUS = Low16bits(SHFOut);
	else if (GetGATE_SSP(CURRENT_LATCHES.MICROINSTRUCTION) == 1) BUS = Low16bits(CURRENT_LATCHES.SSP);
      else if (GetGATE_PSR(CURRENT_LATCHES.MICROINSTRUCTION) == 1) BUS = Low16bits(PSRMuxOut);
      else if (GetGATE_R6(CURRENT_LATCHES.MICROINSTRUCTION) == 1) BUS = Low16bits(CURRENT_LATCHES.REGS[6]);
	else if (GetGATE_MAR(CURRENT_LATCHES.MICROINSTRUCTION) == 1) BUS = CURRENT_LATCHES.MAR;
	else if (GetGATE_ADDR(CURRENT_LATCHES.MICROINSTRUCTION) == 1) BUS = Low16bits(GateADDR);
	else if (GetGATE_VA(CURRENT_LATCHES.MICROINSTRUCTION) == 1) BUS = Low16bits(CURRENT_LATCHES.VA);
	else BUS = 0;
  /*
   * Datapath routine for driving the bus from one of the 5 possible
   * tristate drivers.
   */

}

void latch_datapath_values()
{
      if (flag == 1) NEXT_LATCHES.INTV = 0;
      if (CURRENT_LATCHES.MICROINSTRUCTION[CHECKT] == 1 && CYCLE_COUNT > 300) flag = 1;
      if (CYCLE_COUNT == 299) NEXT_LATCHES.INTV = 0x01;
	if (CURRENT_LATCHES.MICROINSTRUCTION[LD_VA] == 1)
	{
		NEXT_LATCHES.VA = Low16bits(BUS);
	}
	else NEXT_LATCHES.VA = CURRENT_LATCHES.VA;
      if (CURRENT_LATCHES.MICROINSTRUCTION[LD_VOUT] == 1)
      {
            NEXT_LATCHES.VOUT = vPlusOut;
            memCycle = 1;
      }
      else NEXT_LATCHES.VOUT = CURRENT_LATCHES.VOUT;
      if (CURRENT_LATCHES.MICROINSTRUCTION[LD_SSP] == 1)
      {
            NEXT_LATCHES.SSP = SSPMuxOut;
      }
      else NEXT_LATCHES.SSP = CURRENT_LATCHES.SSP;
      if (CURRENT_LATCHES.MICROINSTRUCTION[LD_TEMP] == 1)
      {
            NEXT_LATCHES.TEMP = CURRENT_LATCHES.REGS[6];
      }
      else NEXT_LATCHES.TEMP = CURRENT_LATCHES.TEMP;
      if (CURRENT_LATCHES.MICROINSTRUCTION[LD_IR] == 1)
	{
		NEXT_LATCHES.IR = Low16bits(BUS);
	}
	else NEXT_LATCHES.IR = CURRENT_LATCHES.IR;

	if (CURRENT_LATCHES.MICROINSTRUCTION[LD_REG] == 1)
	{
            int regSel = GetREGSELECT(CURRENT_LATCHES.MICROINSTRUCTION);
            if (regSel == 0) REGSELECTMuxOut = Low16bits(CURRENT_LATCHES.REGS[6] + 2);
            else if (regSel == 1) REGSELECTMuxOut = Low16bits(BUS);
            else if (regSel == 2) REGSELECTMuxOut = Low16bits(CURRENT_LATCHES.SSP);
            else if (regSel == 3) REGSELECTMuxOut = Low16bits(CURRENT_LATCHES.TEMP);
            NEXT_LATCHES.REGS[R6DRMuxOut] = REGSELECTMuxOut;
	}
	else latchRegs();

        if (CURRENT_LATCHES.MICROINSTRUCTION[LD_PC] == 1)
        {
             int PCMuxSelect = GetPCMUX(CURRENT_LATCHES.MICROINSTRUCTION);
             if (PCMuxSelect == 0) PCMuxOut = Low16bits(CURRENT_LATCHES.PC + 2);
             else if (PCMuxSelect == 1) PCMuxOut = Low16bits(BUS);
             else if (PCMuxSelect == 2) PCMuxOut = Low16bits(PlusOut);
             else if (PCMuxSelect == 3) PCMuxOut = Low16bits(CURRENT_LATCHES.PC - 2);

             int VECTORPCMuxSel = CURRENT_LATCHES.MICROINSTRUCTION[VECTORPC];
             if (VECTORPCMuxSel == 0) VECTORPCMuxOut = PCMuxOut;
             else if (VECTORPCMuxSel == 1)
             {
                   VECTORPCMuxOut = CURRENT_LATCHES.VOUT;
             }

             NEXT_LATCHES.PC = VECTORPCMuxOut;
        }
        else NEXT_LATCHES.PC = CURRENT_LATCHES.PC;

        if (CURRENT_LATCHES.MICROINSTRUCTION[LD_CC] == 1)
        {
                if (Low16bits(mask1((BUS) >> 15)) == 1) setCC('N');
                else if (Low16bits(BUS) == 0) setCC('Z');
                else setCC('P');
        }
        else
        {
              NEXT_LATCHES.N = CURRENT_LATCHES.N;
              NEXT_LATCHES.Z = CURRENT_LATCHES.Z;
              NEXT_LATCHES.P = CURRENT_LATCHES.P;
        }
        if (CURRENT_LATCHES.MICROINSTRUCTION[LD_PSR] == 1)
        {
             int PSRLogicNZP = 0;
             int tempN = 0;
             int tempZ = 0;
             int tempP = 0;
             if (Low16bits(mask1((BUS) >> 15)) == 1)
             {
                   tempN = 1;
                   tempZ = 0;
                   tempP = 0;
             }
             else if (Low16bits(BUS) == 0)
             {
                   tempN = 0;
                   tempZ = 1;
                   tempP = 0;
             }
             else
             {
                   tempN = 0;
                   tempZ = 0;
                   tempP = 1;
             }
             PSRLogicNZP += (tempN << 2);
             PSRLogicNZP += (tempZ << 1);
             PSRLogicNZP += (tempP);
             int PSRLogicAnd = CURRENT_LATCHES.PSR & 0xFFF8;
             int PSRLogicOut = PSRLogicNZP | PSRLogicAnd;
             int PSRUPDATESel = CURRENT_LATCHES.MICROINSTRUCTION[PSRUPDATE];
             if (PSRUPDATESel == 0)
             {
                   PSRUPDATEMuxOut = Low16bits(BUS);
                   NEXT_LATCHES.N = mask1(PSRUPDATEMuxOut >> 2);
                   NEXT_LATCHES.Z = mask1(PSRUPDATEMuxOut >> 1);
                   NEXT_LATCHES.P = mask1(PSRUPDATEMuxOut);
             }
             else if (PSRUPDATESel == 1) PSRUPDATEMuxOut = PSRLogicOut;
             NEXT_LATCHES.PSR = PSRUPDATEMuxOut;
        }
        else NEXT_LATCHES.PSR = CURRENT_LATCHES.PSR;

        if (CURRENT_LATCHES.MICROINSTRUCTION[LD_MAR] == 1)
        {
              int MARMUX2Sel = CURRENT_LATCHES.MICROINSTRUCTION[MARMUX2];
              if (MARMUX2Sel == 0) MARMUX2Out = Low16bits(BUS);
              else if (MARMUX2Sel == 1) MARMUX2Out = Low16bits(CURRENT_LATCHES.SSP);
		  int addTemp = CURRENT_LATCHES.PTBR + mask8((CURRENT_LATCHES.MAR >> 9) << 1);
	  	  if (CURRENT_LATCHES.MICROINSTRUCTION[TRANSLATE] == 1) TranslateOut = addTemp;
	  	  else TranslateOut = MARMUX2Out;
              NEXT_LATCHES.MAR = TranslateOut;
        }
        else NEXT_LATCHES.MAR = CURRENT_LATCHES.MAR;

	  if (CURRENT_LATCHES.MICROINSTRUCTION[LD_M] == 1)
	  {
		  NEXT_LATCHES.M = CURRENT_LATCHES.MICROINSTRUCTION[SETM];
	  }
	  else NEXT_LATCHES.M = CURRENT_LATCHES.M;

	  if (CURRENT_LATCHES.MICROINSTRUCTION[LD_MDRB] == 1)
	  {
		  NEXT_LATCHES.MDRB = Low16bits(BUS);
	  }
	  else NEXT_LATCHES.MDRB = CURRENT_LATCHES.MDRB;

        if (CURRENT_LATCHES.MICROINSTRUCTION[LD_MDR] == 1)
        {
		  if (CURRENT_LATCHES.MICROINSTRUCTION[MDRSEL] == 1)
		  {
			  if (CURRENT_LATCHES.M == 0)
			  {
				  NEXT_LATCHES.MDR = (CURRENT_LATCHES.MDR | 0x0001);
			  }
			  else NEXT_LATCHES.MDR = (CURRENT_LATCHES.MDR | 0x0003);
		  }
		  else
		  {
	              if (CURRENT_LATCHES.MICROINSTRUCTION[MIO_EN] == 0)
	              {
	                  if (CURRENT_LATCHES.MICROINSTRUCTION[DATA_SIZE] == 0)
	                  {
	                        NEXT_LATCHES.MDR = mask8(BUS) + (mask8(BUS) << 8);
	                  }
	                  else
	                  {
	                        NEXT_LATCHES.MDR = Low16bits(BUS);
	                  }
	              }
	              else NEXT_LATCHES.MDR = MEMOut;
		  }
        }
        else NEXT_LATCHES.MDR = CURRENT_LATCHES.MDR;
  /*
   * Datapath routine for computing all functions that need to latch
   * values in the data path at the end of this cycle.  Some values
   * require sourcing the bus; therefore, this routine has to come
   * after drive_bus.
   */

}

int SEXT(int immediateLength, int num)
{
  	int tempNum = num;
  	int amountToShift = 16 - immediateLength;
  	int check = (tempNum >> (immediateLength - 1)) & 0x0001;
  	if (check == 0x0000)
	  {
    		return num;
  	}
  	else
  	{
    		int starter = 0x8000;
    		while (amountToShift > 0)
    		{
      		num = num | starter;
      		amountToShift--;
      		starter = starter >> 1;
    		}
    	  return num;
  	}
}

void setCC(char cc)
{
  if (cc == 'N')
  {
    NEXT_LATCHES.N = 1;
    NEXT_LATCHES.Z = 0;
    NEXT_LATCHES.P = 0;
  }
  if (cc == 'Z')
  {
    NEXT_LATCHES.N = 0;
    NEXT_LATCHES.Z = 1;
    NEXT_LATCHES.P = 0;
  }
  if (cc == 'P')
  {
    NEXT_LATCHES.N = 0;
    NEXT_LATCHES.Z = 0;
    NEXT_LATCHES.P = 1;
  }
}
void latchRegs()
{
      memcpy(NEXT_LATCHES.REGS, CURRENT_LATCHES.REGS, sizeof(int)*LC_3b_REGS);
}
void latchNext(int state)
{
      NEXT_LATCHES.STATE_NUMBER = state;
	memcpy(NEXT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[state], sizeof(int)*CONTROL_STORE_BITS);
}

int decipherState(int j5, int j4, int j3, int j2, int j1, int j0)
{
	return (1 * j0) + (2 * j1) + (4 * j2) + (8 * j3) + (16 * j4) + (32 * j5);
}

int mask1(int bin)
{
	return (bin & 0x0001);
}

int mask2(int bin)
{
	return (bin & 0x0003);
}

int mask3(int bin)
{
	return (bin & 0x0007);
}

int mask4(int bin)
{
	return (bin & 0x000F);
}
int mask5(int bin)
{
	return (bin & 0x001F);
}
int mask6(int bin)
{
	return (bin & 0x003F);
}
int mask7(int bin)
{
	return (bin & 0x007F);
}
int mask8(int bin)
{
	return (bin & 0x00FF);
}
int mask9(int bin)
{
	return (bin & 0x01FF);
}
int mask11(int bin)
{
	return (bin & 0x03FF);
}
