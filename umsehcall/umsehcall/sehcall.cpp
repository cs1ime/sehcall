#include <windows.h>
//https://github.com/cs1ime

struct PocStruct {
	ULONG64 PocAddr;
	ULONG64 RoutineAddr;
	ULONG64 firstargument;
};
namespace sehpoc {
	extern "C" bool MySehPoc(PocStruct *info, ULONG64 arg1, ULONG64 arg2, ULONG64 arg3);

	ULONG64 g_pocaddr = 0;
	bool init() {
		HMODULE ModBase = GetModuleHandleA("kernel32.dll");
		if (ModBase == 0)
			return false;
		PUCHAR FindAddr = (PUCHAR)GetProcAddress(ModBase, "IsBadReadPtr");
		if (FindAddr == 0)
			return false;

		for (int i = 0; i < 0x200; i++) {
			if (FindAddr[i] == 0xEB && FindAddr[i + 1] == 0x0E) {
				g_pocaddr = (ULONG64)&FindAddr[i];
				return true;
			}
		}
		return false;
	}
	bool sehcall(PVOID routine, ULONG64 arg1, ULONG64 arg2, ULONG64 arg3, ULONG64 arg4) {
		PocStruct info = { 0 };
		info.PocAddr = g_pocaddr;
		info.RoutineAddr = (ULONG64)routine;
		info.firstargument = arg1;
		return MySehPoc(&info, arg2, arg3, arg4);
	}

}
