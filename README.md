# C Program Help

## Compilation Example
```bash
gcc -W -Wall -ansi -pedantic -O2 -g -o main main.c
```


## valgrind
```bash
valgrind --leak-check=full \
         --show-leak-kinds=all \
         --track-origins=yes \
         --verbose \
         --log-file=valgrind-out.txt \
         ./executable exampleParam1
```
[source](https://stackoverflow.com/questions/5134891/how-do-i-use-valgrind-to-find-memory-leaks)



## MakeFile
```
CC = gcc
CFLAGS = -Wall -W -ansi -pedantic -g
CPPFLAGS := -Iinclude -MMD -MP
LDFLAGS := -Llib
LDLIBS := -lncurses

SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := .

EXE := $(BIN_DIR)/main

SRC := $(wildcard $(SRC_DIR)/*.c)

OBJ := $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

.PHONY: all clean check

all: $(EXE)

check: $(EXE)
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=valgrind-out.txt ./$(EXE)

$(EXE):	$(OBJ) | $(BIN_DIR)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $@

clean:
	@$(RM) -rv $(OBJ_DIR) main valgrind-out.txt

-include $(OBJ:.o=.d)
```
[source](https://stackoverflow.com/questions/30573481/how-to-write-a-makefile-with-separate-source-and-header-directories)
