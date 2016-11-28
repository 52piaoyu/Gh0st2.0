#if !defined(AFX_SEND_H__55FFA10D_FBA7_45EA_9EE5_9DC232146D16__INCLUDED_)
#define AFX_SEND_H__55FFA10D_FBA7_45EA_9EE5_9DC232146D16__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SEND.h : header file
//
/////////////////////////////////////////////////////////////////////////////
// CSEND form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

#include "resource.h"

class CSEND : public CFormView
{
protected:
	CSEND();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CSEND)

// Form Data
public:
	//{{AFX_DATA(CSEND)
	enum { IDD = IDD_SEND };
	CComboBox	m_open_kinds;
	CEdit	m_open;
	CEdit	m_down;
	CEdit	m_msg;
	//}}AFX_DATA

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSEND)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CSEND();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CSEND)
	afx_msg void OnMsg();
	afx_msg void OnShutdown();
	afx_msg void OnLogoff();
	afx_msg void OnReboot();
	afx_msg void OnRemove();
	afx_msg void OnOpen();
	afx_msg void OnDown();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SEND_H__55FFA10D_FBA7_45EA_9EE5_9DC232146D16__INCLUDED_)
