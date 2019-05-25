#include <iostream>
#include "game.hpp"
#include "utility.hpp"

std::ranlux24_base RandomGenerator::gen;

////////////////////////////////////////////////
// GameInfo
//

GameInfo::GameInfo(const PlayerArray& init_players, uint8_t init_next, uint8_t init_cards_left,
				Score init_trickscore, Suit init_tricksuit, Card init_trickwinnercard,
				uint8_t init_trickwinner):
	players(init_players),
	next(init_next),
	cards_left(init_cards_left),
	trickscore(init_trickscore),
	tricksuit(init_tricksuit),
	trickwinnercard(init_trickwinnercard),
	trickwinner(init_trickwinner) {
}

#ifdef TEST_GAMES

PlayerArray test_game1_players = {
	Player(	std::string("Player 1"), CardSet(0b000000000000000000000000000000000000000000000000ul), 0, true ),
	Player( std::string("Player 2"), CardSet(0b000000000000000000000000000000000000000000000000ul), 0, true ),
	Player( std::string("Player 3"), CardSet(0b000000000000000000000000000000000000000000000000ul), 0, true ),
	Player( std::string("Player 4"), CardSet(0b000000000000000000000000000000000000000000000000ul), 0, true )
};

GameInfo GameInfo::test_game1 = GameInfo( test_game1_players, 0, 48, 0, Suit::none, Card::none, 0);

// Test game with only club cards
PlayerArray test_game2_players = {
	Player(	std::string("Player 1"), CardSet(0b000001000000000000000000000001000000000000001000ul), 0, false ),
	Player( std::string("Player 2"), CardSet(0b000000000000000000000000000000000000000000000111ul), 0, false ),
	Player( std::string("Player 3"), CardSet(0b000000000000000000001010010000000000000000000000ul), 0, false ),
	Player( std::string("Player 4"), CardSet(0b010000000000000000000101000000000000000000000000ul), 0, false )
};

GameInfo GameInfo::test_game2 = GameInfo( test_game2_players, 0, 12, 0, Suit::none, Card::none, 0);
#endif

////////////////////////////////////////////////
// Game
//

Game::Game(GameInfo* init): game_info(init) {
}

void Game::Play( Card card ) {
	// remove Card from Player (and set Re if suitable)
	game_info->players[game_info->next].Play( card );
	--(game_info->cards_left);

	// add Value to score
	game_info->trickscore += card.GetValue();

	if ( game_info->tricksuit == Suit::none ) {
		// set new tricksuit if it is a new trick
		game_info->tricksuit = card.GetSuit();
		game_info->trickwinnercard = card;
		game_info->trickwinner = game_info->next;
	} else {
		// set new winner parameters if new card is winning the trick
		if ( card.GetSuit() == game_info->tricksuit || card.GetSuit() == Suit::trump ) {
			if ( card.LowBlock().GetIndex() > game_info->trickwinnercard.LowBlock().GetIndex() || 
					( card.LowBlock() == Card::heart_ten && game_info->cards_left > 4 ) ) {
				game_info->trickwinnercard = card;
				game_info->trickwinner = game_info->next;
			}
		}
	}

	if ( game_info->cards_left % 4 == 0 ) {
		game_info->players[game_info->trickwinner].AddToScore(game_info->trickscore);
		game_info->trickscore = 0;
		game_info->tricksuit = Suit::none;
		game_info->next = game_info->trickwinner;
	} else {
		game_info->next = (game_info->next + 1) % 4;
	}
}

Score Game::GetResult() {
	Score result(0);
	for ( uint8_t p = 0; p < 4; p++ ) {
		if ( game_info->players[p].IsRe() ) {
			result += game_info->players[p].GetScore();
		}
	}
	return result;
}

////////////////////////////////////////////////
// InteractiveGame
//

InteractiveGame::InteractiveGame(GameInfo* init): Game(init) {
}

void InteractiveGame::PlayNext() {
	if ( game_info->players[game_info->next].IsHuman() ) {
		PlayHuman();
	} else {
		PlayMachine();
	}
}

void InteractiveGame::PlayHuman() {
	using namespace std;
	string input;
	cout << game_info->players[game_info->next].GetName() << " to move:" << endl;
	cin >> input;
	Card card(input);
	cout << game_info->players[game_info->next].GetName() << " plays " << card.GetName() << endl;

	// performing move
	if ( game_info->players[game_info->next].Have(card) ) {
		Play(card);
	} else {
		Play(card.HighBlock());
	}
}

void InteractiveGame::PlayMachine() {
	using namespace std;
	int N = 100000;
	int Nlin = 100;
	CardSet hand = game_info->players[game_info->next].GetLegalCards( game_info->tricksuit );
	CardList handlist;
	uint8_t len = hand.GetList(handlist);
	double scores[48];
	for ( int pos = 0; pos < len; pos++ ) {
		scores[pos] = 0.0;
		for ( int i = 0; i < N; i++ ) {
			GameInfo tmp_game_info = *game_info;
			RandomGame tmp_game(&tmp_game_info);
			tmp_game.Play(handlist[pos]);
			tmp_game.Simulate();
			scores[pos] += tmp_game.GetResult();
		}
	}
	
	double max = 0.0;
	uint8_t max_pos = 0;
	double min = 1.0e100;
	uint8_t min_pos = 0;
	for ( uint8_t pos = 0; pos < len; pos++ ){
		if ( scores[pos] > max ) {
			max_pos = pos;
			max = scores[pos];
		}
		if ( scores[pos] < min ) {
			min_pos = pos;
			min = scores[pos];
		}
		cout << " " << handlist[pos].GetShortName() << " ";
		for ( int l = 0; l < Nlin; l++ ) {
			if ( scores[pos]/float(N)/60.0*float(Nlin) > (0.5+l) ) {
				cout << "*";
			}
		}
		cout << endl;
	}
	if ( game_info->players[game_info->next].IsRe() ||
			game_info->players[game_info->next].Have(Card::club_queen) ||
			game_info->players[game_info->next].Have(Card::club_queen_h) ) {
		cout << "Recommended card: " << handlist[max_pos].GetName() << endl;
	} else {
		cout << "Recommended card: " << handlist[min_pos].GetName() << endl;
	}
	PlayHuman();
}

////////////////////////////////////////////////
// Random Game
//

RandomGame::RandomGame(GameInfo* init): Game(init) {
}

void RandomGame::Simulate() {
	while ( game_info->cards_left > 0 ) {
		CardSet hand = game_info->players[game_info->next].GetLegalCards( game_info->tricksuit );
		CardList handlist;
		uint8_t len = hand.GetList( handlist );

		Card card = handlist[RandomGenerator::gen() % len];

		Play(card);
	}
}


