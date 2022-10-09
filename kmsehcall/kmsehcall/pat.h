#pragma once

#ifndef _PAT_H_
#define _PAT_H_

#include <ntifs.h>

PVOID KGetDriverImageBase(LPCSTR name);
PVOID KGetNtoskrnl();
PVOID KGetProcAddress(PVOID ModuleHandle, LPCSTR ProcName);

ULONG64 ScanSection(LPCSTR SectionName, LPCSTR Pattern);

#endif // !_PAT_H_



