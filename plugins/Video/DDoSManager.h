// DDoSManager.h: interface for the CDDoSManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DDOSMANAGER_H__EBFDAA9A_1A17_4B03_B01A_05F4F171CD19__INCLUDED_)
#define AFX_DDOSMANAGER_H__EBFDAA9A_1A17_4B03_B01A_05F4F171CD19__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "Manager.h"

class CDDoSManager : public CManager
{
public:
	void OnReceive(LPBYTE lpBuffer, UINT nSize);
	CDDoSManager(CClientSocket *pClient);
	virtual ~CDDoSManager();

};

#endif // !defined(AFX_DDOSMANAGER_H__EBFDAA9A_1A17_4B03_B01A_05F4F171CD19__INCLUDED_)
