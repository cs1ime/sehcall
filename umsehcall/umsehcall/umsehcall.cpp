// umsehcall.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "sehcall.h"

//https://github.com/cs1ime

int main()
{
	//tested on win11 22h2 and work

	sehpoc::init();
	void *addr = GetProcAddress(GetModuleHandleA("ntdll.dll"), "memcpy");

	DWORD value = 0;
	bool v = sehpoc::sehcall(addr, (ULONG64)&value, (ULONG64)0, 4);
	puts("success");

	return 0;
}

