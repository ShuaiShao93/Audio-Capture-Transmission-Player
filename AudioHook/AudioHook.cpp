#include "AudioHook.h"


HRESULT (STDMETHODCALLTYPE *SysIACInitialize)(IAudioClient *,
											  AUDCLNT_SHAREMODE,
											  DWORD,
											  REFERENCE_TIME,
											  REFERENCE_TIME,
											  const WAVEFORMATEX *,
											  LPCGUID) = NULL;

HRESULT (STDMETHODCALLTYPE *SysIARCGetBuffer)(IAudioRenderClient *,
                                              UINT32,
                                              BYTE **) = NULL;

HRESULT (STDMETHODCALLTYPE *SysIARCReleaseBuffer)(IAudioRenderClient *,
                                              UINT32,
                                              DWORD) = NULL;

Sender *mySender;
HANDLE hMutex[MAXSESSION][MAXPROCESS];

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD dwReason, LPVOID reserved)
{
	REFERENCE_TIME hnsRequestedDuration = 10000000;
	IMMDeviceEnumerator *pEnumerator = NULL;
    IMMDevice *pDevice = NULL;
    IAudioClient *pAudioClient = NULL;
	IAudioRenderClient *pRenderClient = NULL;
	WAVEFORMATEX *pwfx = NULL;
	HRESULT hr;
    LONG error;
    (void)hinst;
    (void)reserved;

    if (DetourIsHelperProcess()) {
        return TRUE;
    }

    if (dwReason == DLL_PROCESS_ATTACH) {
		
		mySender = new Sender;
		mySender->InitArg();

		hMutex[mySender->arg.SessionNum][mySender->arg.ProcessNum] = CreateMutex(NULL, FALSE, NULL);

		CoInitialize(NULL);


		hr = CoCreateInstance(
			   CLSID_MMDeviceEnumerator, NULL,
			   CLSCTX_ALL, IID_IMMDeviceEnumerator,
			   (void**)&pEnumerator);
		hr = pEnumerator->lpVtbl->GetDefaultAudioEndpoint(pEnumerator,
							eRender, eConsole, &pDevice);
		hr = pDevice->lpVtbl->Activate(pDevice,
						IID_IAudioClient, CLSCTX_ALL,
						NULL, (void**)&pAudioClient);

		SysIACInitialize = pAudioClient->lpVtbl->Initialize;

		hr = pAudioClient->lpVtbl->GetMixFormat(pAudioClient, &pwfx);
		mySender->SetWAVFormat(pwfx);

		

		hr = pAudioClient->lpVtbl->Initialize(pAudioClient,
							 AUDCLNT_SHAREMODE_SHARED,
							 0,
							 hnsRequestedDuration,
							 0,
							 pwfx,
							 NULL);
		 hr = pAudioClient->lpVtbl->GetService(pAudioClient,
							 IID_IAudioRenderClient,
							 (void**)&pRenderClient);

		
		SysIARCGetBuffer = pRenderClient->lpVtbl->GetBuffer;
		SysIARCReleaseBuffer = pRenderClient->lpVtbl->ReleaseBuffer;


        DetourRestoreAfterWith();
		printf("Detour start.\n");


		DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)SysIACInitialize, MyIACInitialize);
        error = DetourTransactionCommit();

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)SysIARCGetBuffer, MyIARCGetBuffer);
        error = DetourTransactionCommit();

		DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)SysIARCReleaseBuffer, MyIARCReleaseBuffer);
        error = DetourTransactionCommit();

    }
    else if (dwReason == DLL_PROCESS_DETACH) {

		DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&(PVOID&)SysIACInitialize, MyIACInitialize);
        error = DetourTransactionCommit();

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&(PVOID&)SysIARCGetBuffer, MyIARCGetBuffer);
        error = DetourTransactionCommit();

		DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&(PVOID&)SysIARCReleaseBuffer, MyIARCReleaseBuffer);
        error = DetourTransactionCommit();

		CoUninitialize();

		mySender->DelProcess();

		CloseHandle(hMutex[mySender->arg.SessionNum][mySender->arg.ProcessNum]);
    }
	
    return TRUE;
}



//
///////////////////////////////////////////////////////////////// End of File.
