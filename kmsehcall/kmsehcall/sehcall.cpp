#include <ntifs.h>
#include "pat.h"
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
		PVOID nt = KGetNtoskrnl();
		if (nt == 0)
			return false;
		ULONG_PTR pos = ScanSection(".text", "45 33 C0 48 8B 12 48 8B C2 48 C1 EA 20 0F 30");
		if (pos) {
			g_pocaddr = pos + 15;
			DbgPrint("g_pocaddr:%p\n", g_pocaddr);
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
