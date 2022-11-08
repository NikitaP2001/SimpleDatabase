#include <iostream>

#ifdef _WIN32
#include <w32api.h>
#else
#error Only win32 supported for server
#endif // _WIN32

int main()
{
        std::cout << "hello from server\n";
}