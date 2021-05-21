editor: src/mySQLDB.c
	$(CC) src/*.c -o bin/mySQLDB.o -Wpointer-arith -Wall -Wextra -pedantic -std=c99 -g3

