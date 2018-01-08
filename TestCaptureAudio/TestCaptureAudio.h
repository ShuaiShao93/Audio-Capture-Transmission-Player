#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#pragma comment(lib, "AudioHook2.lib")

#pragma comment(lib, "libfaac.lib")

typedef struct
{
  BYTE *in;
  int channels;
  int samplebytes;
  int samplerate;
  int bigendian;
  int isfloat;
} pcmfile_t;

#define FRAMELENGTH 8192   //faac reads 2048 samples every time, each sample is 4 Bytes, so 8192 Bytes are the max size for reading each time
#define SERVER_PORT "12345"
#define SERVER_ADD "localhost"

extern"C" int __declspec(dllexport) getAudio(BYTE*);