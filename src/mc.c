#include <string.h>

#include "mc.h"
#include "random.h"
#include "simulate.h"

int doko_sort_and_check(doko_card_info_t* card_info) {
    doko_card_info_t card_info_copy = *card_info;
    doko_likeliness_t sum_of_likeliness[36];

    /* for sorting we use a form of radix sort (counting sort) that is very
     * fast if the keys are only in a small range.  We want to sort with the
     * sum of likeliness per card as key, which takes values between 0 = 3*0 and
     * 15 = 3*5. Therefore the radix is 16. For the count array we want an array
     * with 17 places, so that we can calculate the cumulative sum more easily.
     * */

#define RADIX 16
    uint8_t count_array[RADIX + 1] = {0};

    for (doko_count_t c = 0; c < card_info_copy.cards_left; c++) {
        /* check if card is valid */
        if (!DOKO_CARD_VALID(card_info_copy.ids[c])) {
            return -1;
        }

        uint8_t sum = 0;
        /* check if all the scores are valid (0,3,4 or 5) and calculate the sum
         * of scores for this card */
        for (uint8_t p = 0; p < 3; p++) {
            /* check if all scores are 0,3,4 or 5 */
            uint8_t s = card_info_copy.likeliness[c][p];
            switch (s) {
            case 0:
            case 3:
            case 4:
            case 5:
                sum += s;
                break;
            default:
                return -1;
            }
        }

        /* the sum of scores is the key for sorting. We store it for later use
         * and increment the coresponding radix counter in count_array */
        sum_of_likeliness[c] = sum;
        count_array[sum + 1]++;
    }

    /* caclulate cumulative sum. After this, we can index into the radix
     * counter array by our sorting key and get the position at which the
     * element must go to be sorted */
    for (uint8_t i = 1; i < RADIX + 1; i++) {
        count_array[i] += count_array[i - 1];
    }

    /* place each element from the copy at the right destination */
    for (doko_count_t c = 0; c < card_info_copy.cards_left; c++) {
        uint8_t d = count_array[sum_of_likeliness[c]]++;
        card_info->ids[d] = card_info_copy.ids[c];
        memcpy(card_info->likeliness[d], card_info_copy.likeliness[c],
               sizeof(card_info_copy.likeliness[c]));
    }
    return 0;
}

void doko_mc_sample(doko_game_info_t* dest, doko_card_info_t const* card_info,
                    doko_random_state_t* random_state) {
    /* use a slot based sampling. Each player gets as many slot groups as he
     * needs cards. Each slot group consist of three slots.
     *
     * After that each card is randomly put to an open slot of any player. If
     * the card has restrictions some slots may be blocked for this card.  E.g.
     * if the card has likeliness {0,5,4} all slots of player 0 will be blocked
     * as well as a third of all slots of player 2.  If the likeliness is
     * nonzero for a player, at least one slot must be left open for that
     * player. */

    doko_card_info_t card_info_copy = *card_info;
    uint8_t random[4];
    static uint8_t const likeliness_to_slots[] = {0, 0, 0, 1, 2, 3};

    for (doko_count_t c = 0; c < card_info_copy.cards_left; c++) {
        /* deal card at index c */
        random[0] = 0;
        /* block slots according to scores */
        for (uint8_t p = 0; p < 3; p++) {
            random[p + 1] =
                random[p] +
                card_info_copy.player_left[p] *
                    likeliness_to_slots[card_info_copy.likeliness[c][p]];
        }
        uint8_t r = doko_random_uint8(random_state, random[3]);
        for (uint8_t p = 0; p < 3; p++) {
            if (/*random[p] <= r &&*/ r < random[p + 1]) {
                dest->player_cardsets[p + 1] |=
                    DOKO_CARDSHIFT(card_info_copy.ids[c]);
                card_info_copy.player_left[p]--;
                break;
            }
        }
    }
}
