#include "test.h"

int main(void)
{
    int a=__builtin_ctzg((unsigned long)&main,3*TEST);
    int b=__builtin_clzg((unsigned long)&double_value,5*TEST);
    return double_value(TEST)+a+b;
}