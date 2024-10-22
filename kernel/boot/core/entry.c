/*
 * 测试用入口程序。
 * @date 2024-10-17
 */
#include "boot.h"

void atest1()
{
	print_bytes("Hello\n",sizeof("Hello\n")-1);
}
void atest2()
{
	atest1();
	print_bytes("aos\n",sizeof("aos\n")-1);
}
void atest3()
{
	atest2();
	print_bytes("kernel.\n",sizeof("kernel.\n")-1);
}

/*
 * 内核入口。
 * 目前测试用。
 */
aos_status aos_bootstrap_entry(void* arg1,void* arg2)
{
	atest3();
	print_bytes("Hello aos kernel.\n",sizeof("Hello aos kernel.\n")-1);
	return (uint64)arg1;
}