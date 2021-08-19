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


[MakeFile](https://stackoverflow.com/questions/30573481/how-to-write-a-makefile-with-separate-source-and-header-directories)
