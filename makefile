main: main.o utilities.o firstpass.o secondpass.o symbols.o assembler.o
	gcc -g -Wall -ansi -pedantic main.o utilities.o firstpass.o secondpass.o symbols.o assembler.o -o main
main.o: main.c assembler.h symbols.h consts.h
	gcc -c -Wall -ansi -pedantic main.c -o main.o
secondpass.o: secondpass.c assembler.h consts.h utilities.h symbols.h
	gcc -c -Wall -ansi -pedantic secondpass.c -o secondpass.o
firstpass.o: firstpass.c assembler.h utilities.h symbols.h consts.h
	gcc -c -Wall -ansi -pedantic firstpass.c -o firstpass.o
utilities.o: utilities.c utilities.h assembler.h consts.h
	gcc -c -Wall -ansi -pedantic utilities.c -o utilities.o
symbols.o: symbols.c symbols.h consts.h assembler.h
	gcc -c -Wall -ansi -pedantic symbols.c -o symbols.o
assembler.o: assembler.c consts.h
	gcc -c -Wall -ansi -pedantic assembler.c -o assembler.o
