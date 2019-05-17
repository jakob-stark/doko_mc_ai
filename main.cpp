
#include <iostream>
#include "utility.hpp"

int main( int argc, const char* argv[] ) {
	Player N = Player::none;
	Player X = Player::player2;
	std::cout << N.GetName();
	//uint8_t k = 2;
	//eN += k;
	std::cout << "\n" << (int)((uint8_t)X) << "\n";
	return 0;
}

