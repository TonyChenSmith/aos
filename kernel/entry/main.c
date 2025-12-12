#include <klibc.h>

static int64 src[5];

static int64 dest[10];

static uintn reta()
{
    return (uintn)(src[0]+dest[0]);
}

noreturn void aos_kernel_entry(void)
{
    kmemcpy(&dest,&src,sizeof(int64)*5);
    kmemcpy(&dest,&src,reta());
    while(true)
    {
        ;
    }
}