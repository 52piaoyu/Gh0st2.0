#if !defined(AFX_MORECHOOSE_H__D92FACC3_3CAF_4654_84F4_97C812732A7F__INCLUDED_)
#define AFX_MORECHOOSE_H__D92FACC3_3CAF_4654_84F4_97C812732A7F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MoreChoose.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMoreChoose form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

class CMoreChoose : public CFormView
{
protected:
	CMoreChoose();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CMoreChoose)

// Form Data
public:
	//{{AFX_DATA(CMoreChoose)
	enum { IDD = IDD_MORE_CHOOSE };
	BOOL	m_Win2000;
	BOOL	m_Win2003;
	BOOL	m_Win2008;
	BOOL	m_WinNT;
	BOOL	m_WinVista;
	BOOL	m_Win7;
	BOOL	m_WinXP;
	BOOL	m_check_num;
	BOOL	m_check_all;
	int		m_edit_num;
	//}}AFX_DATA

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMoreChoose)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CMoreChoose();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CMoreChoose)
	afx_msg void OnCheckNum();
	afx_msg void OnFind();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MORECHOOSE_H__D92FACC3_3CAF_4654_84F4_97C812732A7F__INCLUDED_)
