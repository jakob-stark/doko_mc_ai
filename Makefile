main: main.o utility.o random.o
	g++ -o $@ $^

main.o: main.cpp utility.hpp
	g++ -c -o $@ $<

utility.o: utility.cpp utility.hpp
	g++ -c -o $@ $<

random.o: random.cpp utility.hpp random.hpp
	g++ -c -o $@ $<
