#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdio.h>
#include "LibAudioPlayer.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <faad.h>
#include <neaacdec.h>

#pragma comment(lib, "ws2_32.lib")

#pragma comment(lib, "LibAudioPlayer.lib")

#pragma comment(lib, "libfaad.lib")

#define SERVER_PORT "12345"
#define SERVER_ADD "localhost"
