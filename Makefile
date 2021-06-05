flags = -Wpointer-arith -Wall -Wextra -pedantic -std=c99 -g3

.PHONY: db
all: clean bin db test
bin:
	mkdir -p bin
db: bin src/main.c src/mySQLDB.c src/storage.c
	$(CC) src/main.c src/mySQLDB.c src/storage.c $(flags) -o bin/mySQLDB.o
test: bin db test/tests.c test/testRequired.c
	$(CC) test/tests.c test/testRequired.c    $(flags) -o bin/test.o
clean:
	rm -dfr bin
