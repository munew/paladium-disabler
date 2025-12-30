#include "hook.h"
#include <Windows.h>
#include <winnt.h>

static bool initialized = false;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
  (void)hinstDLL;
  (void)lpvReserved;
  switch (fdwReason) {
  case DLL_PROCESS_ATTACH:
    initialized = initialize_hooks();
    if (!initialized)
      exit(1); // we currently exit process if we can't init
    break;
  case DLL_PROCESS_DETACH:
    if (initialized)
      uninitialize_hooks();
    break;
  }
  return TRUE;
}
