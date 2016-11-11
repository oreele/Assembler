/*
This file contatins the storage of instructions and data.
*/

#include "symbols.h"
#include "consts.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define SUCCESS	1
#define	FAILURE	0

#define	INIT_ENT	0 /* start saving place for entries */
#define	INIT_EXT	0 /* start saving place for externals */
#define is_digit(a) a<=9 && a>=0
#define DIGIT_TO_ASCII	48 /* offset of converting digit to digit in ascii */
#define LETTER_TO_ASCII	55 /* offset of converting big letter to big letter in ascii */
#define	BITS_MASK	31 /* to isolate 5 most right bits */
#define	BITS_TO_DIGIT	5 /* the number of bits a digit represents */
#define	NUM_OF_DIGITS	3 /* the number of digit we need to convert to base 32 */

int IC = START_INST, DC = START_DATA; /* The current address of IC and DC */

int data[DATA_LEN]; /* an array of data */
int inst[INST_LEN]; /* an array of instructions */

struct {
	char *label;
	int address;
} entries[MAX_ENTRIES], externals[MAX_EXTERNALS]; /* array of entries and externals */
int entries_bound = INIT_ENT, externals_bound = INIT_EXT; /* the current upper bound address in each of the arrays */

/* returns IC */
int get_IC () {
	return IC;
}

/*
advance IC if can and returns if succeed or not
*/
int advance_IC (int num) {
	IC+=num;
	if(IC<INST_LEN)
		return SUCCESS;
	fprintf(stderr, "instructions overflow error\n");
	return FAILURE;
}

/* returns DC */
int get_DC () {
	return DC;
}

/*
insert_inst: get an unsigned int that represents an instruction and insert it into the instructions list. returns 1 if succeed, 0 if not.
*/
int insert_inst (unsigned int val) {
	if(IC-START_INST>=INST_LEN) /* there is no more space */
		return FAILURE;
	inst[IC-START_INST] = val; /* insert the value at the first clear place */
	IC++; /* increase IC */
	return SUCCESS;
}

/*
insert_data: get an int that represents a data and insert it into the data list. returns 1 if succeed, 0 if not.
*/
int insert_data (int val) {
	if(DC-START_DATA>=DATA_LEN) /* there is no more space */
		return FAILURE;
	if(val<MIN_DATA || val>MAX_DATA) /* the data isnot compatible */
		return FAILURE;
	data[DC-START_DATA] = val; /* insert the value at the first clear place */
	DC++; /* increase DC */
	return SUCCESS;
}

/*
reset_data: reset all the data that the program saves: instructions, data, and symbols.
*/
void reset_data () {
	IC = START_INST; /* reset IC */
	DC = START_DATA; /* reset DC */
	free_list(); /* reset the symbol list */
}

/*
reset_IC: reset the IC counter and fetch it into the parameter pointer.
*/
void reset_IC (int *no_ins) {
	*no_ins = IC;
	IC = START_INST;
}

/*
to_base_32: gets an unsigned int and returns a 3 digits base 32 number that represents the number (the output is a string).
*/
char *to_base_32 (unsigned int num) {
	int i;
	int isolated; /* isolated 5 bits */
	char *res = malloc(NUM_OF_DIGITS+1); /* allocation of memory for the output */
	res[NUM_OF_DIGITS]='\0'; /* end of the string */
	for(i=NUM_OF_DIGITS-1; i>=0; i--) { /* encode each digit */
		isolated = num & BITS_MASK;
		if(is_digit(isolated))
			res[i] = isolated+DIGIT_TO_ASCII;
		else res[i] = isolated+LETTER_TO_ASCII;
		num = num>>BITS_TO_DIGIT;
	}
	return res;
}

/*
make_obj: get the file_name and number of instructions and create a object file for it. Returns 1 if succeed and 0 if don't.
*/
int make_obj (char *file_name, int no_ins) {
	int counter; /* counter of addresses */
	FILE *fp; 
	char *file_str = (char *) malloc(strlen(file_name) + strlen(".ob") + 1), *address, *comm; /* hand the address and command that converted to base 32 */
	strcpy(file_str, file_name);
	strcat(file_str, ".ob"); /* file_str is the file name with ending .obj */
	fp = fopen(file_str, "w");
	if(!fp) {
		fprintf(stderr, "ERROR: cannot access object file\n");
		return 0;
	}
	for(counter = START_INST; counter<no_ins; counter++) { /* move on the array of instructions and extract it to the object file */
		address = to_base_32(counter);
		comm = to_base_32(inst[counter-START_INST]);
		fprintf(fp, "%s\t%s\n", address, comm);
		free(address);
		free(comm);
	}
	for(; counter<no_ins+DC; counter++) { /* move on the array of data and extract it to the object file with address continues from earlier */
		address = to_base_32(counter);
		comm = to_base_32(data[counter-no_ins]);
		fprintf(fp, "%s\t%s\n", address, comm);
		free(address);
		free(comm);
	}
	fclose(fp);
	return 1;
}

/*
add_entry: adding the label and val in the input to the entry list. Returns 1 if succeed and 0 if don't.
*/
int add_entry (char *label, int val) {
	if(entries_bound>=MAX_ENTRIES) /* there is no place */
		return FAILURE;
	entries[entries_bound].label = malloc(sizeof(label)+1); /* adding */
	strcpy(entries[entries_bound].label, label);
	entries[entries_bound].address = val;
	entries_bound++; /* increasing the bound */
	return SUCCESS;
}

/*
free_entry: frees the data in the array of entries.
*/
void free_entry () {
	int i;
	for(i=0; i<entries_bound; i++)
		free(entries[i].label);
	entries_bound=INIT_ENT;
}

/*
make_ent: get the file_name and create a entries file for it. Returns 1 if succeed and 0 if don't.
*/
int make_ent (char *file_name) {
	int i;
	FILE *fp;
	char *file_str = (char *) malloc(strlen(file_name) + strlen(".ent")+1), *address;
	strcpy(file_str, file_name);
	strcat(file_str, ".ent"); /* file_str is the file name with ending */
	fp = fopen(file_str, "w");
	free(file_str);
	if(!fp) {
		fprintf(stderr, "ERROR: cannot create entry file\n");
		return FAILURE;
	}
	for(i=0; i<entries_bound; i++) { /* pass on the array and print the address in base 32 and the label */
		address = to_base_32(entries[i].address);
		fprintf(fp, "%s\t%s\n", entries[i].label, address);
		free(address);
	}
	fclose(fp);
	return SUCCESS;
}

/*
add_external: adding the label string (first argument) and offset from the IC (second argument) in the input to the external list. Returns 1 if succeed and 0 if don't.
*/
int add_external (char *label, int offset) {
	if(externals_bound>=MAX_EXTERNALS) /* there is no place */
		return FAILURE;
	externals[externals_bound].label = malloc(sizeof(label)+1); /* adding */
	strcpy(externals[externals_bound].label, label);
	externals[externals_bound].address = IC + offset /* gets IC after offset */;
	externals_bound++; /* increasing the bound */
	return SUCCESS;
}

/*
free_external: frees the data in the array of externals.
*/
void free_external () {
	int i;
	for(i=0; i<externals_bound; i++)
		free(externals[i].label);
	externals_bound=INIT_EXT;
}

/*
make_ext: get the file_name and create a externals file for it. Returns 1 if succeed and 0 if don't.
*/
int make_ext (char *file_name) {
	int i;
	FILE *fp;
	char *file_str = (char *) malloc(strlen(file_name) + 5), *address;
	strcpy(file_str, file_name);
	strcat(file_str, ".ext"); /* file_str is the file name with ending */
	fp = fopen(file_str, "w");
	free(file_str);
	if(!fp) {
		fprintf(stderr, "ERROR: cannot create external file\n");
		return FAILURE;
	}
	for(i=0; i<externals_bound; i++) { /* pass on the array and print the address in base 32 and the label */
		address = to_base_32(externals[i].address);
		fprintf(fp, "%s\t%s\n", externals[i].label, address);
		free(address);
	}
	fclose(fp);
	return SUCCESS;
}
