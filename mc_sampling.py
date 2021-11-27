import matplotlib.pyplot as plt
import numpy as np
import random
import ctypes

class GameInfo(ctypes.Structure):
    _fields_ = [
        ("player_cardsets", ctypes.c_ulonglong * 4),
        ("player_scores"  , ctypes.c_ubyte * 4),
        ("player_isre"    , ctypes.c_bool  * 4),
        ("cards_left"     , ctypes.c_ubyte    ),
        ("next"           , ctypes.c_ubyte    ),
        ("trickscore"     , ctypes.c_ubyte    ),
        ("tricksuit"      , ctypes.c_ubyte    ),
        ("trickwinnercard", ctypes.c_ubyte    ),
        ("trickwinner"    , ctypes.c_ubyte    )
    ]

class CardInfo(ctypes.Structure):
    _fields_ = [
        ("cards_left"     , ctypes.c_ubyte    ),
        ("player_left"    , ctypes.c_ubyte * 3),
        ("ids"            , ctypes.c_ubyte * 36),
        ("scores"         , ctypes.c_ubyte * (36 * 3))
    ]

lib_doko = ctypes.cdll.LoadLibrary('./lib_doko.so')
xgi = GameInfo()
xci = CardInfo()
xrs = ctypes.c_uint()
xrs.value = 1
xci.cards_left = 6
xci.player_left[:] = [2]*3
xci.ids[:6] = range(6)
xci.scores[:6*3] = [5]*6*3

def easy_random_dist(n=6):
    a = np.arange(n)
    np.random.shuffle(a)
    return np.reshape(a,(3,-1))

def blacklist_dist(n=6, blacklist=[[],[],[]]):
    result = [[],[],[]]

    def players(c):
        return [p for p,l in enumerate(blacklist) if not c in l]
    cards = sorted(range(n), key=players, reverse=True)
    
    for c in cards:
        ps = [p for p in players(c) if len(result[p]) < n//3]
        if len(ps) == 0:
            raise Exception('impossible')
        result[random.choice(ps)].append(c)

    return np.array(result)


def slot_dist(card_info):
    # card info must have shape (n, 3)
    rng = np.random.default_rng()
    card_info = card_info[np.argsort(np.sum(card_info,axis=1))]
    n = card_info.shape[0]
    result = np.full(n, fill_value=-1).reshape((3,-1))
    for i in range(n):
        # mask the disallowed
        slots = np.argwhere(result == -1)
        slots = slots[ci[i,slots[:,0]]!=0,:]
        result[tuple(rng.choice(slots, axis=0))] = i
    return result

def slot_dist_advanced(card_info):
    # card info must have shape (n,3) and values 0, 3, 4 and 5
    rng = np.random.default_rng()
    idx = np.array((-1,-1,-1,0,1,2))
    card_info_indices = np.argsort(np.sum(card_info,axis=1))
    n = card_info.shape[0]
    slots = np.full((3,(n//3),3), fill_value=-1)
    for i in card_info_indices:
        open_slots = np.argwhere(slots == -1)
        open_slots = open_slots[idx[card_info[i,open_slots[:,0]]] >= open_slots[:,2],:]
        slots[tuple(rng.choice(open_slots, axis=0)[:-1])] = i
    return slots[:,:,0]

def dist_external(card_info):
    result = [[],[],[]]
    xgi.player_cardsets[:] = [0]*4
    xci.cards_left = 6
    xci.player_left[:] = [2]*3
    xci.ids[:6] = range(6)
    xci.scores[:6*3] = card_info.astype(int).flatten()
    lib_doko.sort_and_check(ctypes.byref(xci))
    lib_doko.mc_sample(ctypes.byref(xgi), ctypes.byref(xci), ctypes.byref(xrs))
    for i in range(6):
        for p in range(3):
            if xgi.player_cardsets[p] & (1 << (2*i)):
                result[p].append(i)
    return np.array(result)


def test(r,N, *args):
    fig, axs = plt.subplots(1,3)

    result = np.empty((N,3,2))
    for i in range(N):
        result[i] = r(*args)
    
    result = result.swapaxes(0,1).reshape((3,-1))
    for i,p in enumerate(result):
        axs[i].hist(p, bins=np.arange(7)-0.5, rwidth=0.8)
        axs[i].set_ylim((0,N*1.1))

    return fig, axs

#ci = np.array(((0,0,1),(0,1,1),(1,1,1),(1,1,1),(1,1,1),(1,1,1)))
ci = np.array(((5,5,5),(5,5,5),(5,3,5),(5,5,5),(5,5,5),(5,5,5)))

if __name__ == "__main__":
    test(dist_external,60000,ci)
    plt.show()

