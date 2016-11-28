#if !defined(AFX_PROXYDLG_H__D0C52873_02FE_47B4_9FD6_8727762E3496__INCLUDED_)
#define AFX_PROXYDLG_H__D0C52873_02FE_47B4_9FD6_8727762E3496__INCLUDED_

#pragma once

#include <afxdialogex.h>

// ProxyDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CProxyDlg dialog

class CProxyDlg : public CDialogEx
{
// Construction
public:
	CProxyDlg(CWnd* pParent = NULL, CIOCPServer* pIOCPServer = NULL, ClientContext *pContext = NULL);   // standard constructor
	static void CALLBACK NotifyProc(LPVOID lpParam, ClientContext *pContext, UINT nCode);
	ClientContext * pContexts[10000];
	void OnReceiveComplete();
	void AddLog(TCHAR * lpText);
	LRESULT OnNotifyProc(WPARAM wParam, LPARAM lParam);
// Dialog Data
	//{{AFX_DATA(CProxyDlg)
	enum { IDD = IDD_SOCKS };
	CEdit	m_edit;
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProxyDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CProxyDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	ClientContext* m_pContext;
	CIOCPServer* m_iocpServer;
	CIOCPLOCAL* m_iocpLocal;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROXYDLG_H__D0C52873_02FE_47B4_9FD6_8727762E3496__INCLUDED_)
