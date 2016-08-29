/*
 * Created by patryk on 19.08.16.
 */

#include <windows.h>
#include <commctrl.h>
#include "threads.h"

DWORD WINAPI TimeThread(LPVOID data) {
	thread_data *thread = data;
	time_thread_data *vars = thread->data;
	LONG time;
	long cf, sec, min, hour;
	WCHAR text[12];
	RECT rect;
	BOOL state = FALSE;

	thread->isRunning = TRUE;

	timeBeginPeriod(33);

	GetClientRect(vars->timeText, &rect);
	InvalidateRect(vars->timeText, &rect, TRUE);
	MapWindowPoints(vars->timeText, vars->window, (POINT *) &rect, 2);
	while(thread->shouldRun) {
		AX_callMethod(vars->ctrl, TEXT("GetPlaybackStatus"), VT_I4, &time, NULL);
		if(time <= 0) {
			time = 0;
		}

		if(time == vars->frame) {
			if(state && vars->playing) {
				vars->playing = FALSE;
			}
			else {
				state = TRUE;
			}
		}
		else {
			state = FALSE;
			vars->frame = time;
			vars->playing = TRUE;

			cf = time % 30;
			sec = time / 30 % 60;
			min = time / (30 * 60) % 60;
			hour = time / (30 * 60 * 60);
			wsprintf(text, L"%.2li:%.2li:%.2li.%.2li", hour, min, sec, cf);

			SetWindowText(vars->timeText, text);
			RedrawWindow(vars->window, &rect, NULL, RDW_ERASE | RDW_INVALIDATE);
		}

		Sleep(33);
	}

	timeEndPeriod(33);

	thread->isRunning = FALSE;

	return 0;
}

DWORD WINAPI MidiThread(LPVOID data) {
	thread_data *thread = data;
	midi_thread_data *vars = thread->data;
	MMRESULT res;
	HMIDIOUT device;
	MIDIMsg msg;
	UCHAR time[4], cnt;

	res = midiOutOpen(&device, vars->device, 0, 0, CALLBACK_NULL);
	if(res != MMSYSERR_NOERROR) {
		return 0;
	}

	thread->isRunning = TRUE;

	SendMessage(vars->status, SB_SETTEXT, 0, (LPARAM) TEXT("MIDI: Connected"));    //creative

	timeBeginPeriod(33);
	/*
	 * TODO: think about preserving calculated time values. We use them in 2 places after all.
	 */
	while(thread->shouldRun) {
		Sleep(33);
		if(!*vars->playing)
			continue;

		time[0] = (UCHAR) (*vars->frame % 30);
		time[1] = (UCHAR) (*vars->frame / 30 % 60);
		time[2] = (UCHAR) (*vars->frame / (30 * 60) % 60);
		time[3] = (UCHAR) (*vars->frame / (30 * 60 * 60) % 31);

		for(cnt = 0; cnt < 8; cnt++) {
			msg.data[0] = 0xF1;
			if(cnt != 7)
				msg.data[1] = (UCHAR) (cnt << 4 | (0x0F & (time[cnt / 2] >> (4 * (cnt & 1)))));
			else
				msg.data[1] = (UCHAR) (cnt << 4 | (1 & time[3] >> 4) | 6);
			midiOutShortMsg(device, msg.word);
		}
	}

	timeEndPeriod(33);
	midiOutClose(device);

	thread->isRunning = FALSE;

	SendMessage(vars->status, SB_SETTEXT, 0, (LPARAM) TEXT("MIDI: Disconnected"));	//creative

	return 0;
}

DWORD WINAPI LoopThread(LPVOID data) {
	thread_data *thread = data;
	loop_thread_data *vars = thread->data;
	LONG frame;

	thread->isRunning = TRUE;

	timeBeginPeriod(10);
	frame = *vars->frame;
	while(thread->shouldRun) {
		Sleep(10);
		if(*vars->frame == frame) {
			continue;
		}
		frame = *vars->frame;
		if(vars->loop) {
			if(frame >= vars->stop || frame < vars->start) {
				AX_callMethod(vars->ctrl, TEXT("Play"), VT_NULL, NULL, VT_I2, 5, VT_BOOL, FALSE, NULL);
			}
		}
	}
	timeEndPeriod(10);

	thread->isRunning = FALSE;

	return 0;
}