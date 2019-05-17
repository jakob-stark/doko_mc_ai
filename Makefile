main: main.o utility.o
	g++ -o $@ $^

main.o: main.cpp utility.hpp
	g++ -c -o $@ $<

utility.o: utility.cpp utility.hpp
	g++ -c -o $@ $<
