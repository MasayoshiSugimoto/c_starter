tictactoe: .build/main.o .build/game.o .build/ui.o
	gcc .build/main.o .build/game.o .build/ui.o -o tictactoe

run: build
	./tictactoe

.build/main.o: .build src/main.c src/game.h
	gcc -c src/main.c -o .build/main.o

.build/game.o: .build src/game.c
	gcc -c src/game.c -o .build/game.o

.build/ui.o: .build src/ui.c src/def.h
	gcc -c src/ui.c -o .build/ui.o

.build:
	mkdir .build

.PHONY: clean build

clean:
	rm -rf .build

build: tictactoe

