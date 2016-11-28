// Buffer.cpp: implementation of the CBuffer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Buffer.h"
#include "Math.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CBuffer::CBuffer()
{
	// Initial size
	m_nSize = 0;

	m_pPtr = m_pBase = NULL;
}

CBuffer::~CBuffer()
{
	if (m_pBase)
		VirtualFree(m_pBase, 0, MEM_RELEASE);
}

BOOL CBuffer::Write(PBYTE pData, UINT nSize)
{
	ReAllocateBuffer(nSize + GetBufferLen());

	CopyMemory(m_pPtr, pData, nSize);

	// Advance Pointer
	m_pPtr += nSize;

	return nSize;
}

BOOL CBuffer::Insert(PBYTE pData, UINT nSize)
{
	ReAllocateBuffer(nSize + GetBufferLen());

	MoveMemory(m_pBase + nSize, m_pBase, GetMemSize() - nSize);
	CopyMemory(m_pBase, pData, nSize);

	// Advance Pointer
	m_pPtr += nSize;

	return nSize;
}

UINT CBuffer::Read(PBYTE pData, UINT nSize)
{
	// Trying to byte off more than ya can chew - eh?
	if (nSize > GetMemSize())
		return 0;

	// all that we have
	if (nSize > GetBufferLen())
		nSize = GetBufferLen();

	if (nSize)
	{
		// Copy over required amount and its not up to us
		// to terminate the buffer - got that!!!
		CopyMemory(pData, m_pBase, nSize);

		// Slide the buffer back - like sinking the data
		MoveMemory(m_pBase, m_pBase + nSize, GetMemSize() - nSize);

		m_pPtr -= nSize;
	}

	DeAllocateBuffer(GetBufferLen());

	return nSize;
}

UINT CBuffer::GetMemSize()
{
	return m_nSize;
}

UINT CBuffer::GetBufferLen()
{
	if (m_pBase == NULL)
		return 0;

	int nSize =
		m_pPtr - m_pBase;
	return nSize;
}

UINT CBuffer::ReAllocateBuffer(UINT nRequestedSize)
{
	if (nRequestedSize < GetMemSize())
		return 0;

	// Allocate new size
	UINT nNewSize = (UINT)ceil(nRequestedSize / 1024.0) * 1024;

	// New Copy Data Over
	PBYTE pNewBuffer = (PBYTE)VirtualAlloc(NULL, nNewSize, MEM_COMMIT, PAGE_READWRITE);

	UINT nBufferLen = GetBufferLen();

	if (m_pBase)
	{
		CopyMemory(pNewBuffer, m_pBase, nBufferLen);
		VirtualFree(m_pBase, 0, MEM_RELEASE);
	}

	// Hand over the pointer
	m_pBase = pNewBuffer;

	// Realign position pointer
	m_pPtr = m_pBase + nBufferLen;

	m_nSize = nNewSize;

	return m_nSize;
}

UINT CBuffer::DeAllocateBuffer(UINT nRequestedSize)
{
	if (nRequestedSize < GetBufferLen())
		return 0;

	// Allocate new size
	UINT nNewSize = (UINT)ceil(nRequestedSize / 1024.0) * 1024;

	if (nNewSize < GetMemSize())
		return 0;

	// New Copy Data Over
	PBYTE pNewBuffer = (PBYTE)VirtualAlloc(NULL, nNewSize, MEM_COMMIT, PAGE_READWRITE);

	UINT nBufferLen = GetBufferLen();
	CopyMemory(pNewBuffer, m_pBase, nBufferLen);

	VirtualFree(m_pBase, 0, MEM_RELEASE);

	// Hand over the pointer
	m_pBase = pNewBuffer;

	// Realign position pointer
	m_pPtr = m_pBase + nBufferLen;

	m_nSize = nNewSize;

	return m_nSize;
}

int CBuffer::Scan(PBYTE pScan, UINT nPos)
{
	if (nPos > GetBufferLen())
		return -1;

	PBYTE pStr = (PBYTE)strstr((char*)(m_pBase + nPos), (char*)pScan);

	int nOffset = 0;

	if (pStr)
		nOffset = (pStr - m_pBase) + strlen((char*)pScan);

	return nOffset;
}

void CBuffer::ClearBuffer()
{
	// Force the buffer to be empty
	m_pPtr = m_pBase;

	DeAllocateBuffer(1024);
}

BOOL CBuffer::Write(CString& strData)
{
	int nSize = strData.GetLength();
	return Write((PBYTE)strData.GetBuffer(nSize), nSize);
}

BOOL CBuffer::Insert(CString& strData)
{
	int nSize = strData.GetLength();
	return Insert((PBYTE)strData.GetBuffer(nSize), nSize);
}

void CBuffer::Copy(CBuffer& buffer)
{
	int nReSize = buffer.GetMemSize();
	int nSize = buffer.GetBufferLen();
	ClearBuffer();
	ReAllocateBuffer(nReSize);

	m_pPtr = m_pBase + nSize;

	CopyMemory(m_pBase, buffer.GetBuffer(), buffer.GetBufferLen());
}

PBYTE CBuffer::GetBuffer(UINT nPos)
{
	return m_pBase + nPos;
}

void CBuffer::FileWrite(const CString& strFileName)
{
	CFile file;

	if (file.Open(strFileName, CFile::modeWrite | CFile::modeCreate))
	{
		file.Write(m_pBase, GetBufferLen());
		file.Close();
	}
}

UINT CBuffer::Delete(UINT nSize)
{
	// Trying to byte off more than ya can chew - eh?
	if (nSize > GetMemSize())
		return 0;

	// all that we have
	if (nSize > GetBufferLen())
		nSize = GetBufferLen();

	if (nSize)
	{
		// Slide the buffer back - like sinking the data
		MoveMemory(m_pBase, m_pBase + nSize, GetMemSize() - nSize);

		m_pPtr -= nSize;
	}

	DeAllocateBuffer(GetBufferLen());

	return nSize;
}