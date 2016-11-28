#include "stdafx.h"
//#include "windows.h"

typedef BOOL (__stdcall *LPSkinH_AttachRes)(LPBYTE pShe,DWORD dwSize,LPCTSTR strPass,int nHue,int nSat,int nBri);
typedef BOOL (__stdcall *LPSkinH_SetAero)(DWORD aero);
typedef BOOL (__stdcall *LPSkinH_SetMenuAlpha)(DWORD id);

int  res2file(LPCTSTR lpName,LPCTSTR lpType,LPCTSTR filename)
{
	HRSRC myres = FindResource (NULL,lpName,lpType);
	HGLOBAL gl = LoadResource (NULL,myres);
	LPVOID lp = LockResource(gl);

	HANDLE fp = CreateFile(filename ,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,0,NULL);
	
	if (!fp) return 0;
	
	DWORD a;

	if(!WriteFile (fp,lp,SizeofResource(NULL,myres),&a,NULL)) 
		return 0;

	CloseHandle(fp);
	FreeResource(gl);
	return 1;
}

int loadSkin(WORD id,TCHAR type[])
{
	TCHAR pDll[64] = _T("SkinH.dll"); 

	GetWindowsDirectory(pDll,sizeof(pDll));
	lstrcat(pDll,_T("\\SkinH.dll"));
	res2file(MAKEINTRESOURCE(IDR_DLL_A),_T("SKIN"),pDll); 
	
	HMODULE hDll = (HMODULE)LoadLibrary(pDll);
	
	if(hDll == NULL)
	{
		MsgErr(L"Load");
		return 0;
	}
	
	LPSkinH_AttachRes AttachRes = (LPSkinH_AttachRes)GetProcAddress(hDll,"SkinH_AttachRes");
	LPSkinH_SetAero SetAero = (LPSkinH_SetAero)GetProcAddress(hDll,"SkinH_SetAero");
	LPSkinH_SetMenuAlpha SetMenuAlpha = (LPSkinH_SetMenuAlpha)GetProcAddress(hDll,"SkinH_SetMenuAlpha");
	
	HGLOBAL hRes;
	HRSRC hResInfo;
	HINSTANCE hinst = AfxGetInstanceHandle();
	hResInfo = FindResource( hinst, MAKEINTRESOURCE(id),type );
	if (hResInfo != NULL)
	{
		hRes = LoadResource( hinst, hResInfo);
		if (hRes != NULL)
		{
			AttachRes( (LPBYTE)hRes, SizeofResource(hinst,hResInfo), NULL, NULL, NULL, NULL );
			SetAero(true);

			SetMenuAlpha(220);

			FreeResource(hRes);
		}
	}

	return 0;
}