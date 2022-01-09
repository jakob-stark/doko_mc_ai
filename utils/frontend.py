#!/usr/bin/python3

import argparse
import sys
import json
from ctypes import *

class InputInfo(Structure):
    _fields_ = [('computer_player_id', c_byte),
                ('starting_player_id', c_byte),
                ('played_cards', c_byte * 48),
                ('played_cards_len', c_byte),
                ('computer_player_hand', c_byte * 12),
                ('computer_player_hand_len', c_byte)]

cardnames = ('cn','ck','ct','ca','sn','sk','st','sa','hn','hk','ha','dn','dk','dt','da',
             'dj','hj','sj','cj','dq','hq','sq','cq','ht')
convertd = {k : i*2 for i, k in enumerate(cardnames)}
rconvertd = { i*2 : k for i, k in enumerate(cardnames)}
rconvertd.update( {i*2+1 : k for i,k in enumerate(cardnames)})

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('-l','--lib', default='./libdoko.so',
                        help='path to the libdoko shared library')
    parser.add_argument('-j','--json', type=str,
                        help='input json string')
    args = parser.parse_args()

    libdoko = CDLL(args.lib)
    GetBestCard = libdoko.GetBestCard
    GetBestCard.argtypes = [POINTER(InputInfo)]
    GetBestCard.restype = c_byte

    line = args.json
    # for line in map(str.strip, sys.stdin):
    if line:
        try:
            json_info = json.loads(line)
            input_info = InputInfo()
            input_info.computer_player_id = int(json_info['computer_player_id'])
            input_info.starting_player_id = int(json_info['starting_player_id'])
            input_info.played_cards_len = len(list(json_info['played_cards']))
            for i, c in enumerate(list(json_info['played_cards'])):
                if convertd[c] in input_info.played_cards[:i]:
                    input_info.played_cards[i] = convertd[c] + 1
                else:
                    input_info.played_cards[i] = convertd[c]
            input_info.computer_player_hand_len = len(list(json_info['computer_player_hand']))
            for i, c in enumerate(list(json_info['computer_player_hand'])):
                if ( convertd[c] in input_info.computer_player_hand[:i] or
                     convertd[c] in input_info.played_cards[:input_info.played_cards_len] ):
                    input_info.computer_player_hand[i] = convertd[c] + 1
                else:
                    input_info.computer_player_hand[i] = convertd[c]

            res = GetBestCard(pointer(input_info))
            result_info = json.dumps({'best_card': rconvertd[res]})
            print(result_info)
        except:
            print(json.dumps('error'))

