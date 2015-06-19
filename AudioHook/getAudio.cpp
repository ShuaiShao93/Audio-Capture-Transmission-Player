#include "AudioHook.h"


extern Sender *mySender;
extern HANDLE hMutex[MAXSESSION][MAXPROCESS];

extern AudioOfSession SessionAudio[MAXSESSION];
extern BOOL AvailSession[MAXSESSION];


int getAll(int SessionNum, int ProcessNum, BYTE* pAudio); 

extern"C" int __declspec(dllexport) delCurSession(){
	int curSession;
	unsigned long ProcessID, SessionID;

	ProcessID = GetCurrentProcessId();
	ProcessIdToSessionId(ProcessID, &SessionID);

	for(curSession = 0; curSession < MAXSESSION; curSession ++){      //search for this session
		if(AvailSession[curSession] == TRUE && SessionAudio[curSession].SessionID == SessionID){
			AvailSession[curSession] = FALSE;
			break;
		}
	}
	return 0;
}
extern"C" int __declspec(dllexport) getAudio(BYTE* pAudio)
{
	BYTE MainBuffer[MAXLENGTH] = {0};
	BYTE mixbuffer[MAXLENGTH] = {0};
	float nTemp;
	int numBytes = 0;
	int MaxBytes = 0;
	int curSession;
	unsigned long ProcessID, SessionID;
	int i;

	ProcessID = GetCurrentProcessId();
	ProcessIdToSessionId(ProcessID, &SessionID);

	for(curSession = 0; curSession < MAXSESSION; curSession ++){      //search for this session
		if(AvailSession[curSession] == TRUE && SessionAudio[curSession].SessionID == SessionID){
			break;
		}
	}

	for(i = 0; i < SessionAudio[curSession].NumOfProcess; i++){
		numBytes = 0;

		WaitForSingleObject(hMutex[curSession][i], INFINITE);
	    
		numBytes = getAll(curSession, i, mixbuffer);

		ReleaseMutex(hMutex[curSession][i]);
		
		for(int j = 0; j < numBytes; j += 4){
			nTemp = (*(float *)&MainBuffer[j]) + (*(float *)&mixbuffer[j]);     //mix the music
			*(float *)&MainBuffer[j] = nTemp;
		}
		if(numBytes > MaxBytes)
			MaxBytes = numBytes;
	}
	memcpy(pAudio, MainBuffer, MaxBytes);
	
	if(MaxBytes > 0)
	    printf("get audio: %d bytes\n", MaxBytes);

	return MaxBytes;
}

int getAll(int SessionNum, int ProcessNum, BYTE* pAudio) {
	int SN = SessionNum;
	int PN = ProcessNum;
	int pLatest = SessionAudio[SN].ProcessAudio[PN].pLatest;
	int pOldest = SessionAudio[SN].ProcessAudio[PN].pOldest;
	int isEmpty = SessionAudio[SN].ProcessAudio[PN].isEmpty;
	BYTE* audioBuffer = SessionAudio[SN].ProcessAudio[PN].AudioBuffer;
	int numBytes = 0;
	while(mySender->getDataSize(SessionNum, ProcessNum) >= 10 * mySender->getBytesPerMilli() && numBytes < MAXLENGTH)
	    numBytes += 10 * mySender->getBytesPerMilli();

	if (pLatest > pOldest || (pLatest == pOldest && isEmpty == 1)) {
		if (numBytes > pLatest - pOldest) 
			numBytes = pLatest - pOldest;
		memcpy(pAudio, audioBuffer + pOldest, numBytes);
		SessionAudio[SN].ProcessAudio[PN].pOldest += numBytes;
	}

	else {
		if (numBytes > pLatest + MAXLENGTH - pOldest) 
			numBytes = pLatest + MAXLENGTH - pOldest;
		if (pOldest + numBytes <= MAXLENGTH) {
			memcpy(pAudio, audioBuffer + pOldest, numBytes);
			SessionAudio[SN].ProcessAudio[PN].pOldest += numBytes;
		}
		else {
			memcpy(pAudio, audioBuffer + pOldest, MAXLENGTH - pOldest);
			memcpy(pAudio + MAXLENGTH - pOldest, audioBuffer, numBytes - (MAXLENGTH - pOldest));
			SessionAudio[SN].ProcessAudio[PN].pOldest = pOldest + numBytes - MAXLENGTH;
		}
	}

	if (SessionAudio[SN].ProcessAudio[PN].pOldest == SessionAudio[SN].ProcessAudio[PN].pLatest) SessionAudio[SN].ProcessAudio[PN].isEmpty = 1;

	return numBytes;
}