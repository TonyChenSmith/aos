#include "efi.h"
#include "base.h"

extern uintn base_ms_call_2(void* func,uintn arg1,uintn arg2);
extern uintn base_ms_call_4(void* func,uintn arg1,uintn arg2,uintn arg3,uintn arg4);

static boot_base_functions table;

void* test(void)
{
	table.boot_ms_call_2=base_ms_call_2;
	table.boot_ms_call_4=base_ms_call_4;
	table.boot_ms_call_2((void*)7,1,5);
	return &table;
}