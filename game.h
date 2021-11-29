#ifndef GAME_H
#define GAME_H

#include <stdint.h>

CardId GetBestCard( GameInfo* game_info, CardInfo* card_info );
void ExecuteMove( GameInfo* game_info, CardInfo* card_info, CardId card );

#endif

