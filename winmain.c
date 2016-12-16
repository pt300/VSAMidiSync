#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <compstui.h>
#include "AXCtrl.h"
#include "subtracks.h"
#include "callbacks.h"
#include "resource.h"
#include "util.h"
#include "MIDI.h"
#include "threads.h"

#define ICON_SIZE 16

typedef struct {
	HWND window, group, time, play, pause, stop, statusBar, list, expand;
	HMENU rmb_menu;
} main_window;

int ConstructMainWindow(main_window *obj, HINSTANCE inst, int cmdShow);

static int scrollbar_width;

int WINAPI WinMain(HINSTANCE inst, HINSTANCE prev_inst, LPSTR cmd_line, int cmd_show) {
	HACCEL accel;
	MSG msg;
	HMENU menu;
	main_window win;
	thread_data time_thread, midi_thread, loop_thread;
	time_thread_data time_data;
	midi_thread_data midi_data;
	loop_thread_data loop_data;
	WCHAR vsa_path[MAX_PATH], file_path[MAX_PATH], midi_name[32], txt_buf[51];
	LONG time, time2;
	INT position;
	INT32 midi_id;
	AX_object ctrl;
	subtracks_list list;
	POINT cursor;
	HANDLE loop_img;
	INT current_track = 0;
	INT looping_track = -1;
	SHORT window = 1;
	/*
	 * 0 - Normal
	 * 1 - Minimized
	 * 2 - Hidden(?)
	 */
	SHORT play_mode = 6;
	/*
	 * Type of Execution       	Value of play_mode
	 * Play all                          1
	 * Play from first marker            2
	 * Play to second marker             3
	 * Play frame at first marker        4
	 * Play from first to second marker  5
	 * Resume play                       6
	 */
	/*
	 * TODO: ENUM it
	 */

	scrollbar_width = GetSystemMetrics(SM_CXVSCROLL);
	loop_img = LoadImage(inst, MAKEINTRESOURCE(IDI_LOOP), IMAGE_ICON, 15, 15, LR_DEFAULTCOLOR);

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

	file_path[0] = '\0';

	AX_setValue(&ctrl, TEXT("vsaPath"), VT_BSTR, vsa_path);
	AX_setValue(&ctrl, TEXT("showWindow"), VT_I2, &window);

	if(ConstructMainWindow(&win, inst, cmd_show)) {
		AX_destroyControl(&ctrl);
		AX_deinit();
		return 1;
	}

	/*
	 * Prepare data for threads and start time and loop threads
	 */
	time_thread.isRunning = FALSE;
	time_thread.shouldRun = TRUE;
	time_thread.data = &time_data;
	time_data.ctrl = &ctrl;
	time_data.playing = FALSE;
	time_data.frame = 0;
	time_data.timeText = win.time;
	time_data.window = win.window;

	CreateThread(NULL, 0, TimeThread, &time_thread, 0, NULL);

	loop_thread.isRunning = FALSE;
	loop_thread.shouldRun = TRUE;
	loop_thread.data = &loop_data;
	loop_data.ctrl = &ctrl;
	loop_data.stop = MIDI_TOP;
	loop_data.start = MIDI_BOTTOM;
	loop_data.loop = FALSE;
	loop_data.frame = &time_data.frame;
	loop_data.idle = 0;
	loop_data.current_track = &current_track;
	loop_data.looping_one = &looping_track;
	loop_data.list = win.list;

	CreateThread(NULL, 0, LoopThread, &loop_thread, 0, NULL);

	midi_thread.isRunning = FALSE;
	midi_thread.shouldRun = TRUE;
	midi_thread.data = &midi_data;
	midi_data.playing = &time_data.playing;
	midi_data.frame = &time_data.frame;
	midi_data.device = 0;
	midi_data.status = win.statusBar;
	midi_data.img_midi_on = LoadImage(inst, MAKEINTRESOURCE(IDI_MIDI_ON), IMAGE_ICON, 13, 13, LR_DEFAULTCOLOR);
	midi_data.img_midi_off = LoadImage(inst, MAKEINTRESOURCE(IDI_MIDI_OFF), IMAGE_ICON, 13, 13, LR_DEFAULTCOLOR);

	SendMessage(win.statusBar, SB_SETICON, MIDI_CELL, (LPARAM) midi_data.img_midi_off);

	if(!GetMIDIName(midi_name)) {
		if((midi_id = MIDI_getDeviceByName(midi_name)) > -1) {
			midi_data.device = (UINT) midi_id;
			midi_thread.shouldRun = TRUE;
			CreateThread(NULL, 0, MidiThread, &midi_thread, 0, NULL);
		}
	}

	menu = GetMenu(win.window);

	InitSubtracksList(&list);

	/*
	 * Input loop
	 */
	accel = LoadAccelerators(inst, MAKEINTRESOURCE(IDR_ACCELERATOR));

	while(GetMessage(&msg, NULL, 0, 0) > 0) {
		if(!TranslateAccelerator(msg.hwnd, accel, &msg)) {
			switch(msg.message) {
				case CANCER:
					midi_thread.shouldRun = FALSE;
					while(midi_thread.isRunning);
					midi_data.device = (UINT) msg.lParam;
					midi_thread.shouldRun = TRUE;
					CreateThread(NULL, 0, MidiThread, &midi_thread, 0, NULL);
					break;
				case DEF_CANCER:
					SetMIDIName((LPWSTR) msg.lParam);
					free((LPWSTR) msg.lParam);
					break;
				case ADD_STRACK:
					if(!AddSubtrack(&list, ((subtrack *) msg.lParam)->start, ((subtrack *) msg.lParam)->stop,
									((subtrack *) msg.lParam)->name)) {
						MessageBox(win.window, TEXT("125 subtracks is the limit."),
								   TEXT("Add subtrack"), MB_ICONERROR | MB_OK);
						break;
					}
					FillList(win.list, &list);
					SetStates(win.list, current_track, looping_track);
					MakeChanges();
					free((subtrack *) msg.lParam);
					break;
				case SWAP_STRACKS:
					if(SwapSubtracks(&list, (UCHAR) msg.lParam, (UCHAR) msg.wParam)) {
						MakeChanges();
					}
				case WM_NOTIFY:
					switch(LOWORD(msg.wParam)) {
						case ID_TRACKS:
							//printf("WM_NOTIFY FOR ID_TRACKS: %x\n", ((LPNMHDR) msg.lParam)->code);
							switch(((LPNMHDR) msg.lParam)->code) {
								case NM_RCLICK:
									GetCursorPos(&cursor);

									CheckMenuItem(win.rmb_menu, ID_LOOP,
												  MF_BYCOMMAND | (loop_data.loop ? MF_CHECKED : MF_UNCHECKED));
									CheckMenuItem(win.rmb_menu, ID_IDLE,
												  MF_BYCOMMAND | (loop_data.idle != 0 ? MF_CHECKED : MF_UNCHECKED));

									TrackPopupMenu((HMENU) GetSubMenu(win.rmb_menu, 0),
												   TPM_LEFTALIGN | TPM_RIGHTBUTTON, cursor.x, cursor.y, 0, win.window,
												   NULL);


									break;
									/*
									 * Have you ever heard windows is shit?
									 * Yeah, it is.
									 */
									/* For Win XP */
								case LVN_ITEMACTIVATE:
									/* For Win 10
									 * TODO: Find out what these even are
									 */
								case 0x76f1c41f:
								case 0x1c0001:
								case 0x11c0001:

									//if(((LPNMITEMACTIVATE) msg.lParam)->iItem >= 0) {

									//printf("LVN_ITEMACTIVATE");
									//position = ((LPNMITEMACTIVATE) msg.lParam)->iItem;

									position = ListView_GetNextItem(win.list, -1, LVNI_SELECTED);

									/*
									 * Stopping thread and playback prevents VSA from crashing.
									 */

									loop_thread.shouldRun = FALSE;
									while(loop_thread.isRunning);

									AX_callMethod(&ctrl, TEXT("Stop"), VT_NULL, NULL, NULL);

									ListView_GetItemText(win.list, position, START, txt_buf, 51);
									time = LongFromString(txt_buf);
									loop_data.start = time;

									ListView_GetItemText(win.list, position, STOP, txt_buf, 51);
									time2 = LongFromString(txt_buf);
									loop_data.stop = time2;

									current_track = position;
									SetStates(win.list, position, looping_track);

									if(loop_data.idle) {
										loop_data.idle = 1;
									}

									SetVSARange(&ctrl, time, time2);

									//ListView_GetItemText(win.list, position, NAME, txt_buf, 51);
									//SendMessage(win.statusBar, SB_SETTEXT, 1, (LPARAM) txt_buf);
									if(time_data.playing) {
										AX_callMethod(&ctrl, TEXT("Stop"), VT_NULL, NULL, NULL);
										AX_callMethod(&ctrl, TEXT("Play"), VT_NULL, NULL, VT_I2, 5, VT_BOOL, FALSE,
													  NULL);
									}

									loop_thread.shouldRun = TRUE;
									CreateThread(NULL, 0, LoopThread, &loop_thread, 0, NULL);
									//}
									break;
								default:
									break;
							}
							break;
						default:
							break;
					}
					break;
				case WM_COMMAND:
					switch(LOWORD(msg.wParam)) {
						case ID_FILE_EXIT:
						case ID_QUIT:
							PostMessage(win.window, WM_CLOSE, 0, 0);
							break;
						case ID_FILE_MIDI:
							DialogBox(inst, MAKEINTRESOURCE(IDD_MIDICHOOSER), win.window, &MIDIChooserProc);
							break;
						case ID_FILE_OPEN:
							if(OpenDialog(msg.hwnd, file_path, VSAFILEFILTER)) {
								break;
							}
							AX_setValue(&ctrl, TEXT("routinePath"), VT_BSTR, file_path);
							if(VSBExists(file_path) &&
							   LoadVSBFile(&list, file_path)) {
								FillList(win.list, &list);
							}
							AX_callMethod(&ctrl, TEXT("Play"), VT_NULL, NULL, VT_I2, play_mode, VT_BOOL, TRUE, NULL);
						case ID_FILE_RELOAD:
							AX_callMethod(&ctrl, TEXT("Destroy"), VT_NULL, NULL, NULL);
							AX_callMethod(&ctrl, TEXT("Create"), VT_NULL, NULL, NULL);
							break;
						case ID_FILE_OPEN_T:
							if(OpenDialog(msg.hwnd, file_path, VSBFILTER)) {
								break;
							}
							if(VSBExists(file_path) &&
							   LoadVSBFile(&list, file_path)) {
								FillList(win.list, &list);
							} else {
								MessageBox(win.window, TEXT("Failed loading VSB file."),
										   TEXT("Welp"), MB_OK | MB_ICONWARNING);
							}
							break;
						case ID_FILE_SAVE_T:
							if(list.length < 1) {
								MessageBox(win.window, TEXT("Nothing to save."),
										   TEXT("Welp"), MB_OK | MB_ICONINFORMATION);
								break;
							}
							if(lstrlen(file_path) != 0) { //else let it slide into next case
								if(SaveVSBFile(&list, file_path) == FALSE) {
									MessageBox(win.window, TEXT("Failed saving subtracks."),
											   TEXT("Save error"), MB_OK);
									break;
								}
								AintNoChanges();
								if(msg.lParam != 0) { //request to shut down
									PostMessage(win.window, WM_CLOSE, 0, 0);
								}
								break;
							}
						case ID_FILE_SAVE_TA:
							if(list.length < 1) {
								MessageBox(win.window, TEXT("Nothing to save."),
										   TEXT("Welp"), MB_OK | MB_ICONINFORMATION);
								break;
							}
							if(SaveDialog(win.window, file_path, VSBFILTER, L"VSB") == S_OK) {
								if(SaveVSBFile(&list, file_path) == FALSE) {
									MessageBox(win.window, TEXT("Failed saving subtracks."),
											   TEXT("Save error"), MB_OK | MB_ICONERROR);
									break;
								}
								AintNoChanges();
								if(msg.lParam != 0) {
									PostMessage(win.window, WM_CLOSE, 0, 0);
								}
							}
							break;
						case ID_STOP:
							AX_callMethod(&ctrl, TEXT("Stop"), VT_NULL, NULL, NULL);
							break;
						case ID_PAUSE:
							AX_callMethod(&ctrl, TEXT("Pause"), VT_NULL, NULL, NULL);
							break;
						case ID_TOGGLE:
							if(time_data.playing) {
								AX_callMethod(&ctrl, TEXT("Pause"), VT_NULL, NULL, NULL);
								break;
							}
						case ID_PLAY:
							if(time_data.frame >= loop_data.start && time_data.frame < loop_data.stop) {
								AX_callMethod(&ctrl, TEXT("Play"), VT_NULL, NULL, VT_I2, 6, VT_BOOL, FALSE, NULL);
							} else {
								AX_callMethod(&ctrl, TEXT("Play"), VT_NULL, NULL, VT_I2, 5, VT_BOOL, FALSE, NULL);
							}
							break;
						case ID_EDIT_ADD:
							DialogBox(inst, MAKEINTRESOURCE(IDD_ADDSUBTRACK), win.window, &AddSubtrackProc);
							break;
						case ID_EDIT_REMOVE:
							position = ListView_GetNextItem(win.list, -1, LVNI_FOCUSED);
							if(position <= 0) {
								MessageBox(win.window, TEXT("This subtrack cannot be removed."),
										   TEXT("Remove subtrack"), MB_ICONERROR | MB_OK);
								break;
							}
							RemoveSubtrack(&list, (UCHAR) (position - 1));
							if(position == current_track) {
								current_track = 0;
							} else if(position < current_track) {
								current_track--;
							}
							if(position == looping_track) {
								looping_track = -1;
								loop_data.idle = 0;
							} else if(position < looping_track) {
								looping_track--;
							}
							FillList(win.list, &list);
							SetStates(win.list, current_track, looping_track);
							if(list.length < 1) {
								AintNoChanges();
							} else {
								MakeChanges();
							}
							break;
						case ID_EDIT_REMALL:
							DestroySubtracksList(&list);
							InitSubtracksList(&list);
							FillList(win.list, &list);
							break;
						case ID_EXPAND: //hehe
							if(GetMenuState(menu, ID_EXPAND, MF_BYCOMMAND) & MF_CHECKED) {
								CheckMenuItem(menu, LOWORD(msg.wParam), MF_BYCOMMAND | MF_UNCHECKED);
								SetWindowPos(win.window, NULL, 0, 0, 265, 165,
											 SWP_NOMOVE | SWP_NOREPOSITION | SWP_NOZORDER);
							} else {
								CheckMenuItem(menu, LOWORD(msg.wParam), MF_BYCOMMAND | MF_CHECKED);
								SetWindowPos(win.window, NULL, 0, 0, 265 + 250 + scrollbar_width + 15, 165,
											 SWP_NOMOVE | SWP_NOREPOSITION | SWP_NOZORDER);
							}
							break;
						case ID_LOOP:
							if(GetMenuState(menu, ID_LOOP, MF_BYCOMMAND) & MF_CHECKED) {
								CheckMenuItem(menu, LOWORD(msg.wParam), MF_BYCOMMAND | MF_UNCHECKED);
								loop_data.loop = FALSE;
								SendMessage(win.statusBar, SB_SETICON, LOOP_CELL, (LPARAM) NULL);
							} else {
								CheckMenuItem(menu, LOWORD(msg.wParam), MF_BYCOMMAND | MF_CHECKED);
								loop_data.loop = TRUE;
								SendMessage(win.statusBar, SB_SETICON, LOOP_CELL, (LPARAM) loop_img);
							}
							break;
						case ID_IDLE:
							if(GetMenuState(menu, ID_IDLE, MF_BYCOMMAND) & MF_CHECKED) {
								CheckMenuItem(menu, LOWORD(msg.wParam), MF_BYCOMMAND | MF_UNCHECKED);
								loop_data.idle = 0;
								looping_track = -1;
							} else {
								CheckMenuItem(menu, LOWORD(msg.wParam), MF_BYCOMMAND | MF_CHECKED);
								position = ListView_GetNextItem(win.list, -1, LVNI_SELECTED);

								ListView_GetItemText(win.list, position, START, txt_buf, 51);
								time = LongFromString(txt_buf);
								loop_data.idleStart = time;

								ListView_GetItemText(win.list, position, STOP, txt_buf, 51);
								time2 = LongFromString(txt_buf);
								loop_data.idleStop = time2;

								loop_data.idle = 1;

								looping_track = position;
							}
							SetStates(win.list, current_track, looping_track);
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

	DestroySubtracksList(&list);

	time_thread.shouldRun = FALSE;
	midi_thread.shouldRun = FALSE;
	loop_thread.shouldRun = FALSE;
	while(time_thread.isRunning || midi_thread.isRunning || loop_thread.isRunning);

	AX_callMethod(&ctrl, TEXT("Destroy"), VT_NULL, NULL, NULL);

	AX_destroyControl(&ctrl);
	AX_deinit();

	return (int) msg.wParam;
}

int ConstructMainWindow(main_window *obj, HINSTANCE inst, int cmdShow) {
	WNDCLASSEX wc;
	LPCTSTR windowClass = TEXT("VSAMidiSync");
	HWND window, group, time, play, pause, stop, status_bar, list, lhead;
	HFONT leFont;
	LVCOLUMN col;
	INT widths[] = {220, 240, -1}; //130, 265

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
	wc.lpszClassName = windowClass;
	wc.hIconSm = (HICON) LoadImage(inst, MAKEINTRESOURCE(IDI_APPICON), IMAGE_ICON,
								   GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON),
								   LR_DEFAULTCOLOR | LR_SHARED);

	if(!RegisterClassEx(&wc)) {
		MessageBox(NULL, TEXT("Error registering window class."), TEXT("Error"), MB_ICONERROR | MB_OK);
		return 1;
	}

	window = CreateWindow(windowClass, windowClass, WS_POPUPWINDOW | WS_CAPTION | WS_MINIMIZEBOX,
						  CW_USEDEFAULT, CW_USEDEFAULT, 265, 165, NULL, NULL, inst, NULL);

	if(!window) {
		MessageBox(NULL, TEXT("Error creating main window."), TEXT("Error"), MB_ICONERROR | MB_OK);
		return 1;
	}

	/*
	 * Build the window's controls
	 */

	leFont = CreateFont(-36, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
						CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, TEXT("Arial"));

	group = CreateWindow(WC_BUTTON, TEXT(""), WS_VISIBLE | WS_CHILD | BS_GROUPBOX | WS_EX_TRANSPARENT,
						 30, 0, 200, 45, window, NULL, inst, 0);

	time = CreateWindow(WC_STATIC, TEXT("00:00:00.00"),
						WS_VISIBLE | WS_CHILD | WS_GROUP | SS_CENTER | WS_EX_TRANSPARENT,
						35, 6, 190, 40, window, (HMENU) 0, inst, 0);
	SendMessage(time, WM_SETFONT, (WPARAM) leFont, FALSE);

	play = CreateWindow(WC_BUTTON, TEXT("Play"), WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_ICON,
						75, 55, 30, 30, window, (HMENU) ID_PLAY, inst, 0);
	SendMessage(play, BM_SETIMAGE, IMAGE_ICON,
				(LPARAM) LoadImage(inst, MAKEINTRESOURCE(IDI_PLAY), IMAGE_ICON, ICON_SIZE, ICON_SIZE,
								   LR_DEFAULTCOLOR));

	pause = CreateWindow(WC_BUTTON, TEXT("Pause"), WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_ICON,
						 115, 55, 30, 30, window, (HMENU) ID_PAUSE, inst, 0);
	SendMessage(pause, BM_SETIMAGE, IMAGE_ICON,
				(LPARAM) LoadImage(inst, MAKEINTRESOURCE(IDI_PAUSE), IMAGE_ICON, ICON_SIZE, ICON_SIZE,
								   LR_DEFAULTCOLOR));

	stop = CreateWindow(WC_BUTTON, TEXT("Stop"), WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_ICON,
						155, 55, 30, 30, window, (HMENU) ID_STOP, inst, 0);
	SendMessage(stop, BM_SETIMAGE, IMAGE_ICON,
				(LPARAM) LoadImage(inst, MAKEINTRESOURCE(IDI_STOP), IMAGE_ICON, ICON_SIZE, ICON_SIZE,
								   LR_DEFAULTCOLOR));

	status_bar = CreateWindow(STATUSCLASSNAME, NULL, WS_CHILD | WS_VISIBLE,
							  0, 0, 0, 0, window, NULL, inst, NULL);

	SendMessage(status_bar, SB_SETPARTS, CELLS_NUMBER, (LPARAM) widths);
	//SendMessage(status_bar, SB_SETTEXT, 0, (LPARAM) TEXT("MIDI: Disconnected"));
	//SendMessage(status_bar, SB_SETTEXT, 1, (LPARAM) TEXT("Entire"));
	SendMessage(status_bar, SB_SETTEXT, HELLIFIKNEW, (LPARAM) TEXT("A baby spider is called a spiderling."));

	list = CreateWindow(WC_LISTVIEW, NULL,
						WS_VISIBLE | WS_CHILD | WS_TABSTOP | LVS_REPORT | LVS_SINGLESEL | LVS_NOSORTHEADER |
						LVS_SHOWSELALWAYS,
						260, 0, 250 + scrollbar_width + 15, 120, window, (HMENU) ID_TRACKS, inst, 0);

	ListView_SetExtendedListViewStyle(list, LVS_EX_FULLROWSELECT);
	lhead = ListView_GetHeader(list);
	ListViewHeaderOriginalProc = (WNDPROC) SetWindowLong(lhead, GWLP_WNDPROC, (INT_PTR) ListViewHeaderProc);

	col.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.iSubItem = STATE;
	col.cx = 15;
	col.pszText = TEXT("");
	ListView_InsertColumn(list, STATE, &col);

	col.iSubItem = NAME;
	col.cx = 100;
	col.pszText = TEXT("Name");
	ListView_InsertColumn(list, NAME, &col);

	col.iSubItem = START_TEXT;
	col.cx = 75;
	col.pszText = TEXT("Start");
	ListView_InsertColumn(list, START_TEXT, &col);

	col.iSubItem = STOP_TEXT;
	col.cx = 75;
	col.pszText = TEXT("Stop");
	ListView_InsertColumn(list, STOP_TEXT, &col);

	col.mask = LVCF_SUBITEM | LVCF_WIDTH;
	col.iSubItem = START;
	col.cx = 0;
	ListView_InsertColumn(list, START, &col);

	col.mask = LVCF_SUBITEM | LVCF_WIDTH;
	col.iSubItem = STOP;
	col.cx = 0;
	ListView_InsertColumn(list, STOP, &col);

	FillList(list, NULL);

	/*
	 * Show window and force a paint.
	 */
	ShowWindow(window, cmdShow);
	UpdateWindow(window);


	/*
	 * the lazy way
	 */
	obj->window = window;
	obj->group = group;
	obj->pause = pause;
	obj->play = play;
	obj->statusBar = status_bar;
	obj->stop = stop;
	obj->time = time;
	obj->list = list;
	obj->rmb_menu = LoadMenu(inst, MAKEINTRESOURCE(IDR_RMBMENU));

	tlist = list; ///EEEEEEEEHHHHHH

	return 0;
}