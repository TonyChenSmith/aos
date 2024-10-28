#include "efi.h"
#include "base.h"

#include "baseimpl.h"

static boot_base_functions table;

void* test(void)
{
	table.boot_ms_call_2=base_ms_call_2;
	table.boot_ms_call_4=base_ms_call_4;
	table.boot_memcpy=base_memcpy;
	return &table;
}