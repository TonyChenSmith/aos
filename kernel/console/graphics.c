/**
 * 控制台图形显示。
 * 在这里操作的图形操作都认为是帧缓冲已经独占的状态。
 * @date 2026-10-20
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#include <init/params.h>
#include <support/format.h>
#include <support/handle.h>
#include <support/memory.h>
#include <support/util.h>

/**
 * 内核内嵌字库头。
 */
typedef struct _aos_kernel_fonts_header
{
    uint8  magic[5]; /*魔数。*/
    uint8  width16;  /*16字形位宽。*/
    uint8  width24;  /*24字形位宽。*/
    uint8  width32;  /*32字形位宽。*/
    uint32 count;    /*码元数目。*/
    uint32 codes;    /*码元数组偏移。*/
    uint32 n16;      /*16n位图偏移。*/
    uint32 b16;      /*16b位图偏移。*/
    uint32 n24;      /*24n位图偏移。*/
    uint32 b24;      /*24b位图偏移。*/
    uint32 n32;      /*32n位图偏移。*/
    uint32 b32;      /*32b位图偏移。*/
} aos_kernel_fonts_header;

/**
 * ANSI转义序列解析状态。
 */
typedef enum _graphics_ansi_state
{
    GRAPHICS_ANSI_STATE_TEXT,   /*不在序列中。*/
    GRAPHICS_ANSI_STATE_ESCAPE, /*收到ESC，等待引导字符。*/
    GRAPHICS_ANSI_STATE_CSI,    /*收到ESC[，正在累积参数。*/
    GRAPHICS_ANSI_STATE_IGNORE  /*未识别的序列，丢弃到最终字节。*/
} graphics_ansi_state;

/**
 * 绘制一个字形。
 * 
 * @param column    绘制列。
 * @param row       绘制行。
 * @param codepoint 绘制码元。
 * @param set       置位像素颜色。
 * @param unset     未置位像素颜色。
 * 
 * @return 无返回值。
 */
typedef void (*graphics_draw_glyph)(uintn column,uintn row,char32 codepoint,uint32 set,uint32 unset);

/**
 * 绘制一块矩形。
 * 
 * @param x     起始横坐标。
 * @param y     起始纵坐标。
 * @param w     宽度。
 * @param h     高度。
 * @param color 像素颜色。
 * 
 * @return 无返回值。
 */
typedef void (*graphics_draw_rect)(uintn x,uintn y,uintn w,uintn h,uint32 color);

/**
 * 内核内嵌字体起始。
 */
extern const uint8 console_fonts_start[];

/**
 * 内核内嵌字体结尾。
 */
extern const uint8 console_fonts_end[];

/**
 * 内核内嵌字库。
 */
static const aos_kernel_fonts_header* const FONTS=(const aos_kernel_fonts_header*)console_fonts_start;

/**
 * ANSI标准16色调色板RGB值。
 * 顺序：黑、红、绿、黄、蓝、品红、青、白、亮黑、亮红、亮绿、亮黄、亮蓝、亮品红、亮青、亮白。
 */
static const uint8 ANSI_PALETTE_RGB[16][3]=
{
    {0,0,0},{170,0,0},{0,170,0},{170,85,0},
    {0,0,170},{170,0,170},{0,170,170},{170,170,170},
    {85,85,85},{255,85,85},{85,255,85},{255,255,85},
    {85,85,255},{255,85,255},{85,255,255},{255,255,255}
};

/**
 * ANSI色立方等级数值。
 */
static const uint8 ANSI_COLOR_CUBE_LEVEL[6]={0,95,135,175,215,255};

/**
 * 兜底选择字体时要求的最小字符行数。
 */
#define GRAPHICS_FALLBACK_MIN_ROWS 50

/**
 * 图形UTF8输出句柄魔数。
 */
#define GRAPHICS_UTF8_HANDLE_MAGIC 0x2026103123150102ULL

/**
 * ANSI转义字符。
 */
#define GRAPHICS_ANSI_ESCAPE 0x1B

/**
 * ANSI控制序列引导符。
 */
#define GRAPHICS_ANSI_CSI_CHAR '['

/**
 * ANSI控制序列参数最大个数。
 */
#define GRAPHICS_ANSI_PARAM_MAX 8

/**
 * ANSI控制序列单个参数的最大值。
 */
#define GRAPHICS_ANSI_PARAM_LIMIT 65535

/**
 * ANSI序列最终字节最小值。
 */
#define GRAPHICS_ANSI_FINAL_MIN 0x40

/**
 * ANSI序列最终字节最大值。
 */
#define GRAPHICS_ANSI_FINAL_MAX 0x7E

/**
 * 当前帧缓冲。
 */
static void* framebuffer=null;

/**
 * 当前帧缓冲副本。
 */
static void* framebuffer_copy=null;

/**
 * 帧缓冲大小。
 */
static uintn framebuffer_size=0;

/**
 * 当前帧缓冲副本大小。
 */
static uintn framebuffer_copy_size=0;

/**
 * 红色位移。
 */
static uint8 red_shift=0;

/**
 * 绿色位移。
 */
static uint8 green_shift=0;

/**
 * 蓝色位移。
 */
static uint8 blue_shift=0;

/**
 * 红色极值。
 */
static uint32 red_max=0;

/**
 * 绿色极值。
 */
static uint32 green_max=0;

/**
 * 蓝色极值。
 */
static uint32 blue_max=0;

/**
 * 颜色字节数。
 */
static uint8 color_bytes=0;

/**
 * 屏幕可用总列数。
 */
static uintn columns=0;

/**
 * 屏幕可用总行数。
 */
static uintn rows=0;

/**
 * 屏幕起始渲染横坐标。
 */
static uintn screen_base_x=0;

/**
 * 屏幕起始渲染纵坐标。
 */
static uintn screen_base_y=0;

/**
 * 屏幕水平分辨率。
 */
static uintn screen_hres=0;

/**
 * 屏幕扫描线。
 */
static uintn screen_scan=0;

/**
 * 屏幕竖直分辨率。
 */
static uintn screen_vres=0;

/**
 * 屏幕背景色。
 */
static uint32 background_color=0;

/**
 * 屏幕前景色。
 */
static uint32 foreground_color=0;

/**
 * 屏幕字体索引。
 */
static uint8 font_index=0;

/**
 * ASCII码索引表。最后一个是0xFFFD的索引。
 */
static uint32 ascii_indexs[129];

/**
 * 字体高度。
 */
static uintn font_height=0;

/**
 * 字体宽度。
 */
static uintn font_width=0;

/**
 * 帧缓冲副本起始行。
 */
static uintn top_row=0;

/**
 * 光标所在行。
 */
static uintn cursor_row=0;

/**
 * 光标所在列。
 */
static uintn cursor_column=0;

/**
 * 起始脏行。
 */
static uintn dirty_row=0;

/**
 * 脏行行数。
 */
static uintn dirty_rows=0;

/**
 * 脏边缘状态。
 */
static bool dirty_edge=false;

/**
 * 绘制字形函数。
 */
static graphics_draw_glyph draw_glyph=null;

/**
 * 绘制矩形函数。
 */
static graphics_draw_rect draw_rect=null;

/**
 * UTF8解码剩余字节数。
 */
static uint8 graphics_utf8_remaining=0;

/**
 * UTF8解码累加值。
 */
static char32 graphics_utf8_accum=0;

/**
 * 图形UTF8输出句柄状态。
 */
static uint64 graphics_utf8_status=IO_STATUS_SUCCESS;

/**
 * 图形UTF8输出句柄。
 */
static io_handle graphics_utf8_handle;

/**
 * ANSI转义序列当前解析状态。
 */
static graphics_ansi_state ansi_state=GRAPHICS_ANSI_STATE_TEXT;

/**
 * ANSI转义序列当前解析参数。
 */
static uintn ansi_params[GRAPHICS_ANSI_PARAM_MAX];

/**
 * ANSI转义序列当前已解析参数数目。
 */
static uintn ansi_count=0;

/**
 * ANSI标准16色调色板。
 * 顺序：黑、红、绿、黄、蓝、品红、青、白、亮黑、亮红、亮绿、亮黄、亮蓝、亮品红、亮青、亮白。
 */
static uint32 ansi_palette[16];

/**
 * 获取字体位图数组基址。
 * 
 * @param font 字体索引。
 * 
 * @return 对应位图偏移，其余为0。
 */
static const uint8* graphics_get_bitmap(uint8 font)
{
    switch(font%6)
    {
        case 0:
            return (const uint8*)((uintn)console_fonts_start+FONTS->n16);
        case 1:
            return (const uint8*)((uintn)console_fonts_start+FONTS->b16);
        case 2:
            return (const uint8*)((uintn)console_fonts_start+FONTS->n24);
        case 3:
            return (const uint8*)((uintn)console_fonts_start+FONTS->b24);
        case 4:
            return (const uint8*)((uintn)console_fonts_start+FONTS->n32);
        case 5:
            return (const uint8*)((uintn)console_fonts_start+FONTS->b32);
        default:
            return 0;
    }
}

/**
 * 根据分辨率和物理尺寸选择字号。
 * 
 * @param hres   水平分辨率。
 * @param vres   竖直分辨率。
 * @param width  屏幕物理宽度。
 * @param height 屏幕物理高度。
 * 
 * @return 无返回值。
 */
static void graphics_select_font(uintn hres,uintn vres,uintn width,uintn height)
{
    /*
     * 物理尺寸按顺序的判断依据：
     * 1.量纲：10毫米以下不可能是面板，4095毫米是首选时序图像尺寸字段的满值。
     * 2.区间：15PPI以下或1000PPI以上都不是正常面板。
     * 3.一致性：物理宽高比与像素宽高比偏差超过15%视为错报。
     * 
     * 物理尺寸认为有效后计算DPI，并按以下规则选择字体：
     * 1.DPI大于等于168或DPI大于等于120且宽度大于550mm的屏幕配32px字体。
     * 2.其余DPI大于等于120的屏幕配24px字体。
     * 3.其余情况配16px字体。
     * 
     * 物理尺寸无效情况下兜底策略按以下规则选择字体：
     * 1.竖直分辨率小于等于800配16px字体。
     * 2.竖直分辨率大于800且小于等于1440配24px字体。
     * 3.其余竖直分辨率配32px字体。
     */
    if(width>=10&&width<=4095&&height>=10&&height<=4095)
    {
        uintn dpi_x=hres*254/(width*10);
        uintn dpi_y=vres*254/(height*10);

        if(dpi_x>=15&&dpi_x<=1000&&dpi_y>=15&&dpi_y<=1000)
        {
            uintn big=dpi_x>dpi_y?dpi_x:dpi_y;
            uintn small=dpi_x>dpi_y?dpi_y:dpi_x;

            if(small*100>=big*85)
            {
                uintn dpi=(dpi_x+dpi_y)/2;

                if(dpi>=168||(width>=550&&dpi>=120))
                {
                    font_index=4;
                }
                else if(dpi>=120)
                {
                    font_index=2;
                }
                else
                {
                    font_index=0;
                }
                return;
            }
        }
    }

    /*兜底方案*/
    if(screen_vres/32>=GRAPHICS_FALLBACK_MIN_ROWS)
    {
        font_index=4;
    }
    else if(screen_vres/24>=GRAPHICS_FALLBACK_MIN_ROWS)
    {
        font_index=2;
    }
    else
    {
        font_index=0;
    }
}

/**
 * 获得像素颜色。
 * 
 * @param r 红色分量。
 * @param g 绿色分量。
 * @param b 蓝色分量。
 * 
 * @return 像素颜色。
 */
static uint32 graphics_get_color(uint8 r,uint8 g,uint8 b)
{
    uint32 red=(uint32)((uint64)r*red_max/255);
    uint32 green=(uint32)((uint64)g*green_max/255);
    uint32 blue=(uint32)((uint64)b*blue_max/255);

    return (red<<red_shift)|(green<<green_shift)|(blue<<blue_shift);
}

/**
 * 获取字体宽高。
 * 
 * @param font 字体索引。
 * 
 * @return 无返回值。
 */
static void graphics_get_font_metrics(void)
{
    font_height=16+8*(font_index>>1);

    switch(font_index>>1)
    {
        case 0:
            font_width=FONTS->width16;
            break;
        case 1:
            font_width=FONTS->width24;
            break;
        case 2:
            font_width=FONTS->width32;
            break;
        default:
            font_width=FONTS->width16;
    }
    return;
}

/**
 * 二分查询码元对应的字形下标。
 * 
 * @param codepoint 码元。
 * 
 * @return 字形下标，未找到返回最大值。
 */
static uint32 graphics_bitmap_lookup(char32 codepoint)
{
    if(FONTS==null)
    {
        return UINT32_MAX;
    }

    const uint32* codes=(const uint32*)(console_fonts_start+FONTS->codes);
    uint32 low=0;
    uint32 high=FONTS->count;

    while(low<high)
    {
        uint32 mid=(low+high)>>1;
        uint32 current=codes[mid];

        if(current==codepoint)
        {
            return mid;
        }
        if(current<codepoint)
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
 * 获取码元对应的字形下标。
 * 
 * @param codepoint 码元。
 * 
 * @return 字形下标，未找到返回0xFFFD对应下标。
 */
static uint32 graphics_get_glyph_index(char32 codepoint)
{
    if(codepoint<128)
    {
        return ascii_indexs[codepoint];
    }
    else
    {
        uint32 result=graphics_bitmap_lookup(codepoint);
        return result==UINT32_MAX?ascii_indexs[128]:result;
    }
}

/**
 * 绘制一个8位色彩的字形。
 * 
 * @param column    绘制列。
 * @param row       绘制行。
 * @param codepoint 绘制码元。
 * @param set       置位像素颜色。
 * @param unset     未置位像素颜色。
 * 
 * @return 无返回值。
 */
static void graphics_draw_glyph8(uintn column,uintn row,char32 codepoint,uint32 set,uint32 unset)
{
    uintn index=graphics_get_glyph_index(codepoint);
    uintn glyph_bytes=(font_width*font_height+7)/8;
    const uint8* glyph=(const uint8*)(graphics_get_bitmap(font_index)+index*glyph_bytes);

    uint8* current=(uint8*)((uintn)framebuffer_copy+(screen_base_y+row*font_height)*screen_scan+screen_base_x+
        column*font_width);
    uint8 mask=1;
    uint8 color_set=(uint8)(set&UINT8_MAX);
    uint8 color_unset=(uint8)(unset&UINT8_MAX);
    uintn bitmap=0;
    for(uintn gy=0;gy<font_height;gy++)
    {
        for(uintn gx=0;gx<font_width;gx++)
        {
            current[gx]=glyph[bitmap]&mask?color_set:color_unset;
            mask<<=1;
            if(mask==0)
            {
                mask=1;
                bitmap++;
            }
        }
        current=(uint8*)((uintn)current+screen_scan);
    }
}

/**
 * 绘制一个16位色彩的字形。
 * 
 * @param column    绘制列。
 * @param row       绘制行。
 * @param codepoint 绘制码元。
 * @param set       置位像素颜色。
 * @param unset     未置位像素颜色。
 * 
 * @return 无返回值。
 */
static void graphics_draw_glyph16(uintn column,uintn row,char32 codepoint,uint32 set,uint32 unset)
{
    uintn index=graphics_get_glyph_index(codepoint);
    uintn glyph_bytes=(font_width*font_height+7)/8;
    const uint8* glyph=(const uint8*)(graphics_get_bitmap(font_index)+index*glyph_bytes);

    uint16* current=(uint16*)((uintn)framebuffer_copy+(((screen_base_y+row*font_height)*screen_scan+screen_base_x+
        column*font_width)<<1));
    uint8 mask=1;
    uint16 color_set=(uint16)(set&UINT16_MAX);
    uint16 color_unset=(uint16)(unset&UINT16_MAX);
    uintn bitmap=0;
    for(uintn gy=0;gy<font_height;gy++)
    {
        for(uintn gx=0;gx<font_width;gx++)
        {
            current[gx]=glyph[bitmap]&mask?color_set:color_unset;
            mask<<=1;
            if(mask==0)
            {
                mask=1;
                bitmap++;
            }
        }
        current=(uint16*)((uintn)current+(screen_scan<<1));
    }
}

/**
 * 绘制一个24位色彩的字形。
 * 
 * @param column    绘制列。
 * @param row       绘制行。
 * @param codepoint 绘制码元。
 * @param set       置位像素颜色。
 * @param unset     未置位像素颜色。
 * 
 * @return 无返回值。
 */
static void graphics_draw_glyph24(uintn column,uintn row,char32 codepoint,uint32 set,uint32 unset)
{
    uintn index=graphics_get_glyph_index(codepoint);
    uintn glyph_bytes=(font_width*font_height+7)/8;
    const uint8* glyph=(const uint8*)(graphics_get_bitmap(font_index)+index*glyph_bytes);

    uint8* current=(uint8*)((uintn)framebuffer_copy+((screen_base_y+row*font_height)*screen_scan+screen_base_x+
        column*font_width)*3);
    uint8 mask=1;
    uint8* color_set=(uint8*)&set;
    uint8* color_unset=(uint8*)&unset;
    uintn bitmap=0;
    for(uintn gy=0;gy<font_height;gy++)
    {
        uint8* point=current;
        for(uintn gx=0;gx<font_width;gx++)
        {
            if(glyph[bitmap]&mask)
            {
                point[0]=color_set[0];
                point[1]=color_set[1];
                point[2]=color_set[2];
            }
            else
            {
                point[0]=color_unset[0];
                point[1]=color_unset[1];
                point[2]=color_unset[2];
            }
            point=&point[3];
            mask<<=1;
            if(mask==0)
            {
                mask=1;
                bitmap++;
            }
        }
        current=(uint8*)((uintn)current+screen_scan*3);
    }
}

/**
 * 绘制一个32位色彩的字形。
 * 
 * @param column    绘制列。
 * @param row       绘制行。
 * @param codepoint 绘制码元。
 * @param set       置位像素颜色。
 * @param unset     未置位像素颜色。
 * 
 * @return 无返回值。
 */
static void graphics_draw_glyph32(uintn column,uintn row,char32 codepoint,uint32 set,uint32 unset)
{
    uintn index=graphics_get_glyph_index(codepoint);
    uintn glyph_bytes=(font_width*font_height+7)/8;
    const uint8* glyph=(const uint8*)(graphics_get_bitmap(font_index)+index*glyph_bytes);

    uint32* current=(uint32*)((uintn)framebuffer_copy+(((screen_base_y+row*font_height)*screen_scan+screen_base_x+
        column*font_width)<<2));
    uint8 mask=1;
    uintn bitmap=0;
    for(uintn gy=0;gy<font_height;gy++)
    {
        for(uintn gx=0;gx<font_width;gx++)
        {
            current[gx]=glyph[bitmap]&mask?set:unset;
            mask<<=1;
            if(mask==0)
            {
                mask=1;
                bitmap++;
            }
        }
        current=(uint32*)((uintn)current+(screen_scan<<2));
    }
}

/**
 * 绘制一块8位色彩的矩形。
 * 
 * @param x     起始横坐标。
 * @param y     起始纵坐标。
 * @param w     宽度。
 * @param h     高度。
 * @param color 像素颜色。
 * 
 * @return 无返回值。
 */
static void graphics_draw_rect8(uintn x,uintn y,uintn w,uintn h,uint32 color)
{
    uint8* current=(uint8*)((uintn)framebuffer_copy+y*screen_scan+x);
    for(uintn ry=0;ry<h;ry++)
    {
        memory_set(current,(uint8)(color&UINT8_MAX),w);
        current=(uint8*)((uintn)current+screen_scan);
    }
}

/**
 * 绘制一块16位色彩的矩形。
 * 
 * @param x     起始横坐标。
 * @param y     起始纵坐标。
 * @param w     宽度。
 * @param h     高度。
 * @param color 像素颜色。
 * 
 * @return 无返回值。
 */
static void graphics_draw_rect16(uintn x,uintn y,uintn w,uintn h,uint32 color)
{
    uint16* current=(uint16*)((uintn)framebuffer_copy+((y*screen_scan+x)<<1));
    for(uintn ry=0;ry<h;ry++)
    {
        memory_set16(current,(uint16)(color&UINT16_MAX),w);
        current=(uint16*)((uintn)current+(screen_scan<<1));
    }
}

/**
 * 绘制一块24位色彩的矩形。
 * 
 * @param x     起始横坐标。
 * @param y     起始纵坐标。
 * @param w     宽度。
 * @param h     高度。
 * @param color 像素颜色。
 * 
 * @return 无返回值。
 */
static void graphics_draw_rect24(uintn x,uintn y,uintn w,uintn h,uint32 color)
{
    uint8* current=(uint8*)((uintn)framebuffer_copy+(y*screen_scan+x)*3);
    uint8* pixel=(uint8*)&color;
    for(uintn ry=0;ry<h;ry++)
    {
        uint8* point=current;
        for(uintn rx=0;rx<w;rx++)
        {
            point[0]=pixel[0];
            point[1]=pixel[1];
            point[2]=pixel[2];
            point=&point[3];
        }
        current=(uint8*)((uintn)current+screen_scan*3);
    }
}

/**
 * 绘制一块32位色彩的矩形。
 * 
 * @param x     起始横坐标。
 * @param y     起始纵坐标。
 * @param w     宽度。
 * @param h     高度。
 * @param color 像素颜色。
 * 
 * @return 无返回值。
 */
static void graphics_draw_rect32(uintn x,uintn y,uintn w,uintn h,uint32 color)
{
    uint32* current=(uint32*)((uintn)framebuffer_copy+((y*screen_scan+x)<<2));
    for(uintn ry=0;ry<h;ry++)
    {
        memory_set32(current,color,w);
        current=(uint32*)((uintn)current+(screen_scan<<2));
    }
}

/**
 * 获取屏幕总行数。
 * 
 * @return 总行数。
 */
uintn get_screen_rows(void)
{
    return rows;
}

/**
 * 获取屏幕总列数。
 * 
 * @return 总列数。
 */
uintn get_screen_columns(void)
{
    return columns;
}

/**
 * 获取光标所在行。
 * 
 * @return 光标所在行。
 */
uintn get_cursor_row(void)
{
    return cursor_row;
}

/**
 * 获取光标所在列。
 * 
 * @return 光标所在列。
 */
uintn get_cursor_column(void)
{
    return cursor_column;
}

/**
 * 设置光标位置。
 * 
 * @param row    行。范围[0,rows-1]。
 * @param column 列。范围[0,columns]。
 * 
 * @return 设置成功返回真。
 */
bool set_cursor(uintn row,uintn column)
{
    if(row>=rows||column>columns)
    {
        return false;
    }
    cursor_row=row;
    cursor_column=column;
    return true;
}

/**
 * 设置后续使用的前景色。
 * 
 * @param r 红色分量。
 * @param g 绿色分量。
 * @param b 蓝色分量。
 * 
 * @return 无返回值。
 */
void set_foreground(uint8 r,uint8 g,uint8 b)
{
    foreground_color=graphics_get_color(r,g,b);
}

/**
 * 设置后续使用的背景色。
 * 
 * @param r 红色分量。
 * @param g 绿色分量。
 * @param b 蓝色分量。
 * 
 * @return 无返回值。
 */
void set_background(uint8 r,uint8 g,uint8 b)
{
    background_color=graphics_get_color(r,g,b);
}

/**
 * 发起填充背景色的清屏并将光标恢复为(0,0)。
 * 
 * @return 无返回值。
 */
void clear_screen(void)
{
    uintn char_width=columns*font_width;
    uintn char_height=rows*font_height;
    uintn right=screen_hres-screen_base_x-char_width;
    uintn bottom=screen_vres-screen_base_y-char_height;

    draw_rect(0,0,screen_hres,screen_base_y,ansi_palette[0]);
    draw_rect(0,screen_base_y+char_height,screen_hres,bottom,ansi_palette[0]);
    draw_rect(0,screen_base_y,screen_base_x,char_height,ansi_palette[0]);
    draw_rect(screen_base_x+char_width,screen_base_y,right,char_height,ansi_palette[0]);

    ansi_state=GRAPHICS_ANSI_STATE_TEXT;
    graphics_utf8_remaining=0;
    graphics_utf8_accum=0;
    cursor_column=0;
    cursor_row=0;
    top_row=0;
    dirty_row=0;
    dirty_rows=rows;
    draw_rect(screen_base_x,screen_base_y,char_width,char_height,background_color);
    dirty_edge=true;
}

/**
 * 手动刷新屏幕。
 * 
 * @return 无返回值。
 */
void flush_screen(void)
{
    if(dirty_edge)
    {
        memory_copy(framebuffer,framebuffer_copy,screen_vres*screen_scan*color_bytes);
        dirty_row=0;
        dirty_rows=0;
        dirty_edge=false;
        return;
    }

    uintn row_bytes=screen_scan*color_bytes;
    uintn line_bytes=font_height*row_bytes;
    uintn first=dirty_row;
    uintn count=dirty_rows;

    while(count>0)
    {
        uintn physical=top_row+first;
        if(physical>=rows)
        {
            physical-=rows;
        }
        
        uintn chunk=min(count,rows-physical);
        memory_copy((uint8*)framebuffer+(screen_base_y+first*font_height)*row_bytes,(uint8*)framebuffer_copy+
            (screen_base_y+physical*font_height)*row_bytes,chunk*line_bytes);
        first+=chunk;
        count-=chunk;
    }

    dirty_row=0;
    dirty_rows=0;
}

/**
 * 执行一个已解析完成的ANSI控制序列。
 * 
 * @param final 最终字节。
 * 
 * @return 无返回值。
 */
static void graphics_ansi_execute(char32 final)
{
    switch(final)
    {
        case 'm':
            {
                /*SGR：选择图形再现*/
                if(ansi_count==0)
                {
                    foreground_color=ansi_palette[15];
                    background_color=ansi_palette[0];
                    return;
                }

                for(uintn index=0;index<ansi_count;index++)
                {
                    uintn value=ansi_params[index];
                    uint8 r=0;
                    uint8 g=0;
                    uint8 b=0;

                    if(value==0)
                    {
                        foreground_color=ansi_palette[15];
                        background_color=ansi_palette[0];
                    }
                    else if(value>=30&&value<=37)
                    {
                        foreground_color=ansi_palette[value-30];
                    }
                    else if(value>=90&&value<=97)
                    {
                        foreground_color=ansi_palette[value-90+8];
                    }
                    else if(value>=40&&value<=47)
                    {
                        background_color=ansi_palette[value-40];
                    }
                    else if(value>=100&&value<=107)
                    {
                        background_color=ansi_palette[value-100+8];
                    }
                    else if(value==39)
                    {
                        foreground_color=ansi_palette[15];
                    }
                    else if(value==49)
                    {
                        background_color=ansi_palette[0];
                    }
                    else if((value==38||value==48)&&index+1<ansi_count)
                    {
                        /*扩展颜色：38;5;n 或 38;2;r;g;b*/
                        uintn mode=ansi_params[++index];

                        if(mode==5&&index+1<ansi_count)
                        {
                            uintn entry=ansi_params[++index];

                            if(entry<16)
                            {
                                r=ANSI_PALETTE_RGB[entry][0];
                                g=ANSI_PALETTE_RGB[entry][1];
                                b=ANSI_PALETTE_RGB[entry][2];
                            }
                            else if(entry<232)
                            {
                                /*6x6x6色立方*/
                                entry-=16;
                                r=ANSI_COLOR_CUBE_LEVEL[entry/36];
                                g=ANSI_COLOR_CUBE_LEVEL[(entry/6)%6];
                                b=ANSI_COLOR_CUBE_LEVEL[entry%6];
                            }
                            else
                            {
                                /*24级灰度*/
                                r=g=b=(uint8)(8+10*(entry-232));
                            }

                            if(value==38)
                            {
                                foreground_color=graphics_get_color(r,g,b);
                            }
                            else
                            {
                                background_color=graphics_get_color(r,g,b);
                            }
                        }
                        else if(mode==2&&index+3<ansi_count)
                        {
                            r=(uint8)min(ansi_params[++index],255);
                            g=(uint8)min(ansi_params[++index],255);
                            b=(uint8)min(ansi_params[++index],255);

                            if(value==38)
                            {
                                foreground_color=graphics_get_color(r,g,b);
                            }
                            else
                            {
                                background_color=graphics_get_color(r,g,b);
                            }
                        }
                    }
                }
                return;
            }
        case 'H':
        case 'f':
            {
                /*CUP：光标定位，缺省和0都按1处理*/
                uintn row=ansi_count>0&&ansi_params[0]?ansi_params[0]:1;
                uintn column=ansi_count>1&&ansi_params[1]?ansi_params[1]:1;
                cursor_row=min(row,rows)-1;
                cursor_column=min(column,columns+1)-1;
                return;
            }
        case 'J':
            {
                /*ED：擦除显示区域*/
                uintn mode=ansi_count>0?ansi_params[0]:0;

                if(mode==2||mode==3)
                {
                    uintn row=cursor_row;
                    uintn column=cursor_column;
                    clear_screen();
                    cursor_row=row;
                    cursor_column=column;
                }
                return;
            }
        case 'K':
            {
                /*EL：擦除行内区域*/
                uintn mode=ansi_count>0?ansi_params[0]:0;
                uintn first=0;
                uintn count=columns;

                if(mode==0)
                {
                    first=cursor_column;
                    count=columns-cursor_column;
                }
                else if(mode==1)
                {
                    count=min(cursor_column+1,columns);
                }
                else if(mode!=2)
                {
                    return;
                }

                draw_rect(screen_base_x+first*font_width,screen_base_y+((top_row+cursor_row)%rows)*font_height,
                    count*font_width,font_height,background_color);

                if(dirty_rows==0)
                {
                    dirty_row=cursor_row;
                    dirty_rows=1;
                }
                else if(dirty_rows<rows)
                {
                    if(dirty_row<=cursor_row)
                    {
                        if(dirty_row+dirty_rows<=cursor_row+1)
                        {
                            dirty_rows=cursor_row+1-dirty_row;
                        }
                    }
                    else
                    {
                        uintn dirty_end=dirty_row+dirty_rows;
                        dirty_row=cursor_row;
                        dirty_rows=dirty_end-cursor_row;
                    }
                }
                return;
            }
        case 'A':
        case 'B':
        case 'C':
        case 'D':
            {
                /*CUU/CUD/CUF/CUB：光标相对移动*/
                uintn amount=ansi_count>0&&ansi_params[0]?ansi_params[0]:1;
                uintn row=cursor_row;
                uintn column=cursor_column;

                if(final=='A')
                {
                    row=row>amount?row-amount:0;
                }
                else if(final=='B')
                {
                    row=min(row+amount,rows-1);
                }
                else if(final=='C')
                {
                    column=min(column+amount,columns);
                }
                else
                {
                    column=column>amount?column-amount:0;
                }

                cursor_row=row;
                cursor_column=column;
                return;
            }
        default:
            {
                return;
            }
    }
}

/**
 * 将码元进行ANSI转义序列解析。
 * 
 * @param codepoint 码元。
 * 
 * @return 成功消费返回真。
 */
static bool graphics_ansi_parse(char32 codepoint)
{
    if(codepoint==GRAPHICS_ANSI_ESCAPE)
    {
        /*转义序列起点，重置全部解析状态*/
        ansi_state=GRAPHICS_ANSI_STATE_ESCAPE;
        ansi_count=0;
        ansi_params[0]=0;
        return true;
    }

    if(ansi_state==GRAPHICS_ANSI_STATE_TEXT||codepoint<0x20||codepoint>0x7F)
    {
        ansi_state=GRAPHICS_ANSI_STATE_TEXT;
        return false;
    }

    switch(ansi_state)
    {
        case GRAPHICS_ANSI_STATE_ESCAPE:
            {
                /*等待序列引导字符*/
                if(codepoint==GRAPHICS_ANSI_CSI_CHAR)
                {
                    ansi_state=GRAPHICS_ANSI_STATE_CSI;
                }
                else if(codepoint>=0x20&&codepoint<=0x2F)
                {
                    ansi_state=GRAPHICS_ANSI_STATE_IGNORE;
                }
                else
                {
                    ansi_state=GRAPHICS_ANSI_STATE_TEXT;
                }
                return true;
            }
        case GRAPHICS_ANSI_STATE_CSI:
            {
                if(codepoint>='0'&&codepoint<='9')
                {
                    /*累积当前参数，超出上限则钳制*/
                    if(ansi_count<GRAPHICS_ANSI_PARAM_MAX)
                    {
                        ansi_params[ansi_count]=min(ansi_params[ansi_count]*10+(uintn)(codepoint-'0'),
                            GRAPHICS_ANSI_PARAM_LIMIT);
                    }
                }
                else if(codepoint==';')
                {
                    /*进入下一个参数*/
                    if(ansi_count+1<GRAPHICS_ANSI_PARAM_MAX)
                    {
                        ansi_count++;
                    }
                    ansi_params[ansi_count]=0;
                }
                else if(codepoint=='?'||codepoint=='>'||codepoint=='<'||codepoint=='=')
                {
                    /*私有前缀，不参与参数计算*/
                    ansi_state=GRAPHICS_ANSI_STATE_IGNORE;
                }
                else if(codepoint>=GRAPHICS_ANSI_FINAL_MIN&&codepoint<=GRAPHICS_ANSI_FINAL_MAX)
                {
                    /*最终字节，结束序列并执行*/
                    if(ansi_count<GRAPHICS_ANSI_PARAM_MAX)
                    {
                        ansi_count++;
                    }
                    ansi_state=GRAPHICS_ANSI_STATE_TEXT;
                    graphics_ansi_execute(codepoint);
                }
                else
                {
                    /*非法字节，丢弃到下一个最终字节*/
                    ansi_state=GRAPHICS_ANSI_STATE_IGNORE;
                }
                return true;
            }
        case GRAPHICS_ANSI_STATE_IGNORE:
            {
                if(codepoint>=GRAPHICS_ANSI_FINAL_MIN&&codepoint<=GRAPHICS_ANSI_FINAL_MAX)
                {
                    ansi_state=GRAPHICS_ANSI_STATE_TEXT;
                }
                return true;
            }
        default:
            {
                ansi_state=GRAPHICS_ANSI_STATE_TEXT;
                return false;
            }
    }
}

/**
 * 将码元解析为对应图形行为。
 * 
 * @param codepoint 码元。
 * 
 * @return 无返回值。
 */
static void graphics_codepoint_parse(char32 codepoint)
{
    if(graphics_ansi_parse(codepoint))
    {
        return;
    }

    if(codepoint=='\n'||codepoint=='\v')
    {
        /*光标移动到下一行相同位置，后自动滚动*/
        cursor_row++;

        if(cursor_row>=rows)
        {
            cursor_row--;
            top_row++;
            if(top_row>=rows)
            {
                top_row=0;
            }

            dirty_row=0;
            dirty_rows=rows;

            draw_rect(screen_base_x,screen_base_y+((top_row+cursor_row)%rows)*font_height,columns*font_width,
                font_height,background_color);
        }
    }
    else if(codepoint=='\r')
    {
        /*光标退到行首*/
        cursor_column=0;
    }
    else if(codepoint=='\t')
    {
        /*光标到下一制表位/行末*/
        uintn next=((cursor_column>>3)+1)<<3;
        cursor_column=min(columns,next);
    }
    else if(codepoint=='\b')
    {
        /*光标退到上一个位置*/
        if(cursor_column==0)
        {
            if(cursor_row>0)
            {
                cursor_row--;
                cursor_column=columns;
            }
        }
        else
        {
            cursor_column--;
        }
    }
    else if(codepoint=='\f')
    {
        /*清屏*/
        clear_screen();
    }
    else if(!(codepoint<0x20||codepoint==0x7F))
    {
        /*先自动换行，再自动滚动，最后绘制字形*/
        if(cursor_column>=columns)
        {
            cursor_column=0;
            cursor_row++;
        }

        if(cursor_row>=rows)
        {
            cursor_row--;
            top_row++;
            if(top_row>=rows)
            {
                top_row=0;
            }

            dirty_row=0;
            dirty_rows=rows;

            draw_rect(screen_base_x,screen_base_y+((top_row+cursor_row)%rows)*font_height,columns*font_width,
                font_height,background_color);
        }

        draw_glyph(cursor_column,(top_row+cursor_row)%rows,codepoint,foreground_color,
            background_color);
        cursor_column++;

        if(dirty_rows==0)
        {
            dirty_row=cursor_row;
            dirty_rows=1;
        }
        else if(dirty_rows<rows)
        {
            if(dirty_row<=cursor_row)
            {
                if(dirty_row+dirty_rows<=cursor_row+1)
                {
                    dirty_rows=cursor_row+1-dirty_row;
                }
            }
            else
            {
                uintn dirty_end=dirty_row+dirty_rows;
                dirty_row=cursor_row;
                dirty_rows=dirty_end-cursor_row;
            }
        }
    }
}

/**
 * 将UTF8字符流式解码成UTF32字符并解析。
 * 
 * @param c UTF8字符。
 * 
 * @return 无返回值。
 */
static void graphics_decode_utf8(char8 c)
{
    uint8 byte=(uint8)c;

    if(graphics_utf8_remaining==0)
    {
        if(byte<0x80)
        {
            graphics_codepoint_parse(byte);
        }
        else if((byte&0xE0)==0xC0)
        {
            graphics_utf8_accum=(char32)(byte&0x1F);
            graphics_utf8_remaining=1;
        }
        else if((byte&0xF0)==0xE0)
        {
            graphics_utf8_accum=(char32)(byte&0x0F);
            graphics_utf8_remaining=2;
        }
        else if((byte&0xF8)==0xF0)
        {
            graphics_utf8_accum=(char32)(byte&0x07);
            graphics_utf8_remaining=3;
        }
        else
        {
            graphics_codepoint_parse(0xFFFD);
        }
        return;
    }

    if((byte&0xC0)!=0x80)
    {
        graphics_utf8_accum=0;
        graphics_utf8_remaining=0;
        graphics_codepoint_parse(0xFFFD);
        graphics_decode_utf8(c);
        return;
    }

    graphics_utf8_accum=(graphics_utf8_accum<<6)|(char32)(byte&0x3F);

    if(--graphics_utf8_remaining==0)
    {
        char32 code=graphics_utf8_accum;
        graphics_utf8_accum=0;

        if(code>0x10FFFF||(code>=0xD800&&code<=0xDFFF))
        {
            code=0xFFFD;
        }
        graphics_codepoint_parse(code);
    }
}

/**
 * 图形UTF8句柄读取。
 * 
 * @param handle 句柄。
 * @param dest   目标数组。
 * @param n      请求读取长度。
 * 
 * @return 屏幕不可读，返回0。
 */
static uintn graphics_utf8_read(io_handle* handle,uint8* dest,uintn n)
{
    (void)handle;
    (void)dest;
    (void)n;
    graphics_utf8_status=IO_STATUS_NOT_SUPPORTED;
    return 0;
}

/**
 * 图形UTF8句柄写入。
 * 
 * @param handle 句柄。
 * @param src    源数组。
 * @param n      请求写入长度。
 * 
 * @return 实际写入长度，恒等于请求长度。
 */
static uintn graphics_utf8_write(io_handle* handle,const uint8* src,uintn n)
{
    (void)handle;
    if(src==null)
    {
        graphics_utf8_status=IO_STATUS_DEVICE_ERROR;
        return 0;
    }

    for(uintn index=0;index<n;index++)
    {
        graphics_decode_utf8((char8)src[index]);
    }

    graphics_utf8_status=IO_STATUS_SUCCESS;
    return n;
}

/**
 * 图形UTF8句柄刷新。
 * 
 * @param handle 句柄。
 * 
 * @return 状态码。
 */
static uint64 graphics_utf8_flush(io_handle* handle)
{
    (void)handle;
    if(graphics_utf8_remaining>0)
    {
        graphics_utf8_accum=0;
        graphics_utf8_remaining=0;
        graphics_codepoint_parse(0xFFFD);
    }
    ansi_state=GRAPHICS_ANSI_STATE_TEXT;

    flush_screen();

    graphics_utf8_status=IO_STATUS_SUCCESS;
    return graphics_utf8_status;
}

/**
 * 图形UTF8句柄获取位置。
 * 
 * @param handle 句柄。
 * 
 * @return 光标线性位置，按字符单元计。
 */
static uint64 graphics_utf8_get_position(io_handle* handle)
{
    (void)handle;
    graphics_utf8_status=IO_STATUS_SUCCESS;
    return (uint64)cursor_row*columns+cursor_column;
}

/**
 * 图形UTF8句柄设置位置。句柄不支持寻址。
 * 
 * @param handle   句柄。
 * @param position 目标位置。
 * 
 * @return 操作不支持。
 */
static uint64 graphics_utf8_set_position(io_handle* handle,uint64 position)
{
    (void)handle;
    (void)position;
    graphics_utf8_status=IO_STATUS_NOT_SUPPORTED;
    return graphics_utf8_status;
}

/**
 * 图形UTF8句柄获取大小。
 * 
 * @param handle 句柄。
 * 
 * @return 字符单元总数。
 */
static uint64 graphics_utf8_get_size(io_handle* handle)
{
    (void)handle;
    graphics_utf8_status=IO_STATUS_SUCCESS;
    return (uint64)columns*rows*sizeof(char32);
}

/**
 * 图形UTF8句柄获取能力。
 * 
 * @param handle 句柄。
 * 
 * @return 只写能力。
 */
static uint64 graphics_utf8_get_capabilities(io_handle* handle)
{
    (void)handle;
    graphics_utf8_status=IO_STATUS_SUCCESS;
    return IO_CAPABILITY_WRITABLE;
}

/**
 * 图形UTF8句柄获取状态。
 * 
 * @param handle 句柄。
 * 
 * @return 最近一次状态码，读取后重置。
 */
static uint64 graphics_utf8_get_io_status(io_handle* handle)
{
    uint64 status=graphics_utf8_status;

    (void)handle;
    graphics_utf8_status=IO_STATUS_SUCCESS;
    return status;
}

/**
 * 图形UTF8句柄关闭。句柄不支持关闭。
 * 
 * @param handle 句柄。
 * 
 * @return 操作不支持。
 */
static uint64 graphics_utf8_close(io_handle* handle)
{
    (void)handle;
    graphics_utf8_status=IO_STATUS_NOT_SUPPORTED;
    return graphics_utf8_status;
}

/**
 * 初始化控制台图形显示。
 * 
 * @param params 启动参数。
 * 
 * @return 成功初始化返回真。
 */
bool console_graphics_init(const aos_boot_params* params)
{
    if(params==null||params->graphics.hres==0||params->graphics.vres==0||
        params->graphics.scan_line<params->graphics.hres)
    {
        return false;
    }

    uintn test_base=-2*SIZE_512GB;
    uintn test_limit=-SIZE_512GB;

    if(params->graphics.kernel_fb_base<test_base||params->graphics.kernel_fb_base>=test_limit)
    {
        return false;
    }

    test_base=-(3*SIZE_512GB-SIZE_128GB);
    test_limit=-(3*SIZE_512GB-SIZE_256GB);

    if(params->graphics.kernel_copy_base<test_base||params->graphics.kernel_copy_base>=test_limit)
    {
        return false;
    }

    framebuffer=(void*)params->graphics.kernel_fb_base;
    framebuffer_copy=(void*)params->graphics.kernel_copy_base;
    framebuffer_size=params->graphics.framebuffer_size;
    framebuffer_copy_size=params->graphics.kernel_copy_size;

    screen_hres=params->graphics.hres;
    screen_vres=params->graphics.vres;
    screen_scan=params->graphics.scan_line;

    red_shift=(uint8)count_trailing_zeros(params->graphics.red_mask)%32;
    red_max=params->graphics.red_mask>>red_shift;
    blue_shift=(uint8)count_trailing_zeros(params->graphics.blue_mask)%32;
    blue_max=params->graphics.blue_mask>>blue_shift;
    green_shift=(uint8)count_trailing_zeros(params->graphics.green_mask)%32;
    green_max=params->graphics.green_mask>>green_shift;

    uint32 color_max=params->graphics.red_mask|params->graphics.blue_mask|params->graphics.green_mask|
        params->graphics.reserved_mask;
    color_bytes=(uint8)((32-count_leading_zeros(color_max)+7)/8);

    if(color_bytes==0)
    {
        return false;
    }
    else
    {
        switch(color_bytes)
        {
            case 1:
                draw_glyph=graphics_draw_glyph8;
                draw_rect=graphics_draw_rect8;
                break;
            case 2:
                draw_glyph=graphics_draw_glyph16;
                draw_rect=graphics_draw_rect16;
                break;
            case 3:
                draw_glyph=graphics_draw_glyph24;
                draw_rect=graphics_draw_rect24;
                break;
            case 4:
            default:
                draw_glyph=graphics_draw_glyph32;
                draw_rect=graphics_draw_rect32;
                break;
        }
    }

    uintn request=(uintn)screen_scan*screen_vres*color_bytes;
    if(framebuffer_size<request||framebuffer_copy_size<request)
    {
        return false;
    }

    graphics_select_font(screen_hres,screen_vres,params->graphics.physical_width,
        params->graphics.physical_height);
    graphics_get_font_metrics();
    if(font_width==0||font_height==0)
    {
        return false;
    }
    columns=screen_hres/font_width;
    rows=screen_vres/font_height;

    if(rows*columns<80*24&&font_index>=2)
    {
        font_index=((font_index>>1)-1)<<1;
        graphics_get_font_metrics();
        columns=screen_hres/font_width;
        rows=screen_vres/font_height;

        if(rows*columns<80*24&&font_index==2)
        {
            font_index=0;
            graphics_get_font_metrics();
            columns=screen_hres/font_width;
            rows=screen_vres/font_height;
        }
    }

    if(rows*columns==0)
    {
        return false;
    }
    screen_base_x=(screen_hres%font_width)>>1;
    screen_base_y=(screen_vres%font_height)>>1;
    cursor_column=0;
    cursor_row=0;
    dirty_row=0;
    dirty_rows=0;

    background_color=graphics_get_color(0,0,0);
    foreground_color=graphics_get_color(255,255,255);

    ascii_indexs[128]=graphics_bitmap_lookup(0xFFFD);
    if(ascii_indexs[128]==UINT32_MAX)
    {
        return false;
    }
    const uint32* codes=(const uint32*)(console_fonts_start+FONTS->codes);
    for(uint32 code=0,index=0;code<128;code++)
    {
        if(code<codes[index])
        {
            ascii_indexs[code]=ascii_indexs[128];
        }
        else
        {
            ascii_indexs[code]=index++;
        }
    }

    for(uintn palette=0;palette<16;palette++)
    {
        ansi_palette[palette]=graphics_get_color(ANSI_PALETTE_RGB[palette][0],ANSI_PALETTE_RGB[palette][1],
            ANSI_PALETTE_RGB[palette][2]);
    }

    graphics_utf8_handle.magic=GRAPHICS_UTF8_HANDLE_MAGIC;
    graphics_utf8_handle.read=graphics_utf8_read;
    graphics_utf8_handle.write=graphics_utf8_write;
    graphics_utf8_handle.flush=graphics_utf8_flush;
    graphics_utf8_handle.get_position=graphics_utf8_get_position;
    graphics_utf8_handle.set_position=graphics_utf8_set_position;
    graphics_utf8_handle.get_size=graphics_utf8_get_size;
    graphics_utf8_handle.get_capabilities=graphics_utf8_get_capabilities;
    graphics_utf8_handle.get_io_status=graphics_utf8_get_io_status;
    graphics_utf8_handle.close=graphics_utf8_close;

    clear_screen();
    flush_screen();

    return true;
}

/**
 * 在控制台图形界面上格式化输出字符串。
 * 
 * @param format 格式化字符串。
 * 
 * @return 输出状态码。
 */
uintn console_graphics_print(const char8* format,...)
{
    va_list args;

    va_start(args,format);
    uintn result=format_print_valist(&graphics_utf8_handle,format,args);
    va_end(args);

    return result;
}

/**
 * 在控制台图形界面上通过可变参数列表格式化输出字符串。
 * 
 * @param format 格式化字符串。
 * @param args   可变参数列表。
 * 
 * @return 输出状态码。
 */
uintn console_graphics_print_valist(const char8* format,va_list args)
{
    va_list list;

    va_copy(list,args);
    uintn result=format_print_valist(&graphics_utf8_handle,format,list);
    va_end(list);

    return result;
}