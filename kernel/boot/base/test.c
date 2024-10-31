#include "efi.h"
#include "base.h"
#include <stdint.h>


#include "baseimpl.h"

static boot_base_functions table;

static void* base_memcpy2(void* restrict s1,const void* restrict s2,uintn n)
{
	for(uintn index=0;index<n;index++)
	{
		((uint8*)s1)[index]=((uint8*)s2)[index];
	}
	return s1;
}

void* test(void)
{
	table.boot_ms_call_2=base_ms_call_2;
	table.boot_ms_call_4=base_ms_call_4;
	table.boot_memcpy=base_memcpy;
	return &base_memcpy2;
}