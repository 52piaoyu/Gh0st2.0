#if !defined(AFX_UPIPDLG_H__8AC0DAC5_4498_45CB_91E2_C76437753990__INCLUDED_)
#define AFX_UPIPDLG_H__8AC0DAC5_4498_45CB_91E2_C76437753990__INCLUDED_

#pragma once

#include <afxdialogex.h>

// UpipDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CUpipDlg dialog
class CBuildServerDlg;
class CUpipDlg : public CDialogEx
{
// Construction
public:
	CUpipDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CUpipDlg)
	enum { IDD = IDD_UPIP };
	CString	m_FtpIP;
	int		m_FtpPort;
	CString	m_Pass;
	CString	m_User;
	CString	m_FtpUrl;
	CString	m_YourPort;
	
	bool m_bFirstShow;
	
	CString	m_Dns;
	CString	m_DnsId;
	CString	m_DnsPass;
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA
//	m_pCBuildServerDlg= pCBuildServerDlg;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUpipDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CUpipDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnUpftp();
	afx_msg void OnUpdnsip();
	afx_msg void OnButtonGet();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UPIPDLG_H__8AC0DAC5_4498_45CB_91E2_C76437753990__INCLUDED_)
