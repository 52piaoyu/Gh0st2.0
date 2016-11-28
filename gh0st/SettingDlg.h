#if !defined(AFX_SETTINGDLG_H__72219B48_05E1_4A73_96E7_74E73EC9F7AF__INCLUDED_)
#define AFX_SETTINGDLG_H__72219B48_05E1_4A73_96E7_74E73EC9F7AF__INCLUDED_

#pragma once

#include <afxdialogex.h>

// SettingDlg.h : header file
//
//#include "control/HoverEdit.h"
/////////////////////////////////////////////////////////////////////////////
// CSettingDlg dialog

class CSettingDlg : public CDialogEx
{
// Construction
public:
	CSettingDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSettingDlg)
	enum { IDD = IDD_SETTING };
	CString	m_remote_host;
	CString	m_remote_port;
//	CString	m_encode;
	UINT	m_listen_port;
	UINT	m_max_connections;
	BOOL	m_connect_auto;
	BOOL	m_bIsDisablePopTips;
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSettingDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSettingDlg)
	afx_msg void OnChangeConfig(UINT id);
	virtual BOOL OnInitDialog();
	afx_msg void OnResetport();
	afx_msg void OnConnectAuto();
	afx_msg void OnDisablePoptips();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	bool m_bFirstShow;
	CEdit	m_Edit[9];
	CButton	m_Btn[3];

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SETTINGDLG_H__72219B48_05E1_4A73_96E7_74E73EC9F7AF__INCLUDED_)
