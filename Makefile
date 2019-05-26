main: main.o game.o
	gcc -o $@ $^

main.o: main.c game.h
	gcc -c -o $@ $<

game.o: game.c game.h
	gcc -c -o $@ $<
