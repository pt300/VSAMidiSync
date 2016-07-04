//
// Created by patryk on 24.06.16.
//

#ifndef WINGUI_UTIL_H
#define WINGUI_UTIL_H

#include <windows.h>

#define VSAFILEFILTER TEXT("Dank memes(*.vsa)\0*.vsa\0")
#define VSAEXEFILTER  TEXT("The good shit(VSA.EXE)\0VSA.EXE\0")
#define VSAEXEREGKEY  TEXT("VSAPATH")
#define MIDIKEY	      TEXT("MIDIDEV")
#define REGROOT       TEXT("Software\\P4t\\MIDI")

HRESULT GetVSAPath(LPWSTR path);
HRESULT GetMIDIName(LPWSTR name);
HRESULT OpenDialog(HWND parent, LPWSTR path, LPCWSTR filter);
HRESULT SetMIDIName(LPWSTR name);

#endif //WINGUI_UTIL_H
