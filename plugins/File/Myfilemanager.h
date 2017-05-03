// MyFileManager.h: interface for the CMyFileManager class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning(disable:4786) 
#if !defined(AFX_MyFileManager_H__359D0039_E61F_46D6_86D6_A405E998FB47__INCLUDED_)
#define AFX_MyFileManager_H__359D0039_E61F_46D6_86D6_A405E998FB47__INCLUDED_
#include <winsock2.h>
#include <list>
#include <string>

#include "Manager.h"
using namespace std;

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

typedef struct _SENDFILEPROGRESS
{
	UINT	nFileSize;	// 文件大小
	UINT	nSendSize;	// 已发送大小
}SENDFILEPROGRESS, *PSENDFILEPROGRESS;


class CMyFileManager : public CManager
{
public:
	void OnReceive(LPBYTE lpBuffer, UINT nSize);
	UINT SendDriveList();
	CMyFileManager(CClientSocket *pClient);
	virtual ~CMyFileManager();
private:
#ifdef UNICODE
	list <wstring> m_UploadList;
#else
	list <string> m_UploadList;
#endif
	UINT m_nTransferMode;
	TCHAR m_strCurrentProcessFileName[MAX_PATH]; // 当前正在处理的文件
	LONGLONG  m_nCurrentProcessFileLength; // 当前正在处理的文件的长度

#ifdef NOUSEAPI
	bool MakeSureDirectoryPathExists(LPCTSTR pszDirPath);
#endif

	bool UploadToRemote(LPBYTE lpBuffer);
	bool FixedUploadList(LPCTSTR lpszDirectory);
	void StopTransfer();
	UINT SendFilesList(LPCTSTR lpszDirectory);
	bool DeleteDirectory(LPCTSTR lpszDirectory);
	UINT SendFileSize(LPCTSTR lpszFileName);
	UINT SendFileData(LPBYTE lpBuffer);
	void CreateFolder(LPBYTE lpBuffer);
	void Rename(LPBYTE lpBuffer);
	int	SendToken(BYTE bToken);

	void CreateLocalRecvFile(LPBYTE lpBuffer);
	void SetTransferMode(LPBYTE lpBuffer);
	void GetFileData();
	void WriteLocalRecvFile(LPBYTE lpBuffer, UINT nSize);
	void UploadNext();
	bool OpenFile(LPCTSTR lpFile, INT nShowCmd);

	void PackFile(LPBYTE lpBuffer);
	void UnpackFile(LPBYTE lpBuffer);
};

#endif // !defined(AFX_MyFileManager_H__359D0039_E61F_46D6_86D6_A405E998FB47__INCLUDED_)
