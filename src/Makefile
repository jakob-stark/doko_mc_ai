CC-FLAGS = -std=c11 -g -Wall -Wextra -fpic
LD-FLAGS = -g

main.ex: main.o game.o log.o mc.o core.o random.o
	gcc $(LD-FLAGS) -o $@ $^

test.ex: test.o mc.o random.o
	gcc $(LD-FLAGS) -o $@ $^

lib_doko.so: mc.o random.o
	gcc $(LD-FLAGS) -shared -o $@ $^

main.o: main.c game.h
	gcc $(CC-FLAGS) -c -o $@ $<

game.o: game.c game.h
	gcc $(CC-FLAGS) -c -o $@ $<

%.o: %.c
	gcc $(CC-FLAGS) -c -o $@ $<

random.c: random.h

core.c: core.h random.h

mc.c: mc.h core.h random.h

mc.h: core.h

clean:
	-rm *.o
	-rm *.ex
	-rm *.so

