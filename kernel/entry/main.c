#include <klibc.h>

static int64 src[5];

static int64 dest[10];

const char8 test[]="Hello AOS.";

static char8* str=(char8*)&test;

uintn size;

static uintn reta()
{
    return (uintn)(src[0]+dest[0]);
}

noreturn void aos_kernel_entry(void)
{
    kmemcpy(&dest,&src,sizeof(int64)*5);
    kmemcpy(&dest,&src,reta());
    size=reta()*5;
    kmemmove(&dest,str,size>>2);
    while(true)
    {
        ;
    }
}