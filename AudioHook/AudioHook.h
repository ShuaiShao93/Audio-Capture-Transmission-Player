#include <stdio.h>
#define CINTERFACE
#include <Windows.h>
#include <detours.h>
#include <Audioclient.h>
#include <mmdeviceapi.h>
#include <io.h>
#include <process.h>
#define MAXLENGTH 76800

#define MAXSESSION 10

#define  MAXPROCESS 40

#pragma comment(lib, "detours.lib")

struct AudioOfProcess{
	int ProcessID;
	int pLatest, pOldest, isEmpty;
    BYTE AudioBuffer[MAXLENGTH];

	void InitProcess();
};

struct AudioOfSession{
	unsigned long SessionID;
	int NumOfProcess;
	AudioOfProcess ProcessAudio[MAXPROCESS];

	void InitSession();
	void DelProcess(int ProcessNum);
};

struct argument{
		int numFrames;
		BYTE* pData;
		int frameSize;
		int SessionNum;
		int ProcessNum;
};

class Sender{
	//BYTE *pBuffer[2];
	int usingBuffer, filledSize;
	UINT32 frameSize, packageSize;
	void SendBuffer(int);
	WAVEFORMATEXTENSIBLE wvFmt;
public:
	argument arg;
	Sender();
	~Sender();
	int getDataSize(int SN, int PN);
	int getBytesPerMilli();
	void InitArg();
	void DelProcess();
	void FillSenderBuffer(UINT32, BYTE*);
	void SetWAVFormat(WAVEFORMATEX*);
	void InitializeSender();
};

HRESULT STDMETHODCALLTYPE MyIACInitialize(IAudioClient *,
											  AUDCLNT_SHAREMODE,
											  DWORD,
											  REFERENCE_TIME,
											  REFERENCE_TIME,
											  const WAVEFORMATEX *,
											  LPCGUID);

HRESULT STDMETHODCALLTYPE MyIARCGetBuffer(IAudioRenderClient *,
                                              UINT32,
                                              BYTE **);

HRESULT STDMETHODCALLTYPE MyIARCReleaseBuffer(IAudioRenderClient *,
                                              UINT32,
                                              DWORD);