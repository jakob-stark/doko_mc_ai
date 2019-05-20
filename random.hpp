
#ifndef _RANDOM_HPP_
#define _RANDOM_HPP_

#include <random>
#include "utility.hpp"

class RandomGenerator {
	public:
		static std::ranlux24_base gen;
};

class RandomGame {
	private:
		Player* beginner;
		uint8_t cards_left;
		Score trickscore;
		Suit tricksuit;
		Card trickwinnercard;
		Player* trickwinner;

	public:
		RandomGame( Player* init_beginner, uint8_t init_cards_left, Score init_trickscore,
				Suit init_tricksuit, Card init_trickwinnercard, Player* init_trickwinner );
		void Play();
};

#endif
