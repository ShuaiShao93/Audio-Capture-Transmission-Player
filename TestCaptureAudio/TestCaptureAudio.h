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

#define FRAMELENGTH 8192   //faac读一次是2048 samples,每个sample 4B，所以这就是读一次最大的容量
#define SERVER_PORT "12345"
#define SERVER_ADD "localhost"

extern"C" int __declspec(dllexport) getAudio(BYTE*);