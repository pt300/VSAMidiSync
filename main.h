//
// Created by patryk on 25.03.16.
//

#ifndef MIDI3_MAIN_H
#define MIDI3_MAIN_H

#include <stdio.h>
#include <windows.h>
#include <guiddef.h>
#include <mmsystem.h>
#include <conio.h>
#include <wchar.h>
#include <cguid.h>
#include <objbase.h>

#define VSA_PATH L"C:\\Program Files\\Brookshire Software\\Visual Show Automation Hobbyist\\VSA.EXE"


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

HRESULT GetLongProperty(IDispatch*, DISPID, LONG *);
HRESULT SetStringProperty(IDispatch*, DISPID, wchar_t *);

#endif //MIDI3_MAIN_H
