cmake_minimum_required(VERSION 3.28)
project(paladium_disabler)

set(CMAKE_CXX_STANDARD 23)

add_library(paladium_disabler SHARED srcs/library.cpp
        srcs/disabler.cpp)

target_include_directories(paladium_disabler PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/minhook/include)
target_include_directories(paladium_disabler PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/jni)
target_include_directories(paladium_disabler PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/includes)

target_link_libraries(paladium_disabler
        ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/minhook/lib/libMinHook-x64-v140-md.lib)