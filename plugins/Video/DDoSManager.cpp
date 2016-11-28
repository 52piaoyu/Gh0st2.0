// DDoSManager.cpp: implementation of the CDDoSManager class.
//
//////////////////////////////////////////////////////////////////////

#include "DDoSManager.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDDoSManager::CDDoSManager(CClientSocket *pClient) : CManager(pClient)
{
}

CDDoSManager::~CDDoSManager()
{

}

void CDDoSManager::OnReceive(LPBYTE lpBuffer, UINT nSize)
{
	switch (lpBuffer[0])
	{
	default:
		break;
	}
}
