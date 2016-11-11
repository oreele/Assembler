#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include "assembler.h"
#include "consts.h"
#include "utilities.h"
#include "symbols.h"
#define max(a,b) (a>b)?a:b
#define ONE_OFFSET	1 /* offset after the instruction counter of one word when inserting to extern */
#define TWO_OFFSET	2 /* offset after the instruction counter of 2 words when inserting to extern */
#define to_num(a) a-48 /* gets a digit in ascii and switch to that digit */
#define INITIAL_SUM	0 /* the initialization number for sum */
#define comm_enc(ARE, dest, source, opcode, group, rnd, not_in_use) \
	ARE+4*(dest+4*(source+4*(opcode+16*(group+4*(rnd+4*not_in_use))))) /* the encoding of a command word to number */
#define imm_enc(ARE, val)\
	ARE+4*val /* the encoding of an immediate word to number */
#define reg_enc(ARE, dest, source, not_in_use)\
	ARE+4*(dest+64*(source+64*not_in_use)) /* the encoding of a register word to number */
typedef union {
	struct {
		unsigned int ARE:2;
		unsigned int dest:2;
		unsigned int source:2;
		unsigned int opcode:4;
		unsigned int group:2;
		unsigned int rnd:2;
		unsigned int not_in_use:1;		
	} action; /* a action statement */
	struct {unsigned int ARE:2;
		int val:13;
		} immidiate; /* immidiate operand - number or label */
	struct {unsigned int ARE:2;
		unsigned int dest:6;
		unsigned int source:6;
		unsigned int not_in_use:1;
		} registers; /* registers operand */
} word; /* represents a word in the object file */

enum addressing { /* the methods that can be used for an operand */
		all, /* (0,1,2,3) all the methods */
		label, /* (1,2***) just method 1 and 2 with 3 asteriks */
		not_randomize, /* (0,1,3) everything but not method 2 */
		direct, /* (1,3) just direct label or direct register */
		no_operand /* () the operand is missing */
};
/* array of the operations and their operands addressing methods. The last row is with null name in order to symbol that it's the end. */		
struct {
	char *name;
	enum addressing source;
	enum addressing dest;
} operations[] = {
	{"mov", all, direct},
	{"cmp", all, not_randomize},
	{"add", all, direct},
	{"sub", all, direct},
	{"not", no_operand, direct},
	{"clr", no_operand, direct},
	{"lea", label, direct},
	{"inc", no_operand, direct},
	{"dec", no_operand, direct},
	{"jmp", no_operand, direct},
	{"bne", no_operand, direct},
	{"red", no_operand, direct},
	{"prn", no_operand, not_randomize},
	{"jsr", no_operand, direct},
	{"rts", no_operand, no_operand},
	{"stop", no_operand, no_operand},
	{NULL, no_operand, no_operand}
};

int encode_action (char *, int);
int random_immidiate(int, int);
unsigned int word_to_num (word, int);
int decode_argument (char *, int *);
/*
second_encode_line: gets the line and it's number and do for it 'second pass': encode the commands to words, make list of entries and extracts.
*/
int second_encode_line (char *line, int line_num) {
	char *op; /* saves the operation name */
	int temp;
	int type; /* saves the line type */
	fetch_label(&line, NULL); /* removes the label if exists */
	delete_spaces(&line); /* delete spaces from the start */
	op = line; /* thats the operation name */
	type = get_line_type(&line); /* saves the line type at type and removes from line the operation name */
	delete_spaces(&line); /* now line includes the parameters */
	if(type == DATA_ST || type == STR_ST || type == EMPTY_ST || type == EXT_ST)
		return SUCCESS; /* we have already treated with it */
	if(type == ENT_ST) {
		cut_after_spaces(line); /* removes spaces after the word */
		search_symbol(line, &temp);
		add_entry(line, temp); /* add an entry to the list */
		return SUCCESS;
	}
	if(type == ACTION_ST) {
		if(!encode_action(op, line_num)) /* trying to encode the command (op, because we need the opname) */
			return FAILURE; /* didn't succeed */
		return SUCCESS;
	}
	return FAILURE;
}

/*
search_operation: gets string of operation name and pointers to variable to put the properties of the operation if exists: the opcode, the source addressing method and the destination addressing method. If succeeded, returns 1.
*/
int search_operation(char *op, int *opcode, enum addressing *source, enum addressing *dest) {
	int i;
	for(i=0; operations[i].name!=NULL; i++) { /* pass on the operations */
			if(strlen(op)==strlen(operations[i].name) && strncmp(operations[i].name, op, strlen(op))==0) { /* the names are equal */
				*source = operations[i].source;
				*dest = operations[i].dest;
				*opcode = i;
				return SUCCESS; /* found operation */
			}
	}
	return FAILURE; /* operation didn't found */
}

/*
encode_argument: gets an argument and pointers to int and saves on their pointed data the value that the argument include (for immidiate is the number, for register is the register number, for label is address), the type of the argument (can be error if it's not valid), the address method, the rnd (number of random method, can be NULL) and the ARE (ABSOLUTE, RELOCATABLE, EXTERNAL). Returns 1 if succeed.
*/
int encode_argument (char *arg, int *value, int *type, int *address, int *rnd, int *are) {
	*type = decode_argument (arg, value); /* gets the type and the value */
	if(*type==REG_RAND || *type==IM_RAND || *type==LABEL_RAND) {
		*address = RAND; /* random method */
		if(rnd) /* if rnd isnot null */
			*rnd = *type-RAND_OFFSET; /* Its arranged that RAND_OFFSET gets the number of random methos from the type */
	} else {
		*address = *type; /* Its arrenged that the address of non randomize methods is the type of them */
		if(rnd) /* if rnd isnot null */
			*rnd = NOT_RAND;
	}
	if(*type==ERR) /* error */
		return FAILURE;
	if(*type==IM_ARG || *type==REG_ARG || *type==IM_RAND || *type==REG_RAND)
		*are=ABSOLUTE;
	else if(*type==LABEL_ARG) {
		if(is_external(arg)) /* check if the argument label is external */
			*are=EXTERNAL;
		else *are=RELOCATABLE;
	} else if(*type==LABEL_RAND) /* an internal randomized label */
		*are=RELOCATABLE;
	return SUCCESS;
}

/*
encode_action: gets an action command line and the line number. The function encode the action into words and saves them to extract later. Returns 1 if succeed.
NOTE: arguments are the data after the operation name. operands are the words after the command word that save the arguments.
*/
int encode_action (char *line, int line_num) {
	int address1, address2; /* the addressing method of each operand */
	int type1, type2; /* the type of each operand */
	int flag1=NO_ARG, flag2=NO_ARG; /* saves the words of operand1 and operand2 type (immidiate or registers) */
	int are1, are2; /* saves the ARE of the arguments */
	int val1, val2; /* saves the value of the arguments (number in immidiate, number of register in register, address in label) */
	int rnd; /* saves the rnd value of the first argument */
	int opcode; /* saves the opcode of the command */
	int operand1_exists=NO, operand2_exists=NO; /* saves if the operands are exist */
	enum addressing source_add, dest_add; /* saves the addressing method of the source and dest arguments */
	word command, operand1, operand2;
	char *op /* the operation name */, *arg1 /* the first argument */, *arg2 /* the second argument */;
	memset((void *) &command, '\0', sizeof(word)); /* reset the operands */
	memset((void *) &operand1, '\0', sizeof(word)); /* " */
	memset((void *) &operand2, '\0', sizeof(word)); /* " */
	op = getword(&line); /* op gets the operation name */
	delete_spaces(&line); /* now line is the arguments */
	command.action.ARE = ABSOLUTE; /* any command is absolute */

	if(!search_operation(op, &opcode, &source_add, &dest_add)) { /* gets the operation properties */
		fprintf(stderr, "line %d: cannot recognize operation name \"%s\"\n", line_num, op);
		return FAILURE;
	}
	free(op);
	command.action.opcode = opcode; /* puts the opcode in the command word */
	if(source_add==no_operand) { /* fill the group field in the command word - the number of arguments in the operation */
		if(dest_add==no_operand)
			command.action.group = NO_OPERANDS; /* 0 operands */
		else command.action.group = ONE_OPERAND; /* 1 operand */
	} else if(dest_add==no_operand) {
		command.action.group = ONE_OPERAND; /* 1 operand */
	} else command.action.group = TWO_OPERANDS; /* two operands */

	fetch_arguments(line, &arg1, &arg2, line_num); /* archive arg1 & arg2 */

	if(source_add==no_operand&&dest_add!=no_operand) {
		if(!isempty(arg2)) { /* 2 operands */
			fprintf(stderr, "line %d: too many arguments for this operation\n", line_num);
			return FAILURE;
		}
		arg2 = arg1; /* the second is the first.. */
		arg1 = NULL;
	}
	if(isempty(arg1)) {
		arg1=NULL;
		if(source_add!=no_operand) { /* arg1 should be exists */
			fprintf(stderr, "line %d: missing source argument\n", line_num);
			return FAILURE;
		}
	}
	if(isempty(arg2)) {
		arg2=NULL;
		if(dest_add!=no_operand) { /* arg1 should be exists */
			fprintf(stderr, "line %d: missing destination argument\n", line_num);
			return FAILURE;
		}
	}
	/* now if argument isnot null, it exists */
	cut_after_spaces(arg1); /* put '\0' at end, deletes spaces at the end */
	cut_after_spaces(arg2);
	if(arg1) {
		if(!encode_argument(arg1, &val1, &type1, &address1, &rnd, &are1)) 			{ /* encoding the argument properties */
			fprintf(stderr, "line %d: source argument isn't valid\n", line_num);
			return FAILURE;
		}
		command.action.source = address1; /* the address method */
		command.action.rnd = rnd; /* rnd is the rnd of arg1 */
	}
	if(arg2) {
		if(!encode_argument(arg2, &val2, &type2, &address2, NULL, &are2)) { /* encoding the argument properties */
			fprintf(stderr, "line %d: destination argument isn't valid\n", line_num);
			return FAILURE;
		}
		command.action.dest = address2; /* the address method */
	}
	if(arg1) {
		if(type1==IM_ARG || type1==LABEL_ARG || type1==IM_RAND || type1==LABEL_RAND) { /* we need the first operand to be immidiate */
			operand1.immidiate.val = val1;
			operand1.immidiate.ARE = are1;
			flag1 = IM_ARG;
			operand1_exists = YES;
			if(arg2) { /* arg2 need to use operand2 */
				if(type2==REG_ARG || type2==REG_RAND) {
					operand2.registers.dest = val2;
					operand2.registers.ARE = are2;
					flag2 = REG_ARG;
				} else {
					operand2.immidiate.val = val2;
					operand2.immidiate.ARE = are2;
					flag2 = IM_ARG;
				}
				operand2_exists = YES;
			}
		} else { /* arg1 is of type register */
			operand1.registers.source = val1;
			operand1.registers.ARE = are1;
			flag1 = REG_ARG;
			operand1_exists = YES;
			if(arg2) {
				if(type2==REG_ARG || type2==REG_RAND) { /* arg1 and arg2 are both in operand1 */
					operand1.registers.dest = val2;
				} else { /* arg2 isnot a register and needs to be in operand2 */
					operand2.immidiate.val = val2;
					operand2.immidiate.ARE = are2;
					flag2 = IM_ARG;
					operand2_exists = YES;
				}
			}
		}
	} else if(arg2)  { /* there is just arg2, so it uses operand1 */
		operand1_exists = YES;
		if(type2==IM_ARG || type2==LABEL_ARG || type2==IM_RAND || type2==LABEL_RAND) {
			operand1.immidiate.val = val2;
			operand1.immidiate.ARE = are2;
			flag1 = IM_ARG;
		} else {
			operand1.registers.dest = val2;
			operand1.registers.ARE = are2;
			flag1 = REG_ARG;
		}
	}
	/* add to extern file */
	if(flag2!=NO_ARG) { /* we have 2 arguments */
		if(type1==LABEL_ARG && is_external(arg1))
			add_external(arg1, ONE_OFFSET); /* add an external about the second word we add */
		if(type2==LABEL_ARG && is_external(arg2))
			add_external(arg2, TWO_OFFSET); /* add an external about the third word we add */
	} else if(flag1!=NO_ARG) { /* we have just 1 argument */
		if(arg1)
			if(type1==LABEL_ARG && is_external(arg1))
				add_external(arg1, ONE_OFFSET); /* add an external about the second word we add */
		if(arg2)
			if(type2==LABEL_ARG && is_external(arg2))
				add_external(arg2, ONE_OFFSET); /* add an external about the second word we add */
	}
	/* insert words to the instruction list */
	if(!insert_inst(word_to_num(command, COMM_ARG)))
		fprintf(stderr, "line %d: cannot write the command\n", line_num);
	if(operand1_exists)
		if(!insert_inst(word_to_num(operand1, flag1)))
			fprintf(stderr, "line %d: cannot write the command\n", line_num);
	if(operand2_exists)
		if(!insert_inst(word_to_num(operand2, flag2)))
			fprintf(stderr, "line %d: cannot write the command\n", line_num);
	return SUCCESS;
}

/*
decode_argument: get an argument in 'argument' and integer pointer 'decoded'. decode the argument and put decoded to point on the value of the argument. If succeeded returns 1.
*/
int decode_argument (char *argument, int *decoded) {
	int temp; /* temporary variable to save the number in immediate value */
	if(*argument=='#') { /* suspected to be immediate */
		argument++;
		temp = atoi(argument); /* temp gets the immediate number */
		if(*argument=='\0') { /* it is a valid immediate number */
			*decoded = temp;
			return IM_ARG;
		}
		if(!isdigit(*argument) && *argument!='-' && *argument!='+')
			return ERR; /* immediate number has to start with a digit or a sign */
		argument++;
		while(*argument!='\0' && !isspace(*argument)) {
			if(!isdigit(*(argument++)))
				return ERR; /* the number cannot include other chars at the middle */
		}
		*decoded = temp; /* its a valid immidiate number */
		return IM_ARG;
	}
	if(strncmp(argument, "***", max(strlen(argument), strlen("***")))==0) { /* randomize label */
		if((*decoded = randomize_label())==ERR) { /* random an internal label */
			fprintf(stderr, "used the *** argument when doesn't exist any internal label\n");
			return ERR; /* an internal label isnot exist */
		}
		return LABEL_RAND;
	}
	if(strncmp(argument, "**", max(strlen(argument), strlen("**")))==0) { /* randomize immidiate value */
		*decoded = random_immidiate(MIN_IMM, MAX_IMM);
		return IM_RAND;
	}
	if(strncmp(argument, "*", max(strlen(argument), strlen("*")))==0) { /* randomized register */
		srand(time(NULL)); /* change random source */
		*decoded = rand()%NUM_OF_REG; /* randomize value 0...7 */
		return REG_RAND;
	}
	if(is_register(argument)) {
		argument++;
		*decoded = to_num(*argument); /* the number of register */
		return REG_ARG;
	}
	if(!search_symbol(argument, decoded)) { /* search for a symbol and put the address in decoded */
		return ERR; /* it should be a symbol but the symbol isnot exists */
	}
	return LABEL_ARG; /* it is a symbol */
}


/*
random_immidiate: gets 2 integers: min and max and returns a randomized integer between them.
*/
int random_immidiate(int min, int max) {
	int bound = max-min; /* find a randomized value between 0 and bound */
	unsigned int num_bins = (unsigned int) bound + 1;
	unsigned long num_rand = (unsigned long) RAND_MAX +1,
			bin_size = num_rand / num_bins,
			defect = num_rand % num_bins; /* the deviation */
	long x;
	srand(time(NULL));
	do {
		x = rand();
	}
  /* in order to not to overflow */
	while (num_rand - defect <= (unsigned long)x);

  /* returns the randomized value (x/bin_size) plus min */
	return x/bin_size+min;
}

/*
word_to_num: convert the word decode to integer that decodes the word according to the type of the word declared by 'flag'.
*/
unsigned int word_to_num (word w, int flag) {
	unsigned int sum = INITIAL_SUM;
	if(flag == COMM_ARG) {
		sum = comm_enc(w.action.ARE, w.action.dest, w.action.source, w.action.opcode, w.action.group, w.action.rnd, w.action.not_in_use); /* the decoding of command */
	} else if(flag == IM_ARG) {
		sum = imm_enc(w.immidiate.ARE, w.immidiate.val); /* the decoding of immediate */
	} else if(flag == REG_ARG) {
		sum = reg_enc(w.registers.ARE, w.registers.dest, w.registers.source, w.registers.not_in_use);
	}
	return sum;
}
