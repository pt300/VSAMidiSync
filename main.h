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

#define STATUS(NAME, STAT) printf("%s: ", NAME); if(STAT == S_OK) printf("[SUCCESS]\n"); else{\
							printf("[FAIL] 0x%lX\n", STAT);\
							LPSTR buff;\
FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS, NULL, STAT, 0, (LPTSTR) &buff, 0, NULL);\
							printf("%s", buff);\
							exit(1);}


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

HRESULT GetLongProperty(IDispatch*, VSA_IDs, LONG *);
HRESULT SetStringProperty(IDispatch*, VSA_IDs, wchar_t *);

#endif //MIDI3_MAIN_H
