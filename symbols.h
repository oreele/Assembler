
#ifndef SYMBOLS_H
#define SYMBOLS_H

/*
add_symbol: add a symbol. gets the name of the symbol, the value of the symbol, and int that sign if its external and int that sign if its symbol of operation statement.
returns 1 if succeed to add and 0 otherwise.
*/
int add_symbol (char *, int, int, int);
/*
search_symbol: gets a string and a pointer to integer and return 1 if found a symbol named as the string. If found, the integer pointer turns to point on the value of the symbol (if the pointer isnot null).
*/
int search_symbol (char *, int *);
/*
change_address_data_symbols: gets a number no and add the address of each data symbol that number.
*/
void change_address_data_symbols (int);
/*
is_external: gets a string of a symbol and returns if it is external (1 for yes, 0 for NO).
ASSUMPTION: the string is a name of symbol.
*/
int is_external (char *);
/*
free_list: free the memory of the list nodes and strings
*/
void free_list ();
/*
randomize_label: returns a randomized internal symbol address.
*/
int randomize_label ();
#endif
