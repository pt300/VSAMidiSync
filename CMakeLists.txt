cmake_minimum_required(VERSION 2.8)
set(CMAKE_SYSTEM_NAME Windows)
project(VSAMidiSync C)

enable_language(RC)

add_definitions(-DUNICODE -D_UNICODE)

add_executable(${PROJECT_NAME} WIN32 winmain callbacks resource.rc util MIDI subtracks threads)
add_library(AXCtrl AXCtrl)
#add_library(Util Util)
#add_library(MIDI MIDI)

target_link_libraries(AXCtrl ole32 oleaut32 uuid)
target_link_libraries(${PROJECT_NAME} comctl32 gdi32 AXCtrl winmm comdlg32 shlwapi)