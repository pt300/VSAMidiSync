cmake_minimum_required(VERSION 3.3)
project(VSAMidiSync C)
set(CMAKE_SYSTEM_NAME Windows)

set(CMAKE_C_COMPILER i686-w64-mingw32-gcc)
set(CMAKE_FIND_ROOT_PATH "/usr/share/mingw-w64")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS)
set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--subsystem,windows -Wall -Wunused -Werror")

ENABLE_LANGUAGE(RC)

set(CMAKE_RC_COMPILER i686-w64-mingw32-windres)

add_definitions(-DUNICODE -D_UNICODE)

add_executable(${PROJECT_NAME}.exe winmain callbacks resource.rc)
add_library(AXCtrl AXCtrl)
add_library(Util Util)
add_library(MIDI MIDI)

target_link_libraries(AXCtrl ole32 oleaut32 uuid)
target_link_libraries(Util comdlg32 shlwapi)
target_link_libraries(MIDI winmm)
target_link_libraries(${PROJECT_NAME}.exe comctl32 gdi32 AXCtrl Util MIDI winmm)