#include <klibc.h>

static char8 buffer_a[512];
static char8 buffer_b[512];

extern void print_bytes(const char8* buf,uint64 size);
extern void sleep();

void print_num(uintn number)
{
    uintn index=0;
    uintn num=number;
    do
    {
        buffer_a[index++]=num%10+'0';
        num=num/10;
    } while(num>0);
    for(num=0;num<index;num++)
    {
        buffer_b[num]=buffer_a[index-1-num];
    }
    print_bytes(buffer_b,index);
}

void line_num(const char8* str,uintn number)
{
    uintn size=0;
    while(str[size]!=0)
    {
        size++;
    }
    print_bytes(str,size);
    print_num(number);
    print_bytes((char8*)"\n",1);
}

void line(const char8* str)
{
    uintn size=0;
    while(str[size]!=0)
    {
        size++;
    }
    print_bytes(str,size);
    print_bytes((char8*)"\n",1);
}

noreturn void aos_kernel_entry(void)
{
    line("[aos.kernel.entry] Welcome to aos.kernel.");
    while(true)
    {
        sleep();
    }
}