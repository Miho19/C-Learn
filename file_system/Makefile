
CC = gcc
CFLAGS = -Wall -W -ansi -pedantic -g -o

SRC = main.c run.c make_file_system.c user_functions.c 

main: $(SRC%.c=%.o)
	@echo "attempting to build"
	$(CC) $(CFLAGS) $@ $(SRC)
	
	./$@