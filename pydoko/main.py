import tkinter as tk
from tkinter import ttk
from PIL import Image, ImageTk

import functools as ft

cards = ['9D','KD','10D','AD', '9S', 'KS', '10S', 'AS', '9C','KC','10C','AC','9H','KH','10H','AH',
         'JD','JH','JS','JC','QD','QH','QS','QC', 'BACK']

class Main:

    class Doko:
        def __init__(self):
            self.play

    class CardGame:
        img = [Image.open('cards/{}.png'.format(c)) for c in cards]
        aspect = 54/48

        def __init__(self, canvas, nw, se):

            self.player_cards = [
                    [0,1,2,3,4,5,6,7,8,9,10,11],
                    [24]*12, [24]*12, [24]*12
            ]
            self.trick_cards = [15,16,None,18]

            self.canvas = canvas
            self.nw = nw
            self.se = se
            self.trick_images  = [None]*4
            self.trick_tags    = [None]*4
            self.player_images = [[None]*12, [None]*12, [None]*12, [None]*12]
            self.player_tags   = [[None]*12, [None]*12, [None]*12, [None]*12]
            self.bind_handle = self.canvas.bind('<Configure>', self.resize, add='+')

        def resize(self, event):
            tx = event.width  * self.nw[0]
            ty = event.height * self.nw[1]
            twidth  = event.width *(self.se[0]-self.nw[0])
            theight = event.height*(self.se[1]-self.nw[1])
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
                            self.img[self.trick_cards[i]].resize((w,h)))
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
                                self.img[self.player_cards[p][i]].resize((w,h)))
                        self.player_tags[p][i]   = self.canvas.create_image(
                                x,y, image=self.player_images[p][i], anchor='nw')
                        if p == 0:
                            self.canvas.tag_bind(self.player_tags[p][i],
                                    '<Button>', ft.partial(self.button, self.player_tags[p][i]))
                    else:
                        self.player_images[p][i] = None
                        self.player_tags[p][i] = None
        
        def button(self, tag, event):
            print(tag)


    def __init__(self):
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
        self.game = self.CardGame(self.canvas, (0.01,0.01), (0.99,0.99))

    def run(self):
        tk.mainloop()


if __name__ == '__main__':
    app = Main()
    app.run()
