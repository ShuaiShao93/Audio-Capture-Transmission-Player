#include "AudioHook.h"


BYTE *pBuffer = NULL;

extern Sender* mySender;

extern HRESULT (STDMETHODCALLTYPE *SysIARCGetBuffer)(IAudioRenderClient *,
                                              UINT32,
                                              BYTE **);

extern HRESULT (STDMETHODCALLTYPE *SysIARCReleaseBuffer)(IAudioRenderClient *,
                                              UINT32,
                                              DWORD);

HRESULT STDMETHODCALLTYPE MyIARCGetBuffer(IAudioRenderClient *This,
                                              UINT32 NumFramesRequested,
                                              BYTE **ppData)
{
	HRESULT hr = S_OK;
	//printf("Hi, I'm getting buffers.\n");
	hr = SysIARCGetBuffer(This, NumFramesRequested, ppData);
	pBuffer = *ppData;
	return hr;
}

HRESULT STDMETHODCALLTYPE MyIARCReleaseBuffer(IAudioRenderClient *This,
                                              UINT32 NumFramesWritten,
                                              DWORD dwFlags)
{
	HRESULT hr = S_OK;
	//printf("Bye, I'm releasing buffers.\n");
	mySender->FillSenderBuffer(NumFramesWritten, pBuffer);
	hr = SysIARCReleaseBuffer(This, NumFramesWritten, dwFlags);
	return hr;
}