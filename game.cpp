#include <iostream>
#include "game.hpp"
#include "utility.hpp"

std::ranlux24_base RandomGenerator::gen;

////////////////////////////////////////////////
// GameInfo
//

GameInfo::GameInfo( Player* init_next, uint8_t init_cards_left, Score init_trickscore,
		Suit init_tricksuit, Card init_trickwinnercard, Player* init_trickwinner ):
	next(init_next),
	cards_left(init_cards_left),
	trickscore(init_trickscore),
	tricksuit(init_tricksuit),
	trickwinnercard(init_trickwinnercard),
	trickwinner(init_trickwinner) {
}

#ifdef TEST_GAMES

extern Player test_game1_player1;
Player test_game1_player4 = Player(	std::string("Player 4"),
									CardSet(0b000000000000000000000000000000000000000000000000ul),
									0,
									&test_game1_player1,
									true );
Player test_game1_player3 = Player( std::string("Player 3"),
									CardSet(0b000000000000000000000000000000000000000000000000ul),
									0,
									&test_game1_player4,
									true );
Player test_game1_player2 = Player( std::string("Player 2"),
									CardSet(0b000000000000000000000000000000000000000000000000ul),
									0,
									&test_game1_player3,
									true );
Player test_game1_player1 = Player( std::string("Player 1"),
									CardSet(0b000000000000000000000000000000000000000000000000ul),
									0,
									&test_game1_player2,
									true );

GameInfo GameInfo::test_game1 = GameInfo( &test_game1_player1, 48, 0, Suit::none, Card::none, nullptr);

#endif

////////////////////////////////////////////////
// Game
//

Game::Game(GameInfo* init): game_info(init) {
}

void Game::Play( Card card ) {
	// remove Card from Player (and set Re if suitable)
	game_info->next->Play( card );
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
		game_info->trickwinner->AddToScore(game_info->trickscore);
		game_info->trickscore = 0;
		game_info->tricksuit = Suit::none;
		game_info->next = game_info->trickwinner;
	} else {
		game_info->next = game_info->next->GetNext();
	}
}

Score Game::GetResult() {
	Score result(0);
	Player* p1 = game_info->next;
	do {
		if ( p1->IsRe() ) {
			result += p1->GetScore();
		}
		p1 = p1->GetNext();
	} while ( p1 != game_info->next );
	return result;
}

////////////////////////////////////////////////
// InteractiveGame
//

InteractiveGame::InteractiveGame(GameInfo* init): Game(init) {
}

void InteractiveGame::PlayNext() {
	if ( game_info->next->IsHuman() ) {
		PlayHuman();
	} else {
		PlayMachine();
	}
}

void InteractiveGame::PlayHuman() {
	using namespace std;
	string input;
	cout << game_info->next->GetName() << " to move:" << endl;
	cin >> input;
	Card card(input);
	cout << game_info->next->GetName() << " plays " << card.GetName() << endl;

	// performing move
	Play(card);
}

void InteractiveGame::PlayMachine {
	int N = 10000;
	CardSet hand = game_info->next->GetLegalCards( game_info->tricksuit )
	CardList handlist;
	uint8_t len = hand.GetList(handlist);

	for ( int i = 0; i < N; i++ ) {
	}
		

}

////////////////////////////////////////////////
// Random Game
//

RandomGame::RandomGame(GameInfo* init): Game(init) {
}

void RandomGame::Simulate() {
	while ( game_info->cards_left > 0 ) {
		CardSet hand = game_info->next->GetLegalCards( game_info->tricksuit );
		CardList handlist;
		uint8_t len = hand.GetList( handlist );

		Card card = handlist[RandomGenerator::gen() % len];

		Play(card);
	}
}


