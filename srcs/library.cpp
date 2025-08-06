#include <windows.h>
#include "disabler.h"

BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,  // handle to DLL module
    DWORD fdwReason,     // reason for calling function
    LPVOID lpvReserved )  // reserved
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
        if (!initialize_disabler())
            FreeLibraryAndExitThread(hinstDLL, 0);
    }
    return TRUE;
}