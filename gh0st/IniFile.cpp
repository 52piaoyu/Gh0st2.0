// IniFile.cpp: implementation of the CIniFile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "gh0st.h"
#include "IniFile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif
#define MAX_LENGTH 256
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CIniFile::CIniFile()
{
	TCHAR szAppName[MAX_PATH];

	GetModuleFileName(GetModuleHandle(NULL), szAppName, MAX_PATH);

#ifdef _UNICODE
	int len = lstrlenW(szAppName);
#else
	int len = lstrlenA(szAppName);
#endif

	for (int i = len; i > 0; i--)
	{
		if (szAppName[i] == '.')
		{
			szAppName[i + 1] = '\0';
			break;
		}
	}
	lstrcat(szAppName, _T("ini"));
	IniFileName = szAppName;
}

CIniFile::~CIniFile()
{
}

CString CIniFile::GetString(CString AppName, CString KeyName, CString Default)
{
	TCHAR buf[MAX_LENGTH];
	::GetPrivateProfileString(AppName, KeyName, Default, buf, sizeof(buf), IniFileName);
	return buf;
}

int CIniFile::GetInt(CString AppName, CString KeyName, int Default)
{
	return ::GetPrivateProfileInt(AppName, KeyName, Default, IniFileName);
}

unsigned long CIniFile::GetDWORD(CString AppName, CString KeyName, unsigned long Default)
{
	TCHAR buf[MAX_LENGTH];
	CString temp;
	temp.Format(_T("%u"), Default);
	::GetPrivateProfileString(AppName, KeyName, temp, buf, sizeof(buf), IniFileName);
#ifdef _UNICODE
	return _wtol(buf);
#else
	return atol(buf);
#endif
}

BOOL CIniFile::SetString(CString AppName, CString KeyName, CString Data)
{
	return ::WritePrivateProfileString(AppName, KeyName, Data, IniFileName);
}

BOOL CIniFile::SetInt(CString AppName, CString KeyName, int Data)
{
	CString temp;
	temp.Format(_T("%d"), Data);
	return ::WritePrivateProfileString(AppName, KeyName, temp, IniFileName);
}

BOOL CIniFile::SetDouble(CString AppName, CString KeyName, double Data)
{
	CString temp;
	temp.Format(_T("%f"), Data);
	return ::WritePrivateProfileString(AppName, KeyName, temp, IniFileName);
}

BOOL CIniFile::SetDWORD(CString AppName, CString KeyName, unsigned long Data)
{
	CString temp;
	temp.Format(_T("%u"), Data);
	return ::WritePrivateProfileString(AppName, KeyName, temp, IniFileName);
}