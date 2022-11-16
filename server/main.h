#pragma once

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#else
#error Only win32 supported for server
#endif // _WIN32

#include <error.hpp>