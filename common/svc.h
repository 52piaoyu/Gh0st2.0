//#include "windows.h"
#include "tchar.h"

int  ReadRegSz(HKEY MainKey,LPCTSTR SubKey,LPCTSTR Vname,TCHAR *szData,DWORD lbSize)
{   
	HKEY   hKey;  
	int    iResult=0;
	TCHAR  ValueSz[MAX_PATH];	
	DWORD  szSize,type = REG_SZ;	 

	memset(ValueSz,0,sizeof(ValueSz));
		 
	__try
	{ 	
		if(RegOpenKeyEx(MainKey,SubKey,0,KEY_READ,&hKey) == ERROR_SUCCESS)
		{
        	szSize = sizeof(ValueSz);
			if(RegQueryValueEx(hKey,Vname,NULL,&type,(LPBYTE)ValueSz,&szSize) == ERROR_SUCCESS)
			{
				lstrcpy(szData,ValueSz);
				iResult =1;
			}
		}	
	}
	__finally
	{
        RegCloseKey(MainKey);
		RegCloseKey(hKey);
	}
     
	return iResult;
}

UINT UninstallService(LPCTSTR ServiceName)
{
    SC_HANDLE       schService;
    SC_HANDLE       schSCManager;

    schSCManager = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
    
    if (!schSCManager)
    {
        return -1;
    }
    
	schService = OpenService(schSCManager, ServiceName, SERVICE_ALL_ACCESS);
    
    if (!schService)
	{
        CloseServiceHandle(schSCManager);
        return -1;
    }
    
    DeleteService(schService);

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);

    return 0;
}

UINT ServiceSetStartMode(LPCTSTR ServiceName,ULONG StartMode)
{
    SC_HANDLE       schService;
    SC_HANDLE       schSCManager;
 
    schSCManager = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
    
    if (!schSCManager)
    {
        return -1;
    }
    
    schService = OpenService(schSCManager, ServiceName, SERVICE_ALL_ACCESS);
    
    if (!schService) 
    {
        CloseServiceHandle(schSCManager);

		return -1;
    }
    
    ChangeServiceConfig(schService, SERVICE_NO_CHANGE,StartMode,SERVICE_NO_CHANGE,
						NULL, NULL, NULL,NULL, NULL, NULL,NULL);

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);

    return 0;
}


UINT LxStartService(LPCTSTR lpService)
{
    SC_HANDLE        schSCManager;
    SC_HANDLE        schService;

    schSCManager=OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
    
    if(!schSCManager)
    {
        return -1;
    }
    
    if (NULL!=schSCManager)
    {
        schService=OpenService(schSCManager,lpService,SERVICE_ALL_ACCESS);

        if (schService!=NULL)
        {
            if(StartService(schService,0,NULL)==0)//已经存在该服务,就启动服务                        
            {
                CloseServiceHandle(schSCManager);  
                CloseServiceHandle(schService);
                
				return 1;
            }

            CloseServiceHandle(schService);
        }
        CloseServiceHandle(schSCManager);
    }
    else
    {
        CloseServiceHandle(schSCManager);
        return -1;
    }

    return 1;
}

UINT LxStopService(LPCTSTR lpService,ULONG   TimeoutSeconds)
{
    SC_HANDLE        schSCManager;
    SC_HANDLE        schService;
    SERVICE_STATUS   RemoveServiceStatus;

    schSCManager = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
    
    if (schSCManager!=NULL)
    {
        schService=OpenService(schSCManager,lpService,SERVICE_ALL_ACCESS);
        
        if (schService!=NULL)
        {
            ControlService(schService,SERVICE_CONTROL_STOP,&RemoveServiceStatus);

            CloseServiceHandle(schService);
        } 
		  
        CloseServiceHandle(schSCManager);
    }
    else
    {
        CloseServiceHandle(schSCManager);
        return -1;
    }

    return TRUE;
}
