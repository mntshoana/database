Flags = -Wpointer-arith -Wall -Wextra -pedantic -std=c99 -g
Database: src/mySQLDB.c
	mkdir -p bin
	$(CC) src/*.c -o bin/mySQLDB.o $(flags)
	$(CC) src/mySQLDB.c test/*.c -o bin/test.o $(flags)