
#ifndef _GAME_HPP_
#define _GAME_HPP_

#include <random>
#include "utility.hpp"

#define TEST_GAMES
#define SHOW_HISTOS

class RandomGenerator {
	public:
		static std::ranlux24_base gen;
};

class GameInfo{
	public:
		Player* next;
		uint8_t cards_left;
		Score trickscore;
		Suit tricksuit;
		Card trickwinnercard;
		Player* trickwinner;

		GameInfo(Player* init_beginner, uint8_t init_cards_left, Score init_trickscore,
				Suit init_tricksuit, Card init_trickwinnercard, Player* init_trickwinner);
		
#ifdef TEST_GAMES
		static GameInfo test_game1;
#endif
};

class Game {
	protected:
		GameInfo* game_info;
	public:
		Game(GameInfo* init);
		void Play( Card card );
		Score GetResult();
};

class InteractiveGame : public Game {
	public:
		InteractiveGame(GameInfo* init);
		void PlayNext();
		void PlayHuman();
		void PlayMachine();
};

class RandomGame : public Game {
	public:
		RandomGame(GameInfo* init);
		void Simulate();
};

#endif
