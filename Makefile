CC-FLAGS = -std=c11 -g
LD-FLAGS = -pthread -g

main: main.o game.o
	gcc $(LD-FLAGS) -o $@ $^

main.o: main.c game.h
	gcc $(CC-FLAGS) -c -o $@ $<

game.o: game.c game.h
	gcc $(CC-FLAGS) -c -o $@ $<
