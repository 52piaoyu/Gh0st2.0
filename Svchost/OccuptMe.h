#include <windows.h>

extern HMODULE hSelf;

#define SE_DEBUG_PRIVILEGE (20L)

typedef enum {
	AdjustCurrentProcess,
	AdjustCurrentThread
} ADJUST_PRIVILEGE_TYPE;

LONG(__stdcall *RtlAdjustPrivilege)(DWORD, BOOLEAN, ADJUST_PRIVILEGE_TYPE, PBOOLEAN);
HINSTANCE hNtDll = 0;

BOOL WINAPI OccupyFile(LPCTSTR lpFileName)
{
	BOOL    bRet;

	//提升权限
	hNtDll = GetModuleHandle(_T("ntdll.dll"));
	*(FARPROC *)&RtlAdjustPrivilege = GetProcAddress(hNtDll, "RtlAdjustPrivilege");
	BOOLEAN bprev = FALSE;
	RtlAdjustPrivilege(SE_DEBUG_PRIVILEGE, TRUE, AdjustCurrentProcess, &bprev);

	HANDLE hProcess = OpenProcess(PROCESS_DUP_HANDLE, FALSE, 4);    // 4为system进程号

	if (hProcess == NULL)
	{
		hProcess = OpenProcess(PROCESS_DUP_HANDLE, FALSE, 8);        // 2K下是 8??
	}

	if (hProcess == NULL)
		return FALSE;

	HANDLE hFile;
	HANDLE hTargetHandle;

	hFile = CreateFile(lpFileName, GENERIC_READ, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		CloseHandle(hProcess);
		return FALSE;
	}

	bRet = DuplicateHandle(GetCurrentProcess(), hFile, hProcess, &hTargetHandle, 0, FALSE, DUPLICATE_SAME_ACCESS | DUPLICATE_CLOSE_SOURCE);

	CloseHandle(hProcess);

	return bRet;
}

BOOL WINAPI OccuptMe()
{
	TCHAR MyPath[MAX_PATH * 2];

	bool result = false;

	memset(MyPath, 0, sizeof(MyPath));

	GetModuleFileName(hSelf, MyPath, sizeof(MyPath));

	if (OccupyFile(MyPath))
	{
		result = true;
	}
	return result;
}
