#pragma once
#pragma warning( disable : 4311 4312 )

#include <Windows.h>
#include <winnt.h>
#ifdef DEBUG
#include <stdio.h>
#endif

typedef void *HMEMORYMODULE;

#if _MSC_VER > 1300
DWORD IMAGE_SIZEOF_BASE_RELOCATION = 2 * sizeof(DWORD);
#endif

#ifdef __cplusplus
extern "C" {
#endif

	HMEMORYMODULE MemoryLoadLibrary(const void *);

	FARPROC MemoryGetProcAddress(HMEMORYMODULE, const char *);

	void MemoryFreeLibrary(HMEMORYMODULE);

#ifdef __cplusplus
}
#endif

typedef struct {
	PIMAGE_NT_HEADERS headers;
	unsigned char *codeBase;
	HMODULE *modules;
	int numModules;
	int initialized;
} MEMORYMODULE, *PMEMORYMODULE;

typedef BOOL(WINAPI *DllEntryProc)(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved);

#define GET_HEADER_DICTIONARY(module, idx)	&(module)->headers->OptionalHeader.DataDirectory[idx]

#ifdef DEBUG
static void
OutputLastError(LPCTSTR msg)
{
	LPVOID tmp;
	TCHAR *tmpmsg;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&tmp, 0, NULL);
	tmpmsg = (TCHAR *)LocalAlloc(LPTR, lstrlen((LPCTSTR)msg) + lstrlen((LPCTSTR)tmp) + 3);
	wsprintf(tmpmsg, _T("%s: %s"), msg, tmp);
	OutputDebugString(tmpmsg);
	LocalFree(tmpmsg);
	LocalFree(tmp);
}
#endif

static void CopySections(const unsigned char *data, PIMAGE_NT_HEADERS old_headers, PMEMORYMODULE module)
{
	unsigned char *codeBase = module->codeBase;
	unsigned char *dest;
	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(module->headers);
	for (int i = 0; i < module->headers->FileHeader.NumberOfSections; i++, section++)
	{
		if (section->SizeOfRawData == 0)
		{
			// section doesn't contain data in the dll itself, but may define
			// uninitialized data
			int size = old_headers->OptionalHeader.SectionAlignment;
			if (size > 0)
			{
				dest = (unsigned char *)VirtualAlloc(codeBase + section->VirtualAddress,
					size,
					MEM_COMMIT,
					PAGE_EXECUTE_READWRITE);

				section->Misc.PhysicalAddress = (DWORD)dest;
				memset(dest, 0, size);
			}

			// section is empty
			continue;
		}

		// commit memory block and copy data from dll
		dest = (unsigned char *)VirtualAlloc(codeBase + section->VirtualAddress,
			section->SizeOfRawData,
			MEM_COMMIT,
			PAGE_EXECUTE_READWRITE);

		memcpy(dest, data + section->PointerToRawData, section->SizeOfRawData);
		section->Misc.PhysicalAddress = (DWORD)dest;
	}
}

// Protection flags for memory pages (Executable, Readable, Writeable)
static int ProtectionFlags[2][2][2] =
{
	{
		// not executable
		{ PAGE_NOACCESS, PAGE_WRITECOPY },
		{ PAGE_READONLY, PAGE_READWRITE },
	},
	{
		// executable
		{ PAGE_EXECUTE, PAGE_EXECUTE_WRITECOPY },
		{ PAGE_EXECUTE_READ, PAGE_EXECUTE_READWRITE },
	},
};

static void
FinalizeSections(PMEMORYMODULE module)
{
	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(module->headers);

	// loop through all sections and change access flags
	for (int i = 0; i < module->headers->FileHeader.NumberOfSections; i++, section++)
	{
		DWORD oldProtect;
		int executable = (section->Characteristics & IMAGE_SCN_MEM_EXECUTE) != 0;
		int readable = (section->Characteristics & IMAGE_SCN_MEM_READ) != 0;
		int writeable = (section->Characteristics & IMAGE_SCN_MEM_WRITE) != 0;

		if (section->Characteristics & IMAGE_SCN_MEM_DISCARDABLE)
		{
			// section is not needed any more and can safely be freed
			VirtualFree((LPVOID)section->Misc.PhysicalAddress, section->SizeOfRawData, MEM_DECOMMIT);
			continue;
		}

		// determine protection flags based on characteristics
		DWORD protect = ProtectionFlags[executable][readable][writeable];
		if (section->Characteristics & IMAGE_SCN_MEM_NOT_CACHED)
			protect |= PAGE_NOCACHE;

		// determine size of region
		DWORD size = section->SizeOfRawData;
		if (size == 0)
		{
			if (section->Characteristics & IMAGE_SCN_CNT_INITIALIZED_DATA)
				size = module->headers->OptionalHeader.SizeOfInitializedData;
			else if (section->Characteristics & IMAGE_SCN_CNT_UNINITIALIZED_DATA)
				size = module->headers->OptionalHeader.SizeOfUninitializedData;
		}

		if (size > 0)
		{
			// change memory access flags
			if (VirtualProtect((LPVOID)section->Misc.PhysicalAddress, section->SizeOfRawData, protect, &oldProtect) == 0)
			{
				//OutputLastError("Error protecting memory page");
			}
		}
	}
}

static void PerformBaseRelocation(PMEMORYMODULE module, DWORD delta)
{
	unsigned char *codeBase = module->codeBase;

	PIMAGE_DATA_DIRECTORY directory = GET_HEADER_DICTIONARY(module, IMAGE_DIRECTORY_ENTRY_BASERELOC);
	if (directory->Size > 0)
	{
		PIMAGE_BASE_RELOCATION relocation = (PIMAGE_BASE_RELOCATION)(codeBase + directory->VirtualAddress);
		for (; relocation->VirtualAddress > 0;)
		{
			unsigned char *dest = (unsigned char *)(codeBase + relocation->VirtualAddress);
			unsigned short *relInfo = (unsigned short *)((unsigned char *)relocation + IMAGE_SIZEOF_BASE_RELOCATION);
			for (DWORD i = 0; i < (relocation->SizeOfBlock - IMAGE_SIZEOF_BASE_RELOCATION) / 2; i++, relInfo++)
			{
				// the upper 4 bits define the type of relocation
				int type = *relInfo >> 12;
				// the lower 12 bits define the offset
				int offset = *relInfo & 0xfff;

				switch (type)
				{
				case IMAGE_REL_BASED_ABSOLUTE:
					// skip relocation
					break;

				case IMAGE_REL_BASED_HIGHLOW:
					// change complete 32 bit address
					DWORD *patchAddrHL = (DWORD *)(dest + offset);
					*patchAddrHL += delta;
					break;

				//default:
					//printf("Unknown relocation: %d\n", type);
					//break;
				}
			}

			// advance to next relocation block
			relocation = (PIMAGE_BASE_RELOCATION)(((DWORD)relocation) + relocation->SizeOfBlock);
		}
	}
}

static int BuildImportTable(PMEMORYMODULE module)
{
	int result = 1;
	unsigned char *codeBase = module->codeBase;

	PIMAGE_DATA_DIRECTORY directory = GET_HEADER_DICTIONARY(module, IMAGE_DIRECTORY_ENTRY_IMPORT);
	if (directory->Size > 0)
	{
		PIMAGE_IMPORT_DESCRIPTOR importDesc = (PIMAGE_IMPORT_DESCRIPTOR)(codeBase + directory->VirtualAddress);
		for (; !IsBadReadPtr(importDesc, sizeof(IMAGE_IMPORT_DESCRIPTOR)) && importDesc->Name; importDesc++)
		{
			DWORD *thunkRef, *funcRef;
			HMODULE handle = LoadLibraryA((LPCSTR)(codeBase + importDesc->Name));
			if (handle == INVALID_HANDLE_VALUE)
			{
#if DEBUG
				OutputLastError(_T("Can't load library"));
#endif
				result = 0;
				break;
			}

			if (module->modules)
			{
				module->modules = (HMODULE *)HeapReAlloc(GetProcessHeap(), 0,
					module->modules, (module->numModules + 1)*(sizeof(HMODULE)));
			}
			else
			{
				module->modules = (HMODULE *)HeapAlloc(GetProcessHeap(), 0,
					(module->numModules + 1)*(sizeof(HMODULE)));
			}

			if (module->modules == NULL)
			{
				result = 0;
				break;
			}

			module->modules[module->numModules++] = handle;
			if (importDesc->OriginalFirstThunk)
			{
				thunkRef = (DWORD *)(codeBase + importDesc->OriginalFirstThunk);
				funcRef = (DWORD *)(codeBase + importDesc->FirstThunk);
			}
			else
			{
				// no hint table
				thunkRef = (DWORD *)(codeBase + importDesc->FirstThunk);
				funcRef = (DWORD *)(codeBase + importDesc->FirstThunk);
			}
			for (; *thunkRef; thunkRef++, funcRef++)
			{
				if IMAGE_SNAP_BY_ORDINAL(*thunkRef)
					*funcRef = (DWORD)GetProcAddress(handle, (LPCSTR)IMAGE_ORDINAL(*thunkRef));
				else {
					PIMAGE_IMPORT_BY_NAME thunkData = (PIMAGE_IMPORT_BY_NAME)(codeBase + *thunkRef);
					*funcRef = (DWORD)GetProcAddress(handle, (LPCSTR)&thunkData->Name);
				}
				if (*funcRef == 0)
				{
					result = 0;
					break;
				}
			}

			if (!result)
				break;
		}
	}

	return result;
}

HMEMORYMODULE MemoryLoadLibrary(const void *data)
{
	PIMAGE_DOS_HEADER dos_header = (PIMAGE_DOS_HEADER)data;
	if (dos_header->e_magic != IMAGE_DOS_SIGNATURE)
	{
#if DEBUG
		OutputDebugString(_T("Not a valid executable file.\n"));
#endif
		return NULL;
	}

	PIMAGE_NT_HEADERS old_header = (PIMAGE_NT_HEADERS)&((const unsigned char *)(data))[dos_header->e_lfanew];
	if (old_header->Signature != IMAGE_NT_SIGNATURE)
	{
#if DEBUG
		OutputDebugString(_T("No PE header found.\n"));
#endif
		return NULL;
	}

	// reserve memory for image of library
	unsigned char *code = (unsigned char *)VirtualAlloc((LPVOID)(old_header->OptionalHeader.ImageBase),
	                                                    old_header->OptionalHeader.SizeOfImage,
	                                                    MEM_RESERVE,
	                                                    PAGE_EXECUTE_READWRITE);

	if (code == NULL)
	{	// try to allocate memory at arbitrary position
		code = (unsigned char *)VirtualAlloc(NULL, old_header->OptionalHeader.SizeOfImage,
			MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	}

	if (code == NULL)
	{
#if DEBUG
		OutputLastError(_T("Can't reserve memory"));
#endif
		return NULL;
	}

	PMEMORYMODULE result = (PMEMORYMODULE)HeapAlloc(GetProcessHeap(), 0, sizeof(MEMORYMODULE));
	result->codeBase = code;
	result->numModules = 0;
	result->modules = NULL;
	result->initialized = 0;

	// XXX: is it correct to commit the complete memory region at once?
	//      calling DllEntry raises an exception if we don't...
	//unsigned char *image = (unsigned char *)VirtualAlloc(code, old_header->OptionalHeader.SizeOfImage, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	// commit memory for headers
	unsigned char *headers = (unsigned char *)VirtualAlloc(code,
	                                                       old_header->OptionalHeader.SizeOfHeaders,
	                                                       MEM_COMMIT,
	                                                       PAGE_READWRITE);

	// copy PE header to code
	memcpy(headers, dos_header, dos_header->e_lfanew + old_header->OptionalHeader.SizeOfHeaders);
	result->headers = (PIMAGE_NT_HEADERS)&((const unsigned char *)(headers))[dos_header->e_lfanew];

	// update position
	result->headers->OptionalHeader.ImageBase = (DWORD)code;

	// copy sections from DLL file block to new memory location
	CopySections((const unsigned char *)data, old_header, result);

	// adjust base address of imported data
	DWORD locationDelta = (DWORD)(code - old_header->OptionalHeader.ImageBase);
	if (locationDelta != 0)
		PerformBaseRelocation(result, locationDelta);

	// load required dlls and adjust function table of imports
	if (!BuildImportTable(result))
	{
		MemoryFreeLibrary(result);
		return NULL;
	}

	// mark memory pages depending on section headers and release
	// sections that are marked as "discardable"
	FinalizeSections(result);

	// get entry point of loaded library
	if (result->headers->OptionalHeader.AddressOfEntryPoint != 0)
	{
		DllEntryProc DllEntry = (DllEntryProc)(code + result->headers->OptionalHeader.AddressOfEntryPoint);
		if (DllEntry == 0)
		{
#if DEBUG
			OutputDebugString(_T("Library has no entry point.\n"));
#endif
			MemoryFreeLibrary(result);
			return NULL;
		}

		// notify library about attaching to process
		BOOL successfull = (*DllEntry)((HINSTANCE)code, DLL_PROCESS_ATTACH, 0);
		if (!successfull)
		{
#if DEBUG
			OutputDebugString(_T("Can't attach library.\n"));
#endif
			MemoryFreeLibrary(result);
			return NULL;
		}
		result->initialized = 1;
	}

	return (HMEMORYMODULE)result;
}

///-------------------------------------------MyFunctions
__declspec(naked) char __fastcall upcaseA(char c)
{
	__asm
	{
		mov al, cl
		cmp al, 'a'
		jl m1
		cmp al, 'z'
		jg m1
		and al, 0xdf
		m1:
		retn
	}
}

int __fastcall strcmpiA(const char *s1, const char *s2)
{
	unsigned long k = 0;
	unsigned char c1, c2;

	do
	{
		c1 = upcaseA(s1[k]);
		c2 = upcaseA(s2[k]);
		if (c1 > c2)
		{
			return 1;
		}
		else if (c1 < c2)
		{
			return -1;
		};

		k++;
	} while ((c1 | c2) != 0);
	return 0;
}
////------------------------------------

FARPROC MemoryGetProcAddress(HMEMORYMODULE module, const char *name)
{
	unsigned char *codeBase = ((PMEMORYMODULE)module)->codeBase;
	int idx = -1;
	PIMAGE_DATA_DIRECTORY directory = GET_HEADER_DICTIONARY((PMEMORYMODULE)module, IMAGE_DIRECTORY_ENTRY_EXPORT);
	if (directory->Size == 0)
		// no export table found
		return NULL;

	PIMAGE_EXPORT_DIRECTORY exports = (PIMAGE_EXPORT_DIRECTORY)(codeBase + directory->VirtualAddress);
	if (exports->NumberOfNames == 0 || exports->NumberOfFunctions == 0)
		// DLL doesn't export anything
		return NULL;

	// search function name in list of exported names
	DWORD *nameRef = (DWORD *)(codeBase + exports->AddressOfNames);
	WORD *ordinal = (WORD *)(codeBase + exports->AddressOfNameOrdinals);
	for (DWORD i = 0; i < exports->NumberOfNames; i++, nameRef++, ordinal++)
		if (strcmpiA(name, (const char *)(codeBase + *nameRef)) == 0)
		{
			idx = *ordinal;
			break;
		}

	if (idx == -1)
		// exported symbol not found
		return NULL;

	if ((DWORD)idx > exports->NumberOfFunctions)
		// name <-> ordinal number don't match
		return NULL;

	// AddressOfFunctions contains the RVAs to the "real" functions
	return (FARPROC)(codeBase + *(DWORD *)(codeBase + exports->AddressOfFunctions + (idx * 4)));
}

void MemoryFreeLibrary(HMEMORYMODULE mod)
{
	PMEMORYMODULE module = (PMEMORYMODULE)mod;

	if (module != NULL)
	{
		if (module->initialized != 0)
		{
			// notify library about detaching from process
			DllEntryProc DllEntry = (DllEntryProc)(module->codeBase + module->headers->OptionalHeader.AddressOfEntryPoint);
			(*DllEntry)((HINSTANCE)module->codeBase, DLL_PROCESS_DETACH, 0);
			module->initialized = 0;
		}

		if (module->modules != NULL)
		{
			// free previously opened libraries
			for (int i = 0; i < module->numModules; i++)
			{
				if (module->modules[i] != INVALID_HANDLE_VALUE)
				{
					FreeLibrary(module->modules[i]);
				}
			}

			HeapFree(GetProcessHeap(), 0, module->modules);
		}

		if (module->codeBase != NULL)
		{
			VirtualFree(module->codeBase, 0, MEM_RELEASE);
		}

		HeapFree(GetProcessHeap(), 0, module);
	}
}
