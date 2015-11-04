#author: raincomplex
import math
import pyperclip

# https://github.com/PetitPrince/TeDiGe-2
# http://petitprince.github.io/TeDiGe-2/doc/symbols/src/TeDiGe-2_src_tedige-editor.js.html

enctbl = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/'
asctbl = ' !"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~'

# see Frame.lock() for how this is laid out
piecedata = [
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,1,1,1,2,1,3,1,1,0,1,1,1,2,1,3,0,1,1,1,2,1,3,1,1,0,1,1,1,2,1,3,
        0,1,1,1,2,1,0,2,1,0,1,1,1,2,2,2,2,0,0,1,1,1,2,1,0,0,1,0,1,1,1,2,
        1,1,2,1,1,2,2,2,1,1,2,1,1,2,2,2,1,1,2,1,1,2,2,2,1,1,2,1,1,2,2,2,
        0,1,1,1,1,2,2,2,2,0,1,1,2,1,1,2,0,1,1,1,1,2,2,2,2,0,1,1,2,1,1,2,
        0,1,1,1,2,1,1,2,1,0,1,1,2,1,1,2,1,0,0,1,1,1,2,1,1,0,0,1,1,1,1,2,
        0,1,1,1,2,1,2,2,1,0,2,0,1,1,1,2,0,0,0,1,1,1,2,1,1,0,1,1,0,2,1,2,
        1,1,2,1,0,2,1,2,0,0,0,1,1,1,1,2,1,1,2,1,0,2,1,2,0,0,0,1,1,1,1,2
]

class Frame:
        def __init__(self):
                # colors of blocks (0=black, ..., 8=gray)
                # extra rows at top and bottom of playfield
                self.field = [0 for i in range(220)]
                
                self.willlock = False
                self.rise = False
                self.mirror = False
                self.piece = Piece()
                self.comment = ''

        def isrep(self, f):
                return self.field == f.field

        def lock(self):
                if self.piece.kind != 0:
                        for j in range(4):
                                self.field[self.piece.pos + piecedata[self.piece.kind * 32 + self.piece.rot * 8 + j * 2 + 1] * 10 + piecedata[self.piece.kind * 32 + self.piece.rot * 8 + j * 2] - 11] = self.piece.kind
                        self.piece = Piece()

        def copy(self):
                f = Frame()
                f.field = list(self.field)
                f.willlock = self.willlock
                f.rise = self.rise
                f.mirror = self.mirror
                f.piece = self.piece.copy()
                f.comment = self.comment
                return f

        def clearfield(self):
                self.field = [0 for i in range(220)]

        def next(self):
                f = self.copy()
                if f.willlock:
                        self.lock()
                f.field = clearlines(f.field)
                # TODO mirror and rise
                return f

class Piece:
        def __init__(self):
                self.kind = 0
                self.rot = 0
                self.pos = 0

        def copy(self):
                p = Piece()
                p.kind = self.kind
                p.rot = self.rot
                p.pos = self.pos
                return p

def write(data, n, *args):
        val = 0
        for v, base in args:
                assert 0 <= v < base, 'value out of range for base'
                val = v + val * base
        for i in range(n):
                data.append(val % 64)
                val = int(math.floor(val / 64))
        assert val == 0, 'data left over'

def clearlines(field):
        out = [0 for i in range(220)]
        m = 21
        for i in range(21, 0, -1):
                c = 0
                for k in range(10):
                        if field[i * 10 + k] != 0:
                                c += 1
                if c < 10:
                        for k in range(10):
                                out[m * 10 + k] = field[i * 10 + k]
                        m -= 1
        return out

def getdiff(a, b):
        lst = []
        for i in range(220):
                lst.append(b.field[i] - a.field[i])
        last = None
        count = 0
        for d in lst:
                if d == last:
                        count += 1
                else:
                        if count > 0:
                                yield (last, count)
                        count = 1
                        last = d
        if count > 0:
                yield (last, count)

def make(frames, rotmode, url='http://fumen.zui.jp/'):
        data = []
        rep = 0
        for i in range(len(frames)):
                f = frames[i]

                if rep == 0:
                        # TODO repeat frame detection could be better (save a couple chars per frame)
                        for d, count in getdiff(frames[i - 1] if i > 0 else Frame(), f):
                                write(data, 2, (d + 8, 17), (count - 1, 220))
                                if d == 0 and count == 220:
                                        rep = 0
                                        write(data, 1, (rep, 64))
                else:
                        rep -= 1

                write(data, 3,
                        (not f.willlock, 2),
                        (bool(f.comment), 2),
                        (rotmode, 2),  # only has effect on the first frame
                        (f.mirror, 2),
                        (f.rise, 2),
                        (f.piece.pos, 220),
                        (f.piece.rot, 4),
                        (f.piece.kind, 8)
                )

                # TODO write comment
                if f.comment:
                        pass

        return url + '?v110@' + ''.join(enctbl[i] for i in data) + '#english.js'

if __name__ == '__main__':
        frames = []
        last = ''
        count = 100
        allowdup = False

        realframe = framenum = 0
        for line in open('test.dat'):
                realframe += 1
                if allowdup or line != last:
                        f = Frame()
                        i = 10
                        for c in line:
                                if c == 'X':
                                        f.field[i] = 8
                                        i += 1
                                elif c == '.':
                                        f.field[i] = 0
                                        i += 1
                        #f.field = clearlines(f.field)
                        framenum += 1
                        print(realframe, framenum)
                        frames.append(f)
                        count -= 1

                last = line

                if count <= 0:
                        break

        url = make(frames, 0)
        print(url)
        pyperclip.copy(url)
