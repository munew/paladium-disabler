#include <Windows.h>
#include <libloaderapi.h>
#include <stdint.h>
#include "MinHook.h"
#include "offsets.h"
#include "hook.h"

typedef int(*t_jni_get_created_java_vms)(void **vms, int buffer_len, int *num_vms);
static t_jni_get_created_java_vms orig_jni_get_created_java_vms = NULL;
int hk_get_created_java_vms(void **vms, int buffer_len, int *num_vms) {
	// OutputDebugStringA("disabler: JNI_GetCreatedJavaVMs call");
	if (buffer_len > 0) {
		uintptr_t jvm_module = (uintptr_t) GetModuleHandleA("jvm.dll");
		uintptr_t jvm_addr = jvm_module + VA_JVM_ADDR - JVM_BASE_IMAGE;

		if (num_vms)
			*num_vms = 1;
		if (vms)
			*vms = (void*) jvm_addr;
	}
	// OutputDebugStringA("disabler: JNI_GetCreatedJavaVMs call end");
	return 0;
}

typedef void*(*t_windows_os_dll_load)(const char *name, char *ebuf, int ebuflen);
static t_windows_os_dll_load orig_windows_os_dll_load = NULL;
void* hk_windows_os_dll_load(const char *name, char *ebuf, int ebuflen) {
	// OutputDebugStringA("disabler: os::dll_load call");
	if (name) {
		return orig_windows_os_dll_load(name, ebuf, ebuflen);
	}
	return NULL;
}

typedef bool(*t_class_check)(void* jstr);
static t_class_check orig_class_check = NULL;
bool hk_class_check(void* jstr) {
	(void)jstr;
	return false;
}

bool initialize_hooks() {
	if (MH_Initialize() != MH_OK) {
		OutputDebugStringA("disabler: Failed to initialize MinHook");
		return false;
	}

	HMODULE jvm_module = GetModuleHandleA("jvm.dll");
	if (!jvm_module) {
		OutputDebugStringA("disabler: Failed to get jvm.dll handle");
		MH_Uninitialize();
		return false;
	}

	uintptr_t jvm_addr = (uintptr_t) jvm_module;
	void* jni_get_created_java_vms = (void*)GetProcAddress(jvm_module, "JNI_GetCreatedJavaVMs");
	void* windows_os_dll_load = (void*) (jvm_addr + VA_JVM_WINDOWS_OS_DLL_LOAD - JVM_BASE_IMAGE);
	void* class_check = (void*) (jvm_addr + VA_JVM_CLASS_CHECK - JVM_BASE_IMAGE);

	if (MH_CreateHook(jni_get_created_java_vms, (LPVOID) hk_get_created_java_vms, (void**) &orig_jni_get_created_java_vms) != MH_OK) {
		OutputDebugStringA("disabler: Failed to create hook for JNI_GetCreatedJavaVMs");
		MH_Uninitialize();
		return false;
	}

	if (MH_CreateHook(windows_os_dll_load, (LPVOID) hk_windows_os_dll_load, (void**) &orig_windows_os_dll_load) != MH_OK) {
		OutputDebugStringA("disabler: Failed to create hook for Windows OS DLL load");
		MH_Uninitialize();
		return false;
	}

	if (MH_CreateHook(class_check, (LPVOID) hk_class_check, (void**) &orig_class_check) != MH_OK) {
		OutputDebugStringA("disabler: Failed to create hook for class check");
		MH_Uninitialize();
		return false;
	}

	if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK) {
		OutputDebugStringA("disabler: Failed to enable hooks");
		MH_Uninitialize();
		return false;
	}

	return true;
}

void uninitialize_hooks() {
	MH_DisableHook(MH_ALL_HOOKS);
	MH_Uninitialize();
}
