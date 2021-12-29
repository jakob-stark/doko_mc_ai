import tkinter as tk
from PIL import Image, ImageTk

import functools as ft

cards = ['9D','KD','10D','AD', '9S', 'KS', '10S', 'AS', '9C','KC','10C','AC','9H','KH','10H','AH',
         'JD','JH','JS','JC','QD','QH','QS','QC', 'BACK']

class Main:

    class CardGame:
        img = [Image.open('cards/{}.png'.format(c)) for c in cards]
        aspect = 54/48

        def __init__(self, canvas, nw, se):
            self.canvas = canvas
            self.nw = nw
            self.se = se
            self.trick_images = [None]*4
            self.trick_tags   = [None]*4
            self.hand_images = [None]*12
            self.hand_tags   = [None]*12
            self.back_images = None
            self.back_tags   = [None]*36
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
                self.trick_images[i] = ImageTk.PhotoImage(self.img[i].resize((w,h)))
                self.trick_tags[i] = self.canvas.create_image(x,y, image=self.trick_images[i], anchor='nw')

            for i in range(12):
                if not self.hand_tags[i] is None:
                    self.canvas.delete(self.hand_tags[i])
                x, y = int(left + dx*(12+2*i)), int(top + dy*36)
                self.hand_images[i] = ImageTk.PhotoImage(self.img[i].resize((w,h)))
                self.hand_tags[i]   = self.canvas.create_image(x,y, image=self.hand_images[i], anchor='nw')
                self.canvas.tag_bind(self.hand_tags[i], '<Button>', ft.partial(self.button, self.hand_tags[i]))
                #self.canvas.tag_bind(self.tags[i], '<Enter>', ft.partial(self.enter , self.tags[i]))
                #self.canvas.tag_bind(self.tags[i], '<Leave>', ft.partial(self.leave , self.tags[i]))

            self.back_images = ImageTk.PhotoImage(self.img[24].resize((w,h)))
            for i in range(12):
                if not self.back_tags[i*3+0] is None:
                    self.canvas.delete(self.back_tags[i*3+0])
                x, y = int(left + dx*(12+i*2)), int(top + dy*0)
                self.back_tags[i*3+0] = self.canvas.create_image(x,y, image=self.back_images, anchor='nw')
            for i in range(12):
                if not self.back_tags[i*3+1] is None:
                    self.canvas.delete(self.back_tags[i*3+1])
                x, y = int(left + dx*0), int(top + dy*(7+i*2))
                self.back_tags[i*3+1] = self.canvas.create_image(x,y, image=self.back_images, anchor='nw')
            for i in range(12):
                if not self.back_tags[i*3+2] is None:
                    self.canvas.delete(self.back_tags[i*3+2])
                x, y = int(left + dx*(46)), int(top + dy*(7+i*2))
                self.back_tags[i*3+2] = self.canvas.create_image(x,y, image=self.back_images, anchor='nw')


        
        def button(self, tag, event):
            print(cards[self.hand_tags.index(tag)])

        def enter(self, tag, event):
            if event.y > 5:
                self.canvas.move(tag, 0, 5)
                self.last_tag = tag
            else:
                self.last_tag = None

        def leave(self, tag, event):
            if self.last_tag == tag:
                self.canvas.move(tag, 0, -5)


    def __init__(self):
        self.root = tk.Tk()
        #self.root.grid()
        self.root.columnconfigure(0,weight=1)
        self.root.rowconfigure(0,weight=1)

        self.main = tk.Frame(self.root, bg='#00ff00')
        self.main.grid(row=0,column=0,sticky=tk.N+tk.S+tk.W+tk.E)
        self.main.columnconfigure(0,weight=1)
        self.main.rowconfigure(0,weight=1)

        self.lp = tk.Canvas(self.main, bg='#ff0000',width=0)
        self.lp.grid(row=0,column=0,sticky=tk.N+tk.S+tk.W+tk.E)
        self.cs1 = self.CardGame(self.lp, (0.01,0.01), (0.99,0.99))

    def run(self):
        tk.mainloop()


if __name__ == '__main__':
    app = Main()
    app.run()
