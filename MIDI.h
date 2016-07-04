//
// Created by patryk on 28.06.16.
//

#ifndef VSAMIDISYNCH_MIDI_H
#define VSAMIDISYNCH_MIDI_H

#include <windows.h>

size_t MIDI_listDevices(HWND list);
INT32 MIDI_getDeviceByName(LPWSTR name);

#endif //VSAMIDISYNCH_MIDI_H
