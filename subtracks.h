/*
 * Created by patryk on 13.08.16.
 */

#ifndef VSAMIDISYNC_SUBTRACKS_H
#define VSAMIDISYNC_SUBTRACKS_H

#include <windows.h>

#define S_NAMELEN 21

typedef struct {
	WCHAR name[S_NAMELEN + 1];
	LONG start;
	LONG stop;
} subtrack;

typedef struct {
	CHAR length;
	UCHAR alloc;
	subtrack **tracks;
} subtracks_list;

void InitSubtracksList(subtracks_list *obj);
BOOL VSBExists(LPWSTR path);
BOOL LoadVSBFile(subtracks_list *obj, LPWSTR path);
BOOL SaveVSBFile(subtracks_list *obj, LPWSTR path);
void MakeChanges(void);
void AintNoChanges(void);
BOOLEAN AreThereAnyFuckingChanges(void);
BOOL AddSubtrack(subtracks_list *obj, LONG start, LONG stop, LPWSTR name);
BOOL RemoveSubtrack(subtracks_list *obj, UCHAR id);
BOOL SwapSubtracks(subtracks_list *obj, UCHAR from, UCHAR to);
void DestroySubtracksList(subtracks_list *obj);

#endif //VSAMIDISYNC_SUBTRACKS_H
