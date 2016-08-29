/*
 * Created by patryk on 28.06.16.
 */

#ifndef VSAMIDISYNCH_MIDI_H
#define VSAMIDISYNCH_MIDI_H

#include <windows.h>

#define MIDI_TOP	3347999
#define MIDI_BOTTOM	0

UINT MIDI_listDevices(HWND list);
INT32 MIDI_getDeviceByName(LPWSTR name);

#endif
