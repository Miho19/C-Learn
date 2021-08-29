CC = gcc
CFLAGS = -W -Wall -ansi -pedantic -g

EXE := main

all: $(EXE)

$(EXE): main.o
	$(CC) $^ -o $@
	

main.o: main.c
	$(CC) $(CFLAGS) -c $< -o $@
	
clean:
	@$(RM) -rv main.o main


