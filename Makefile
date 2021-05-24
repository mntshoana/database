flags = -Wpointer-arith -Wall -Wextra -pedantic -std=c99 -g3

.PHONY: db
all: clean bin db test
bin:
	mkdir -p bin
db: bin src/main.c src/mySQLDB.c
	$(CC) src/main.c src/mySQLDB.c  $(flags) -o bin/mySQLDB.o
test: bin src/mySQLDB.c test/test.c
	$(CC) test/test.c src/mySQLDB.c    $(flags) -o bin/test.o
clean:
	rm -dfr bin
