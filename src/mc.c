#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "simulate.h"
#include "random.h"
#include "mc.h"

int sort_and_check( CardInfo* card_info ) {
    uint8_t c, p, i, s, sum;

    CardInfo card_info_copy = *card_info;
    uint8_t sum_of_scores[36];

    /* for sorting we use a form of radix sort (counting sort) that is very
     * fast if the keys are only in a small range.  We want to sort with the
     * sum of scores per card as key, which takes values between 0 = 3*0 and 15
     * = 3*5. Therefore the radix is 16. For the count array we want an array
     * with 17 places, so that we can calculate the cumulative sum more easily.
     * */
    #define RADIX 16
    uint8_t count_array[RADIX+1] = {0};

    for ( c = 0; c < card_info_copy.cards_left; c++ ) {
        /* check if card is valid */
        if ( !CARD_VALID(card_info_copy.ids[c]) ) {
            return -1;
        }

        sum = 0;
        /* check if all the scores are valid (0,3,4 or 5) and calculate the sum
         * of scores for this card */
        for ( p = 0; p < 3; p++ ) {
            /* check if all scores are 0,3,4 or 5 */
            switch ( s = card_info_copy.scores[c][p] ) {
                case 0: case 3: case 4: case 5:
                    sum += s;
                    break;
                default:
                    return -1;
            }
        }

        /* the sum of scores is the key for sorting. We store it for later use
         * and increment the coresponding radix counter in count_array */
        sum_of_scores[c] = sum;
        count_array[sum+1]++;
    }

    /* caclulate cumulative sum. After this, we can index into the radix
     * counter array by our sorting key and get the position at which the
     * element must go to be sorted */
    for ( i = 1; i < RADIX+1; i++ ) {
        count_array[i] += count_array[i-1];
    }

    /* place each element from the copy at the right destination */
    for ( c = 0; c < card_info_copy.cards_left; c++ ) {
        uint8_t d = count_array[sum_of_scores[c]]++;
        card_info->ids[d] = card_info_copy.ids[c];
        memcpy(card_info->scores[d], card_info_copy.scores[c],
            sizeof(card_info_copy.scores[c]));
    }
    return 0;
}

void mc_sample( GameInfo* dest, const CardInfo* card_info, uint32_t* random_state ) {
    /* use a slot based sampling. Each player gets as many slot groups as he
     * needs cards. Each slot group consist of three slots.
     *
     * After that each card is randomly put to an open slot of any player. If
     * the card has restrictions some slots may be blocked for this card.  E.g.
     * if the card has score {0,5,4} all slots of player 0 will be blocked as
     * well as a third of all slots of player 2.  If the score is nonzero for a
     * player, at least one slot must be left open for that player. */

    uint8_t c, p, r;
    //uint8_t nslots[3];      // number of slots of each player
    CardInfo ci = *card_info;
    uint8_t random[4];
    const uint8_t score_to_slots[] = {0,0,0,1,2,3};

    for ( c = 0; c < ci.cards_left; c++ ) {
        /* deal card at index c */
        random[0] = 0;
        /* block slots according to scores */
        for ( p = 0; p < 3; p++ ) {
            random[p+1] = random[p] +
                ci.player_left[p] * score_to_slots[ci.scores[c][p]];
        }
        r = RandomC(random_state, random[3]);
        for ( p = 0; p < 3; p++ ) {
            if ( /*random[p] <= r &&*/ r < random[p+1] ) {
                dest->player_cardsets[p+1] += CARDSHIFT(ci.ids[c]);
                ci.player_left[p]--;
                break;
            }
        }
    }
}

