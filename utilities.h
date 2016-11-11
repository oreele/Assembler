#ifndef UTILITIES_H
#define UTILITIES_H
/*
get_line_type: gets a line and returns its type according to the first word.
*/
int get_line_type (char **);

/*
delete_spaces: gets pointer to string and set it point after the spaces.
*/
void delete_spaces(char **);
/*
skipword: gets a pointer to string and cut there the first word (with spaces before)
*/
void skipword (char **);
/*
getword: returns a string of the first word in the input that allocated in the memory.
*/
char *getword (char **);
/*
isempty: check if str has just white spaces.
*/
int isempty (char *);
/*
fetch the command arguments from line from the first 2 parameters into the second and the third parameter. Gets also the line number in order to print errors. Returns 1 if succeeded and 0 otherwise.
*/
int fetch_arguments (char *, char **, char **, int);
/*
fetch_label: Get a string of line at the first argument. Returns if there is a label, put the label in the second parameter (if the second parameter isnot NULL), and set the first parameter to point after the label.
*/
int fetch_label (char **, char **);
/*
is_register: gets a string and returns if there is a register.
*/
int is_register (char *argument);
/*
cut_after_spaces: gets a string and finish the string after the first word by putting '\0'.
*/
void cut_after_spaces (char *str);
#endif

