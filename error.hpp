#pragma once

#ifdef __linux__
#elif _WIN32
        #include <windows.h>
        #include <psapi.h>
        #include <tlhelp32.h>
#else

#endif

#include <stdexcept>
#include <iostream>

char *get_api_err_msg();

#ifdef DEBUG
#include <time.h>
#include <iomanip>

#define INFO(x)                                                                 \
{                                                                               \
        float t = ((float)clock()) / (CLOCKS_PER_SEC);                          \
        std::cout << "[i]" << "[" << __FILE__ << "]["                          \
        << __FUNCTION__ << "][" << __LINE__ << "] :" << std::fixed              \
        << std::setprecision(2) << t << " " << x << std::endl;                  \
}                                                                               \
do {} while (0)

#define ERR(x)                                                                  \
{                                                                               \
        float t = ((float)clock()) / (CLOCKS_PER_SEC);                          \
        std::cerr << "[-]" << "[" << __FILE__ << "]["                          \
        << __FUNCTION__ << "][" << __LINE__ << "] :" << std::fixed              \
        << std::setprecision(2) << t << " " << x << std::endl;                  \
}                                                                               \
do {} while (0)

#define ERR2(x)                                                                 \
{                                                                               \
        char *msg = get_api_err_msg();                                          \
        float t = ((float)clock()) / (CLOCKS_PER_SEC);                          \
        std::cerr << "[-]" << "[" << __FILE__ << "]["                          \
        << __FUNCTION__ << "][" << __LINE__ << "] :" << std::fixed              \
        << std::setprecision(2) << t << " " << x << " : " << msg                \
        << std::endl;                                                           \
        delete[] msg;                                                           \
}                                                                               \
do {} while (0)

#define SUCC(x)                                                                 \
{                                                                               \
        float t = ((float)clock()) / (CLOCKS_PER_SEC);                          \
        std::cout << "[+]" << "[" << __FILE__ << "]["                          \
        << __FUNCTION__ << "][" << __LINE__ << "] :" << std::fixed              \
        << std::setprecision(2) << t << " " << x << std::endl;                  \
}                                                                               \
do {} while (0)

#else   // not debug

#define INFO(x) do {} while (0)
#define ERR(x) do {} while (0)
#define ERR2(x) do {} while (0)
#define SUCC(x) do {} while (0)
        
#endif
