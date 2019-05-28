#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "game.h"

int main(int argc, char ** argv) {
	srand(3);
	GameInfo g;
	g.player_cardsets[0] = 0ul;
	g.player_cardsets[1] = 0ul;
	g.player_cardsets[2] = 0ul;
	g.player_cardsets[3] = 0ul;

	CardInfo c = {
		.cards_left = 6,
		.scores = {
			{0.5, 0.5, 1.0, 0.0, 0.0, 0.0},
			{0.5, 0.5, 0.0, 1.0, 0.0, 0.0},
			{0.0, 0.5, 0.0, 0.0, 0.5, 1.0}
		 },
		.ids = {0,1,2,3,4,5},
		.metric_sum = {1.0,1.0,1.0}
	};

	clock_t start = clock();
	/*Do something*/
	for ( int i = 0; i < 1000000000; i++ ) {
		c.ids[8] = (uint8_t)i;
		MCSample( &g, &c );
	}
	clock_t end = clock();
	float seconds = (float)(end - start) / CLOCKS_PER_SEC;
	printf("%fs\n",seconds);


	printf("0x%x\n", g.player_cardsets[1]);
	printf("0x%x\n", g.player_cardsets[2]);
	printf("0x%x\n", g.player_cardsets[3]);
	return 0;
}
