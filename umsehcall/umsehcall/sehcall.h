#pragma once

#include <windows.h>
namespace sehpoc {
	bool init();
	bool sehcall(PVOID routine, ULONG64 arg1 = 0, ULONG64 arg2 = 0, ULONG64 arg3 = 0, ULONG64 arg4 = 0);
}