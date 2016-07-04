#include <windows.h>
#include <commctrl.h>
#include "resource.h"
#include "callbacks.h"
#include "AXCtrl.h"
#include "Util.h"
#include "MIDI.h"

#define ICON_SIZE 16

//TODO: reformat names a bit :/

struct main_window {
	HWND window, group, time, play, pause, stop, status_bar;
};

struct thread_data {
	void *data;
	volatile BOOL shouldRun;
	volatile BOOL isRunning;
};

struct time_thread_data {
	AXObj *ctrl;
	HWND time_text, window;
	volatile long frame;
	volatile BOOL playing;
};

struct midi_thread_data {
	volatile long *frame;
	volatile BOOL *playing;
	HWND status;
	UINT device;
};

typedef union {
	DWORD word;
	UCHAR data[4];
} MIDIMsg;

int constructMainWindow(struct main_window *obj, HINSTANCE inst, int cmd_show);

DWORD WINAPI timeThread(LPVOID data) {
	struct thread_data *thread = data;
	struct time_thread_data *vars = thread->data;
	LONG time;
	long cf, sec, min, hour;
	WCHAR text[12];
	RECT rect;
	BOOL state = FALSE;

	thread->isRunning = TRUE;

	timeBeginPeriod(33);


	GetClientRect(vars->time_text, &rect);
	InvalidateRect(vars->time_text, &rect, TRUE);
	MapWindowPoints(vars->time_text, vars->window, (POINT *) &rect, 2);
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

			SetWindowText(vars->time_text, text);
			RedrawWindow(vars->window, &rect, NULL, RDW_ERASE | RDW_INVALIDATE);
		}

		Sleep(33);
	}

	timeEndPeriod(33);

	thread->isRunning = FALSE;

	return 0;
}

DWORD WINAPI midiThread(LPVOID data) {
	struct thread_data *thread = data;
	struct midi_thread_data *vars = thread->data;
	MMRESULT res;
	HMIDIOUT device;
	MIDIMsg msg;
	UCHAR time[4], cnt;

	res = midiOutOpen(&device, vars->device, 0, 0, CALLBACK_NULL);
	if(res != MMSYSERR_NOERROR) {
		return 0;
	}

	thread->isRunning = TRUE;

	SendMessage(vars->status, SB_SETTEXT, 0, (LPARAM) TEXT("MIDI: Connected"));	//creative

	timeBeginPeriod(33);
	//TODO: move a bit to MIDI.c
	//TODO: think about preserving calculated time values. We use them in 2 places after all.
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

int WINAPI WinMain(HINSTANCE inst, HINSTANCE prev_inst, LPSTR cmd_line, int cmd_show) {
	HACCEL accel;
	MSG msg;
	struct main_window win;
	struct thread_data time_thread, midi_thread;
	struct time_thread_data time_data;
	struct midi_thread_data midi_data;
	WCHAR vsa_path[MAX_PATH], file_path[MAX_PATH], midi_name[32];
	INT32 midi_id;
	AXObj ctrl;
	SHORT window = 1;    //0 - Normal, 1 - Minimized, 2 - Hidden(?)
	SHORT playMode = 6;
	BOOL preload = FALSE;
	/*
	 * Type of Execution       	Value of playMode
	 * Play all                          1
	 * Play from first marker            2
	 * Play to second marker             3
	 * Play frame at first marker        4
	 * Play from first to second marker  5
	 * Resume play                       6
	 */

	AX_init();

	if(AX_getControl(&ctrl, TEXT("VSACONSOLE.VSAConsoleCtrl.1"))) {
		MessageBox(NULL, TEXT("Unable to get hold of VSA ActiveX control.\nProgram will now exit."),
				   TEXT("VSAMidiSync"), MB_ICONERROR);
		AX_deinit();
		return 1;
	}

	if(GetVSAPath(vsa_path)) {
		MessageBox(NULL, TEXT("No path choosen or it is invalid.\nProgram will now exit."),
				   TEXT("VSAMidiSync"), MB_ICONWARNING);
		AX_deinit();
		return 1;
	}

	AX_setValue(&ctrl, TEXT("vsaPath"), VT_BSTR, vsa_path);
//	AX_setValue(&ctrl, TEXT("routinePath"), VT_BSTR, TEXT("C:\\Documents and Settings\\Yo_ZiOm\\Pulpit\\supernatural.vsa"));
	AX_setValue(&ctrl, TEXT("showWindow"), VT_I2, &window);

//	AX_callMethod(&ctrl, TEXT("Create"), VT_NULL, NULL, NULL);

	if(constructMainWindow(&win, inst, cmd_show)) {    //It will display own error dialog
		AX_destroyControl(&ctrl);
		AX_deinit();
		return 1;
	}

	//start time thread
	time_thread.isRunning = FALSE;
	time_thread.shouldRun = TRUE;
	time_thread.data = &time_data;
	time_data.ctrl = &ctrl;
	time_data.playing = FALSE;
	time_data.frame = 0;
	time_data.time_text = win.time;
	time_data.window = win.window;

	CreateThread(NULL, 0, timeThread, &time_thread, 0, NULL);

	midi_thread.isRunning = FALSE;
	midi_thread.shouldRun = TRUE;
	midi_thread.data = &midi_data;
	midi_data.playing = &time_data.playing;
	midi_data.frame = &time_data.frame;
	midi_data.device = 0;
	midi_data.status = win.status_bar;

	if(!GetMIDIName(midi_name)) {
		if((midi_id = MIDI_getDeviceByName(midi_name)) > -1) {
			midi_data.device = (UINT) midi_id;
			midi_thread.shouldRun = TRUE;
			CreateThread(NULL, 0, midiThread, &midi_thread, 0, NULL);
		}
	}

	//input loop
	accel = LoadAccelerators(inst, MAKEINTRESOURCE(IDR_ACCELERATOR));

	while(GetMessage(&msg, NULL, 0, 0) > 0) {    //return 0 on WM_QUIT and -1 on error
		if(!TranslateAccelerator(msg.hwnd, accel, &msg)) {
			switch(msg.message) {
				case CANCER:
					midi_thread.shouldRun = FALSE;
					while(midi_thread.isRunning);
					midi_data.device = (UINT) msg.lParam;
					midi_thread.shouldRun = TRUE;
					CreateThread(NULL, 0, midiThread, &midi_thread, 0, NULL);
					break;
				case DEF_CANCER:
					SetMIDIName((LPWSTR) msg.lParam);
					free((LPWSTR) msg.lParam);
					break;
				case WM_COMMAND:
					switch((~(1<<16) & msg.wParam)) { //I have no idea why
						case ID_QUIT:
							PostQuitMessage(0);
						case ID_FILE_MIDI:
							DialogBox(inst, MAKEINTRESOURCE(IDD_MIDICHOOSER), win.window, &MIDIChooserProc);
							break;
						case ID_FILE_OPEN:
							if(OpenDialog(msg.hwnd, file_path, VSAFILEFILTER)) {
								break;
							}
							AX_setValue(&ctrl, TEXT("routinePath"), VT_BSTR, file_path);
						case ID_FILE_RELOAD:
							AX_callMethod(&ctrl, TEXT("Destroy"), VT_NULL, NULL, NULL);
							AX_callMethod(&ctrl, TEXT("Create"), VT_NULL, NULL, NULL);
							break;
						case ID_STOP:
							AX_callMethod(&ctrl, TEXT("Stop"), VT_NULL, NULL, NULL);
							break;
						case ID_PAUSE:
							AX_callMethod(&ctrl, TEXT("Pause"), VT_NULL, NULL, NULL);
							break;
						case ID_PLAY:
							AX_callMethod(&ctrl, TEXT("Play"), VT_NULL, NULL, VT_I2, playMode, VT_BOOL, preload, NULL);
						case ID_TOGGLE:
							if(time_data.playing) {
								AX_callMethod(&ctrl, TEXT("Pause"), VT_NULL, NULL, NULL);
							}
							else {
								AX_callMethod(&ctrl, TEXT("Play"), VT_NULL, NULL, VT_I2, playMode, VT_BOOL, preload, NULL);
							}
							break;
						default:
							TranslateMessage(&msg);
							DispatchMessage(&msg);
							break;
					}
					break;
				default:
					TranslateMessage(&msg);
					DispatchMessage(&msg);
					break;
			}
		}
	}

	time_thread.shouldRun = FALSE;
	midi_thread.shouldRun = FALSE;
	while(time_thread.isRunning || midi_thread.isRunning);

	AX_callMethod(&ctrl, TEXT("Destroy"), VT_NULL, NULL, NULL);

	AX_destroyControl(&ctrl);
	AX_deinit();

	return (int) msg.wParam;
}

int constructMainWindow(struct main_window *obj, HINSTANCE inst, int cmd_show) {
	WNDCLASSEX wc;
	LPCTSTR window_class = TEXT("VSAMidiSync");
	HWND window, group, time, play, pause, stop, status_bar;
	HFONT leFont;

	InitCommonControls();

	wc.cbSize = sizeof(wc);
	wc.style = 0;
	wc.lpfnWndProc = &MainWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = inst;
	wc.hIcon = (HICON) LoadImage(inst, MAKEINTRESOURCE(IDI_APPICON), IMAGE_ICON, 0, 0,
								 LR_DEFAULTSIZE | LR_DEFAULTCOLOR | LR_SHARED);
	wc.hCursor = (HCURSOR) LoadImage(NULL, IDC_ARROW, IMAGE_CURSOR, 0, 0, LR_SHARED);
	wc.hbrBackground = (HBRUSH) (COLOR_3DFACE + 1);
	wc.lpszMenuName = MAKEINTRESOURCE(IDR_MAINMENU);
	wc.lpszClassName = window_class;
	wc.hIconSm = (HICON) LoadImage(inst, MAKEINTRESOURCE(IDI_APPICON), IMAGE_ICON,
								   GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON),
								   LR_DEFAULTCOLOR | LR_SHARED);

	if(!RegisterClassEx(&wc)) {
		MessageBox(NULL, TEXT("Error registering window class."), TEXT("Error"), MB_ICONERROR | MB_OK);
		return 1;
	}

	window = CreateWindowEx(0, window_class, window_class, WS_POPUPWINDOW | WS_CAPTION | WS_MINIMIZEBOX,
							CW_USEDEFAULT, CW_USEDEFAULT, 265, 165, NULL, NULL, inst, NULL);

	if(!window) {
		MessageBox(NULL, TEXT("Error creating main window."), TEXT("Error"), MB_ICONERROR | MB_OK);
		return 1;
	}

	//Build the window's controls

	leFont = CreateFont(-36, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
						CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, TEXT("Arial"));

	group = CreateWindowEx(0, WC_BUTTON, TEXT(""), WS_VISIBLE | WS_CHILD | BS_GROUPBOX | WS_EX_TRANSPARENT,
						   30, 0, 200, 45, window, NULL, inst, 0);

	time = CreateWindowEx(0, WC_STATIC, TEXT("00:00:00.00"), WS_VISIBLE | WS_CHILD | WS_GROUP | SS_CENTER | WS_EX_TRANSPARENT,
						  35, 6, 190, 40, window, (HMENU) 0, inst, 0);
	SendMessage(time, WM_SETFONT, (WPARAM) leFont, FALSE);

	play = CreateWindowEx(0, WC_BUTTON, TEXT("Play"), WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_ICON,
						  75, 55, 30, 30, window, (HMENU) ID_PLAY, inst, 0);
	SendMessage(play, BM_SETIMAGE, IMAGE_ICON,
				(LPARAM) LoadImage(inst, MAKEINTRESOURCE(IDI_PLAY), IMAGE_ICON, ICON_SIZE, ICON_SIZE,
								   LR_DEFAULTCOLOR));

	pause = CreateWindowEx(0, WC_BUTTON, TEXT("Pause"), WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_ICON,
						   115, 55, 30, 30, window, (HMENU) ID_PAUSE, inst, 0);
	SendMessage(pause, BM_SETIMAGE, IMAGE_ICON,
				(LPARAM) LoadImage(inst, MAKEINTRESOURCE(IDI_PAUSE), IMAGE_ICON, ICON_SIZE, ICON_SIZE,
								   LR_DEFAULTCOLOR));

	stop = CreateWindowEx(0, WC_BUTTON, TEXT("Stop"), WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_ICON,
						  155, 55, 30, 30, window, (HMENU) ID_STOP, inst, 0);
	SendMessage(stop, BM_SETIMAGE, IMAGE_ICON,
				(LPARAM) LoadImage(inst, MAKEINTRESOURCE(IDI_STOP), IMAGE_ICON, ICON_SIZE, ICON_SIZE,
								   LR_DEFAULTCOLOR));

	status_bar = CreateWindowEx(0, STATUSCLASSNAME, NULL, WS_CHILD | WS_VISIBLE,
								0, 0, 0, 0, window, NULL, inst, NULL);
	SendMessage(status_bar, SB_SETTEXT, 0, (LPARAM) TEXT("MIDI: Disconnected"));

	// Show window and force a paint.
	ShowWindow(window, cmd_show);
	UpdateWindow(window);


	//the lazy way
	obj->window = window;
	obj->group = group;
	obj->pause = pause;
	obj->play = play;
	obj->status_bar = status_bar;
	obj->stop = stop;
	obj->time = time;

	return 0;
}