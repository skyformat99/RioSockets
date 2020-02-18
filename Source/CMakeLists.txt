cmake_minimum_required(VERSION 2.6)
project(riosockets C)

set(RIOSOCKETS_STATIC "0" CACHE BOOL "Create a static library")
set(RIOSOCKETS_SHARED "0" CACHE BOOL "Create a shared library")

if (MSYS OR MINGW)
    set(CMAKE_C_FLAGS "-static") 

    add_definitions(-DWINVER=0x0602)
    add_definitions(-D_WIN32_WINNT=0x0602)
endif()

if (RIOSOCKETS_STATIC)
    add_library(riosockets_static STATIC riosockets.c ${SOURCES})

    if (NOT UNIX)
        target_link_libraries(riosockets_static ws2_32)
        SET_TARGET_PROPERTIES(riosockets_static PROPERTIES PREFIX "")
    endif()
endif()

if (RIOSOCKETS_SHARED)
    add_definitions(-DRIOSOCKETS_DLL)
    add_library(riosockets SHARED riosockets.c ${SOURCES})

    if (NOT UNIX)
        target_link_libraries(riosockets ws2_32)
        SET_TARGET_PROPERTIES(riosockets PROPERTIES PREFIX "")
    endif()
endif()