#CC-FLAGS = -std=c11 -g -Wall -Wextra -fpic
#LD-FLAGS = -g

CC-FLAGS = -std=c11 -Wall -Wextra -O3 -Ofast
LD-FLAGS = -O3 -Ofast

main.ex: main.o core.o log.o mc.o core.o random.o simulate.o -lpthread
	gcc $(LD-FLAGS) -o $@ $^

test.ex: test.o mc.o random.o
	gcc $(LD-FLAGS) -o $@ $^

lib_doko.so: mc.o random.o
	gcc $(LD-FLAGS) -shared -o $@ $^

%.o: %.c
	gcc $(CC-FLAGS) -c -o $@ $<

clean:
	-rm *.o
	-rm *.ex
	-rm *.so

