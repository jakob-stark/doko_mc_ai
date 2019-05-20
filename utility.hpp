
#ifndef _UTILITY_HPP_
#define _UTILITY_HPP_

#include <cstdint>
#include <string>

typedef uint8_t Score;

class CardSet;

class Suit {
	private:
		uint8_t suit;
		Suit( uint8_t init );

		static const std::string names[6];
		static const CardSet sets[6];
	public:
		bool operator == ( const Suit& op ) const;
		bool operator != ( const Suit& op ) const;
	
		const std::string& GetName() const;
		const CardSet& GetSet() const;

		static const Suit none;
		static const Suit club;
		static const Suit spade;
		static const Suit heart;
		static const Suit diamond;
		static const Suit trump;
};

class Card {
	private:
		uint8_t card;

		static const Suit suits[48];
		static const std::string names[48];
		static const Score values[48];

	public:
		Card( uint8_t init );
		Card( std::string name );

		bool IsHighBlock() const;
		bool IsLowBlock() const;
		bool IsNone() const;
		const Suit& GetSuit() const;

		bool operator == ( Card& op ) const;
		bool operator != ( Card& op ) const;

		Card LowBlock() const;
		Card HighBlock() const;

		uint8_t GetIndex() const;
		const std::string& GetName() const;
		const Score& GetValue() const;

		static const Card none;
		static const Card club_nine;
		static const Card club_king;
		static const Card club_ten;
		static const Card club_ace;
		static const Card spade_nine;
		static const Card spade_king;
		static const Card spade_ten;
		static const Card spade_ace;
		static const Card heart_nine;
		static const Card heart_king;
		static const Card heart_ace;
		static const Card diamond_nine;
		static const Card diamond_king;
		static const Card diamond_ten;
		static const Card diamond_ace;
		static const Card diamond_jack;
		static const Card heart_jack;
		static const Card spade_jack;
		static const Card club_jack;
		static const Card diamond_queen;
		static const Card heart_queen;
		static const Card spade_queen;
		static const Card club_queen;
		static const Card heart_ten;
		static const Card club_nine_h;
		static const Card club_king_h;
		static const Card club_ten_h;
		static const Card club_ace_h;
		static const Card spade_nine_h;
		static const Card spade_king_h;
		static const Card spade_ten_h;
		static const Card spade_ace_h;
		static const Card heart_nine_h;
		static const Card heart_king_h;
		static const Card heart_ace_h;
		static const Card diamond_nine_h;
		static const Card diamond_king_h;
		static const Card diamond_ten_h;
		static const Card diamond_ace_h;
		static const Card diamond_jack_h;
		static const Card heart_jack_h;
		static const Card spade_jack_h;
		static const Card club_jack_h;
		static const Card diamond_queen_h;
		static const Card heart_queen_h;
		static const Card spade_queen_h;
		static const Card club_queen_h;
		static const Card heart_ten_h;

};

typedef Card CardList[48];

class CardSet {
	private:
		uint64_t cardset;
	public:
		CardSet( uint64_t init );
		CardSet( const Card& init );
		CardSet();
		void Add( const Card& op );
		void Add( const CardSet& op );
		void Remove( const Card& op );
		void Remove( const CardSet& op );
		void Reset();
		bool Have( const Card& op ) const;
		bool Have( const Suit& op ) const;
		bool IsEmpty() const;

		CardSet CompressLow() const;
		CardSet CompressHigh() const;
		CardSet MaskSuit( const Suit& op ) const;
		CardSet MaskSuitAndTrump( const Suit& op ) const;

		uint64_t GetSet() const;
		uint8_t GetList( CardList& list) const;
		std::string GetInfo() const;

		static const CardSet empty;
		static const CardSet all;
		static const CardSet low_block;
		static const CardSet high_block;
};

class CardSetIt {
	private:
		const CardSet cardset;
		uint8_t pos;
	public:
		CardSetIt( const CardSet& init );
		bool GoOn();
		void Increase();
		Card GetCard();
};

class Player {
	private:
		CardSet cardset;
		Player* next;
		Score score;
		const bool is_re;
	public:
		Player( const CardSet& cardset_init, const Score& score_init=0, Player* next_init=nullptr );
		CardSet GetLegalCards( const Suit& tricksuit ) const;
		void Play( const Card& card );

		Player* GetNext() const;
		void SetNext( Player* op );

		void AddToScore( const Score& op );
		const Score& GetScore() const;

		bool IsRe() const;
};



/*
class RandomGame: {
	public:
		RandomGame( uint8_t cards_left, CardSet Trick, CardSet Players[4], Player Next, Player Winner )


// stores trick information in the upper bits of set
class Trick: public CardSet {
  public:
	Trick();
	Trick( card init );


	//uint8_t GetCount();
	score GetScore();
	player GetNext();
	player GetWinner();
	void SetNext(player p);
	void SetWinner(player p);
	void SetWinnerToNext();

	suit GetTrickSuit();

	bool IsHigher( card c );

	//bool IsFull();

	void Empty();
	
	void Play(card_i e);	//track counter and specify winner


	//the following functions are not fast enought for tree search only use them rarely
	
	void PrintInfo();
};

struct Game {
	Game();

	Trick trick;
	CardSet player_sets[4];
	score player_scores[4];
	
	uint8_t count;
	void Play(card_i i);

//	uint32_t zorbrist;
//	uint32_t zor_random[4][48];

	void PrintInfo();

	static Game TestGame( int test);
};

struct Game2 {
	Game2();
	Game2(Game game);
	Trick trick;
	CardSet player_sets[4];
	uint8_t count;
	void Play(card_i i);

	void PrintInfo();
};

*/

#endif
