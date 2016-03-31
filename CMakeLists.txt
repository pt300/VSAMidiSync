cmake_minimum_required(VERSION 3.3)
project(MIDI3)

set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
set(CMAKE_FIND_ROOT_PATH  "/usr/share/mingw-w64")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "-lwinmm -lole32 -loleaut32 -luuid -lcomdlg32 -lshlwapi -Wall") ##the fuck -mwindows
#i don't know how to cmake
set(SOURCE_FILES main.c)
add_executable(MIDI3.exe ${SOURCE_FILES})