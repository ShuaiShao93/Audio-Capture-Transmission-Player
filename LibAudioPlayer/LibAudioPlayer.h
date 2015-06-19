#include <Windows.h>
#include <AudioClient.h>
#include <mmdeviceapi.h>
#include <process.h>
#include <stdio.h>
#define MAXLENGTH 98304

class AudioPlayer {
public:
	LARGE_INTEGER starttime4, endtime4;
	CRITICAL_SECTION dataProcessSection;
	int playThreshold, lowerLimit;
	DWORD flags;
	IAudioClient *pAudioClient;
    IAudioRenderClient *pRenderClient;
	IMMDeviceEnumerator *pEnumerator;
    IMMDevice *pDevice;
	REFERENCE_TIME hnsRequestedDuration;
    REFERENCE_TIME hnsActualDuration;
	UINT32 bufferFrameCount;
	WAVEFORMATEXTENSIBLE wvFmt;
	BYTE* audioBuffer;
	int pOldest, pLatest, isEmpty;

	HRESULT GetData(UINT32, BYTE*);
	void RunThread();
	int getDataSize();

	AudioPlayer();
	HRESULT Start();
	HRESULT LoadData(int numBytes, BYTE* pData);
	~AudioPlayer();
};