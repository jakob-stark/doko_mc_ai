#ifndef GAME_H
#define GAME_H

#include "core.h"
#include "mc.h"

CardId GetBestCard( const GameInfo* game_info, const CardInfo* card_info );
void ExecuteMove( GameInfo* game_info, CardInfo* card_info, CardId card );

#endif

