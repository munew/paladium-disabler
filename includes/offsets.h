#ifndef OFFSETS_H
#define OFFSETS_H

#define JVM_BASE_IMAGE 0x8000000
#define VA_JVM_ADDR 0x876E630

// https://github.com/openjdk/jdk/blob/92c6799b401eb786949e88cd7142002b2a875ce0/src/hotspot/os/windows/os_windows.cpp#L1716
#define VA_JVM_WINDOWS_OS_DLL_LOAD 0x826C690

#define VA_JVM_CLASS_CHECK 0x813C790

#endif
