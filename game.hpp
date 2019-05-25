
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
		PlayerArray players;
		uint8_t next;
		uint8_t cards_left;
		Score trickscore;
		Suit tricksuit;
		Card trickwinnercard;
		uint8_t trickwinner;

		GameInfo(const PlayerArray& init_players, uint8_t init_next, uint8_t init_cards_left,
				Score init_trickscore, Suit init_tricksuit, Card init_trickwinnercard,
				uint8_t init_trickwinner);
		
#ifdef TEST_GAMES
		static GameInfo test_game1;
		static GameInfo test_game2;
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
