main: main.o game.o
	gcc -g -o $@ $^

main.o: main.c game.h
	gcc -g -c -o $@ $<

game.o: game.c game.h
	gcc -g -c -o $@ $<
