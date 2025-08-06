#include <Windows.h>
#include <format>
#include "jni.h"
#include "disabler.h"
#include "MinHook.h"

#define PALADIUM_CHECK_CLASS_FN 0x813D6C0
#define PALADIUM_CHECK_LIB_FN   0x80D87F0
#define PALADIUM_JVM_ADDR       0x8771630
#define JVM_DLL_IMAGE_BASE      0x8000000

typedef bool(*t_paladium_check_class_fn)(void* path);
t_paladium_check_class_fn orig_paladium_check_class_fn = nullptr;

bool __fastcall hk_check_class(void *path) {
    // Paladium's JVM blocks any jni calls that try to access to ehacks.* and net.minecraft.* (except if it's net.minecraft.launchwrapper.*).
    // We bypass this by hooking the function that check the class name and just always return false.
//    orig_paladium_check_class_fn(path);
    return false;
}

typedef HANDLE(__stdcall *t_create_thread)(LPSECURITY_ATTRIBUTES, SIZE_T, LPTHREAD_START_ROUTINE, LPVOID, DWORD, LPDWORD);
t_create_thread orig_create_thread = nullptr;

HANDLE __stdcall hk_create_thread(LPSECURITY_ATTRIBUTES lpThreadAttributes, SIZE_T dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId) {
    // Paladium's JVM only hooks NtCreateThread and CreateThread...
    return CreateRemoteThread(GetCurrentProcess(), lpThreadAttributes, dwStackSize, lpStartAddress, lpParameter, dwCreationFlags, lpThreadId);
}

typedef char(__fastcall *t_check_lib)(LPCCH);
t_check_lib orig_check_lib = nullptr;

char __fastcall hk_check_lib(LPCCH lib) {
    // Simple trusted dll check
    // If it's not inside some .paladium folders, not from System32/SysWOW64 and not signed, then it's not trusted.
    // We are simply hooking and making the func always return 1 (saying that every DLL is trusted, basically.)
//    orig_check_lib(lib);
    return 1;
}

typedef __int64(__fastcall *t_jni_get_created_java_vms)(void **vms_buffer, int buf_len, int *num_vms);
t_jni_get_created_java_vms orig_jni_get_created_java_vms = nullptr;

__int64 __fastcall hk_jni_get_created_java_vms(void **vms_buffer, int buf_len, int *num_vms) {
    // Paladium's JVM retrieves module that called JNI_GetCreatedJavaVMs and validates it, if it does not contain "Overflow", "gep_minecraft.dll" or "jre/bin/unpack.dll", then it flags.
    // We can bypass that by just writing our own JNI_GetCreatedJavaVMs logic that retrieve the actual vm in the memory.
    if (num_vms)
        *num_vms = 1;
    if (buf_len > 0) {
        const auto jvm = reinterpret_cast<uintptr_t>(GetModuleHandleA("jvm.dll"));
        const auto addr = jvm + PALADIUM_JVM_ADDR - JVM_DLL_IMAGE_BASE;
        vms_buffer[0] = reinterpret_cast<void*>(addr);
    }
    return (0);
}

typedef jstring(__fastcall *t_hwid_get)(JNIEnv*);
t_hwid_get orig_hwid_get = nullptr;

bool create_hooks(HMODULE jvm_handle, uintptr_t jvm_addr) {
    const auto kernel32 = GetModuleHandleA("kernel32.dll");

    const auto jni_get_created_java_vms_fn = GetProcAddress(jvm_handle, "JNI_GetCreatedJavaVMs");
    const auto create_thread_fn = GetProcAddress(kernel32, "CreateThread");
    const auto check_class_fn = jvm_addr + PALADIUM_CHECK_CLASS_FN - JVM_DLL_IMAGE_BASE;
    const auto check_lib_fn = jvm_addr + PALADIUM_CHECK_LIB_FN - JVM_DLL_IMAGE_BASE;
    int err;

    if ((err = MH_CreateHook(
            reinterpret_cast<void*>(check_class_fn),
            reinterpret_cast<LPVOID>(&hk_check_class),
            reinterpret_cast<void**>(&orig_paladium_check_class_fn))
             ) != MH_OK) {
        OutputDebugStringA(std::format("pala: could not create hook check class fn. minhook err: {}", err).c_str());
        return false;
    }

    if ((err = MH_CreateHook(
            reinterpret_cast<void*>(check_lib_fn),
            reinterpret_cast<LPVOID>(&hk_check_lib),
            reinterpret_cast<void**>(&orig_check_lib))
        ) != MH_OK) {
        OutputDebugStringA(std::format("pala: could not create check lib hook fn. minhook err: {}", err).c_str());
        return false;
    }

    if ((err = MH_CreateHook(
            reinterpret_cast<void*>(create_thread_fn),
            reinterpret_cast<LPVOID>(&hk_create_thread),
            reinterpret_cast<void**>(&orig_create_thread))
        ) != MH_OK) {
        OutputDebugStringA(std::format("pala: could not create create thread hook. minhook err: {}", err).c_str());
        return false;
    }

    if ((err = MH_CreateHook(
            reinterpret_cast<void*>(jni_get_created_java_vms_fn),
            reinterpret_cast<LPVOID>(&hk_jni_get_created_java_vms),
            reinterpret_cast<void**>(&orig_jni_get_created_java_vms))
        ) != MH_OK) {
        OutputDebugStringA(std::format("pala: could not create jni get created java vms hook. minhook err: {}", err).c_str());
        return false;
    }

    return true;
}

bool initialize_disabler() {
    if (MH_Initialize() != MH_OK) {
        OutputDebugStringA("pala: minhook initialization failed. exiting.");
        return false;
    }

    auto jvm_handle = GetModuleHandleA("jvm.dll");
    if (!jvm_handle) {
        OutputDebugStringA("pala: could not find jvm.dll. exiting.");
        MH_Uninitialize();
        return false;
    }

    if (!create_hooks(jvm_handle, reinterpret_cast<uintptr_t>(jvm_handle))) {
        MH_Uninitialize();
        return false;
    }

    int err;
    if ((err = MH_EnableHook(MH_ALL_HOOKS)) != MH_OK) {
        OutputDebugStringA(std::format("pala: could not enable hooks (err={}). exiting.", err).c_str());
        MH_Uninitialize();
        return false;
    }
    return true;
}