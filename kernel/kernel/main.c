#include "main.h"

int cct=564;

static int atest(int a,int b)
{
	return A+(A<<A)+a*b-cct;
}

EFIAPI int aos_kernel_entry(void* a, void* b)
{
	cct=(int)((long)&atest)+sizeof(int);
	return atest(7,13)+cct*2;
}