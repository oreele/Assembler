/*
this file is responsible for the first pass of the compilation. In the first pass we create table of symbols and thier addresses, add the data and count how many lines the object file will contain.
*/

#include "assembler.h"
#include "utilities.h"
#include "symbols.h"
#include "consts.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>



#define NO_ARGUMENTS	1 /* symbols the statement has no argument */
#define ONE_ARGUMENT	2 /* symbols the statement has one argument */
#define TWO_ARGUMENTS	3 /* symbols the statement has two argument */
#define TWO_REGISTERS	2 /* symbols the statement has two argument of type register */


int decode_str (char *, int);
int decode_data (char *, int);
int statesize (char *line, int *size, int line_num);
int analyze_argument (char *argument);


/*
first_encode_line: gets the line and it's number and do for it 'first pass': if there is a label, saves it. check how many lines will take and adds to IC and adds data if its data or string line.
*/
int first_encode_line (char *line, int line_num) {
	char *label; /* saves label if exists */
	int label_flag = NO /* saves if there is a label */, size /* number of lines the command will take */;
	if(fetch_label(&line, &label)) /* fetch the label if there is */
		label_flag = YES; /* if there is a label */
	/* now 'label' contains the label and line contains the command after it */
	delete_spaces(&line); /* delete spaces in the start of line */
	switch(get_line_type(&line)) { /* detect the type of line */
		case ERR_ST: /* there is an error */
			fprintf(stderr, "line %d: There is no command such as '%s'\n", line_num, line);
			return FAILURE;
			break;
		case EMPTY_ST: /* the line is empty */
			if(label_flag) { /* empty line cannot contain a label */
				fprintf(stderr, "line %d: an empty or comment row cannot include a label\n", line_num);
				return FAILURE;
			} else return SUCCESS;
			break;
		case DATA_ST: /* .data line */
			if(label_flag) /* if there is a label adds symbol*/
				if(!add_symbol(label, get_DC(), NO, NO))
					return FAILURE;
			if(!decode_data(line, line_num)) /* decodes the data and saves in the data segment */
				return FAILURE; /* failed decoding the data */
			break;
		case STR_ST: /* .string line */
			if(label_flag) /* if there is a label adds symbol*/
				if(!add_symbol(label, get_DC(), NO, NO))
					return FAILURE;
			if(!decode_str(line, line_num)) /* decodes the string and saves in the data segment */
				return FAILURE; /* failed decoding */
			break;
		case EXT_ST: /* .extern line */
			if(label_flag) { /* cannot contain label */
				fprintf(stderr, "line %d: extern statment cannot include label\n", line_num);
				return FAILURE;
			} else {
				if(!add_symbol(getword(&line), DEFAULT_ADDRESS, YES, MAYBE)) /* adds the label */
					return FAILURE;
				if(!isempty(line)) { /* too many parameters */
					fprintf(stderr, "line %d: too many parameters\n", line_num);
					return FAILURE;
				}
			}
			break;
		case ENT_ST: /* .entry line */
			if(label_flag) {
				fprintf(stderr, "line %d: entry statment cannot include label\n", line_num);
				return FAILURE;
			}
			break;
		case ACTION_ST: /* any action command */
			if(label_flag) { /* adds label if exists */
				if(!add_symbol(label, get_IC(), NO, YES))
					return FAILURE;
			}
			if(!statesize(line, &size, line_num)) /* calculates the statement size of words */
				return FAILURE; /* failed calculate */
			advance_IC(size); /* adds to IC size */
			break;
	}
	return SUCCESS;
}

/*
statesize: calculates the size in words that statement is. the statement in line. the result is written in *size. returns 1 if succeed
*/
int statesize (char *line, int *size, int line_num) { /* returns number of words the statement in line is */
	int first_arg, second_arg; /* saves the type of the arguments */
	char *first, *second; /* saves the first and second arguments */
	if(!fetch_arguments(line, &first, &second, line_num)) /* fetch the arguments */
		return FAILURE;
	if(first==NULL) /* there is no arguments */
		*size=NO_ARGUMENTS;
	else if(second==NULL) /* there is one argument */
		*size=ONE_ARGUMENT;
	else	*size=TWO_ARGUMENTS; /* there is two arguments */
	
	if(first!=NULL)
		first_arg = analyze_argument(first); /* analyze the type of the first argument */
	if(second!=NULL)
		second_arg = analyze_argument(second); /* analyze the type of the second argument */
	if(first && second && first_arg == REG_ARG && second_arg == REG_ARG)
		*size = TWO_REGISTERS; /* if the two arguments are registers */
	return SUCCESS;
}

/*
analyze_argument: gets the argument and returns the type of it.
*/
int analyze_argument (char *argument) {
	if(*argument=='#') /* first char is # */
		return IM_ARG;
	if(strncmp(argument, "***", strlen("***"))==0) /* starts with *** */
		return LABEL_ARG;
	if(strncmp(argument, "**", strlen("**"))==0) /* starts with ** */
		return IM_ARG;
	if(*argument=='*') /* starts with * */
		return REG_ARG;
	if(is_register(argument))
		return REG_ARG;
	return LABEL_ARG; /* if its not any of the previous it should be a label */
}

/*
decode_data: saves data in data segment and returns 1 if succeeded. Also gets the line num to use in case of error.
*/
int decode_data (char *args, int line_num) {
	int num /* temporary variable that saves the numeric value */, comma_flag=NO /* saves if there should be a comma before an additional number */;
	if(isempty(args))
		return FAILURE; /* not enough parameters */
	while(*args!='\0') {
		delete_spaces(&args); /* skip spaces from the start */
		if(*args=='\0') /* reached to end */
			return SUCCESS;
		if(comma_flag) {
			if(*args!=',') { /* the comma must be there */
				fprintf(stderr, "line %d: missing comma\n", line_num);
				return FAILURE; /* missing comma */
			}
			args++;
			delete_spaces(&args); /* there can be more spaces after the comma */
			if(isempty(args)) { /* there had a comma but not a number */
				fprintf(stderr, "line %d: surplus comma\n", line_num);
				return FAILURE; /* surplus comma */
			}
		}
		if(!isdigit(*args) && *args!='+' && *args!='-')
			return FAILURE; /* the parameter is not proper to be a number */
		num = atoi(args); /* numb gets the number */
		do { /* skip on the number */
			args++;
		} while(isdigit(*args));
		if(!insert_data(num)) /* trying to insert the data */
			return FAILURE;
		comma_flag = YES; /* there had a number so we need a comma before another number */
	}
	return SUCCESS;
}

/*
decode_str: saves string args in data segment and returns 1 if succeeded. saves '\0' at the end. Also gets the line num to use in case of error.
*/
int decode_str (char *args, int line_num) {
	char *str; /* the string without brackets */
	delete_spaces(&args);
	if(*args != '"') { /* string has to start with bracket */
		fprintf(stderr, "line %d: a string should start with bracket\n", line_num);
		return FAILURE;
	}
	str = ++args; /* after the bracket the string should start */
	while(*args != '"' && *args != '\0') /* the string didn't finished */
		args++;
	if(*args == '"') /* end of the string. '\0' sign the string finished */
		*args = '\0';
	else {
		fprintf(stderr, "line %d: there is no closing bracket\n", line_num);
		return FAILURE; /* there is no closing bracket */
	}
	args++; /* put 'args' after the string */
	if(!isempty(args)) {
		fprintf(stderr, "line %d: there is more parameters than needed\n", line_num);
		return FAILURE; /* there is more parameters than needed */
	}
	while(*str) /* passing on the string and insert to the data segment */
		if(!insert_data(*(str++))) {
			fprintf(stderr, "line %d: cannot store the data\n", line_num);
			return FAILURE; /* there is an error with insertion */
		}
	if(!insert_data('\0')) { /* insert 0 in the end */
		fprintf(stderr, "line %d: cannot store the data\n", line_num);
		return FAILURE; /* there is an error with insertion */
	}
	return SUCCESS;
}
