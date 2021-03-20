CC = gcc
CFLAGS = -Wall -W -ansi -pedantic -g -o

SRC = main.c

main:	$(SRC%.c=%.o)
	$(CC) $(CFLAGS) $@ $(SRC)
	./$@