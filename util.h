/*
 * Created by patryk on 24.06.16.
 */

#ifndef WINGUI_UTIL_H
#define WINGUI_UTIL_H

#include <windows.h>
#include "subtracks.h"
#include "AXCtrl.h"

#define VSAFILEFILTER	TEXT("Dank memes(*.vsa)\0*.vsa\0")
#define VSAEXEFILTER	TEXT("The good shit(VSA.EXE)\0VSA.EXE\0")
#define VSBFILTER		TEXT("Subtracks(*.vsb)\0*.vsb\0")
#define VSAEXEREGKEY	TEXT("VSAPATH")
#define MIDIKEY			TEXT("MIDIDEV")
#define REGROOT			TEXT("Software\\P4t\\MIDI")

enum columns {
	NAME,
	START_TEXT,
	STOP_TEXT,
	START,
	STOP,
	COL_LEN
};

HRESULT GetVSAPath(LPWSTR path);
HRESULT GetMIDIName(LPWSTR name);
HRESULT OpenDialog(HWND parent, LPWSTR path, LPCWSTR filter);
HRESULT SaveDialog(HWND parent, LPWSTR path, LPCWSTR filter, LPCWSTR ext);
HRESULT SetMIDIName(LPWSTR name);
void FillList(HWND list, subtracks_list *obj);
void LongIntoString(LONG number, LPWSTR str);
LONG LongFromString(LPWSTR str);
void SetVSARange(AX_object *ctrl, long start, long stop);

#endif //WINGUI_UTIL_H
