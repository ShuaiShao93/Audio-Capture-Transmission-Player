#include "LibAudioPlayer.h"

#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);

AudioPlayer::AudioPlayer() {
	HRESULT hr;
	WAVEFORMATEX *pwfx;

	pAudioClient = NULL;
	pRenderClient = NULL;
	pEnumerator = NULL;
	pDevice = NULL;
	flags = 0;
	hnsRequestedDuration = 1000000;
	audioBuffer = new BYTE[MAXLENGTH];

	pLatest = 0;
	pOldest = 0;
	isEmpty = 1;

	InitializeCriticalSection(&dataProcessSection);

	hr = CoInitialize(NULL);

    hr = CoCreateInstance(
           CLSID_MMDeviceEnumerator, NULL,
           CLSCTX_ALL, IID_IMMDeviceEnumerator,
           (void**)&pEnumerator);


    hr = pEnumerator->GetDefaultAudioEndpoint(
                        eRender, eConsole, &pDevice);


    hr = pDevice->Activate(
                    IID_IAudioClient, CLSCTX_ALL,
                    NULL, (void**)&pAudioClient);


    hr = pAudioClient->GetMixFormat(&pwfx);
	wvFmt = *(WAVEFORMATEXTENSIBLE*)pwfx;

    hr = pAudioClient->Initialize(
                         AUDCLNT_SHAREMODE_SHARED,
                         0,
                         hnsRequestedDuration,
                         0,
						 pwfx,
                         NULL);

    // Get the actual size of the allocated buffer.
    hr = pAudioClient->GetBufferSize(&bufferFrameCount);


    hr = pAudioClient->GetService(
                         IID_IAudioRenderClient,
                         (void**)&pRenderClient);

}

void runThread(void*);

HRESULT AudioPlayer::Start() {

	_beginthread(runThread, 0, (void*)this);
	return S_OK;
}

void runThread(void *audioPlayer) {
	UINT32 numFramesAvailable;
    UINT32 numFramesPadding;
	UINT32 size;
	HRESULT hr;
	BYTE *pData;
	AudioPlayer *ap = (AudioPlayer*)audioPlayer;
	LARGE_INTEGER frequency;
	
	EnterCriticalSection(&(ap->dataProcessSection));
	hr = ap->pAudioClient->GetCurrentPadding(&numFramesPadding);
	numFramesAvailable = (ap->bufferFrameCount - numFramesPadding < ap->getDataSize()) ?ap->bufferFrameCount - numFramesPadding : ap->getDataSize();
	hr = ap->pRenderClient->GetBuffer(numFramesAvailable, &pData);


	ap->GetData(numFramesAvailable, pData);


	hr = ap->pRenderClient->ReleaseBuffer(numFramesAvailable, ap->flags);
	LeaveCriticalSection(&(ap->dataProcessSection));
	hr = ap->pAudioClient->Start();  // Start playing.
		// Each loop fills about half of the shared buffer.
		while (TRUE){
			if(ap->getDataSize() >= 3840){
			    EnterCriticalSection(&(ap->dataProcessSection));
			    hr = ap->pAudioClient->GetCurrentPadding(&numFramesPadding);
			    numFramesAvailable = (ap->bufferFrameCount - numFramesPadding < ap->getDataSize()) ?ap->bufferFrameCount - numFramesPadding : ap->getDataSize();
			    hr = ap->pRenderClient->GetBuffer(numFramesAvailable, &pData);

			    ap->GetData(numFramesAvailable, pData);

			    hr = ap->pRenderClient->ReleaseBuffer(numFramesAvailable, ap->flags);
		        LeaveCriticalSection(&(ap->dataProcessSection));
			}
		}
		hr = ap->pAudioClient->Stop();
}

AudioPlayer::~AudioPlayer() {
	DeleteCriticalSection(&dataProcessSection);

	SAFE_RELEASE(pEnumerator)
    SAFE_RELEASE(pDevice)
    SAFE_RELEASE(pAudioClient)
    SAFE_RELEASE(pRenderClient)
	CoUninitialize();
}

HRESULT AudioPlayer::GetData(UINT32 numFramesAvailable, BYTE* pData) {

	int numBytes = numFramesAvailable * wvFmt.Format.nChannels * wvFmt.Format.wBitsPerSample / 8;

	if (pLatest > pOldest) {
		if (numBytes > pLatest - pOldest) 
			numBytes = pLatest - pOldest;
		memcpy(pData, audioBuffer + pOldest, numBytes);
		pOldest += numBytes;
	}

	else {
		if (numBytes > pLatest + MAXLENGTH - pOldest) 
			numBytes = pLatest + MAXLENGTH - pOldest;
		if (pOldest + numBytes <= MAXLENGTH) {
			memcpy(pData, audioBuffer + pOldest, numBytes);
			pOldest += numBytes;
		}
		else {
			memcpy(pData, audioBuffer + pOldest, MAXLENGTH - pOldest);
			memcpy(pData + MAXLENGTH - pOldest, audioBuffer, numBytes - (MAXLENGTH - pOldest));
			pOldest = pOldest + numBytes - MAXLENGTH;
		}
	}

	if (pOldest == pLatest) isEmpty = 1;
	return S_OK;
}

HRESULT AudioPlayer::LoadData(int numBytes, BYTE* pData) {

	EnterCriticalSection(&dataProcessSection);
	if (pLatest + numBytes <= MAXLENGTH)
		memcpy(audioBuffer + pLatest, pData, numBytes);
	else {
		memcpy(audioBuffer + pLatest, pData, MAXLENGTH - pLatest);
		memcpy(audioBuffer, pData + (MAXLENGTH - pLatest), numBytes - (MAXLENGTH - pLatest));
	}

	if (pOldest < pLatest || (pOldest == pLatest && isEmpty == 1)) {
		if (pLatest + numBytes - MAXLENGTH > pOldest) {
			pLatest = pOldest = pLatest + numBytes - MAXLENGTH;
			isEmpty = 0;
		}
		else{ 
			pLatest = (pLatest + numBytes) % MAXLENGTH;
			if(numBytes != 0)
				isEmpty = 0;
		}
	}
	else {
		if (pLatest + numBytes > pOldest) {
			pLatest = pOldest = (pLatest + numBytes) % MAXLENGTH;
		}
		else
			pLatest += numBytes;
	}
	LeaveCriticalSection(&dataProcessSection);

	return S_OK;
	
}

int AudioPlayer::getDataSize() {
	if (pOldest < pLatest)
		return (pLatest - pOldest);
	else if (pOldest > pLatest)
		return (pLatest + MAXLENGTH - pOldest);
	else
		return isEmpty?0:MAXLENGTH;
}