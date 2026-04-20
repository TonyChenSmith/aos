/**
 * 内核格式化输出库函数。
 * @date 2026-04-14
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#include <support/char.h>
#include <support/convert.h>
#include <support/io_handle.h>
#include <support/memory.h>
#include <support/string.h>
#include <support/util.h>
#include <support/varargs.h>

/**
 * 输出器。
 */
typedef struct _output_writer
{
    bool  (*put_char)(void* context,char8 c);                         /*输出字符。*/
    bool  (*put_string)(void* context,const char8* str,uintn length); /*输出字符串。*/
    bool  (*put_repeat)(void* context,char8 c,uintn count);           /*重复输出字符。*/
    uintn (*get_written_chars)(void* context);                        /*获取已输出字符数。*/
} output_writer;

/**
 * 解析状态。
 */
typedef enum _parse_state
{
    STATE_NORMAL,    /*一般字符。*/
    STATE_PERCENT,   /*百分号。*/
    STATE_WIDTH,     /*宽度。*/
    STATE_PRECISION  /*精度。*/
} parse_state;

/**
 * 格式说明符。
 */
typedef struct _format_specifier
{
    bool  left_align; /*左对齐。*/
    uintn width;      /*宽度。*/
    uintn precision;  /*精度。*/
    char8 specifier;  /*说明符。*/
} format_specifier;

/**
 * 格式化处理输出有符号整数。
 * 
 * @param writer  输出器。
 * @param context 上下文。
 * @param spec    格式说明符。
 * @param value   输出数值。
 * 
 * @return 正常输出返回真。
 */
static bool format_process_int(output_writer* writer,void* context,format_specifier* spec,int64 value)
{
    char8 buffer[21];
    
    if(!int_to_string8(buffer,value,sizeof(buffer)))
    {
        return false;
    }
    
    bool neg=buffer[0]=='-';
    uintn digits=neg?string_length8(&buffer[1]):string_length8(buffer);
    uintn zero=spec->precision>digits?spec->precision-digits:0;
    uintn width=digits+zero+(neg?1:0);
    width=spec->width>width?spec->width-width:0;

    if(spec->left_align)
    {
        if(neg)
        {
            if(!writer->put_char(context,'-'))
            {
                return false;
            }
        }
        if(!writer->put_repeat(context,'0',zero))
        {
            return false;
        }
        if(!writer->put_string(context,neg?&buffer[1]:buffer,digits))
        {
            return false;
        }
        if(!writer->put_repeat(context,' ',width))
        {
            return false;
        }
    }
    else
    {
        if(!writer->put_repeat(context,' ',width))
        {
            return false;
        }
        if(neg)
        {
            if(!writer->put_char(context,'-'))
            {
                return false;
            }
        }
        if(!writer->put_repeat(context,'0',zero))
        {
            return false;
        }
        if(!writer->put_string(context,neg?&buffer[1]:buffer,digits))
        {
            return false;
        }
    }
    return true;
}

/**
 * 格式化处理输出无符号整数。
 * 
 * @param writer  输出器。
 * @param context 上下文。
 * @param spec    格式说明符。
 * @param value   输出数值。
 * @param scale   进制。
 * 
 * @return 正常输出返回真。
 */
static bool format_process_uint(output_writer* writer,void* context,format_specifier* spec,uint64 value,uint8 scale)
{
    char8 buffer[65];
    
    if(!uint_to_string8(buffer,value,sizeof(buffer),scale))
    {
        return false;
    }
    
    uintn digits=string_length8(buffer);
    if(spec->specifier=='p'||spec->specifier=='x'||spec->specifier=='h')
    {
        for(uintn i=0;i<digits;i++)
        {
            if(buffer[i]>='A')
            {
                buffer[i]=buffer[i]-'A'+'a';
            }
        }
    }
    uintn zero=spec->precision>digits?spec->precision-digits:0;
    uintn width=digits+zero;
    width=spec->width>width?spec->width-width:0;

    if(spec->left_align)
    {
        if(!writer->put_repeat(context,'0',zero))
        {
            return false;
        }
        if(!writer->put_string(context,buffer,digits))
        {
            return false;
        }
        if(!writer->put_repeat(context,' ',width))
        {
            return false;
        }
    }
    else
    {
        if(!writer->put_repeat(context,' ',width))
        {
            return false;
        }
        if(!writer->put_repeat(context,'0',zero))
        {
            return false;
        }
        if(!writer->put_string(context,buffer,digits))
        {
            return false;
        }
    }
    return true;
}

/**
 * 格式化处理输出字符。
 * 
 * @param writer  输出器。
 * @param context 上下文。
 * @param c       字符。
 * 
 * @return 正常输出返回真。
 */
static bool format_process_char(output_writer* writer,void* context,char32 c)
{
    if(c>0x10FFFF)
    {
        c=0xFFFD;
    }

    char32 src[2]={c,0};
    char8 buffer[5];
    uintn length=string_convert8_char32(buffer,src,5)-1;
    return writer->put_string(context,buffer,length);
}

/**
 * 格式化处理输出字符串。
 * 
 * @param writer  输出器。
 * @param context 上下文。
 * @param spec    格式说明符。
 * @param str     字符串。
 * 
 * @return 正常输出返回真。
 */
static bool format_process_string(output_writer* writer,void* context,format_specifier* spec,const char8* str)
{
    if(str==null)
    {
        str="";
    }
    
    uintn length=min(string_length8(str),spec->precision);
    uintn space=spec->width>length?spec->width-length:0;
    
    if(spec->left_align)
    {
        if(!writer->put_string(context,str,length))
        {
            return false;
        }
        if(!writer->put_repeat(context,' ',space))
        {
            return false;
        }
    }
    else
    {
        if(!writer->put_repeat(context,' ',space))
        {
            return false;
        }
        if(!writer->put_string(context,str,length))
        {
            return false;
        }
    }
    
    return true;
}

/**
 * 处理格式说明符。
 * 
 * @param writer  输出器。
 * @param context 上下文。
 * @param spec    格式说明符。
 * @param args    参数列表。
 * 
 * @return 正常输出返回真。
 */
static bool format_process_specifier(output_writer* writer,void* context,format_specifier* spec,va_list* args)
{
    switch(spec->specifier)
    {
        case 'i':
        {
            spec->precision=max(spec->precision,1);
            return format_process_int(writer,context,spec,va_arg(*args,int32));
        }
        case 'I':
        {
            spec->precision=max(spec->precision,1);
            return format_process_int(writer,context,spec,va_arg(*args,int64));
        }
        case 'u':
        {
            spec->precision=max(spec->precision,1);
            return format_process_uint(writer,context,spec,va_arg(*args,uint32),10);
        }
        case 'U':
        {
            spec->precision=max(spec->precision,1);
            return format_process_uint(writer,context,spec,va_arg(*args,uint64),10);
        }
        case 'c':
        {
            return format_process_char(writer,context,va_arg(*args,char32));
        }
        case 's':
        {
            spec->precision=spec->precision==0?UINTN_MAX:spec->precision;
            return format_process_string(writer,context,spec,va_arg(*args,const char8*));
        }
        case 'h':
        {
            spec->precision=max(spec->precision,1);
            return format_process_uint(writer,context,spec,va_arg(*args,uint32),16);
        }
        case 'H':
        {
            spec->precision=max(spec->precision,1);
            return format_process_uint(writer,context,spec,va_arg(*args,uint32),16);
        }
        case 'x':
        {
            spec->precision=max(spec->precision,1);
            return format_process_uint(writer,context,spec,va_arg(*args,uint64),16);
        }
        case 'X':
        {
            spec->precision=max(spec->precision,1);
            return format_process_uint(writer,context,spec,va_arg(*args,uint64),16);
        }
        case 'n':
        {
            spec->precision=max(spec->precision,1);
            return format_process_int(writer,context,spec,va_arg(*args,intn));
        }
        case 'N':
        {
            spec->precision=max(spec->precision,1);
            return format_process_uint(writer,context,spec,va_arg(*args,uintn),10);
        }
        case 'p':
        {
            spec->width=0;
            spec->precision=sizeof(uintn)<<1;
            return format_process_uint(writer,context,spec,va_arg(*args,uintn),16);
        }
        case 'P':
        {
            spec->width=0;
            spec->precision=sizeof(uintn)<<1;
            return format_process_uint(writer,context,spec,va_arg(*args,uintn),16);
        }
        case 'b':
        {
            spec->precision=max(spec->precision,1);
            return format_process_uint(writer,context,spec,va_arg(*args,uint32),2);
        }
        case 'B':
        {
            spec->precision=max(spec->precision,1);
            return format_process_uint(writer,context,spec,va_arg(*args,uint64),2);
        }
        default:
        case '%':
        {
            return writer->put_char(context,spec->specifier);
        }
    }
}

/**
 * 格式化引擎。
 * 
 * @param writer  输出器。
 * @param context 上下文。
 * @param format  格式字符串。
 * @param args    参数列表。
 * 
 * @return 无返回值。
 */
static void format_engine(output_writer* writer,void* context,const char8* format,va_list* args)
{
    parse_state state=STATE_NORMAL;
    format_specifier spec;
    
    for(const char8* p=format;*p!='\0';p++) {
        switch(state)
        {
            case STATE_PERCENT:
                if(*p=='-')
                {
                    spec.left_align=true;
                    state=STATE_WIDTH;
                }
                else if(ascii_is_digit(*p))
                {
                    spec.width=*p-'0';
                    state=STATE_WIDTH;
                }
                else if(*p=='.')
                {
                    spec.precision=0;
                    state=STATE_PRECISION;
                }
                else
                {
                    spec.specifier=*p;
                    if(!format_process_specifier(writer,context,&spec,args))
                    {
                        return;
                    }
                    state=STATE_NORMAL;
                }
                break;
            case STATE_WIDTH:
                if(ascii_is_digit(*p))
                {
                    spec.width=spec.width*10+(*p-'0');
                    state=STATE_WIDTH;
                }
                else if(*p=='.')
                {
                    spec.precision=0;
                    state=STATE_PRECISION;
                }
                else
                {
                    spec.specifier=*p;
                    if(!format_process_specifier(writer,context,&spec,args))
                    {
                        return;
                    }
                    state=STATE_NORMAL;
                }
                break;
            case STATE_PRECISION:
                if(ascii_is_digit(*p))
                {
                    spec.precision=spec.precision*10+(*p-'0');
                    state=STATE_PRECISION;
                }
                else
                {
                    spec.specifier=*p;
                    if(!format_process_specifier(writer,context,&spec,args))
                    {
                        return;
                    }
                    state=STATE_NORMAL;
                }
                break;
            default:
                state=STATE_NORMAL;
            case STATE_NORMAL:
                if(*p=='%')
                {
                    state=STATE_PERCENT;
                    memory_zero(&spec,sizeof(spec));
                }
                else
                {
                    if(!writer->put_char(context,*p))
                    {
                        return;
                    }
                }
                break;
        }
    }
}

/**
 * 输入输出句柄输出器上下文。
 */
typedef struct _io_writer_context
{
    io_handle* handle;  /*输入输出句柄。*/
    uint64     status;  /*句柄状态。*/
    uintn      written; /*已写入字符数。*/
} io_writer_context;

/**
 * 输入输出句柄输出字符。
 * 
 * @param context 输入输出句柄输出器上下文。
 * @param c       输出字符。
 * 
 * @return 输出成功返回真，失败返回假。
 */
static bool format_io_put_char(void* context,char8 c)
{
    io_writer_context* ctx=(io_writer_context*)context;
    if(ctx->status!=IO_STATUS_SUCCESS)
    {
        return false;
    }

    uint8 byte=(uint8)c;
    uintn written=ctx->handle->write(ctx->handle,&byte,1);
    if(written==0)
    {
        ctx->status=ctx->handle->get_io_status(ctx->handle);
        return false;
    }
    ctx->written++;
    return true;
}

/**
 * 输入输出句柄输出字符串。
 * 
 * @param context 输入输出句柄输出器上下文。
 * @param str     输出字符串。
 * @param length  输出上限长度。
 * 
 * @return 输出成功返回真，失败返回假。
 */
static bool format_io_put_string(void* context,const char8* str,uintn length)
{
    io_writer_context* ctx=(io_writer_context*)context;
    if(ctx->status!=IO_STATUS_SUCCESS)
    {
        return false;
    }

    uintn written=ctx->handle->write(ctx->handle,(const uint8*)str,length);
    ctx->written+=written;
    if(written!=length)
    {
        ctx->status=ctx->handle->get_io_status(ctx->handle);
        return false;
    }
    return true;
}

/**
 * 输入输出句柄重复输出字符。
 * 
 * @param context 输入输出句柄输出器上下文。
 * @param c       输出字符。
 * @param count   输出次数。
 * 
 * @return 输出成功返回真，失败返回假。
 */
static bool format_io_put_repeat(void* context,char8 c,uintn count)
{
    io_writer_context* ctx=(io_writer_context*)context;
    if(ctx->status!=IO_STATUS_SUCCESS)
    {
        return false;
    }

    while(count>0)
    {
        uint8 byte=(uint8)c;
        uintn written=ctx->handle->write(ctx->handle,&byte,1);
        if(written==0)
        {
            ctx->status=ctx->handle->get_io_status(ctx->handle);
            return false;
        }
        ctx->written++;
        count--;
    }
    return true;
}

/**
 * 获取输入输出句柄输出器上下文已写入字符数。
 * 
 * @param context 输入输出句柄输出器上下文。
 * 
 * @return 返回已写入字符数。
 */
static uintn format_io_get_written_chars(void* context)
{
    return ((io_writer_context*)context)->written;
}

/**
 * 字符串格式化输出到输入输出句柄，参数通过可变参数列表提供。这里输出字符串按UTF-8编码。
 * 
 * @param handle 输入输出句柄。要求其可写能力。
 * @param format 格式化字符串。
 * @param args   可变参数列表。
 * 
 * @return 返回输出状态码。
 */
uintn format_print_valist(io_handle* handle,const char8* format,va_list args)
{
    if(handle==null||format==null) {
        return IO_STATUS_DEVICE_ERROR;
    }

    if(!(handle->get_capabilities(handle)&IO_CAPABILITY_WRITABLE))
    {
        return IO_STATUS_NOT_SUPPORTED;
    }
    
    output_writer writer={format_io_put_char,format_io_put_string,format_io_put_repeat,format_io_get_written_chars};
    io_writer_context context={handle,IO_STATUS_SUCCESS,0};
    
    format_engine(&writer,&context,format,(va_list*)&args);
    
    if(context.status!=IO_STATUS_SUCCESS)
    {
        return context.status;
    }
    return handle->flush(handle);
}

/**
 * 字符串格式化输出到输入输出句柄。这里输出字符串按UTF-8编码。
 * 
 * @param handle 输入输出句柄。要求其可写能力。
 * @param format 格式化字符串。
 * 
 * @return 返回输出状态码。
 */
uint64 format_print(io_handle* handle,const char8* format,...)
{
    va_list args;
    va_start(args,format);
    uint64 result=format_print_valist(handle,format,args);
    va_end(args);
    return result;
}

/**
 * 缓存区输出器上下文。
 */
typedef struct _buffer_writer_context
{
    char8* buffer; /*缓存区。*/
    uintn  size;   /*缓存区大小。*/
    uintn  used;   /*已使用字符数。*/
} buffer_writer_context;

/**
 * 缓存区输出字符。
 * 
 * @param context 缓存区输出器上下文。
 * @param c       输出字符。
 * 
 * @return 输出成功返回真，失败返回假。
 */
static bool format_buffer_put_char(void* context,char8 c)
{
    buffer_writer_context* ctx=(buffer_writer_context*)context;
    
    if(ctx->used<ctx->size)
    {
        ctx->buffer[ctx->used++]=c;
        return true;
    }
    else
    {
        return false;
    }
}

/**
 * 缓存区输出字符。
 * 
 * @param context 缓存区输出器上下文。
 * @param str     输出字符串。
 * @param length  输出上限长度。
 * 
 * @return 输出成功返回真，失败返回假。
 */
static bool format_buffer_put_string(void* context,const char8* str,uintn length)
{
    buffer_writer_context* ctx=(buffer_writer_context*)context;
    
    uintn available=ctx->size-ctx->used;
    uintn to_copy=min(length,available);
    
    if(to_copy>0)
    {
        memory_copy(&ctx->buffer[ctx->used],str,to_copy);
        ctx->used+=to_copy;
    }
    
    return to_copy==length;
}

/**
 * 缓存区重复输出字符。
 * 
 * @param context 缓存区输出器上下文。
 * @param c       输出字符。
 * @param count   输出次数。
 * 
 * @return 输出成功返回真，失败返回假。
 */
static bool format_buffer_put_repeat(void* context,char8 c,uintn count)
{
    buffer_writer_context* ctx=(buffer_writer_context*)context;

    while(count>0)
    {
        if(ctx->used<ctx->size)
        {
            ctx->buffer[ctx->used++]=c;
            count--;
        }
        else
        {
            return false;
        }
    }
    return true;
}

/**
 * 获取缓存区输出器上下文已写入字符数。
 * 
 * @param context 缓存区输出器上下文。
 * 
 * @return 返回已写入字符数。
 */
static uintn format_buffer_get_written_chars(void* context)
{
    return ((buffer_writer_context*)context)->used;
}

/**
 * 字符串格式化输出到字符串缓存，参数通过可变参数列表提供。这里输出字符串按UTF-8编码，保证缓存有结尾字符。
 * 
 * @param buffer 字符串缓存。
 * @param size   字符串缓存长度。包括存储结尾字符的长度。
 * @param format 格式化字符串。
 * @param args   可变参数列表。
 * 
 * @return 已写入长度，包括结尾0。
 */
uintn format_string_valist(char8* buffer,uintn size,const char8* format,va_list args)
{
    if(buffer==null||size==0||format==null)
    {
        return 0;
    }
    
    output_writer writer={format_buffer_put_char,format_buffer_put_string,format_buffer_put_repeat,
        format_buffer_get_written_chars};
    buffer_writer_context context={buffer,size-1,0};
    
    format_engine(&writer, &context,format,(va_list*)&args);
    buffer[context.used]='\0';

    return context.used+1;
}

/**
 * 字符串格式化输出到字符串缓存。这里输出字符串按UTF-8编码，保证缓存有结尾字符。
 * 
 * @param buffer 字符串缓存。
 * @param size   字符串缓存长度。包括存储结尾字符的长度。
 * @param format 格式化字符串。
 * 
 * @return 已写入长度，包括结尾0。
 */
uintn format_string(char8* buffer,uintn size,const char8* format,...)
{
    va_list args;
    va_start(args,format);
    uintn result=format_string_valist(buffer,size,format,args);
    va_end(args);
    return result;
}