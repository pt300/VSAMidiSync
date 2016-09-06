//
// Created by patryk on 13.08.16.
//

#include "subtracks.h"
#include "MIDI.h"
#include <stdio.h>
#include <shlwapi.h>

BOOLEAN changes = FALSE;

static long GetNum(FILE *stream) {
	wchar_t ch;
	long out;

	for(out = 0, ch = fgetwc(stream); ch >= '0' && ch <= '9' && out < 3348000; ch = fgetwc(stream)) {
		out *= 10;
		out += ch - '0';
	}

	if(ch == '\r') {
		ch = fgetwc(stream);
	}

	if((ch == ' ' || ch == '\n') && out < 3348000) {
		return out;
	}

	return -1;
}

BOOL GetStr(FILE *stream, WCHAR str[]) {
	INT len;
	memset(str, '\0', (S_NAMELEN + 1) * sizeof *str);

	fgetws(str, S_NAMELEN, stream);

	len = lstrlen(str);

	if(len > 2 && str[len - 1] == '\n' && str[len - 2] == '\r') {
		str[lstrlen(str) - 2] = '\0';
		len -= 2;
	}

	if(len == 0) {
		return FALSE;
	}

	return TRUE;
}

void InitSubtracksList(subtracks_list *obj) {
	obj->length = 0;
	obj->alloc = 5;
	obj->tracks = malloc(5 * sizeof *obj->tracks);
}

BOOL VSBExists(LPWSTR path) {
	WCHAR mpath[MAX_PATH];

	lstrcpy(mpath, path);
	mpath[lstrlen(mpath) - 1] = 'b';

	return PathFileExists(mpath);
}

/*
 * TODO: error reporting maybe?
 */
BOOL LoadVSBFile(subtracks_list *obj, LPWSTR path) {
	WCHAR mpath[MAX_PATH], magic[7];
	subtrack *track;
	FILE *file;
	UINT alloc;

	if(obj->length >= 0) {
		DestroySubtracksList(obj);
	}

	lstrcpy(mpath, path);
	mpath[lstrlen(mpath) - 1] = 'b';

	if(!PathFileExists(mpath) ||
	   ((file = _wfopen(mpath, TEXT("rb"))) == NULL)) {
		return FALSE;
	}

	if((fgetws(magic, 7, file) == NULL) ||
	   (StrCmp(TEXT("\xfeffVSB\r\n"), magic) != 0)) {
		fclose(file);
		return FALSE;
	}

	obj->length = 0;
	obj->tracks = malloc(5 * sizeof track);
	alloc = 5;
	obj->alloc = 5;

	while(obj->length < 125) {
		track = malloc(sizeof *track);
		if(((track->start = GetNum(file)) == -1) ||
		   ((track->stop = GetNum(file)) == -1) ||
		   (GetStr(file, track->name) == FALSE)) {
			fclose(file);
			free(track);
			return TRUE;
		}
		if(track->start > track->stop) { //bitch
			free(track);
		}
		else {
			obj->tracks[(BYTE) obj->length++] = track;
			if(!--alloc) {
				if(obj->alloc == 125) {
					break;
				}
				obj->tracks = realloc(obj->tracks, (obj->alloc += 5) * sizeof track);
				alloc = 5;
			}
		}
	}
	return TRUE;
}

BOOL SaveVSBFile(subtracks_list *obj, LPWSTR path) {
	FILE *file;
	WCHAR mpath[MAX_PATH];
	UCHAR cnt;

	lstrcpy(mpath, path);
	mpath[lstrlen(mpath) - 1] = 'b';

	if(obj->length < 0) {
		return FALSE;
	}

	if(((file = _wfopen(mpath, TEXT("wb"))) == NULL)) {
		return FALSE;
	}
	if(fputws(L"\xfeffVSB\r\n", file) == EOF) {
		fclose(file);
		return FALSE;
	}

	for(cnt = 0; cnt < obj->length; cnt++) {
		if(fwprintf(file, L"%li %li\r\n%1.21ls\r\n", obj->tracks[cnt]->start, obj->tracks[cnt]->stop,
				obj->tracks[cnt]->name) < 0) {
			return FALSE;
		}
	}

	fclose(file);

	return TRUE;
}

void MakeChanges(void) {
	changes = TRUE;
}

BOOLEAN AreThereAnyFuckingChanges(void) {
	return changes;
}

void AintNoChanges(void) {
	changes = FALSE;
}

BOOL AddSubtrack(subtracks_list *obj, LONG start, LONG stop, LPWSTR name) {
	subtrack *track;

	if(obj->length < 0) {
		return FALSE;
	}

	if(obj->length == 125 ||
	   start < MIDI_BOTTOM || start > MIDI_TOP ||
	   stop < MIDI_BOTTOM || stop > MIDI_TOP ||
	   name == NULL || lstrlen(name) > S_NAMELEN) {
		return FALSE;
	}

	if(obj->length == obj->alloc) {
		obj->tracks = realloc(obj->tracks, (obj->alloc += 5) * sizeof *obj->tracks);
	}

	track = malloc(sizeof *track);
	track->start = start;
	track->stop = stop;
	lstrcpyn(track->name, name, S_NAMELEN + 1);
	track->name[S_NAMELEN] = '\0';

	obj->tracks[(BYTE) obj->length++] = track;

	return TRUE;
}

BOOL RemoveSubtrack(subtracks_list *obj, UCHAR id) {
	if(obj->length < 0) {
		return FALSE;
	}

	if(obj->length == 0 ||
	   id > obj->length - 1) {
		return FALSE;
	}

	free(obj->tracks[id]);

	for(obj->length--; id < obj->length; id++) {
		obj->tracks[id] = obj->tracks[id + 1];
	}

	return TRUE;
}

BOOL SwapSubtracks(subtracks_list *obj, UCHAR from, UCHAR to) {
	subtrack *tmp;

	if(from >= obj->length || to >= obj->length ||
	   from == to) {
		return FALSE;
	}

	tmp = obj->tracks[from];

	if(from > to) {
		for(; from > to; from--) {
			obj->tracks[from] = obj->tracks[from - 1];
		}
	}
	else {
		for(; from < to; from++) {
			obj->tracks[from] = obj->tracks[from + 1];
		}
	}

	obj->tracks[to] = tmp;

	return TRUE;
}

void DestroySubtracksList(subtracks_list *obj) {
	if(obj->length < 0) {
		return;
	}

	while(obj->length--) {
		free(obj->tracks[(BYTE) obj->length]);
	}
	free(obj->tracks);
	obj->alloc = 0;
}
