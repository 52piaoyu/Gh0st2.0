#if !defined(AFX_UPDATEIP_H__CFA44E5D_9E8C_4377_8946_E4CEDC73FD9B__INCLUDED_)
#define AFX_UPDATEIP_H__CFA44E5D_9E8C_4377_8946_E4CEDC73FD9B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// UPDATEIP.h : header file
//
//#include "include/StatLink.h"
//#include "include/IniFile.h"
/////////////////////////////////////////////////////////////////////////////
// CUPDATEIP form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

class CUPDATEIP : public CFormView
{
protected:
	CUPDATEIP();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CUPDATEIP)

// Form Data
public:
	//{{AFX_DATA(CUPDATEIP)
	enum { IDD = IDD_UPDATEIP };
	CComboBox	m_Combo_DnsIP;
	CComboBox	m_Combo_FtpIP;
//	CStaticLink	m_3322;
	CString	m_FtpUrl;
	CString	m_FtpUser;
	UINT	m_FtpPort;
	CString	m_FtpPass;
	CString	m_FtpIP;
	CString	m_DnsUser;
	CString	m_DnsPass;
	CString	m_DnsDomain;
	CString	m_zifushuan;
	//}}AFX_DATA

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUPDATEIP)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CUPDATEIP();
	DWORD FtpUpdate();
	static DWORD __stdcall FtpUpdateThread(void* pThis);
	CIniFile m_Ini;
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CUPDATEIP)
	afx_msg void OnBtnDnsupdate();
	afx_msg void OnEditchangeComboFtpip();
	afx_msg void OnBtnFtpupdate();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UPDATEIP_H__CFA44E5D_9E8C_4377_8946_E4CEDC73FD9B__INCLUDED_)
