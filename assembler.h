/* include header that enables redundents */
#ifndef ASSEMBLER_H
#define ASSEMBLER_H
/*
first_encode_line: gets the line and it's number and do for it 'first pass': if there is a label, saves it. check how many lines will take and adds to IC and adds data if its data or string line.
*/
int first_encode_line (char *, int);

/*
second_encode_line: gets the line and it's number and do for it 'second pass': encode the commands to words, make list of entries and extracts.
*/
int second_encode_line (char *, int);

/*
advance IC if can and returns if succeed or not
*/
int advance_IC (int num);

/* returns IC - the instructions counter */
int get_IC ();

/* returns DC - the data counter */
int get_DC ();

/*
insert_inst: get an unsigned int that represents an instruction and insert it into the instructions list. returns 1 if succeed, 0 if not.
*/
int insert_inst (unsigned int);

/*
insert_data: get an int that represents a data and insert it into the data list. returns 1 if succeed, 0 if not.
*/
int insert_data (int);

/*
reset_data: reset all the data that the program saves: instructions, data, and symbols.
*/
void reset_data ();

/*
reset_IC: reset the IC counter and fetch it into the parameter pointer.
*/
void reset_IC (int *);

/*
to_base_32: gets an unsigned int and returns a 3 digits base 32 number that represents the number (the output is a string).
*/
char *to_base_32 (unsigned int);

/*
make_obj: get the file_name and number of instructions and create a object file for it. Returns 1 if succeed and 0 if don't.
*/
int make_obj (char *, int);

/*
add_entry: adding the label and val in the input to the entry list. Returns 1 if succeed and 0 if don't.
*/
int add_entry (char *, int);

/*
free_entry: frees the data in the array of entries.
*/
void free_entry ();

/*
make_ent: get the file_name and create a entries file for it. Returns 1 if succeed and 0 if don't.
*/
int make_ent (char *);

/*
add_external: adding the label string (first argument) and offset from the IC (second argument) in the input to the external list. Returns 1 if succeed and 0 if don't.
*/
int add_external (char *, int);

/*
free_external: frees the data of the externals.
*/
void free_external ();

/*
make_ext: get the file_name and create a externals file for it. Returns 1 if succeed and 0 if don't.
*/
int make_ext (char *file_name);
#endif

