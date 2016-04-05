//
// Created by patryk on 25.03.16.
//

#ifndef MIDI3_MAIN_H
#define MIDI3_MAIN_H

#define UNICODE 1
#define _UNICODE 1

#include <stdio.h>
#include <windows.h>
#include <guiddef.h>
#include <mmsystem.h>
#include <conio.h>
#include "conio2.c"
#include <wchar.h>
#include <cguid.h>
#include <objbase.h>
#include <ocidl.h>
#include <shlwapi.h>
#include <winreg.h>
#include <process.h>

#ifdef DEBUG
#define STATUS(NAME, STAT) printf("%s: ", NAME); if(STAT == S_OK) printf("[SUCCESS]\n"); else{\
							printf("[FAIL] 0x%lX\n", STAT);\
							LPSTR buff;\
FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS, NULL, STAT, 0, (LPTSTR) &buff, 0, NULL);\
							printf("%s", buff);\
							exit(1);}
#endif
#ifndef DEBUG
#define STATUS(a, b) if(b != S_OK) {printf("\nError was encountered. Shutting down.\nPress any key..."); _getch(); _setcursortype(_NORMALCURSOR); exit(0);}
#endif

#define HELPSTR "To exit press [q]. Play/pause [spacebar]. Stop [s]. Reload VSA [r].\n" \
				"Change MIDI output [m]. Open other file [n].\n\n"

#define VSAFILEFILTER L"Pics of horse dicks(*.vsa)\0*.vsa\0"
#define VSAEXEFILTER L"THE SHIT(VSA.EXE)\0VSA.EXE\0"
#define VSAEXEREGKEY L"VSAPATH"

typedef union {
	DWORD word;
	unsigned char data[4];
} MIDIMsg;

/*
typedef struct IVSAs IVSA;

typedef struct
{
	// IUnknown methods
	HRESULT (*QueryInterface) (IVSA *, REFIID, void **);
	ULONG (*AddRef) (IVSA *);
	ULONG (*Release) (IVSA *);
	// IVSA methods
	void (*Destroy) (IVSA *);
	VARIANT_BOOL (*Create) (IVSA *);
	void (*SetStartMarker) (IVSA *, long);
	void (*SetStopMarker) (IVSA *, long);
	void (*Play) (IVSA *, short, VARIANT_BOOL);
	void (*Stop) (IVSA *);
	void (*Pause) (IVSA *);
	void (*SetComPort) (IVSA *, short, short, short);
	long (*GetPlaybackStatus) (IVSA *);
	void (*AboutBox) (IVSA *);
} IVSAVtbl;

struct IVSAs {
	IVSAVtbl *lpVtbl;
	BSTR vsaPath;
	BSTR routinePath;
	short showWindow;
};
*/


//THIS TOTALLY WILL NEVER FAIL!!!!!
//I could use GetIDsOfNames but fuck me
typedef enum {
	vsaPath = 1,
	routinePath,
	showWindow,
	Destroy,
	Create,
	SetStartMarker,
	SetStopMarker,
	Play,
	Stop,
	Pause,
	SetComPort,
	GetPlaybackStatus,
	AboutBox
} VSA_IDs;

typedef enum {
	Normal,
	Minimized,
	Hidden
} WSA_WINODW;

HRESULT GetVSAPath(PWSTR);
HRESULT MIDIOutputChooser(void);
void freeNames(char ***, unsigned int);
void fillNames(char ***, unsigned int);
HRESULT Reload(IDispatch *);
HRESULT TogglePlay(IDispatch *);
HRESULT CallStupidPlayMethodFuckingHell(IDispatch *);
HRESULT GetLongFromMethod(IDispatch *, VSA_IDs, volatile LONG *);
HRESULT GetBoolFromMethod(IDispatch *, VSA_IDs, BOOL *);
HRESULT GetNothingFromMethod(IDispatch *, VSA_IDs);
HRESULT SetStringProperty(IDispatch *, VSA_IDs, LPWSTR);
HRESULT SetShortProperty(IDispatch *, VSA_IDs, SHORT);
HRESULT PutValue(IDispatch *, VSA_IDs, VARIANT *);
HRESULT OpenDialog(LPWSTR, LPCWSTR);
HRESULT CallMethod(IDispatch *, VSA_IDs, VARIANT *);


#endif //MIDI3_MAIN_H
