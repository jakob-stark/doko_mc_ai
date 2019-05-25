main: main.o utility.o game.o
	g++ -o $@ $^

main.o: main.cpp utility.hpp
	g++ -c -o $@ $<

utility.o: utility.cpp utility.hpp
	g++ -c -o $@ $<

game.o: game.cpp utility.hpp game.hpp
	g++ -c -o $@ $<
