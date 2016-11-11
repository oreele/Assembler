/*
definitions of main constants.
*/
#ifndef CONSTS_H
#define CONSTS_H
#define NO		0 /* symbol for functions that returns bool value */
#define YES		1 /* symbol for functions that returns bool value */
#define MAYBE		2 /* symbol for a non known information for boolean */
#define FAILURE		0 /* symbol for functions that returns if succeed */
#define SUCCESS		1 /* symbol for functions that returns if succeed */

#define DEFAULT_ADDRESS	0 /* when an address isnot given, uses this (for external labels) */

#define ERR_ST 0 /* symbol that the statement includes errors */
#define EMPTY_ST 1 /* symbol of an empty or comment statement */
#define DATA_ST 2 /* symbol of a data statement */
#define STR_ST 3 /* symbol of a string statement */
#define EXT_ST 4 /* symbol of an extern statement */
#define ENT_ST 5 /* symbol of an entry statement */
#define ACTION_ST 6 /* symbol of an action statement (command) */

#define NO_ARG -1 /* symbols the arg not exitsts */
#define VAL_ARG 0 /* anything that isnot a register */
#define REG_ARG 3 /* register argument */
#define IM_ARG 0 /* immediate argument */
#define LABEL_ARG 1 /* label argument */
#define COMM_ARG 4 /* the argument is a command */
#define REG_RAND 4 /* symbols randomize register argument */
#define IM_RAND 5 /* symbols randomize immediate argument */
#define LABEL_RAND 6 /* symbols randomize label argument */
#define RAND 2 /* the type of random argument */
#define RAND_OFFSET 3 /* offset from the randomize symbols to the rnd field values */
#define NOT_RAND 0 /* rnd value for non randomized argument */

/* ARE field codes */
#define ABSOLUTE 0
#define EXTERNAL 1
#define RELOCATABLE 2

#define ERR -1 /* symbols that an error occurred */

/* group field codes */
#define NO_OPERANDS	0
#define ONE_OPERAND	1
#define TWO_OPERANDS	2

/* bounders */
#define MAX_IMM 4095 /* max immidiate value */
#define MIN_IMM -4096 /* min immidiate value */
#define MAX_DATA 32767 /* max data value */
#define MIN_DATA -16384 /* min data value */
#define MEMORY_SIZE	1000 /* the size of memory in the machine */
#define MAX_ENTRIES	MEMORY_SIZE /* max number of entries */
#define	MAX_EXTERNALS	MEMORY_SIZE /* max number of externals */
#define DATA_LEN	MEMORY_SIZE /* max number of data words */
#define INST_LEN	MEMORY_SIZE /* max number of instruction words */

/* initializations */
#define START_DATA 0 /* first address for data */
#define START_INST 100 /* first address for instructions */

/* registers */
#define NUM_OF_REG	8 /* the number of registers exist */
#define MAX_REG_DIG	'7' /* the max character of register number */
#define MIN_REG_DIG	'0' /* the min character of register number */
#endif
