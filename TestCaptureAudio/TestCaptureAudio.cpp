#include "TestCaptureAudio.h"
#include <faac.h>
#include <faaccfg.h>

//#define TIME 10

#define MAXLENGTH 98304

BYTE get_buffer[MAXLENGTH];
BYTE read_buffer[FRAMELENGTH];
BYTE cir_buffer[MAXLENGTH];
float encoder_buffer[FRAMELENGTH];
BYTE buffer[MAXLENGTH];
faacEncHandle hEncoder;
unsigned long samplesInput, maxBytesOutput;
SOCKET ClientSocket = INVALID_SOCKET, ListenSocket = INVALID_SOCKET;
int InitEncoder();
int InitSocket();
pcmfile_t sndf;
int pLatest = 0, pOldest = 0, isEmpty = 1;
size_t wav_read_float32(pcmfile_t *sndf, float *buf, size_t num);
int getframe(BYTE* pAudio);
int fillbuffer(BYTE *pData, int numBytes);
int getDataSize();
LARGE_INTEGER starttime2,endtime2,frequency;

int main() {
	int numGet, iResult= 0;
	int samplesRead, bytesEncode = 0, numFromCir;
	int send_offset;
	//FILE * out = fopen("d:\\out.wav","wb");
	while (1) {
		
		InitEncoder();
		InitSocket();
		
		do {
			numGet = 0;
			numGet = getAudio(get_buffer);
			fillbuffer(get_buffer, numGet);
			send_offset = 0;
			while(getDataSize() >= 8192){
			    numFromCir = getframe(read_buffer);
			    samplesRead = wav_read_float32(&sndf, encoder_buffer, numFromCir/4);
				if(samplesRead > 0){
				//QueryPerformanceCounter(&starttime2);
					if((bytesEncode = faacEncEncode(hEncoder, (int32_t *)encoder_buffer, samplesRead, buffer + send_offset, maxBytesOutput)) < 0)
					    printf("faacEncEncode Failed!\n");  //numGet字节每10ms，每个Sample 32bit, 4字节
					send_offset += bytesEncode;
					//QueryPerformanceCounter(&endtime2);
				    //QueryPerformanceFrequency(&frequency);
				    //printf("TIME 2 :%f\n",(float)(endtime2.QuadPart - starttime2.QuadPart) /(float)frequency.QuadPart);
				}
			}
			    if(bytesEncode > 0){
					iResult = send(ClientSocket, (const char*)buffer, send_offset, 0);
					//if(iResult >0)
					   // printf("send: %d\n",iResult);
			}
				Sleep(80);
		} while (iResult >= 0);

		printf("recv failed: %d\n", WSAGetLastError());

		closesocket(ClientSocket);
		faacEncClose(hEncoder);
		WSACleanup();
	}

	return 0;
}

int InitEncoder(){
	faacEncConfigurationPtr myFormat;
	unsigned int sampleRate = 48000;  
	unsigned int nChannels = 2;  //采样率是48KHz(48k个samples每秒), 2声道
	unsigned int mpegVersion = MPEG2;
    unsigned int objectType = LOW;
    unsigned int useMidSide = 1;
	unsigned int useTns = 0;
	unsigned int useLfe = 0;
	unsigned int bitRate = 0;
	unsigned int bandWidth = 16000;
	unsigned int quantqual = 100;
	unsigned int outputFormat = 1; //ADTS
	unsigned int inputFormat = FAAC_INPUT_FLOAT;
	unsigned int shortctl = SHORTCTL_NORMAL;

	sndf.bigendian = 0;
	sndf.channels =2;
	sndf.samplebytes = 4;
	sndf.isfloat = 1;
	sndf.samplerate = 48000;
	sndf.in = read_buffer;
	
	if((hEncoder = faacEncOpen(sampleRate, nChannels, &samplesInput, &maxBytesOutput)) == NULL)
		printf("faacEncOpen Failed!");   

    if((myFormat = faacEncGetCurrentConfiguration(hEncoder)) == NULL)
		printf("faacEncGetCurrentConfiguration Failed!");
	myFormat->mpegVersion = mpegVersion;
	myFormat->aacObjectType = objectType;
	myFormat->allowMidside = useMidSide;
	myFormat->useTns = useTns;
	myFormat->useLfe = useLfe;
	myFormat->bitRate = bitRate;
	myFormat->bandWidth = bandWidth;
	myFormat->quantqual = quantqual;
	myFormat->outputFormat = outputFormat;
	myFormat->inputFormat = inputFormat;
	myFormat->shortctl = shortctl;

    if (!faacEncSetConfiguration(hEncoder, myFormat)) {
        printf("Unsupported output format!\n");
    }
	return 0;
}

int InitSocket() {
	int iResult;
	WSADATA wsaData;
	struct addrinfo *result = NULL, *ptr = NULL, hints;

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {                             
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }	

	ZeroMemory( &hints, sizeof(hints) );
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;
	
	iResult = getaddrinfo(NULL, SERVER_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	ptr=result;

	ListenSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	iResult = bind( ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	freeaddrinfo(result);

	if ( listen( ListenSocket, SOMAXCONN ) == SOCKET_ERROR ) {
		printf( "Listen failed with error: %ld\n", WSAGetLastError() );
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	printf("Listening.\n");

	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) {
		printf("accept failed: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	printf("Accepted.\n");
}

size_t wav_read_float32(pcmfile_t *sndf, float *buf, size_t num)
{
  size_t i = 0;
  long int offset = 0;
  unsigned char bufi[8];

  if ((sndf->samplebytes > 8) || (sndf->samplebytes < 1))
    return 0;

  while (i<num) {
	  memcpy(bufi, sndf->in + offset, sndf->samplebytes);
	  offset += sndf->samplebytes;

    if (sndf->isfloat){
        buf[i] = (*(float *)&bufi) * (float)32768;
    }
    i++;
  }
  return i;
}


int getframe(BYTE* pAudio) {
	//int numBytes = 10 * mySender->getBytesPerMilli();
	int numBytes = FRAMELENGTH;

	if (pLatest > pOldest || (pLatest == pOldest && isEmpty == 1)) {
		if (numBytes > pLatest - pOldest) 
			numBytes = pLatest - pOldest;
		memcpy(pAudio, cir_buffer + pOldest, numBytes);
		pOldest += numBytes;
	}

	else {
		if (numBytes > pLatest + MAXLENGTH - pOldest) 
			numBytes = pLatest + MAXLENGTH - pOldest;
		if (pOldest + numBytes <= MAXLENGTH) {
			memcpy(pAudio, cir_buffer + pOldest, numBytes);
			pOldest += numBytes;
		}
		else {
			memcpy(pAudio, cir_buffer + pOldest, MAXLENGTH - pOldest);
			memcpy(pAudio + MAXLENGTH - pOldest, cir_buffer, numBytes - (MAXLENGTH - pOldest));
			pOldest = pOldest + numBytes - MAXLENGTH;
		}
	}

	if (pOldest == pLatest) isEmpty = 1;

	return numBytes;
}

int fillbuffer(BYTE *pData, int numBytes) {

	if(numBytes > MAXLENGTH){
		numBytes = MAXLENGTH;
		pData += numBytes - MAXLENGTH;
	}
	if (pLatest + numBytes <= MAXLENGTH)
		memcpy(cir_buffer + pLatest, pData, numBytes);
	else {
		memcpy(cir_buffer + pLatest, pData, MAXLENGTH - pLatest);
		memcpy(cir_buffer, pData + (MAXLENGTH - pLatest), numBytes - (MAXLENGTH - pLatest));
	}

	if (pOldest < pLatest || (pOldest == pLatest && isEmpty == 1)) {
		if (pLatest + numBytes - MAXLENGTH > pOldest) {
			pLatest = pOldest = pLatest + numBytes - MAXLENGTH;
			isEmpty = 0;
		}
		else 
			pLatest = (pLatest + numBytes) % MAXLENGTH;
	}
	else {
		if (pLatest + numBytes > pOldest) {
			pLatest = pOldest = (pLatest + numBytes) % MAXLENGTH;
			isEmpty = 0;
		}
		else
			pLatest += numBytes;
	}
	return numBytes;
}

int getDataSize() {
	if (pOldest < pLatest)
		return (pLatest - pOldest);
	else if (pOldest > pLatest)
		return (pLatest + MAXLENGTH - pOldest);
	else
		return isEmpty?0:MAXLENGTH;
}