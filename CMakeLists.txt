cmake_minimum_required(VERSION 2.8)
project(VSAMidiSync C)
set(CMAKE_SYSTEM_NAME Windows)

set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS) #fuck CMake and MinGW

ENABLE_LANGUAGE(RC)

add_definitions(-DUNICODE -D_UNICODE)

add_executable(${PROJECT_NAME}.exe WIN32 winmain callbacks resource.rc util MIDI subtracks threads)
add_library(AXCtrl AXCtrl)
#add_library(Util Util)
#add_library(MIDI MIDI)

target_link_libraries(AXCtrl ole32 oleaut32 uuid)
target_link_libraries(${PROJECT_NAME}.exe comctl32 gdi32 AXCtrl winmm comdlg32 shlwapi )