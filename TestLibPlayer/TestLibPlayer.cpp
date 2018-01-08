#include "TestLibPlayer.h"
#include<Winbase.h>
#include <strsafe.h>
#define FRAMELENGTH 8192
#define MAXLENGTH 98304
//#define TIME 10

BYTE recv_buffer[MAXLENGTH];
BYTE decoder_buffer[FRAMELENGTH];
void *buffer;
SOCKET ConnectSocket;
NeAACDecHandle hAac;
NeAACDecFrameInfo hInfo;
int InitSocket();
int InitDecoder();
int get_one_ADTS_frame(unsigned char* buffer, size_t buf_size, unsigned char* data ,size_t* data_size);

void main() {
	AudioPlayer *ap = new AudioPlayer;
	int numGet;
	size_t size;
	unsigned char *decoder_offset;
	unsigned long samplerate;
    unsigned char channels;

	InitDecoder();
	InitSocket();

	numGet = recv(ConnectSocket, (char*)recv_buffer, (6144/8)*2*5, 0);
	printf("send: %d\n", numGet);

	if(get_one_ADTS_frame(recv_buffer, numGet, decoder_buffer, &size) == 0)
		if(NeAACDecInit(hAac, (unsigned char*)decoder_buffer, size, &samplerate, &channels) < 0){
		    printf("Error initializing decoder library.\n");
	}

	ap->Start();

	while (1) {
		numGet = recv(ConnectSocket, (char*)recv_buffer, (6144/8)*2*10, 0);  //maxBytesOutput
        // Loop until decoding finished
        if(numGet > 0){
			printf("send: %d\n", numGet);
			decoder_offset = recv_buffer;
		   while(get_one_ADTS_frame(decoder_offset, numGet, decoder_buffer, &size) == 0){
		       buffer = NeAACDecDecode(hAac, &hInfo, (unsigned char*)decoder_buffer, size);
		       if ((hInfo.error == 0) && (hInfo.samples > 0)){
			      ap->LoadData(hInfo.samples * 4, (BYTE *)buffer);   //4B Ò»¸ösample
			      printf("LoadData: %d\n", hInfo.samples * 4);
                } else if (hInfo.error != 0) {
			        printf("NeAACDecDecode Failed!\n");
			        printf("Error: %s\n", NeAACDecGetErrorMessage(hInfo.error));
			        NeAACDecClose(hAac);
                }
			   numGet -= size;
			   decoder_offset += size;
		    }
		}
    }
NeAACDecClose(hAac);
}

int InitSocket() {
	WSADATA wsaData;
    int iResult;
	struct addrinfo *result = NULL, *ptr = NULL, hints;
	ConnectSocket = INVALID_SOCKET;

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {                             
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }	

	ZeroMemory( &hints, sizeof(hints) );
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	
	iResult = getaddrinfo(SERVER_ADD, SERVER_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	ptr=result;

	ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
	if (ConnectSocket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	// Connect to server.
	iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("Connect failed: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		ConnectSocket = INVALID_SOCKET;
		WSACleanup();
		return 1;
	}

	freeaddrinfo(result);
}

int InitDecoder(){
	NeAACDecConfigurationPtr conf;

	unsigned int sampleRate = 48000;  
	unsigned int nChannels = 2;  

    // Open the library
    hAac = NeAACDecOpen();

    // Get the current config
    conf = NeAACDecGetCurrentConfiguration(hAac);
	conf->defObjectType = LC;
	conf->defSampleRate = sampleRate;
	conf->outputFormat = FAAD_FMT_FLOAT;
	//conf->downMatrix = 1;
	//conf->useOldADTSFormat = 0;

    // Set the new configuration
    if(NeAACDecSetConfiguration(hAac, conf)  == 0)
		printf("NeAACDecSetConfiguration Failed!");
    return 0;   
}

int get_one_ADTS_frame(unsigned char* buffer, size_t buf_size, unsigned char* data ,size_t* data_size)
{
    size_t size = 0;

    if(!buffer || !data || !data_size )
    {
        return -1;
    }

    while(1)
    {
        if(buf_size  < 7 )
        {
            return -1;
        }

        if((buffer[0] == 0xff) && ((buffer[1] & 0xf0) == 0xf0) )
        {
            size |= ((buffer[3] & 0x03) <<11);     //high 2 bit
            size |= buffer[4]<<3;                //middle 8 bit
            size |= ((buffer[5] & 0xe0)>>5);        //low 3bit
            break;
        }
        --buf_size;
        ++buffer;
    }

    if(buf_size < size)
    {
        return -1;
    }

    memcpy(data, buffer, size);
    *data_size = size;
    
    return 0;
}
