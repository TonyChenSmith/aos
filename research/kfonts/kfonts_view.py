# 
# kfonts查看/测试工具。
# 读取kfonts文件，按码表与紧凑位流还原字形：
# 不带 --dump 时打开 tkinter 网格窗口；带 --dump 时直接在终端输出字符画。
# 用法：
#     python kfonts_view.py out/kfonts 41
#     python kfonts_view.py out/kfonts 41 --font 24n --dump
# 在窗口里键盘1到6切换字体，←/→切换字形，PgUp/PgDn跳16个，Home/End首尾，Esc退出。
# @date 2026-10-17
# 
# Copyright (c) 2026 Tony Chen Smith
# 
# SPDX-License-Identifier: MIT
# 
import argparse
import bisect
import struct
import sys

MAGIC=b"KFONT"
HEADER=struct.Struct("<5sBBBI7I")

FONTS=(
    ("16n",16),
    ("16b",16),
    ("24n",24),
    ("24b",24),
    ("32n",32),
    ("32b",32),
)

COLOR_BG="#101418"
COLOR_OFF="#000000"
COLOR_ON="#E8F4FF"
COLOR_TEXT="#8FA6B8"

GAP=1
MARGIN=16
INFO_HEIGHT=40


def load_kfonts(path):
    """
    读取kfonts文件，返回(fonts,blob,codes)。
    fonts每项包含name/size/width/height/offset/glyph_bytes。
    """
    with open(path,"rb") as stream:
        blob=stream.read()

    if len(blob)<HEADER.size:
        raise ValueError("%s：文件太小" % path)

    (magic,w16,w24,w32,gcount,codeo,
     n16o,b16o,n24o,b24o,n32o,b32o)=HEADER.unpack_from(blob,0)

    if magic!=MAGIC:
        raise ValueError("%s：不是kfonts文件" % path)
    if gcount==0:
        raise ValueError("%s：字形数为0" % path)

    widths={16:w16,24:w24,32:w32}
    offsets=(n16o,b16o,n24o,b24o,n32o,b32o)

    fonts=[]

    for (name,size),offset in zip(FONTS,offsets):
        width=widths[size]

        if width==0:
            raise ValueError("%s：%s 的宽度为0" % (path,name))

        glyph_bytes=(width*size+7)//8

        if offset+glyph_bytes*gcount>len(blob):
            raise ValueError("%s：%s 的位图超出文件" % (path,name))

        fonts.append({
            "name":name,
            "size":size,
            "width":width,
            "height":size,
            "offset":offset,
            "glyph_bytes":glyph_bytes,
        })

    if codeo+4*gcount>len(blob):
        raise ValueError("%s：码表超出文件" % path)

    codes=list(struct.unpack_from("<%dI" % gcount,blob,codeo))

    return fonts,blob,codes


def lookup(codes,code):
    """
    在升序码表中查找码点。
    """
    index=bisect.bisect_left(codes,code)

    if index<len(codes) and codes[index]==code:
        return index

    return None


def glyph_data(font,blob,index):
    """
    取出一个字形。
    """
    base=font["offset"]+index*font["glyph_bytes"]
    return blob[base:base+font["glyph_bytes"]]


def dump_glyph(font,glyph,code):
    """
    输出字符画。
    """
    print("%s U+%04X %dx%d" % (font["name"],code,font["width"],font["height"]))

    for y in range(font["height"]):
        line=""

        for x in range(font["width"]):
            pos=y*font["width"]+x
            line+="##" if (glyph[pos>>3]>>(pos&7))&1 else "  "

        print(line)


class Viewer:
    """
    网格预览窗口。
    """

    def __init__(self,tk,fonts,blob,codes,start=None,font_index=0):
        self.tk=tk
        self.fonts=fonts
        self.blob=blob
        self.codes=codes
        self.font_index=font_index
        self.glyph_index=0

        if start is not None:
            index=lookup(codes,start)

            if index is None:
                sys.exit("错误：字体中不存在 U+%04X。" % start)

            self.glyph_index=index

        self.root=tk.Tk()
        self.root.configure(bg=COLOR_BG)
        self.root.resizable(False,False)
        self.root.title("kfonts")

        from tkinter import font as tkfont

        # 按最大字体固定网格，格子大小只算一次。
        self.max_width=max(font["width"] for font in fonts)
        self.max_height=max(font["height"] for font in fonts)
        self.cell=max(10,min(22,(self.root.winfo_screenheight()-200)//self.max_height))
        self.pitch=self.cell+GAP
        self.grid_w=self.max_width*self.pitch-GAP
        self.grid_h=self.max_height*self.pitch-GAP

        # 窗口宽度至少能完整放下最长标签。
        self.label_font=tkfont.Font(family="Consolas",size=12)
        sample="32b  U+10FFFF  [1356/1356]"
        self.canvas_w=max(self.grid_w+MARGIN*2,
                          self.label_font.measure(sample)+MARGIN*2)
        self.canvas_h=self.grid_h+MARGIN*2+INFO_HEIGHT
        self.canvas=tk.Canvas(self.root,width=self.canvas_w,height=self.canvas_h,bg=COLOR_BG,highlightthickness=0)
        self.canvas.pack()

        self.root.bind_all("<Left>",lambda event:self.step(-1))
        self.root.bind_all("<Right>",lambda event:self.step(1))
        self.root.bind_all("<Prior>",lambda event:self.step(-16))
        self.root.bind_all("<Next>",lambda event:self.step(16))
        self.root.bind_all("<Home>",lambda event:self.jump(0))
        self.root.bind_all("<End>",lambda event:self.jump(len(codes)-1))
        self.root.bind_all("<Escape>",lambda event:self.root.destroy())

        for number in range(len(fonts)):
            self.root.bind_all(str(number+1),
                               lambda event,index=number:self.set_font(index))

        self.draw()
        self.root.focus_force()
        self.root.mainloop()

    def set_font(self,index):
        """
        切换字体。
        """
        self.font_index=index
        self.draw()

    def step(self,delta):
        """
        切换字形。
        """
        self.glyph_index=(self.glyph_index+delta)%len(self.codes)
        self.draw()

    def jump(self,index):
        """
        跳到指定下标。
        """
        self.glyph_index=index
        self.draw()

    def draw(self):
        """
        重画当前字形。
        """
        tk=self.tk
        font=self.fonts[self.font_index]
        width=font["width"]
        height=font["height"]
        glyph=glyph_data(font,self.blob,self.glyph_index)

        grid_w=width*self.pitch-GAP
        ox=(self.canvas_w-grid_w)//2
        oy=MARGIN+(self.grid_h-(height*self.pitch-GAP))//2

        self.canvas.delete("all")

        for y in range(height):
            for x in range(width):
                pos=y*width+x
                on=(glyph[pos>>3]>>(pos&7))&1
                x0=ox+x*self.pitch
                y0=oy+y*self.pitch
                self.canvas.create_rectangle(x0,y0,x0+self.cell,y0+self.cell,fill=COLOR_ON if on else COLOR_OFF,outline="",width=0)

        code=self.codes[self.glyph_index]
        label="%s  U+%04X  [%d/%d]" % (font["name"],code,self.glyph_index+1,len(self.codes))
        self.canvas.create_text(self.canvas_w//2,MARGIN+self.grid_h+16,anchor="center",text=label,fill=COLOR_TEXT,font=self.label_font)

def parse_codepoint(text):
    """
    解析十六进制码点。
    """
    text=text.strip()

    if text.lower().startswith("u+"):
        text=text[2:]
    elif text.lower().startswith("0x"):
        text=text[2:]

    return int(text,16)


def main():
    """
    入口。
    """
    parser=argparse.ArgumentParser(description="kfonts查看/测试工具。")
    parser.add_argument("path",help="kfonts文件。")
    parser.add_argument("codepoint",nargs="?",default=None,
                        help="起始码点，十六进制（如41、U+0041）。")
    parser.add_argument("--font",default="16n",
                        choices=[name for name,_ in FONTS],
                        help="字体（默认16n）。")
    parser.add_argument("--dump",action="store_true",
                        help="输出字符画，不打开窗口。")
    args=parser.parse_args()

    fonts,blob,codes=load_kfonts(args.path)
    font_index=[name for name,_ in FONTS].index(args.font)
    code=parse_codepoint(args.codepoint) if args.codepoint else 0x41

    if args.dump:
        index=lookup(codes,code)

        if index is None:
            sys.exit("错误：字体中不存在 U+%04X。" % code)

        dump_glyph(fonts[font_index],glyph_data(fonts[font_index],blob,index),code)
        return

    try:
        import tkinter
    except ImportError:
        sys.exit("错误：未找到tkinter，请使用带tkinter的Python。")

    Viewer(tkinter,fonts,blob,codes,code,font_index)

if __name__=="__main__":
    main()