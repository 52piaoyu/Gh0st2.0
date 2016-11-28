
#pragma once

#include <afxdialogex.h>
// SystemDlg.h : header file
//

#define CListCtrl	CListCtrl
/////////////////////////////////////////////////////////////////////////////
// CSystemDlg dialog


#include <vector>
using namespace std;
typedef struct SERVICE_INFO_S
{
	TCHAR			szServiceName[FILENAME_MAX];	// 服务名称	
	TCHAR			szDisplayName[FILENAME_MAX];	// 服务显示名称	
	DWORD			dwRunState;						// 服务运行状态	
	DWORD			dwStartType;					// 服务启动类型
	
}SERVICE_INFO_S;


class CSystemDlg : public CDialogEx
{
// Construction
public:
	CSystemDlg(CWnd* pParent = NULL, CIOCPServer* pIOCPServer = NULL, ClientContext *pContext = NULL);   // standard constructor
	void OnReceiveComplete();
// Dialog Data
	//{{AFX_DATA(CSystemDlg)
	enum { IDD = IDD_SYSTEM };
	std::vector<SERVICE_INFO_S>		m_vectService;
	CListCtrl	m_list_Services;
	CListCtrl	m_list_windows;
	CListCtrl	m_list_process;
	CListCtrl	m_list_sysinfo;
	CTabCtrl	m_tab;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSystemDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);	
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
private:
	void AdjustList();

	void ShowProcessList();
	void ShowWindowsList();
	void ShowServicesList();
	void ShowSysinfoList();
	void ShowSelectWindow();
	void GetProcessList();
	void GetWindowsList();
	void GetServicesList();
	void GetSysinfoList();

	void OnSetAuto();
	void OnSetDemand();
	void OnSetDisable();
	void OnSetStart();
	void OnSetStop();
	void OnSetDelete();
	HICON m_hIcon;
	ClientContext* m_pContext;
	CIOCPServer* m_iocpServer;

	CString FormatServiceState(DWORD dwRunState);
	CString FormatServiceStartType(DWORD dwStartType);
	// Generated message map functions
	//{{AFX_MSG(CSystemDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnRclickList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRclickServicesList();
	afx_msg void OnKillprocess();
	afx_msg void OnRefreshPsList();
	afx_msg void OnSelchangeTab(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
