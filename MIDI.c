/*
 * Created by patryk on 28.06.16.
 */

#include "MIDI.h"

UINT MIDI_listDevices(HWND list) {
	UINT count, i, lcnt;
	MIDIOUTCAPS device;
	MMRESULT res;

	count = midiOutGetNumDevs();
	if(!count) {
		SendMessage(list, LB_ADDSTRING, 0, (LPARAM) TEXT("No MIDI devices found."));
	}
	else {
		for(i = lcnt = 0; i < count; i++) {
			res = midiOutGetDevCaps(i, &device, sizeof device);
			if(res == MMSYSERR_NOERROR) {
				SendMessageW(list, LB_ADDSTRING, 0, (LPARAM) device.szPname);
				SendMessageW(list, LB_SETITEMDATA, lcnt++, (LPARAM) i);
			}
		}
	}

	return count;
}

INT32 MIDI_getDeviceByName(LPWSTR name) {
	UINT count, i;
	MIDIOUTCAPS device;
	MMRESULT res;

	count = midiOutGetNumDevs();
	if(!count) {
		return -1;
	}
	else {
		for(i = 0; i < count; i++) {
			res = midiOutGetDevCaps(i, &device, sizeof device);
			if(res == MMSYSERR_NOERROR && !lstrcmp(name, device.szPname)) {
				return i;
			}
		}
	}

	return -1;
}