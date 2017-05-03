#if !defined(XICAO_INSTALLSERVICE_H__)
#define XICAO_INSTALLSERVICE_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#pragma once
//#include <windows.h>
#include <tchar.h>

//作用:启动一个已存在服务,如存在，多次重复直到其成功
//返回值:UNIT
UINT NtStartService(LPCTSTR lpService);

//作用:检查一个服务是否存在
//返回值:BOOL
BOOL NtServiceIsExist(LPCTSTR servicename);

//作用:停止一个已存在服务
//返回值:UINT
UINT NtStopService(LPCTSTR lpService, ULONG TimeoutSeconds);


//作用:创建一个系统服务.SERVICE_AUTO_START
//返回值UINT
UINT NtInstallService(LPCTSTR strServiceName, //服务名称
	LPCTSTR strDisplayName, //服务显示名称
	LPCTSTR strDescription,//服务描述
	LPCTSTR strPathName,   //可执行文件的路径
	LPCTSTR Dependencies,//指定启动该服务前必须先启动的服务或服务组,一般为NULL
	BOOLEAN KernelDriver, //是否安装驱动程序
	ULONG   StartType		//启动类型
);


//作用:卸载一个系统服务
//返回值UINT
UINT NtUninstallService(LPCTSTR ServiceName);

//SERVICE_AUTO_START
//作用:改变服务启动状态
//返回值:UINT  成功返回1,失败返回0
//
UINT NtServiceSetStartMode(LPCTSTR ServiceName, ULONG StartMode);

//作用:创建一个svchost启动的服务,SERVICE_AUTO_START
//返回值BOOL
BOOL NtInstallSvchostService(LPCTSTR strServiceName,
	LPCTSTR strServiceDisp,
	LPCTSTR strServiceDesc,
	LPCTSTR strDllPath);

//---------------------------------------------------------


#endif // !defined(XICAO_INSTALLSERVICE_H__)