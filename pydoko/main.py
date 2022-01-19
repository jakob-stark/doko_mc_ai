import tkinter as tk
from tkinter import ttk
from PIL import Image, ImageTk

import random as rd
import functools as ft
from ctypes import *
import time
import threading
import queue

cards = ['9C','KC','10C','AC','9S','KS','10S','AS','9H','KH','AH','9D','KD','10D','AD',
         'JD','JH','JS','JC','QD','QH','QS','QC','10H','BACK']

class InputInfo(Structure):
    _fields_ = [('computer_player_id', c_byte),
                ('starting_player_id', c_byte),
                ('played_cards', c_byte * 48),
                ('played_cards_len', c_byte),
                ('computer_player_hand', c_byte * 12),
                ('computer_player_hand_len', c_byte)]

cardnames = ('cn','ck','ct','ca','sn','sk','st','sa','hn','hk','ha','dn','dk','dt','da',
             'dj','hj','sj','cj','dq','hq','sq','cq','ht')
cardvalues = (0,4,10,11,0,4,10,11,0,4,11,0,4,10,11,2,2,2,2,3,3,3,3,10)
cardsuits = (0,0,0,0,1,1,1,1,2,2,2,4,4,4,4,4,4,4,4,4,4,4,4,4)
cconvertd = {k : i*2 for i, k in enumerate(cardnames)}
rconvertd = { i*2 : k for i, k in enumerate(cardnames)}
rconvertd.update( {i*2+1 : k for i,k in enumerate(cardnames)})

class Doko:
    def __init__(self, get_card, draw):
        # shuffle the cards
        icards = list(range(48))
        rd.shuffle(icards)
        self.played_cards = []

        self.player_cards = [sorted(icards[i*12:(i+1)*12]) for i in range(4)]
        self.player_cardsn = [sorted(icards[i*12:(i+1)*12]) for i in range(4)]
        self.player_scores = [0 for i in range(4)]
        self.player_isre = [44 in self.player_cards[i] or 45 in self.player_cards[i] for i in range(4)]
        self.cards_left = 48

        # reset the trick
        self.next = 0
        self.trick = [None]*4
        self.trickscore = 0
        self.tricksuit = 5
        self.trickwinnercard = 48
        self.trickwinner = 0

        # save get card function
        self.get_card = get_card
        self.draw = draw

    def play(self):
        card = self.get_card(self.next, 0, self.played_cards, self.player_cards[self.next])
        if card >= 48:
            print('error', self.next, self.played_cards, self.player_cards[self.next])
            return False

        self.trick[self.next] = card
        self.player_cards[self.next].remove(card)
        self.played_cards.append(card)
        self.player_cardsn[self.next] = [e if e != card else None for e in self.player_cardsn[self.next]]
        self.cards_left -= 1

        self.trickscore += cardvalues[card//2]
        if self.tricksuit == 5:
            self.tricksuit = cardsuits[card//2]
            self.trickwinnercard = card
            self.trickwinner = self.next
        else:
            if cardsuits[card//2] == self.tricksuit or cardsuits[card//2] == 4:
                if card//2 > self.trickwinnercard//2 or (card/2 == 23 and self.cards_left > 4):
                    self.trickwinnercard = card
                    self.trickwinner = self.next


        if self.cards_left % 4 == 0:
            self.update()
            time.sleep(1.5)
            self.player_scores[self.trickwinner] += self.trickscore
            self.trickscore = 0
            self.trick = [None]*4
            self.tricksuit = 5
            self.next = self.trickwinner
        else:
            self.next = (self.next + 1) % 4

        self.update()
        return self.cards_left != 0

    def update(self):
        self.draw(self.player_cardsn, self.trick)


class Main:

    class CardGame:
        img = [Image.open('cards/{}.png'.format(c)) for c in cards]
        aspect = 54/48

        def __init__(self, canvas,iqueue, nw, se):

            self.player_cards = [
                    [0,1,2,3,4,5,6,7,8,9,10,11],
                    [24]*12, [24]*12, [24]*12
            ]
            self.trick_cards = [15,16,None,18]

            self.iqueue = iqueue
            self.canvas = canvas
            self.nw = nw
            self.se = se
            self.trick_images  = [None]*4
            self.trick_tags    = [None]*4
            self.player_images = [[None]*12, [None]*12, [None]*12, [None]*12]
            self.player_tags   = [[None]*12, [None]*12, [None]*12, [None]*12]
            self.bind_handle = self.canvas.bind('<Configure>', self.resize, add='+')

        def resize(self, event):
            self.redraw()

        def redraw(self):
            cw = self.canvas.winfo_width()
            ch = self.canvas.winfo_height()
            tx = cw  * self.nw[0]
            ty = ch * self.nw[1]
            twidth  = cw *(self.se[0]-self.nw[0])
            theight = ch *(self.se[1]-self.nw[1])
            if twidth/theight > self.aspect:
                width  = theight * self.aspect
                height = theight
                left = tx + (twidth - width)/2
                top  = ty
            else:
                height = twidth / self.aspect
                width  = twidth
                left   = tx
                top    = ty + (theight - height)/2

            dx, dy = width/54, height/48
            w, h = int(8*dx), int(12*dy)

            pos = ((7,10),(2,6),(9,2),(14,8))
            for i in range(4):
                if not self.trick_tags[i] is None:
                    self.canvas.delete(self.trick_tags[i])
                x, y = int(left + dx*(12+3+pos[i][0])), int(top + dy*(12+pos[i][1]))
                if not self.trick_cards[i] is None:
                    self.trick_images[i] = ImageTk.PhotoImage(
                            self.img[self.trick_cards[i]//2].resize((w,h)))
                    self.trick_tags[i] = self.canvas.create_image(
                            x,y, image=self.trick_images[i], anchor='nw')
                else:
                    self.trick_images[i] = None
                    self.trick_tags[i] = None

            pos = ((12,36),(0,7),(12,0),(46,7))
            inc = ((2,0),(0,2),(2,0),(0,2))
            for p in range(4):
                for i in range(12):
                    if not self.player_tags[p][i] is None:
                        self.canvas.delete(self.player_tags[p][i])
                    x, y = int(left+dx*(pos[p][0]+inc[p][0]*i)), int(top+dy*(pos[p][1]+inc[p][1]*i))
                    if not self.player_cards[p][i] is None:
                        self.player_images[p][i] = ImageTk.PhotoImage(
                                self.img[self.player_cards[p][i]//2].resize((w,h)))
                        self.player_tags[p][i]   = self.canvas.create_image(
                                x,y, image=self.player_images[p][i], anchor='nw')
                        if p == 0:
                            self.canvas.tag_bind(self.player_tags[p][i],
                                    '<Button>', ft.partial(self.button, self.player_tags[p][i]))
                    else:
                        self.player_images[p][i] = None
                        self.player_tags[p][i] = None
        
        def button(self, tag, event):
            self.iqueue.put(self.player_cards[0][self.player_tags[0].index(tag)])

    def __init__(self, iqueue):
        self.root = tk.Tk()
        self.root.columnconfigure(0,weight=1)
        self.root.rowconfigure(0,weight=1)

        self.main = tk.Frame(self.root, bg='#000000')
        self.main.grid(row=0,column=0,sticky=tk.N+tk.S+tk.W+tk.E)
        self.main.columnconfigure(0,weight=0)
        self.main.columnconfigure(1,weight=1)
        self.main.rowconfigure(0,weight=1)

        self.side = tk.Frame(self.main, bg='#c0c0c0')
        self.side.grid(row=0,column=0,sticky=tk.N+tk.S+tk.W+tk.E)

        self.restart = ttk.Button(self.side,text='Restart')
        self.restart.grid(row=0,column=0,sticky=tk.N)

        self.quit = ttk.Button(self.side,text='Quit')
        self.quit.grid(row=1,column=0,sticky=tk.N)

        self.canvas = tk.Canvas(self.main, bg='#006000',width=0)
        self.canvas.grid(row=0,column=1,sticky=tk.N+tk.S+tk.W+tk.E)
        self.game = self.CardGame(self.canvas,iqueue, (0.01,0.01), (0.99,0.99))

    def run(self):
        tk.mainloop()


if __name__ == '__main__':

    iqueue = queue.Queue()

    app = Main(iqueue)
    libdoko = CDLL('../src/lib_doko.so')
    GetBestCard = libdoko.GetBestCard
    GetBestCard.argtypes = [POINTER(InputInfo)]
    GetBestCard.restype = c_byte

    def draw(player_cards, trick):
        app.game.player_cards = [x[:] for x in player_cards]
        app.game.trick_cards = trick[:]
        app.canvas.event_generate('<Configure>')

    def get_card(n, s, played_cards, player_cards):
        if n == 0:
            return iqueue.get()
        else:
            input_info = InputInfo()
            input_info.computer_player_id = n
            input_info.starting_player_id = s
            input_info.played_cards_len = len(played_cards)
            input_info.computer_player_hand_len = len(player_cards)
            for i, c in enumerate(played_cards):
                input_info.played_cards[i] = c
            for i, c in enumerate(player_cards):
                input_info.computer_player_hand[i] = c
            return GetBestCard(pointer(input_info))

    def mainloop():
        doko = Doko(get_card, draw)
        doko.update()
        while doko.play():
            pass
        print(doko.player_isre)
        print(doko.player_scores)
        re = sum([x for i,x in enumerate(doko.player_scores) if doko.player_isre[i] == doko.player_isre[0]])
        co = sum([x for i,x in enumerate(doko.player_scores) if doko.player_isre[i] != doko.player_isre[0]])
        print('your score {}, enemy score {}'.format(re,co))


    t = threading.Thread(target=mainloop)
    t.start()
    app.run()

