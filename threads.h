//
// Created by patryk on 19.08.16.
//

#ifndef VSAMIDISYNC_THREADS_H
#define VSAMIDISYNC_THREADS_H

#include <windows.h>
#include "AXCtrl.h"

typedef struct {
	void *data;
	volatile BOOL shouldRun;
	volatile BOOL isRunning;
} thread_data;

typedef struct {
	AX_object *ctrl;
	HWND timeText, window;
	volatile long frame;
	volatile BOOL playing;
} time_thread_data;

typedef struct {
	volatile long *frame;
	volatile BOOL *playing;
	HWND status;
	UINT device;
} midi_thread_data;

typedef union {
	DWORD word;
	UCHAR data[4];
} MIDIMsg;

typedef struct {
	AX_object *ctrl;
	volatile long *frame;
	volatile long start;
	volatile long stop;
	volatile BOOL loop;
	volatile char idle;
	volatile long idleStart;
	volatile long idleStop;
} loop_thread_data;


DWORD WINAPI TimeThread(LPVOID data);
DWORD WINAPI MidiThread(LPVOID data);
DWORD WINAPI LoopThread(LPVOID data);

#endif //VSAMIDISYNC_THREADS_H
