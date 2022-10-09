#include <ntifs.h>
#include "sehcall.h"
#include "pat.h"

extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT drv, PUNICODE_STRING reg_path) {
	sehpoc::init();

	PVOID pmemcpy = KGetProcAddress(KGetNtoskrnl(), "memcpy");

	ULONG val = 0;
	sehpoc::sehcall(pmemcpy, (ULONG64)&val, 0, 4);

	DbgPrint("success\n");

	return STATUS_SUCCESS;
}
