# 
# 将选择的BDF字体进行检查后打包成kfonts文件。
# @date 2026-10-15
# 
# Copyright (c) 2026 Tony Chen Smith
# 
# SPDX-License-Identifier: MIT
# 
import os
from dataclasses import dataclass
from fonts.font_dict import BDF_FONTS

@dataclass
class Glyph:
    """
    一个字形：码元、名称与紧凑位流。
    """
    code:int
    name:str
    data:bytes

def _reverse_bits(value,count):
    """
    把低count位按位颠倒。
    """
    result=0

    for _ in range(count):
        result=(result<<1)|(value&1)
        value>>=1

    return result

def _glyph_bitmap(rows,width,height):
    """
    将逐行MSB-first的BDF位图压成行间无填充的LSB-first位流。
    第y行第x个像素位于第y*width+x位，bit0为最左。
    """
    stride=(width+7)//8
    value=0

    for y,row in enumerate(rows):
        row_value=int(row,16)>>(stride*8-width)
        value|=_reverse_bits(row_value,width)<<(y*width)

    return value.to_bytes((width*height+7)//8,"little")

def parse_bdf(path):
    """
    解析BDF文件并返回(width,height,glyphs)，码元从低到高排序字形。
    """
    width=height=None
    glyphs=[]
    name=code=None
    rows=None

    with open(path,"r",encoding="utf-8",errors="replace") as stream:
        for raw in stream:
            line=raw.strip()
            if not line:
                continue

            if rows is not None:
                if line=="ENDCHAR":
                    glyphs.append(Glyph(code,name,_glyph_bitmap(rows,width,height)))
                    name=code=None
                    rows=None
                else:
                    rows.append(line)
                continue

            if line.startswith("FONTBOUNDINGBOX "):
                values=line.split()
                width,height=int(values[1]),int(values[2])
            elif line.startswith("STARTCHAR "):
                parts=line.split(None,1)
                name=parts[1].strip() if len(parts)>1 else ""
            elif line.startswith("ENCODING "):
                code=int(line.split()[1])
            elif line=="BITMAP":
                rows=[]

    glyphs.sort(key=lambda glyph:glyph.code)
    return width,height,glyphs

def show_glyph(path,code):
    """
    打印指定码元的字形。
    """
    width,height,glyphs=parse_bdf(path)
    glyph=next(glyph for glyph in glyphs if glyph.code==code)

    for y in range(height):
        line=""

        for x in range(width):
            pos=y*width+x
            line+="##" if (glyph.data[pos>>3]>>(pos&7))&1 else "  "

        print(line)

def pack_kfonts():
    """
    打包字体。
    """
    n16w,n16h,n16g=parse_bdf(BDF_FONTS["16n"])
    b16w,b16h,b16g=parse_bdf(BDF_FONTS["16b"])
    n24w,n24h,n24g=parse_bdf(BDF_FONTS["24n"])
    b24w,b24h,b24g=parse_bdf(BDF_FONTS["24b"])
    n32w,n32h,n32g=parse_bdf(BDF_FONTS["32n"])
    _,_,b32g=parse_bdf(BDF_FONTS["32b"])

    gcount=len(n16g)

    os.makedirs("out",exist_ok=True)
    with open("out/kfonts","wb") as stream:
        stream.write(b"KFONT")
        stream.write(n16w.to_bytes(1,"little"))
        stream.write(n24w.to_bytes(1,"little"))
        stream.write(n32w.to_bytes(1,"little"))
        stream.write(gcount.to_bytes(4,"little"))
        codeo=40
        n16o=codeo+4*gcount
        b16o=n16o+(n16h*n16w*gcount)//8
        n24o=b16o+(b16h*b16w*gcount)//8
        b24o=n24o+(n24h*n24w*gcount)//8
        n32o=b24o+(b24h*b24w*gcount)//8
        b32o=n32o+(n32h*n32w*gcount)//8
        stream.write(codeo.to_bytes(4,"little"))
        stream.write(n16o.to_bytes(4,"little"))
        stream.write(b16o.to_bytes(4,"little"))
        stream.write(n24o.to_bytes(4,"little"))
        stream.write(b24o.to_bytes(4,"little"))
        stream.write(n32o.to_bytes(4,"little"))
        stream.write(b32o.to_bytes(4,"little"))

        for g in n16g:
            stream.write(g.code.to_bytes(4,"little"))
        
        for g in n16g:
            stream.write(g.data)
        for g in b16g:
            stream.write(g.data)
        for g in n24g:
            stream.write(g.data)
        for g in b24g:
            stream.write(g.data)
        for g in n32g:
            stream.write(g.data)
        for g in b32g:
            stream.write(g.data)

if __name__=="__main__":
    pack_kfonts()