// LogView.cpp : implementation file
//

#include "stdafx.h"
#include "gh0st.h"
#include "LogView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define IDM_EVENT_DELETE                32845
#define IDM_ALL_DELETE                  32846
#define IDM_EVENT_SAVE                  32847
#define IDM_EVENT_COPY                  32848

CLogView* g_pLogView;

typedef struct
{
	char	*title;
	int		nWidth;
}COLUMNSTRUCT;

COLUMNSTRUCT g_Log_Data[] = 
{
	{"时间",			150	},
	{"事件",			350	}
};

int g_Log_Width = 0;
int	g_Log_Count = (sizeof(g_Log_Data) / 8);
/////////////////////////////////////////////////////////////////////////////
// CLogView

IMPLEMENT_DYNCREATE(CLogView, CListView)

CLogView::CLogView()
{
	g_pLogView = this;
}

CLogView::~CLogView()
{
}


BEGIN_MESSAGE_MAP(CLogView, CListView)
	//{{AFX_MSG_MAP(CLogView)
	ON_WM_SIZE()
	ON_NOTIFY_REFLECT(NM_RCLICK, OnRclick)
	ON_COMMAND(IDM_EVENT_DELETE, OnEventDelete)
	ON_COMMAND(IDM_ALL_DELETE, OnAllDelete)
	ON_COMMAND(IDM_EVENT_SAVE, OnEventSave)
	ON_COMMAND(IDM_EVENT_COPY, OnEventCopy)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLogView drawing

void CLogView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CLogView diagnostics

#ifdef _DEBUG
void CLogView::AssertValid() const
{
	CListView::AssertValid();
}

void CLogView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CLogView message handlers

void CLogView::OnInitialUpdate() 
{
	CListView::OnInitialUpdate();
	
	// TODO: Add your specialized code here and/or call the base class
	m_pLogList = &GetListCtrl();
	
	I_LogList.Create(16, 16, ILC_COLOR32,10, 0);
	HICON hIcon = NULL;//以下为加入3个图标资源
	hIcon = (HICON)::LoadImage(::AfxGetInstanceHandle(),MAKEINTRESOURCE(IDI_EVENT_INFO), IMAGE_ICON, 16, 16, 0);
	I_LogList.Add(hIcon);
	hIcon = (HICON)::LoadImage(::AfxGetInstanceHandle(),MAKEINTRESOURCE(IDI_EVENT_ERROR), IMAGE_ICON, 16, 16, 0);
	I_LogList.Add(hIcon);
	ListView_SetImageList(m_pLogList->m_hWnd, I_LogList, LVSIL_SMALL);

	m_pLogList->SetExtendedStyle(/*LVIF_PARAM |*/ LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP|/*LVS_EX_FLATSB|*/
		LVS_EX_ONECLICKACTIVATE | LVS_EX_UNDERLINEHOT |LVS_EX_SUBITEMIMAGES /*|LVS_EX_GRIDLINES*/);
	
	for (int i = 0; i < g_Log_Count; i++)
	{
		WCHAR tmp[256];
		MultiByteToWideChar(CP_ACP,0,g_Log_Data[i].title,-1,tmp,256);
		m_pLogList->InsertColumn(i,tmp);
		m_pLogList->SetColumnWidth(i, g_Log_Data[i].nWidth);
		g_Log_Width += g_Log_Data[i].nWidth; // 总宽度
	}
}

BOOL CLogView::PreCreateWindow(CREATESTRUCT& cs) 
{
	// TODO: Add your specialized code here and/or call the base class
	cs.style |=  LVS_REPORT;
	return CListView::PreCreateWindow(cs);
}

void CLogView::OnSize(UINT nType, int cx, int cy) 
{
	CListView::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	if (m_pLogList == NULL)
		return;
	
	CRect rcClient;
	GetClientRect(rcClient);
	
	int	nClientWidth = rcClient.Width();
	int nIndex = g_Log_Count - 1;
	
	if (nClientWidth < g_Log_Width)
		return;
	
	m_pLogList->SetColumnWidth(nIndex, nClientWidth - g_Log_Width + g_Log_Data[nIndex].nWidth);
}

void CLogView::InsertLogItem(LPCTSTR Text,int Mode, int Flag)
{
	CTime time = CTime::GetCurrentTime();		//构造CTime对象 

	CString strTime = time.Format("[%Y-%m-%d %H:%M:%S]");
	m_pLogList->InsertItem(0, strTime, Flag);//int InsertItem( int nItem, LPCTSTR lpszItem, int nImage );
	m_pLogList->SetItemText(0, 1, Text);  //BOOL SetItemText( int nItem, int nSubItem, LPTSTR lpszText );
	switch(Mode)
	{
	case 0:
		{
			m_pLogList->SetTextColor(RGB(0,100,250));//蓝色
		}
		break;	
		case 1:
		{
			m_pLogList->SetTextColor(RGB(255,0,0));//红色
		}
		break;
		case 2:
		{
			m_pLogList->SetTextColor(RGB(0,180,250));//蓝绿色
		}
		break;
		case 3:
		{
			m_pLogList->SetTextColor(RGB(200,0,200));//粉色
		}
		break;
		case 4:
		{
			m_pLogList->SetTextColor(RGB(0,20,100));//貌似黑色
		}
		break;
		
	default:
		m_pLogList->SetTextColor(RGB(0,100,255));//蓝色
		break;
	}
}

void CLogView::OnRclick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	*pResult = 0;
	CMenu	popup;
	popup.LoadMenu(IDR_LOG);
	CMenu*	pM = popup.GetSubMenu(0);
	CPoint	p;
	GetCursorPos(&p);
	pM->TrackPopupMenu(TPM_LEFTALIGN, p.x, p.y, this);

}

void CLogView::OnEventDelete() 
{
	// TODO: Add your command handler code here
	POSITION pos = m_pLogList->GetFirstSelectedItemPosition();
	if  (pos  ==  NULL)
	{
		::MessageBoxA(NULL,"请先选择要删除的事件记录 ...","提示",MB_ICONINFORMATION);
		return;
	}
	else
	{
		while (pos)
		{
			int nItem = m_pLogList->GetNextSelectedItem(pos);
			m_pLogList->DeleteItem(nItem);
			pos = m_pLogList->GetFirstSelectedItemPosition();
		}
	}
}

void CLogView::OnAllDelete() 
{
	// TODO: Add your command handler code here
	m_pLogList->DeleteAllItems();
}

void CLogView::OnEventSave() 
{
	// TODO: Add your command handler code here
	POSITION pos = m_pLogList->GetFirstSelectedItemPosition();
	if  (pos  ==  NULL)
	{
		::MessageBoxA(NULL,"请先选择要保存的事件记录 ...","提示",MB_ICONINFORMATION);
		return;
	}
	else
	{
		CTime time = CTime::GetCurrentTime(); ///构造CTime对象 
		CString strTime = time.Format("%Y-%m-%d %H-%M-%S");
		
		CFileDialog dlg(FALSE, _T("log"), strTime, OFN_OVERWRITEPROMPT, _T("*.log|*.log|*.txt|*.log"), NULL);
		if (dlg.DoModal() != IDOK)
			return;
		CFile file;

		if (file.Open(dlg.GetPathName(), CFile::modeCreate | CFile::modeWrite))
		{
			while (pos)
			{
				int nItem = m_pLogList->GetNextSelectedItem(pos);
				CString strTitle;
				strTitle.Format(_T("时间:%s 事件:%s"), m_pLogList->GetItemText(nItem, 0),m_pLogList->GetItemText(nItem, 1));
				WCHAR *strip = strTitle.GetBuffer(strTitle.GetLength() + 1);	//CString转换为char*
				strTitle.ReleaseBuffer();
				lstrcat(strip, _T("\r\n"));
				file.Write(strip, lstrlen(strip));
				memset(strip, 0, 50);
			}
			file.Close();
			strTime = time.Format("[%Y-%m-%d %H:%M:%S]") + " 日志导出成功 ...";
			::MessageBox(0,strTime,_T("提示"),MB_ICONINFORMATION);
	  }
	}
}

void CLogView::OnEventCopy() 
{
	// TODO: Add your command handler code here
	int nItem;
	CString strText(_T(""));
	POSITION pos = m_pLogList->GetFirstSelectedItemPosition();
	if  (pos  ==  NULL)
	{
		::MessageBoxA(NULL,"请先选择要复制的事件记录 ...","提示",MB_ICONINFORMATION);
		return;
	}
	else
	{
		//获取所有选中项目的内容。
		while (pos)
		{
			nItem = m_pLogList->GetNextSelectedItem(pos);
			strText += m_pLogList->GetItemText(nItem, 0) + " ";
			strText += m_pLogList->GetItemText(nItem, 1) + _T("\r\n");
		}
		//将内容保存到剪贴板。
		if (!strText.IsEmpty())
		{
			if (OpenClipboard())
			{
				EmptyClipboard();
				HGLOBAL hClipboardData = GlobalAlloc(GHND | GMEM_SHARE, (strText.GetLength() + 1) * sizeof(TCHAR));
				if (hClipboardData)
				{
					TCHAR* pszData = (TCHAR *)GlobalLock(hClipboardData);
					_tcscpy(pszData, strText);
					GlobalUnlock(hClipboardData);
					SetClipboardData(CF_TEXT, hClipboardData);
				}
				CloseClipboard();
			}
		}
	}
}

