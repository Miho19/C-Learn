INCL = test.h
SRC = main.c test.c test2.c

OBJ = $(SRC:.c=.o)
EXE = main


cc = gcc
CFLAGS = -W -Wall -ansi -pedantic -O2

%.o: %.c
	$(CC) -c $(CFLAGS) $*.c

$(EXE): $(OBJ)
	$(CC) -o $(EXE) $(OBJ)



clean:
	rm -f $(OBJ) $(EXE) cire a.out



