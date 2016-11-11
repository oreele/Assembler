/* 
	This program is an assembler of an imaginary assembly language. It gets in as arguments of the command line list of file names (without ending .as) and creates 3 files: object file (.ob) - the machine code, externals file (.ext) and entries file (.ent).
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "assembler.h"
#include "symbols.h"
#include "consts.h"
#define MAX_LINE_LENGTH		80 /* max length of a line in the input files */
#define INIT_LINE_COUNTER	1 /* initial value for the counter of lines */

int compile (char *);

/* the main function, sending for compilation each file
*/
int main (int argc, char *argv[]) {
	int i, counter=0; /* c is the number of files compiled successfully */
	for(i=1; i<argc; i++) { /* compile each file from command line and counters how many of them compiled successfuly */
		if(compile(argv[i]))
			counter++;
	}
	printf("Succeed compile %d files out of %d\n", counter, argc-1);
	return 0;
}

/*
compile: gets a file name and compile it
*/
int compile (char *file_name) {
	int no_ins; /* the number of instructions the object file contains */
	FILE *fp; /* a pointer to FILE of file_name */
	char buff[MAX_LINE_LENGTH]; /* a buffer to hold each line of the file */
	int succeed = SUCCESS, /* saves if succeed compile so far */
		line_counter = INIT_LINE_COUNTER; /* count the lines of the file */
	char *file_str = (char *) malloc(strlen(file_name) + 4); /* string that saves the file name with ending */
	strcpy(file_str, file_name);
	strcat(file_str, ".as"); /* concating the ending */
	printf(">>compile %s :\n", file_str);
	fp = fopen(file_str, "r");
	if(!fp) { /* error with opening the file */
		fprintf(stderr, "ERROR: cannot access to file %s\n", file_str);
		return FAILURE;
	}
	free(file_str);
	/* here starts first pass of compilation: */
	while(fgets(buff, MAX_LINE_LENGTH, fp)) { /* read to buff each line */
		if(!first_encode_line(buff, line_counter)) /* do first pass on line */
			succeed = FAILURE; /* didn't succeeded do first pass on line */
		line_counter++;
	}
	fseek(fp, 0L, SEEK_SET); /* back to the start of the file */
	reset_IC(&no_ins); /* reset the counter of instructions and put the value in no_ins */
	change_address_data_symbols(no_ins); /* adding to each of the data symbols address the number of instructions in order to fix thier addresses */
	line_counter = INIT_LINE_COUNTER; /* reset line_counter */
	if(succeed) { /* second pass if succeeded so far */
		while(fgets(buff, MAX_LINE_LENGTH, fp)) {
			if(!second_encode_line(buff, line_counter))
				succeed = FAILURE;
			line_counter++;
		}
	}
	fclose(fp);

	/* making the files if succeed*/
	if(succeed) {
		if(!make_obj(file_name, no_ins)) /* make the object file */
			succeed = FAILURE; /* if didn't succeed */
		if(!make_ent(file_name)) /* make the entries file */
			succeed = FAILURE;
		if(!make_ext(file_name)) /* make the externals file */
			succeed = FAILURE;
	}
	if(succeed)
		printf("File compiled successfuly\n");
	else	printf("File didn't compile due to an error\n");
	free_entry(); /* free the saved entries */
	free_external(); /* free the saved externals */
	reset_data(); /* delete the data saved for this file */
	return succeed; /* returns if succeeded */
}
