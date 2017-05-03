#if !defined(XICAO_INSTALLSERVICE_H__)
#define XICAO_INSTALLSERVICE_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#pragma once
//#include <windows.h>
#include <tchar.h>

//����:����һ���Ѵ��ڷ���,����ڣ�����ظ�ֱ����ɹ�
//����ֵ:UNIT
UINT NtStartService(LPCTSTR lpService);

//����:���һ�������Ƿ����
//����ֵ:BOOL
BOOL NtServiceIsExist(LPCTSTR servicename);

//����:ֹͣһ���Ѵ��ڷ���
//����ֵ:UINT
UINT NtStopService(LPCTSTR lpService, ULONG TimeoutSeconds);


//����:����һ��ϵͳ����.SERVICE_AUTO_START
//����ֵUINT
UINT NtInstallService(LPCTSTR strServiceName, //��������
	LPCTSTR strDisplayName, //������ʾ����
	LPCTSTR strDescription,//��������
	LPCTSTR strPathName,   //��ִ���ļ���·��
	LPCTSTR Dependencies,//ָ�������÷���ǰ�����������ķ���������,һ��ΪNULL
	BOOLEAN KernelDriver, //�Ƿ�װ��������
	ULONG   StartType		//��������
);


//����:ж��һ��ϵͳ����
//����ֵUINT
UINT NtUninstallService(LPCTSTR ServiceName);

//SERVICE_AUTO_START
//����:�ı��������״̬
//����ֵ:UINT  �ɹ�����1,ʧ�ܷ���0
//
UINT NtServiceSetStartMode(LPCTSTR ServiceName, ULONG StartMode);

//����:����һ��svchost�����ķ���,SERVICE_AUTO_START
//����ֵBOOL
BOOL NtInstallSvchostService(LPCTSTR strServiceName,
	LPCTSTR strServiceDisp,
	LPCTSTR strServiceDesc,
	LPCTSTR strDllPath);

//---------------------------------------------------------


#endif // !defined(XICAO_INSTALLSERVICE_H__)