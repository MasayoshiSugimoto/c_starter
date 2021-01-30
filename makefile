

build: .build/main.o
	gcc .build/main.o -o tictactoe

run: build
	./tictactoe

.build/main.o: .build src/main.c
	gcc -c src/main.c -o .build/main.o

.build:
	mkdir .build

.PHONY: clean

clean:
	rm -rf .build
