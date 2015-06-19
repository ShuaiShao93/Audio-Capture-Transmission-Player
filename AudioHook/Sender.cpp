#include "AudioHook.h"
//#define PACKAGE_TIME 10

#pragma data_seg("SharedData")
BOOL AvailSession[MAXSESSION] = {0};
AudioOfSession SessionAudio[MAXSESSION] = {0};
WAVEFORMATEXTENSIBLE waveFormat = {0};
#pragma data_seg()
#pragma comment(linker,"/SECTION:SharedData,RWS")

//int processId;
//char filename[30];
//FILE *fwav;
//int dataChunkSize, dataChunkPosition;
extern HANDLE hMutex[MAXSESSION][MAXPROCESS];


extern"C" WAVEFORMATEXTENSIBLE __declspec(dllexport) getFormat() {
	return waveFormat;
}

void AudioOfProcess::InitProcess(){
	ProcessID = 0;
	pLatest = 0; pOldest = 0; isEmpty = 1;
	for(int i = 0; i < MAXLENGTH; i++)
		AudioBuffer[i] = 0;
}

void AudioOfSession::InitSession(){
	SessionID = 0;
	NumOfProcess = 0;
	for(int i = 0; i< MAXPROCESS; i++)
	    ProcessAudio[i].InitProcess();
}

void AudioOfSession::DelProcess(int ProcessNum){
	NumOfProcess --;
	ProcessAudio[ProcessNum] = ProcessAudio[NumOfProcess];
	ProcessAudio[NumOfProcess].InitProcess();
}

Sender::Sender() {
}

void Sender::SetWAVFormat(WAVEFORMATEX *pwfx) {
	if (pwfx->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
		wvFmt = *(WAVEFORMATEXTENSIBLE*)pwfx;
	else
		wvFmt.Format = *pwfx;
	waveFormat = wvFmt;
	frameSize = pwfx->nChannels * pwfx->wBitsPerSample / 8;
}


void fillBuffer(void*);


void Sender::InitArg(){
    int curSession, curProcess;
	unsigned long ProcessID;
	unsigned long SessionID;

	ProcessID = GetCurrentProcessId();
	ProcessIdToSessionId(ProcessID, &SessionID);
	arg.SessionNum = -1;
	arg.ProcessNum = -1;
	for(curSession = 0; curSession < MAXSESSION; curSession ++){      //search for this session
		if(AvailSession[curSession] == TRUE && SessionAudio[curSession].SessionID == SessionID){
			arg.SessionNum = curSession;
			break;
		}
		else if(AvailSession[curSession] == FALSE && SessionAudio[curSession].SessionID != 0)       //clear spare sessions
			SessionAudio[curSession].InitSession();
	}
	if(curSession == MAXSESSION){                  //if this is a new sesssion
		for(curSession = 0; curSession < MAXSESSION; curSession++){
			if(AvailSession[curSession] == FALSE){ 
				AvailSession[curSession] = TRUE;
				arg.SessionNum = curSession;
				SessionAudio[curSession].SessionID = SessionID;
		        break;
			}
		}
	}
	if(arg.SessionNum >= 0){        //if sessions are not full
		for(curProcess = 0; curProcess < SessionAudio[arg.SessionNum].NumOfProcess; curProcess ++){   //search for this process
			if(ProcessID == SessionAudio[arg.SessionNum].ProcessAudio[curProcess].ProcessID){
				arg.ProcessNum = curProcess;
				break;
			}
		}
		if(curProcess == SessionAudio[arg.SessionNum].NumOfProcess){   //if this is a new process
			arg.ProcessNum = curProcess;
			SessionAudio[arg.SessionNum].ProcessAudio[curProcess].ProcessID = ProcessID;
			SessionAudio[arg.SessionNum].NumOfProcess ++;
		}
	}
}

void Sender::DelProcess(){
	if(arg.SessionNum >=0 && arg.ProcessNum >= 0)
		SessionAudio[arg.SessionNum].DelProcess(arg.ProcessNum);
}

void Sender::FillSenderBuffer(UINT32 numFrames, BYTE* pData) {
	arg.numFrames = numFrames;
	arg.frameSize = frameSize;
	arg.pData = pData;
	if(arg.ProcessNum >= 0)  //if processes are not full
	        _beginthread(fillBuffer, 0, &arg);
}

void fillBuffer(void *arg) {
	struct argument *a = (struct argument*)arg;
	int numBytes = a->numFrames * a->frameSize;
	int SN = a->SessionNum;
	int PN = a->ProcessNum;
	int pLatest = SessionAudio[SN].ProcessAudio[PN].pLatest;
	int pOldest = SessionAudio[SN].ProcessAudio[PN].pOldest;
	int isEmpty = SessionAudio[SN].ProcessAudio[PN].isEmpty;
	BYTE* audioBuffer = SessionAudio[SN].ProcessAudio[PN].AudioBuffer;

	WaitForSingleObject(hMutex[SN][PN], INFINITE);
	if(numBytes > MAXLENGTH){
		numBytes = MAXLENGTH;
		a->pData += numBytes - MAXLENGTH;
	}
	if (pLatest + numBytes <= MAXLENGTH)
		memcpy(audioBuffer + pLatest, a->pData, numBytes);
	else {
		memcpy(audioBuffer + pLatest, a->pData, MAXLENGTH - pLatest);
		memcpy(audioBuffer, a->pData + (MAXLENGTH - pLatest), numBytes - (MAXLENGTH - pLatest));
	}

	if (pOldest < pLatest || (pOldest == pLatest && isEmpty == 1)) {
		if (pLatest + numBytes - MAXLENGTH > pOldest) {
			SessionAudio[SN].ProcessAudio[PN].pLatest = SessionAudio[SN].ProcessAudio[PN].pOldest = pLatest + numBytes - MAXLENGTH;
			SessionAudio[SN].ProcessAudio[PN].isEmpty = 0;
		}
		else {
			SessionAudio[SN].ProcessAudio[PN].pLatest = (pLatest + numBytes) % MAXLENGTH;
			if(numBytes != 0)
				SessionAudio[SN].ProcessAudio[PN].isEmpty = 0;
		}
	}
	else {
		if (pLatest + numBytes > pOldest) {
			SessionAudio[SN].ProcessAudio[PN].pLatest = SessionAudio[SN].ProcessAudio[PN].pOldest = (pLatest + numBytes) % MAXLENGTH;
		}
		else
			SessionAudio[SN].ProcessAudio[PN].pLatest += numBytes;
	}
	ReleaseMutex(hMutex[SN][PN]);
}

int Sender::getDataSize(int SN, int PN) {
	int pLatest = SessionAudio[SN].ProcessAudio[PN].pLatest;
	int pOldest = SessionAudio[SN].ProcessAudio[PN].pOldest;
	int isEmpty = SessionAudio[SN].ProcessAudio[PN].isEmpty;

	if (pOldest < pLatest)
		return (pLatest - pOldest);
	else if (pOldest > pLatest)
		return (pLatest + MAXLENGTH - pOldest);
	else
		return isEmpty?0:MAXLENGTH;
}

/*void Sender::SendBuffer(int nBuffer) {
	fwrite(pBuffer[nBuffer], 1, packageSize, fwav);
	dataChunkSize += packageSize;

}

Sender::~Sender() {
}*/

int Sender::getBytesPerMilli() {
	return wvFmt.Format.nAvgBytesPerSec / 1000;
}

/*void Sender::InitializeSender() {
	frameSize = wvFmt.Format.nChannels * wvFmt.Format.wBitsPerSample / 8;
	dataChunkSize = 0;

	packageSize =wvFmt.Format.nAvgBytesPerSec * PACKAGE_TIME / 1000;
	pBuffer[0] = new BYTE[packageSize];
	pBuffer[1] = new BYTE[packageSize];
	

	fwrite("RIFF\0\0\0\0WAVEfmt ", 1, 16, fwav);
	int length;

	if (wvFmt.Format.wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
		length = 18 + wvFmt.Format.cbSize;
		fwrite(&length, sizeof(long), 1, fwav);
		fwrite(&wvFmt, length, 1, fwav);
	}
	else {
		length = 18;
		fwrite(&length, sizeof(long), 1, fwav);
		fwrite(&(wvFmt.Format), length, 1, fwav);
	}
	
	fwrite("data", sizeof(char), 4, fwav);
	dataChunkPosition = ftell(fwav);
	fwrite("\0\0\0\0", sizeof(char), 4, fwav);
}*/