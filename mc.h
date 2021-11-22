#ifndef MC_H
#define MC_H

#include "core.h"

typedef struct {
	uint8_t cards_left;
	float scores[3][36];
	CardId ids[36];
	float sum[3];
	uint8_t one;
} CardInfo;

#endif

