#include "AudioHook.h"

extern Sender *mySender;

extern HRESULT (STDMETHODCALLTYPE *SysIACInitialize)(IAudioClient *,
											  AUDCLNT_SHAREMODE,
											  DWORD,
											  REFERENCE_TIME,
											  REFERENCE_TIME,
											  const WAVEFORMATEX *,
											  LPCGUID);

HRESULT STDMETHODCALLTYPE MyIACInitialize(IAudioClient *This,
											  AUDCLNT_SHAREMODE ShareMode,
											  DWORD StreamFlags,
											  REFERENCE_TIME hnsBufferDuration,
											  REFERENCE_TIME hnsPeriodicity,
											  const WAVEFORMATEX *pFormat,
											  LPCGUID AudioSessionGuid)
{
	HRESULT hr = S_OK;

	hr = SysIACInitialize(This, ShareMode, StreamFlags, hnsBufferDuration, hnsPeriodicity, pFormat, AudioSessionGuid);

	mySender->SetWAVFormat((WAVEFORMATEX*)pFormat);

	return hr;
}
