/**
 * 内核终端显示。
 * @date 2026-10-17
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#include <init/params.h>
#include <support/format.h>
#include <support/memory.h>

/**
 * 内核内嵌字库头。
 */
typedef struct _aos_kernel_fonts_header
{
    uint8  magic[5];    /*魔数。*/
    uint8  width16;     /*16字形位宽。*/
    uint8  width24;     /*24字形位宽。*/
    uint8  width32;     /*32字形位宽。*/
    uint32 count;       /*码元数目。*/
    uint32 offsetcodes; /*码元数组偏移。*/
    uint32 offset16n;   /*16n位图偏移。*/
    uint32 offset16b;   /*16b位图偏移。*/
    uint32 offset24n;   /*24n位图偏移。*/
    uint32 offset24b;   /*24b位图偏移。*/
    uint32 offset32n;   /*32n位图偏移。*/
    uint32 offset32b;   /*32b位图偏移。*/
} aos_kernel_fonts_header;

/**
 * 内核内嵌字体起始。
 */
extern const uint8 kfonts_header[];

/**
 * 内核内嵌字体结尾。
 */
extern const uint8 kfonts_end[];

/**
 * 显示句柄魔数。
 */
#define DISPLAY_HANDLE_MAGIC 0x4C505344ULL

/**
 * 内核内嵌字库。
 */
static const aos_kernel_fonts_header* fonts=null;

/**
 * 获取字体位图偏移。
 * 
 * @param font 字体索引。
 * 
 * @return 对应位图偏移，其余为0。
 */
static uint32 display_kernel_fonts_offset(uint32 font)
{
    switch(font)
    {
        case 0:
            return fonts->offset16n;
        case 1:
            return fonts->offset16b;
        case 2:
            return fonts->offset24n;
        case 3:
            return fonts->offset24b;
        case 4:
            return fonts->offset32n;
        case 5:
            return fonts->offset32b;
        default:
            return 0;
    }
}

/**
 * 初始化内核字库。
 * 
 * @return 成功返回真。
 */
static bool display_kernel_fonts_init(void)
{
    uintn size=(uintn)(kfonts_end-kfonts_header);

    if(size<sizeof(aos_kernel_fonts_header))
    {
        return false;
    }

    fonts=(const aos_kernel_fonts_header*)kfonts_header;

    if(fonts->magic[0]!='K'||fonts->magic[1]!='F'||fonts->magic[2]!='O'||fonts->magic[3]!='N'||fonts->magic[4]!='T'||
        fonts->count==0)
    {
        fonts=null;
        return false;
    }

    return true;
}

/**
 * 查询码元对应的字形下标。
 * 
 * @param code 码元。
 * 
 * @return 字形下标，未找到返回最大值。
 */
static uint32 display_kernel_fonts_lookup(uint32 code)
{
    if(fonts==null)
    {
        return UINT32_MAX;
    }

    const uint32* codes=(const uint32*)(kfonts_header+fonts->offsetcodes);
    uint32 low=0;
    uint32 high=fonts->count;

    while(low<high)
    {
        uint32 mid=(low+high)/2;
        uint32 current=codes[mid];

        if(current==code)
        {
            return mid;
        }

        if(current<code)
        {
            low=mid+1;
        }
        else
        {
            high=mid;
        }
    }

    return UINT32_MAX;
}

/**
 * 获取字体位宽。
 * 
 * @param font 字体索引。
 * 
 * @return 对应位宽，参数非法返回0。
 */
static uint8 display_kernel_fonts_width(uint32 font)
{
    if(fonts==null||font>=6)
    {
        return 0;
    }

    if(font<2)
    {
        return fonts->width16;
    }

    if(font<4)
    {
        return fonts->width24;
    }

    return fonts->width32;
}

/**
 * 获取字体位高。
 * 
 * @param font 字体索引。
 * 
 * @return 对应位高，参数非法返回0。
 */
static uint8 display_kernel_fonts_height(uint32 font)
{
    if(fonts==null||font>=6)
    {
        return 0;
    }

    if(font<2)
    {
        return 16;
    }

    if(font<4)
    {
        return 24;
    }

    return 32;
}

/**
 * 获取字形位图。
 * 
 * @param font  字体索引。
 * @param index 字形下标。
 * 
 * @return 位图指针，参数非法返回空。
 */
static const uint8* display_kernel_fonts_glyph(uint32 font,uint32 index)
{
    if(fonts==null||font>=6||index>=fonts->count)
    {
        return null;
    }

    uint32 width=display_kernel_fonts_width(font);
    uint32 height=display_kernel_fonts_height(font);
    uint32 bytes=(width*height+7)/8;

    return kfonts_header+display_kernel_fonts_offset(font)+(uintn)index*bytes;
}

/**
 * 内核图形信息。
 */
static aos_graphics_info display_graphics;

/**
 * 当前字体索引。
 */
static uint32 display_font;

/**
 * 当前字宽。
 */
static uint32 display_width;

/**
 * 当前字高。
 */
static uint32 display_height;

/**
 * 屏幕字符列数。
 */
static uint32 display_cols;

/**
 * 屏幕字符行数。
 */
static uint32 display_rows;

/**
 * 光标列。
 */
static uint32 display_cursor_x;

/**
 * 光标行。
 */
static uint32 display_cursor_y;

/**
 * 前景色。
 */
static uint32 display_foreground;

/**
 * 背景色。
 */
static uint32 display_background;

/**
 * 红色通道偏移。
 */
static uint8 display_red_shift;

/**
 * 绿色通道偏移。
 */
static uint8 display_green_shift;

/**
 * 蓝色通道偏移。
 */
static uint8 display_blue_shift;

/**
 * 红色通道最大值。
 */
static uint32 display_red_max;

/**
 * 绿色通道最大值。
 */
static uint32 display_green_max;

/**
 * 蓝色通道最大值。
 */
static uint32 display_blue_max;

/**
 * 每像素字节数。
 */
static uint32 display_bytes;

/**
 * 计算掩码低位偏移。
 * 
 * @param mask 颜色掩码。
 * 
 * @return 低位偏移。
 */
static uint8 display_shift(uint32 mask)
{
    if(mask==0)
    {
        return 0;
    }
    uint8 shift=0;

    while((mask&1)==0)
    {
        mask>>=1;
        shift++;
    }

    return shift;
}

/**
 * 合成像素颜色。
 * 
 * @param r 红色分量。
 * @param g 绿色分量。
 * @param b 蓝色分量。
 * 
 * @return 像素颜色。
 */
static uint32 display_color(uint8 r,uint8 g,uint8 b)
{
    uint32 red=(uint32)r*display_red_max/255;
    uint32 green=(uint32)g*display_green_max/255;
    uint32 blue=(uint32)b*display_blue_max/255;

    return (red<<display_red_shift)|(green<<display_green_shift)|(blue<<display_blue_shift);
}

/**
 * 写入像素。
 * 
 * @param x     横坐标。
 * @param y     纵坐标。
 * @param color 像素颜色。
 * 
 * @return 无返回值。
 */
static void display_put(uint32 x,uint32 y,uint32 color)
{
    uint8* pixel=(uint8*)(uintn)display_graphics.framebuffer_base+((uintn)y*display_graphics.scan_line+x)*display_bytes;

    if(display_bytes==4)
    {
        *(volatile uint32*)pixel=color;
    }
    else if(display_bytes==3)
    {
        pixel[0]=(uint8)color;
        pixel[1]=(uint8)(color>>8);
        pixel[2]=(uint8)(color>>16);
    }
    else if(display_bytes==2)
    {
        *(volatile uint16*)pixel=(uint16)color;
    }
}

/**
 * 填充矩形。
 * 
 * @param x     起始横坐标。
 * @param y     起始纵坐标。
 * @param w     宽度。
 * @param h     高度。
 * @param color 填充颜色。
 * 
 * @return 无返回值。
 */
static void display_fill(uint32 x,uint32 y,uint32 w,uint32 h,uint32 color)
{
    for(uint32 dy=0;dy<h;dy++)
    {
        uint8* line=(uint8*)(uintn)display_graphics.framebuffer_base+((uintn)(y+dy)*display_graphics.scan_line+x)*
            display_bytes;

        if(display_bytes==4)
        {
            volatile uint32* target=(volatile uint32*)line;

            for(uint32 dx=0;dx<w;dx++)
            {
                target[dx]=color;
            }
        }
        else
        {
            for(uint32 dx=0;dx<w;dx++)
            {
                display_put(x+dx,y+dy,color);
            }
        }
    }
}

/**
 * 绘制一个字形。
 * 
 * @param x    起始横坐标。
 * @param y    起始纵坐标。
 * @param code 码元。
 * 
 * @return 无返回值。
 */
static void display_glyph(uint32 x,uint32 y,uint32 code)
{
    if(fonts==null)
    {
        return;
    }

    uint32 index=display_kernel_fonts_lookup(code);
    if(index==UINT32_MAX)
    {
        index=display_kernel_fonts_lookup(0xFFFD);
    }

    const uint8* glyph=display_kernel_fonts_glyph(display_font,index);

    for(uint32 gy=0;gy<display_height;gy++)
    {
        for(uint32 gx=0;gx<display_width;gx++)
        {
            uint32 pos=gy*display_width+gx;

            if((glyph[pos>>3]>>(pos&7))&1)
            {
                display_put(x+gx,y+gy,display_foreground);
            }
        }
    }
}

/**
 * 向上滚动一行。
 * 
 * @return 无返回值。
 */
static void display_scroll(void)
{
    uint32 line=display_height;
    uint32 area=display_rows*display_height;
    uintn pitch=(uintn)display_graphics.scan_line*display_bytes;
    uintn width=(uintn)display_graphics.hres*display_bytes;
    uint8* base=(uint8*)(uintn)display_graphics.framebuffer_base;

    if(area>line)
    {
        uint8* dest=base;
        uint8* source=base+(uintn)line*pitch;

        for(uint32 y=0;y+line<area;y++)
        {
            memory_copy(dest,source,width);
            dest+=pitch;
            source+=pitch;
        }

        display_fill(0,area-line,display_graphics.hres,line,display_background);
        display_cursor_y=display_rows-1;
    }
    else
    {
        /*只有一个字符行，没有可滚动的空间，直接清掉文本区。*/
        display_fill(0,0,display_graphics.hres,area,display_background);
        display_cursor_y=0;
    }
}

/**
 * 输出一个字符。
 * 
 * @param c 字符。
 * 
 * @return 无返回值。
 */
static void display_emit(char32 c)
{
    if(display_cols==0||display_rows==0||fonts==null)
    {
        return;
    }

    if(c=='\n')
    {
        display_cursor_x=0;
        display_cursor_y++;
    }
    else if(c=='\r')
    {
        display_cursor_x=0;
    }
    else if(c=='\b')
    {
        if(display_cursor_x>0)
        {
            display_cursor_x--;
        }
    }
    else
    {
        display_glyph(display_cursor_x*display_width,display_cursor_y*display_height,c);
        display_cursor_x++;

        if(display_cursor_x>=display_cols)
        {
            display_cursor_x=0;
            display_cursor_y++;
        }
    }

    if(display_cursor_y>=display_rows)
    {
        display_scroll();
    }
}

/**
 * 清屏。
 * 
 * @return 无返回值。
 */
void display_clear(void)
{
    display_fill(0,0,display_graphics.hres,display_graphics.vres,display_background);
    display_cursor_x=0;
    display_cursor_y=0;
}

/**
 * 初始化屏幕。
 * 
 * @param params 启动参数。
 * 
 * @return 成功返回真。
 */
bool display_init(const aos_boot_params* params)
{
    if(params==null)
    {
        return false;
    }

    display_cols=0;
    display_rows=0;
    display_width=0;
    display_height=0;
    fonts=null;
    uint8 bits=0;

    memory_copy(&display_graphics,&params->graphics,sizeof(aos_graphics_info));

    if(display_graphics.hres==0||display_graphics.vres==0||display_graphics.framebuffer_base==0||
        display_graphics.scan_line<display_graphics.hres)
    {
        return false;
    }

    uint32 mask=display_graphics.red_mask|display_graphics.green_mask|display_graphics.blue_mask|
        display_graphics.reserved_mask;
    if(mask==0)
    {
        return false;
    }

    display_red_shift=display_shift(display_graphics.red_mask);
    display_green_shift=display_shift(display_graphics.green_mask);
    display_blue_shift=display_shift(display_graphics.blue_mask);
    display_red_max=display_graphics.red_mask>>display_red_shift;
    display_green_max=display_graphics.green_mask>>display_green_shift;
    display_blue_max=display_graphics.blue_mask>>display_blue_shift;

    while(mask!=0)
    {
        bits++;
        mask>>=1;
    }

    display_bytes=(bits+7)/8;
    display_font=0;
    display_cursor_x=0;
    display_cursor_y=0;

    if(display_bytes!=2&&display_bytes!=3&&display_bytes!=4)
    {
        return false;
    }

    if(display_graphics.framebuffer_size<(uintn)display_graphics.scan_line*display_graphics.vres*display_bytes)
    {
        return false;
    }

    if(display_kernel_fonts_init())
    {
        /*物理尺寸可信时按DPI选择字号：110DPI以上用24像素，150DPI以上用32像素。*/
        if(display_graphics.physical_width>=10&&display_graphics.physical_width<=2000&&
            display_graphics.physical_height>=10&&display_graphics.physical_height<=2000)
        {
            uint32 dpi_x=display_graphics.hres*254/(display_graphics.physical_width*10);
            uint32 dpi_y=display_graphics.vres*254/(display_graphics.physical_height*10);

            /*忽略越界值和横纵差异过大的信息。*/
            if(dpi_x>=15&&dpi_x<=1000&&dpi_y>=15&&dpi_y<=1000&&dpi_x*4>=dpi_y*3&&dpi_y*4>=dpi_x*3)
            {
                uint32 dpi=(dpi_x+dpi_y)/2;
                if(dpi>=150)
                {
                    display_font=4;
                }
                else if(dpi>=110)
                {
                    display_font=2;
                }
            }
        }

        /*字号不能大到连基本终端尺寸都放不下，逐档降到16像素。*/
        while(display_font>0)
        {
            uint32 width=display_kernel_fonts_width(display_font);
            uint32 height=display_kernel_fonts_height(display_font);

            if(width!=0&&height!=0&&display_graphics.hres/width>=80&&display_graphics.vres/height>=24)
            {
                break;
            }
            display_font-=2;
        }

        display_width=display_kernel_fonts_width(display_font);
        display_height=display_kernel_fonts_height(display_font);

        if(display_width==0||display_height==0)
        {
            return false;
        }

        display_cols=display_graphics.hres/display_width;
        display_rows=display_graphics.vres/display_height;
        display_foreground=display_color(0xE8,0xF4,0xFF);
        display_background=display_color(0x10,0x14,0x18);
        display_clear();
        return true;
    }
    else
    {
        return false;
    }
}

/**
 * 输出一个字符。
 * 
 * @param c 字符。
 * 
 * @return 无返回值。
 */
void display_putc(char32 c)
{
    display_emit(c);
}

/**
 * 输出字符串。
 * 
 * @param text 字符串。
 * 
 * @return 无返回值。
 */
void display_puts(const char32* text)
{
    while(*text!=0)
    {
        display_emit(*text);
        text++;
    }
}

/**
 * UTF-8解码剩余字节数。
 */
static uint8 display_utf8_remaining;

/**
 * UTF-8解码累加值。
 */
static char32 display_utf8_accum;

/**
 * 显示句柄最近一次状态。
 */
static uint64 display_io_status;

/**
 * 输出一个UTF-8编码的字节。
 * 
 * @param c UTF-8字节。
 * 
 * @return 无返回值。
 */
static void display_put8(char8 c)
{
    uint8 byte=(uint8)c;

    if(display_utf8_remaining==0)
    {
        if(byte<0x80)
        {
            display_emit((char32)byte);
        }
        else if((byte&0xE0)==0xC0)
        {
            display_utf8_accum=(char32)(byte&0x1F);
            display_utf8_remaining=1;
        }
        else if((byte&0xF0)==0xE0)
        {
            display_utf8_accum=(char32)(byte&0x0F);
            display_utf8_remaining=2;
        }
        else if((byte&0xF8)==0xF0)
        {
            display_utf8_accum=(char32)(byte&0x07);
            display_utf8_remaining=3;
        }
        else
        {
            display_emit(0xFFFD);
        }
        return;
    }

    if((byte&0xC0)!=0x80)
    {
        /*截断序列：丢弃累积值，把当前字节当作新序列起点重新解析。*/
        display_utf8_accum=0;
        display_utf8_remaining=0;
        display_emit(0xFFFD);
        display_put8(c);
        return;
    }

    display_utf8_accum=(display_utf8_accum<<6)|(char32)(byte&0x3F);

    if(--display_utf8_remaining==0)
    {
        char32 code=display_utf8_accum;

        display_utf8_accum=0;

        if(code>0x10FFFF||(code>=0xD800&&code<=0xDFFF))
        {
            code=0xFFFD;
        }

        display_emit(code);
    }
}

/**
 * 显示句柄读取。
 * 
 * @param handle 输入输出句柄。
 * @param dest   目标数组。
 * @param n      请求读取长度。
 * 
 * @return 屏幕不可读，返回0。
 */
static uintn display_handle_read(io_handle* handle,uint8* dest,uintn n)
{
    (void)handle;
    (void)dest;
    (void)n;
    display_io_status=IO_STATUS_NOT_SUPPORTED;
    return 0;
}

/**
 * 显示句柄写入。
 * 
 * @param handle 输入输出句柄。
 * @param src    源数组。
 * @param n      请求写入长度。
 * 
 * @return 实际写入长度，恒等于请求长度。
 */
static uintn display_handle_write(io_handle* handle,const uint8* src,uintn n)
{
    (void)handle;

    if(src==null||display_cols==0||display_rows==0||fonts==null)
    {
        display_io_status=IO_STATUS_DEVICE_ERROR;
        return 0;
    }

    for(uintn index=0;index<n;index++)
    {
        display_put8((char8)src[index]);
    }

    display_io_status=IO_STATUS_SUCCESS;
    return n;
}

/**
 * 显示句柄刷新。
 * 
 * @param handle 输入输出句柄。
 * 
 * @return 状态码。
 */
static uint64 display_handle_flush(io_handle* handle)
{
    (void)handle;
    display_io_status=IO_STATUS_SUCCESS;
    return display_io_status;
}

/**
 * 显示句柄获取位置。
 * 
 * @param handle 输入输出句柄。
 * 
 * @return 光标线性位置，按字符单元计。
 */
static uint64 display_handle_get_position(io_handle* handle)
{
    (void)handle;
    display_io_status=IO_STATUS_SUCCESS;
    return (uint64)display_cursor_y*display_cols+display_cursor_x;
}

/**
 * 显示句柄设置位置。屏幕句柄不支持数据寻址，光标定位请用独立接口。
 * 
 * @param handle   输入输出句柄。
 * @param position 目标位置。
 * 
 * @return 操作不支持。
 */
static uint64 display_handle_set_position(io_handle* handle,uint64 position)
{
    (void)handle;
    (void)position;
    display_io_status=IO_STATUS_NOT_SUPPORTED;
    return display_io_status;
}

/**
 * 显示句柄获取大小。
 * 
 * @param handle 输入输出句柄。
 * 
 * @return 字符单元总数，不是字节数。
 */
static uint64 display_handle_get_size(io_handle* handle)
{
    (void)handle;
    display_io_status=IO_STATUS_SUCCESS;
    return (uint64)display_cols*display_rows*sizeof(char32);
}

/**
 * 显示句柄获取能力。
 * 
 * @param handle 输入输出句柄。
 * 
 * @return 只写能力。
 */
static uint64 display_handle_get_capabilities(io_handle* handle)
{
    (void)handle;
    display_io_status=IO_STATUS_SUCCESS;
    return IO_CAPABILITY_WRITABLE;
}

/**
 * 显示句柄获取状态。
 * 
 * @param handle 输入输出句柄。
 * 
 * @return 最近一次状态码，读取后重置。
 */
static uint64 display_handle_get_io_status(io_handle* handle)
{
    uint64 status=display_io_status;

    (void)handle;
    display_io_status=IO_STATUS_SUCCESS;
    return status;
}

/**
 * 显示句柄关闭。屏幕不支持关闭。
 * 
 * @param handle 输入输出句柄。
 * 
 * @return 操作不支持。
 */
static uint64 display_handle_close(io_handle* handle)
{
    (void)handle;
    display_io_status=IO_STATUS_NOT_SUPPORTED;
    return display_io_status;
}

/**
 * 显示输入输出句柄。
 */
static io_handle display_handle=
{
    .magic=DISPLAY_HANDLE_MAGIC,
    .read=display_handle_read,
    .write=display_handle_write,
    .flush=display_handle_flush,
    .get_position=display_handle_get_position,
    .set_position=display_handle_set_position,
    .get_size=display_handle_get_size,
    .get_capabilities=display_handle_get_capabilities,
    .get_io_status=display_handle_get_io_status,
    .close=display_handle_close
};

/**
 * 格式化输出字符串。
 * 
 * @param format 格式化字符串。
 * 
 * @return 输出状态码。
 */
uintn display_printf(const char8* format,...)
{
    va_list args;

    va_start(args,format);
    uintn result=format_print_valist(&display_handle,format,args);
    va_end(args);

    return result;
}