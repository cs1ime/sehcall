#include <ntifs.h>
#include "NtFunctionDefine.h"
#include <ntimage.h>

//sunday memory search 
namespace sd {
#define sd_c2v(v)((v>='0'&&v<='9')?(v-'0'):(v-'A'+10))
	__forceinline int sd_toval(const char *vs) {
		int hi = sd_c2v(vs[0]);
		int lo = sd_c2v(vs[1]);

		return hi * 0x10 + lo;
	}
	__forceinline char sd_toupper(char v) {
		if (v >= 'a' && v <= 'z') {
			v -= 0x20;
		}
		return v;
	}
	struct sd_skip {
		int offset;
		unsigned char cmap[256];
	};
	//#define sd_setcmap(m,p)(m[p/8]|=(1<<((p%8))))
	//#define sd_iscmap(m,p)( (m[p/8]>>(p%8)) & 0b00000001)
#define sd_setcmap(m,p)(m[p]=1)
#define sd_iscmap(m,p)(m[p]==1)
	__forceinline int sd_convert(const char *pattern, unsigned short *pat) {
		int len = strlen(pattern);

		char val[3];
		val[2] = 0;

		int val_idx = 0;

		bool flag_fuzzy = false;
		bool flag_char = false;
		int pat_idx = 0;

		bool flag_cvt = false;
		for (int i = 0; i < len + 1; i++) {
			if (i == len)
				flag_cvt = true;
			else {
				char v = sd_toupper(pattern[i]);
				if ((v >= '0'&&v <= '9') || (v >= 'A' && v <= 'Z')) {
					if (flag_fuzzy)
						return 0;
					if (val_idx >= 2)
						return 0;
					val[val_idx++] = v;
					flag_char = true;
				}
				else if (v == '?') {
					if (flag_char)
						return 0;
					flag_fuzzy = true;
				}
				else if (v == ' ')
					flag_cvt = true;
				else
					return 0;
			}


			if (flag_cvt) {
				if (flag_fuzzy && flag_char)
					return 0;
				if (flag_fuzzy) {
					pat[pat_idx++] = 0x100;
					flag_fuzzy = false;
				}
				else if (flag_char) {
					if (val_idx != 2)
						return false;
					pat[pat_idx++] = sd_toval(val);
					flag_char = false;
				}

				val_idx = 0;

				flag_cvt = false;
			}
		}
		return pat_idx;
	}
	__forceinline int sd_praseskip(unsigned short *pat, int pat_len, sd_skip* skip) {
		int prev_v = 0;
		for (int i = 0; i < pat_len; i++) {
			if ((pat[i] == 0x100 && prev_v != 0x100)) {
				skip[0].offset = i;
				return 1;
			}
			else
				sd_setcmap(skip[0].cmap, pat[i]);
			prev_v = pat[i];
		}
		return 0;
	}
	__forceinline int sd_dosearch
	(
		const unsigned char *	buf, size_t	buf_len,
		const unsigned short *	pat, int		pat_len,
		sd_skip*				skip, int		skip_cnt
	)
	{
		size_t i = 0;
		size_t match_size = 0;

		int pat_idx = 0;
		bool flag_notmatch = false;

		while (i < buf_len) {
			if (pat[pat_idx] == 0x100 || buf[i] == pat[pat_idx]) {
				match_size++;
				i++;
				pat_idx++;
			}
			else {
				flag_notmatch = true;
			}
			if (flag_notmatch) {
				int skip_v = 1;

				for (int j = 0; j < skip_cnt; j++) {
					size_t next_idx = i - match_size + skip[j].offset;
					unsigned char next_v = buf[next_idx];
					if (!sd_iscmap(skip[j].cmap, next_v))
						skip_v = skip[j].offset;
					else
						break;
				}
				i = i - match_size + skip_v;

				match_size = 0;
				pat_idx = 0;
				flag_notmatch = false;
			}
			if (match_size == pat_len)
				return i - match_size;
		}

		return -1;
	}
	__forceinline int sd_dosearch_noskip
	(
		const unsigned char *	buf, size_t		buf_len,
		const unsigned short *	pat, int		pat_len
	)
	{
		bool cmap[256];
		memset(cmap, 0, sizeof(cmap));
		for (int i = 0; i < pat_len; i++) {
			cmap[pat[i]] = 1;
		}
		int prev_sz = pat_len;

		size_t i = 0;
		size_t match_size = 0;

		int pat_idx = 0;
		bool flag_notmatch = false;

		while (i < buf_len) {
			if (pat[pat_idx] == 0x100 || buf[i] == pat[pat_idx]) {
				match_size++;
				i++;
				pat_idx++;
			}
			else {
				flag_notmatch = true;
			}
			if (flag_notmatch) {
				int skip_v = 1;
				size_t next_idx = i - match_size + prev_sz;
				if (cmap[buf[next_idx]] == 0) {
					skip_v = prev_sz + 1;
				}
				i = i - match_size + skip_v;

				match_size = 0;
				pat_idx = 0;
				flag_notmatch = false;
			}
			if (match_size == pat_len)
				return i - match_size;
		}

		return -1;
	}
	__forceinline int sd_lstrip(unsigned short *pat, int pat_len) {
		int start = 0;
		for (int i = 0; i < pat_len; i++) {
			if (pat[i] == 0x100)
				start++;
			else
				break;
		}
		return start;
	}
	__forceinline int sd_rstrip(unsigned short *pat, int pat_len) {
		int off = 0;
		for (int i = pat_len - 1; i >= 0; i--) {
			if (pat[i] == 0x100)
				off++;
			else
				break;
		}
		return off;
	}
	int sd_search(const unsigned char *buf, size_t buf_len, const char *pattern) {
		unsigned short cmp_pat[100];
		int cmp_pat_len = sd_convert(pattern, cmp_pat);

		int left = sd_lstrip(cmp_pat, cmp_pat_len);
		int right = sd_rstrip(cmp_pat, cmp_pat_len);

		sd_skip skip[1];
		memset(skip, 0, sizeof(skip));
		int skip_cnt = sd_praseskip(cmp_pat + left, cmp_pat_len - left - right, skip);

		int result = 0;
		if (skip_cnt == 0)
			result = sd_dosearch_noskip(buf, buf_len, cmp_pat + left, cmp_pat_len - left - right);
		else
			result = sd_dosearch(buf, buf_len, cmp_pat + left, cmp_pat_len - left - right, skip, skip_cnt);
		return result != -1 ? (result - left) : -1;
	}

}
LPCSTR GetSuffixFileNameA(LPCSTR a) {
	LPCSTR resu = a;

	int len = strlen(a);
	for (int i = 0; i < len; i++) {
		if (a[i] == '\\' || a[i] == '/') {
			resu = &a[i + 1];
		}
	}

	return resu;
}
PVOID KGetDriverImageBase(LPCSTR name) {
	PVOID addr = 0;

	ULONG size = 0;
	NTSTATUS status = ZwQuerySystemInformation(SystemModuleInformation, 0, 0, &size);
	if (STATUS_INFO_LENGTH_MISMATCH != status) {
		//DbgPrint("! ZwQuerySystemInformation for size failed: %p !\n", status);
		return addr;
	}

	PSYSTEM_MODULE_INFORMATION modules = (PSYSTEM_MODULE_INFORMATION)ExAllocatePool(NonPagedPoolNx, size);
	if (!modules) {
		//DbgPrint("! failed to allocate %d bytes for modules !\n", size);
		return addr;
	}

	if (!NT_SUCCESS(status = ZwQuerySystemInformation(SystemModuleInformation, modules, size, 0))) {
		//DbgPrint("! ZwQuerySystemInformation failed: %p !\n", status);
		return addr;
	}
	int name_len = strlen(name);
	for (ULONG i = 0; i < modules->NumberOfModules; ++i) {
		SYSTEM_MODULE m = modules->Modules[i];
		UCHAR buf[256 + 1];
		RtlZeroMemory(buf, sizeof(buf));
		memcpy(buf, m.FullPathName, 256);
		LPCSTR n = GetSuffixFileNameA((LPCSTR)buf);
		if (!strnicmp(n, name, name_len)) {
			addr = m.ImageBase;
			break;
		}
	}

	ExFreePool(modules);
	return addr;
}
PVOID KGetNtoskrnl() {
	static PVOID CachedNtosBase = 0;
	if (CachedNtosBase) {
		return CachedNtosBase;
	}
	PVOID ret = KGetDriverImageBase("ntoskrnl.exe");
	CachedNtosBase = ret;
	return CachedNtosBase;
}

#define WORD USHORT
PVOID KGetProcAddress(PVOID ModuleHandle, LPCSTR ProcName) {
	IMAGE_DOS_HEADER *idh = (IMAGE_DOS_HEADER *)ModuleHandle;
	IMAGE_NT_HEADERS64 *inh = (IMAGE_NT_HEADERS64 *)(idh->e_lfanew + (PUCHAR)idh);
	IMAGE_EXPORT_DIRECTORY *ied = (IMAGE_EXPORT_DIRECTORY *)((PUCHAR)ModuleHandle + inh->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);

	for (int i = 0; i < ied->NumberOfNames; i++) {
		WORD index = ((WORD *)((PUCHAR)ModuleHandle + ied->AddressOfNameOrdinals))[i];
		ULONG NameRVA = ((ULONG *)((PUCHAR)ModuleHandle + ied->AddressOfNames))[i];
		PCSTR Name = (PCSTR)(((ULONG64)ModuleHandle) + NameRVA);

		if (!strcmp(Name, ProcName)) {
			ULONG FunRVA = ((ULONG *)((PUCHAR)ModuleHandle + ied->AddressOfFunctions))[index];
			PUCHAR FunAddress = ((PUCHAR)ModuleHandle + FunRVA);

			BOOLEAN IsBoundImport = FALSE;
			ULONG BoundImportNameLenth = 0;
			for (ULONG i = 0; i < 50; i++) {
				PUCHAR pAddr = FunAddress + i;
				//if (!CommonMmIsAddressValidEx(pAddr))break;
				UCHAR c = *pAddr;
				if (c == '.' && i > 0) {
					IsBoundImport = TRUE;
					BoundImportNameLenth = i;
					break;
				}
				else {
					if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))) {
						break;
					}
				}
			}
			if (IsBoundImport) {
				UCHAR BoundImportModuleName[40];
				RtlZeroMemory(BoundImportModuleName, sizeof(BoundImportModuleName));
				memcpy(BoundImportModuleName, FunAddress, BoundImportNameLenth);
				//*(ULONG *)(BoundImportModuleName + BoundImportNameLenth) = 'lld.';

				LPCSTR BoundImportFunctionName = (LPCSTR)(FunAddress + BoundImportNameLenth + 1);
				ULONG64 base = (ULONG64)KGetDriverImageBase((LPCSTR)BoundImportModuleName);
				if (base) {
					return KGetProcAddress((PVOID)base, BoundImportFunctionName);
				}


			}

			return FunAddress;
		}

	}

	return NULL;
}

ULONG64 ScanSection(LPCSTR SectionName, LPCSTR Pattern) {
	PIMAGE_NT_HEADERS pHdr;
	PIMAGE_SECTION_HEADER pFirstSec;
	PIMAGE_SECTION_HEADER pSec;
	PUCHAR ntosBase;

	ntosBase = (PUCHAR)KGetNtoskrnl();

	if (!ntosBase)
		return NULL;
	IMAGE_DOS_HEADER* idh = (IMAGE_DOS_HEADER*)ntosBase;
	pHdr = (IMAGE_NT_HEADERS*)(ntosBase + idh->e_lfanew);
	pFirstSec = IMAGE_FIRST_SECTION(pHdr);
	for (pSec = pFirstSec; pSec < pFirstSec + pHdr->FileHeader.NumberOfSections; pSec++)
	{
		CHAR Name[9];
		RtlZeroMemory(&Name, 9);
		memcpy(Name, pSec->Name, 8);
		if (!strcmp(SectionName, Name))
		{
			PUCHAR pFound = NULL;
			INT64 pos = sd::sd_search(ntosBase + pSec->VirtualAddress, pSec->Misc.VirtualSize, Pattern);
			if (pos != -1)
			{
				return (ULONG64)(pos + ntosBase + pSec->VirtualAddress);
			}

		}
	}
	return NULL;
}
