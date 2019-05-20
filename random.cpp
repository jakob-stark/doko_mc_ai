#include "random.hpp"
#include "utility.hpp"

std::ranlux24_base RandomGenerator::gen;

RandomGame::RandomGame( Player* init_beginner, uint8_t init_cards_left, Score init_trickscore,
		Suit init_tricksuit, Card init_trickwinnercard, Player* init_trickwinner ):
	beginner(init_beginner),
	cards_left(init_cards_left),
	trickscore(init_trickscore),
	tricksuit(init_tricksuit),
	trickwinnercard(init_trickwinnercard),
	trickwinner(init_trickwinner) {
}

void RandomGame::Play() {
	while ( cards_left > 0 ) {
		CardSet hand = beginner->GetLegalCards( tricksuit );
		CardList handlist;
		uint8_t len = hand.GetList( handlist );

		Card card = handlist[RandomGenerator::gen() % len];

		// remove Card from Player
		beginner->Play( card );
		--cards_left;

		// add Value to score
		trickscore += card.GetValue();

		if ( tricksuit == Suit::none ) {
			// set new tricksuit if it is a new trick
			tricksuit = card.GetSuit();
			trickwinnercard = card;
			trickwinner = beginner;
		} else {
			// set new winner parameters if new card is winning the trick
			if ( card.GetSuit() == tricksuit || card.GetSuit() == Suit::trump ) {
				if ( card.LowBlock().GetIndex() > trickwinnercard.LowBlock().GetIndex() || 
						( card.LowBlock() == Card::heart_ten && cards_left > 4 ) ) {
					trickwinnercard = card;
					trickwinner = beginner;
				}
			}
		}

		if ( cards_left % 4 == 0 ) {
			trickwinner->AddToScore(trickscore);
			trickscore = 0;
			tricksuit = Suit::none;
			beginner = trickwinner;
		} else {
			beginner = beginner->GetNext();
		}
	}
}


