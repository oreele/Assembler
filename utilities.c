#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "utilities.h"
#include "assembler.h"
#include "consts.h"

/*
This file contains a set of utility functions for the first and second pass.
*/

#define INIT_COUNTER	1 /* start value for counter */
#define INIT_SIZE	2 /* the initial value of a string size counter (starts from 1 character) */
#define without_end(a) a-1 /* the size without the end '\0' character */
#define is_data(a) !strncmp(a, ".data", 5) /* check if starts with 'data' */
#define is_string(a) !strncmp(a, ".string", 7) /* check if starts with 'string' */
#define is_entry(a) !strncmp(a, ".entry", 6) /* check if starts with 'entry' */
#define is_extern(a) !strncmp(a, ".extern", 7) /* check if starts with 'extern' */
/*
get_line_type: gets a line and returns its type according to the first word.
*/
int get_line_type (char **line) {
	char *temp=*line; /* saves temporary pointer to the start of the line */
	if(**line == '\0' || **line == ';') /* start of empty statement */
		return EMPTY_ST;
	skipword(line); /* skip on the first word in the line */
	/*now it returns the type of the line */
	if(is_data(temp)) return DATA_ST;
	if(is_string(temp)) return STR_ST;
	if(is_extern(temp)) return EXT_ST;
	if(is_entry(temp)) return ENT_ST;
	return ACTION_ST; /* default */
}

/*
fetch the command arguments from line from the first 2 parameters into the second and the third parameter. Gets also the line number in order to print errors. Returns 1 if succeeded and 0 otherwise.
*/
int fetch_arguments (char *line, char **arg1, char **arg2, int line_num) {
	int colon = NO; /* saves if there was a colon */
	*arg1 = NULL, *arg2 = NULL; /* initialization */
	delete_spaces(&line); /* delete spaces from start of line */
	if(isempty(line)) { /* there is no arguments */
		return SUCCESS;
	}
	*arg1 = line++; /* arg1 is in the start */
	while(!isspace(*line) && *line!=',' && *line!='\0') /* keep going while its still arg1*/
		line++;
	if(*line==',')
		colon = YES;
	*line = '\0'; /* put '\0' at the end (in place of space or colon)*/
	line++;
	delete_spaces(&line);
	if(isempty(line)) { /* just one argument */
		return SUCCESS;
	}
	if(!colon) {
		if(*line!=',') { /* missing colon */
			fprintf(stderr, "line %d: there is a missing colon\n", line_num);
			return FAILURE;
		} else *(line++) = '\0'; /* end of arg1. note that here we increase 'line' */
	}
	delete_spaces(&line);
	*arg2 = line;
	if(isempty(*arg2)) { /* there was a colon so we need second argument */
		fprintf(stderr, "line %d: missing second argument or dundent colon\n", line_num);
		return FAILURE;
	}
	return SUCCESS; /* two arguments */
}

/*
delete_spaces: gets pointer string 'line' and change '*line' to point to the start of the string after the whitespaces.
*/
void delete_spaces(char **line) {
	while(isspace(**line))
		(*line)++;
}

/*
skipword: gets a pointer to string and cut there the first word (with spaces before)
*/
void skipword (char **line) {
	delete_spaces(line); /* deletes spaces before */
	while(!isspace(**line)) /* skip chars while its not a space */
		(*line)++;
}

/*
getword: returns a string of the first word in the input that allocated in the memory.
*/
char *getword (char **line) {
	char *word /* a pointer to the result string */, *temp /* saves the start of the word at '*line' */;
	int count = INIT_COUNTER; /* count the length of the word */
	delete_spaces(line);
	temp = *line; /* this is the start of the word */
	while(!isspace(**line) && **line!='\0') { /* while its still the word*/
		(*line)++;
		count++;
	}
	word = malloc(count*sizeof(char));
	strncpy(word, temp, count); /* copies the word */
	*(word+without_end(count))='\0'; /* puts '\0' at the end */
	return word;
}

/*
isempty: check if str has just white spaces.
*/
int isempty (char *str) { /* check if str has just spaces */
	if(str==NULL)
		return YES;
	while(*str != '\0')
		if(!isspace(*(str++))) /* has non-space character */
			return NO;
	return YES;
}

/*
fetch_label: returns 1 if there is a label, put the label in the parameter array 'label' if label is not NULL and put the line after the label in 'label'.
*/
int fetch_label (char **line, char **label) {
	char *curr = *line; /* pointer to the current location in the string */
	int size = INIT_SIZE /* the size of the label */, i;
	if(!isalpha(*curr))
		return NO; /* a label has to start with a letter */
	curr++; /* the first letter checked */
	while(isalpha(*curr) || isdigit(*curr)) { /* each other character in the label has to be a letter or a digit*/
		size++;
		curr++;
	}
	if(*curr != ':')
		return NO; /* a label has to finish with a colon */
	if(label!=NULL) {
		*label = malloc(size); /* size is the number of bytes required: length+1 */
		for(i=0; i<without_end(size); i++) /* copies the label */
			(*label)[i] = (*line)[i];
		(*label)[i]='\0'; /* end of string */
	}
	(*line) += size;
	return YES; /* there is a label */
}

/*
is_register: gets a string and returns if there is a register.
*/
int is_register (char *argument) {
	if(*argument=='r') { /* first char is r */
		argument++;
		if(*argument<=MAX_REG_DIG && *argument>=MIN_REG_DIG) { /* second char is 0...7*/
			argument++;
			if(isspace(*argument) || *argument=='\0') { /*third char is space or end */
				return YES;
			}
		}
	}
	return NO;
}

/*
cut_after_spaces: gets a string and finish the string after the first word by putting '\0'.
*/
void cut_after_spaces (char *str) {
	if(str==NULL)
		return;
	while(!isspace(*str) && *str!='\0')
		str++;
	*str='\0';
}
