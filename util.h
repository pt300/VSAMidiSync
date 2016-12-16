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

#define STATE_PLAYING (TEXT(">"))
#define STATE_LOOPING (TEXT("o"))

enum columns {
	STATE,
	NAME,
	START_TEXT,
	STOP_TEXT,
	START,
	STOP,
	COL_LEN
};

enum statusbar_cells {
	HELLIFIKNEW,
	MIDI_CELL,
	LOOP_CELL,
	CELLS_NUMBER
};

HRESULT GetVSAPath(LPWSTR path);
HRESULT GetMIDIName(LPWSTR name);
HRESULT OpenDialog(HWND parent, LPWSTR path, LPCWSTR filter);
HRESULT SaveDialog(HWND parent, LPWSTR path, LPCWSTR filter, LPCWSTR ext);
HRESULT SetMIDIName(LPWSTR name);
void FillList(HWND list, subtracks_list *obj);
void SetStates(HWND list, INT playing, INT looping);
void LongIntoString(LONG number, LPWSTR str);
LONG LongFromString(LPWSTR str);
void SetVSARange(AX_object *ctrl, long start, long stop);

#endif //WINGUI_UTIL_H
