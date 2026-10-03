# Erzeugt tiles.h (2bpp Gameboy-Tiles) fuer Breakout
font5x7 = {
' ':[0,0,0,0,0],
'0':[0x3E,0x51,0x49,0x45,0x3E],'1':[0x00,0x42,0x7F,0x40,0x00],'2':[0x42,0x61,0x51,0x49,0x46],
'3':[0x21,0x41,0x45,0x4B,0x31],'4':[0x18,0x14,0x12,0x7F,0x10],'5':[0x27,0x45,0x45,0x45,0x39],
'6':[0x3C,0x4A,0x49,0x49,0x30],'7':[0x01,0x71,0x09,0x05,0x03],'8':[0x36,0x49,0x49,0x49,0x36],
'9':[0x06,0x49,0x49,0x29,0x1E],'A':[0x7E,0x11,0x11,0x11,0x7E],'B':[0x7F,0x49,0x49,0x49,0x36],
'C':[0x3E,0x41,0x41,0x41,0x22],'D':[0x7F,0x41,0x41,0x22,0x1C],'E':[0x7F,0x49,0x49,0x49,0x41],
'F':[0x7F,0x09,0x09,0x09,0x01],'G':[0x3E,0x41,0x49,0x49,0x7A],'H':[0x7F,0x08,0x08,0x08,0x7F],
'I':[0x00,0x41,0x7F,0x41,0x00],'J':[0x20,0x40,0x41,0x3F,0x01],'K':[0x7F,0x08,0x14,0x22,0x41],
'L':[0x7F,0x40,0x40,0x40,0x40],'M':[0x7F,0x02,0x0C,0x02,0x7F],'N':[0x7F,0x04,0x08,0x10,0x7F],
'O':[0x3E,0x41,0x41,0x41,0x3E],'P':[0x7F,0x09,0x09,0x09,0x06],'Q':[0x3E,0x41,0x51,0x21,0x5E],
'R':[0x7F,0x09,0x19,0x29,0x46],'S':[0x46,0x49,0x49,0x49,0x31],'T':[0x01,0x01,0x7F,0x01,0x01],
'U':[0x3F,0x40,0x40,0x40,0x3F],'V':[0x1F,0x20,0x40,0x20,0x1F],'W':[0x3F,0x40,0x38,0x40,0x3F],
'X':[0x63,0x14,0x08,0x14,0x63],'Y':[0x07,0x08,0x70,0x08,0x07],'Z':[0x61,0x51,0x49,0x45,0x43],
'!':[0x00,0x00,0x5F,0x00,0x00],':':[0x00,0x36,0x36,0x00,0x00],'-':[0x08,0x08,0x08,0x08,0x08],
'.':[0x00,0x60,0x60,0x00,0x00],'>':[0x00,0x41,0x22,0x14,0x08],"'":[0x00,0x00,0x07,0x00,0x00],
}
CHARS = " 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ!:-.>'"

def enc(px):  # px: 8 Strings mit 8 Zeichen '0'-'3' -> 16 Bytes 2bpp
    out=[]
    for row in px:
        lo=hi=0
        for i,c in enumerate(row):
            v=int(c)
            if v&1: lo|=0x80>>i
            if v&2: hi|=0x80>>i
        out+= [lo,hi]
    return out

def glyph(ch):
    cols=font5x7[ch]
    rows=[]
    for y in range(8):
        r=''
        for x in range(8):
            cx=x-1
            r+= '3' if (0<=cx<5 and y<7 and (cols[cx]>>y)&1) else '0'
        rows.append(r)
    return rows

def brick(fill, hl, left):
    # 16x8 Stein aus zwei Tiles, Rand schwarz, Glanzlinie oben, 1px Fuge rechts/unten
    W=16; img=[]
    for y in range(8):
        r=''
        for x in range(W):
            if y==7 or x==15: v=0
            elif y==0 or y==6 or x==0 or x==14: v=3
            elif y==1 and x<14: v=hl
            else: v=fill
            r+=str(v)
        img.append(r)
    return [row[0:8] if left else row[8:16] for row in img]

wall = ["22222223","22222223","22222223","33333333","22232222","22232222","22232222","33333333"]
def shift(t,n=3): return ["00000000"]*n + t[:8-n]
paddle_l = shift(["01333333","13111111","31222222","32222222","03333333","00000000","00000000","00000000"])
paddle_m = shift(["33333333","11111111","22222222","22222222","33333333","00000000","00000000","00000000"])
paddle_r = shift(["33333310","11111131","22222213","22222223","33333330","00000000","00000000","00000000"])
ball     = ["03333000","31133300","31333300","33333300","33333300","03333000","00000000","00000000"]  # 6x6
# Laser-Paddle: Kanonen auf den Enden
laser_l  = ["00330000","00320000","00320000","01333333","13111111","31222222","32222222","03333333"]
laser_r  = ["00003300","00002300","00002300","33333310","11111131","22222213","22222223","33333330"]
beam     = ["00033000","00033000","00033000","00033000","00033000","00033000","00000000","00000000"]
def capsule(letter):
    # schwarze Kapsel (7 px breit) mit hellgrauem Symbol
    rows=["03333300"]
    for l in letter:
        rows.append("3"+l.replace('.','3').replace('#','1')+"30")
    rows.append("03333300"); rows.append("00000000")
    return rows
pu_laser = capsule(["#....","#....","#....","#....","#####"])
pu_wide  = capsule(["#...#","#...#","#.#.#","##.##","#...#"])
pu_life  = capsule([".#.#.","#####","#####",".###.","..#.."])
bg=[]
bg += enc(["0"*8]*8)                      # 0 leer
for ch in CHARS[1:]: bg+=enc(glyph(ch))   # 1.. Schrift
FONT_END = len(CHARS)
names=[]
for fill,hl in [(3,2),(2,1),(1,0)]:
    bg+=enc(brick(fill,hl,True)); bg+=enc(brick(fill,hl,False))
bg+=enc(wall)
# Pong-Mittellinie (gestrichelt, 2 px breit, ueber zwei Tiles verteilt)
TILE_NET = len(bg)//16
bg += enc(["00000001"]*4 + ["00000000"]*4)
bg += enc(["10000000"]*4 + ["00000000"]*4)
# Grosses Logo: 5x7-Glyphen doppelt skaliert (16x16 = 4 Tiles) mit hellgrauem Schatten
BIG_TEXT = "THUNDER'S"
TILE_BIG0 = len(bg)//16
for ch in BIG_TEXT:
    cols = font5x7[ch]
    img = [[0]*16 for _ in range(16)]
    on = lambda x,y: 0<=x<5 and 0<=y<7 and (cols[x]>>y)&1
    for y in range(16):
        for x in range(16):
            gx, gy = (x-3)//2 if x>=3 else -1, (y-1)//2 if y>=1 else -1
            sx, sy = (x-4)//2 if x>=4 else -1, (y-2)//2 if y>=2 else -1
            if on(gx,gy): img[y][x]=3
            elif on(sx,sy): img[y][x]=1
    rows=["".join(str(v) for v in r) for r in img]
    for (ox,oy) in [(0,0),(8,0),(0,8),(8,8)]:
        bg += enc([r[ox:ox+8] for r in rows[oy:oy+8]])
# Bild im Hauptmenue: assets/avatar.png -> 56x56 Pixel (7x7 Tiles), 4 Graustufen ohne Dithering
from PIL import Image, ImageOps
AV = 7
TILE_AVATAR0 = len(bg)//16
_im = Image.open("assets/avatar.png").convert("L").resize((AV*8, AV*8), Image.LANCZOS)
_im = ImageOps.autocontrast(_im, cutoff=1)
_px = [[((_im.getpixel((x, y)) / 255.0) ** 0.75) for x in range(AV*8)] for y in range(AV*8)]
def _shade(v): return 0 if v > 0.83 else 1 if v > 0.5 else 2 if v > 0.17 else 3
_rows = []
for y in range(AV*8):
    r = ""
    for x in range(AV*8):
        edge = x == 0 or y == 0 or x == AV*8-1 or y == AV*8-1
        r += "3" if edge else str(_shade(_px[y][x]))      # schwarzer Rahmen
    _rows.append(r)
for ty in range(AV):
    for tx in range(AV):
        bg += enc([_rows[ty*8+yy][tx*8:tx*8+8] for yy in range(8)])
spr = b''
spr = []
# Pong-Schlaeger: 4 px breit, 24 px hoch (3 Tiles uebereinander)
pong_t = ["0330","3113","3123","3123","3123","3123","3123","3123"]
pong_m = ["3123"]*8
pong_b = ["3123","3123","3123","3123","3123","3223","3223","0330"]
pong_t = [r+"0000" for r in pong_t]; pong_m=[r+"0000" for r in pong_m]; pong_b=[r+"0000" for r in pong_b]
for t in [paddle_l,paddle_m,paddle_r,ball,laser_l,laser_r,beam,pu_laser,pu_wide,pu_life,pong_t,pong_m,pong_b]: spr+=enc(t)

def carr(name,data):
    s="const unsigned char %s[] = {\n"%name
    for i in range(0,len(data),16):
        s+="  "+",".join("0x%02X"%b for b in data[i:i+16])+",\n"
    return s+"};\n"

with open("tiles.h","w") as f:
    f.write("// automatisch erzeugt von gen_tiles.py\n#ifndef TILES_H\n#define TILES_H\n")
    f.write('#define FONT_CHARS "%s"\n'%CHARS)
    f.write("#define TILE_BRICK0 %d\n#define TILE_WALL %d\n#define TILE_NET %d\n#define TILE_BIG0 %d\n#define BIG_LEN %d\n#define TILE_AVATAR0 %d\n#define AVATAR_TILES_W %d\n#define BG_TILE_COUNT %d\n#define SPR_TILE_COUNT %d\n"%(FONT_END, FONT_END+6, TILE_NET, TILE_BIG0, len(BIG_TEXT), TILE_AVATAR0, AV, len(bg)//16, len(spr)//16))
    f.write(carr("bg_tiles",bg)); f.write(carr("spr_tiles",spr))
    f.write("#endif\n")
print("ok", len(bg)//16)
