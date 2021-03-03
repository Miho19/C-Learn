
CC = gcc
CFLAGS = -Wall -W -ansi -pedantic -g -o

SRC = main.c make_file_system.c 

main: $(SRC%.c=%.o)
	@echo "attempting to build"
	$(CC) $(CFLAGS) $@ $(SRC)
	./$@