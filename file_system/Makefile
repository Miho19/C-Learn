CC = gcc
CFLAGS = -Wall -W -ansi -pedantic -g -I.
DEPS = test_file_system.h file_system_error.h

OBJ = main.o run.o make_file_system.o user_functions.o


%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) 

usf: $(OBJ)
	$(CC) -o  $@ $^ $(CFLAGS)
	./usf   




.PHONY: clean

clean:
	rm -rf *o usf