//
// Created by patryk on 25.03.16.
//

#include "main.h"
//pause 186
//stop 254
//play 16
//heart 3

unsigned int devNo = 0;
volatile long frame;
volatile BOOL playing = FALSE;
volatile char threadStatus = 0;
volatile BOOL threadRun = TRUE;

void MIDIThread(void *non) {
	if(!threadRun) return;
	threadStatus = 0;
	MMRESULT res;
	HMIDIOUT device;
	res = midiOutOpen(&device, devNo, 0, 0, CALLBACK_NULL);
	if(res != MMSYSERR_NOERROR) {
		threadStatus = -1;
		return;
	}
	timeBeginPeriod(8);
	long fr;
	unsigned char time[4];
//	{
//		fr = frame > 0 ? frame : 0;
//		time[0] = fr % 30;
//		time[1] = fr / 30 % 60;
//		time[2] = fr / (30 * 60) % 60;
//		time[3] = fr / (30 * 60 * 60) % 31;
//		MIDIHDR syncFrame = {0};
//		syncFrame.lpData = malloc(10 * sizeof(CHAR)); //F0 7F cc 01 01 hr mn sc fr F7
//		syncFrame.lpData[0] = 0xF0;
//		syncFrame.lpData[0] = 0x7F;
//		syncFrame.lpData[0] = 0x7F;
//		syncFrame.lpData[0] = 0x01;
//		syncFrame.lpData[0] = 0x01;
//		syncFrame.lpData[0] = time[3];
//		syncFrame.lpData[0] = time[2];
//		syncFrame.lpData[0] = time[1];
//		syncFrame.lpData[0] = time[0];
//		syncFrame.lpData[0] = 0xF7;
//		syncFrame.dwBufferLength = 10;
//		midiOutPrepareHeader(device, &syncFrame, sizeof(MIDIHDR));
//		midiOutLongMsg(device, &syncFrame, sizeof(MIDIHDR));
//		midiOutUnprepareHeader(device, &syncFrame, sizeof(MIDIHDR));
//	} apparently we don't need it
	char cnt;
	while(threadRun) {
		if(!playing)
			continue;
		fr = frame>0?frame:0;
		time[0] = fr % 30;
		time[1] = fr / 30 % 60;
		time[2] = fr / (30*60) % 60;
		time[3] = fr / (30*60*60)%31;
		cnt = 0;
		for(;cnt<8;cnt++){
			MIDIMsg msg = {0};
			msg.data[0] = 0xF1;
			if(cnt != 7)
				msg.data[1] = cnt<<4 | (0x0F & (time[cnt/2]>>(4*(cnt&1))));
			else
				msg.data[1] = cnt<<4 | (1 & time[3]>>4) | 6;
			midiOutShortMsg(device, msg.word);
			Sleep(8); //soooooo accurate
		}
	}
	timeEndPeriod(8);
	midiOutClose(device);
	threadStatus = -1;
}

int main(void) {
	BOOL gotMidiDevice = FALSE;
	textcolor(LIGHTGRAY);
	textbackground(BLACK);
	_setcursortype(_NOCURSOR);
	printf("Initializing...\n");
	HRESULT res;
	PWSTR path = malloc(MAX_PATH * sizeof *path);
	res = GetVSAPath(path);
	if(res != S_OK)
		printf("No path to VSA.EXE");
	STATUS("Path to VSA.EXE", res);
	res = MIDIOutputChooser();
	//STATUS("MIDIOutputChooser", res);
	gotMidiDevice = !res;
	res = OleInitialize(NULL);
	STATUS("OleInitialize", res);
	CLSID VSACLSID;
	res = CLSIDFromProgID(L"VSACONSOLE.VSAConsoleCtrl.1", &VSACLSID);
	STATUS("Get CLSID for VSACONSOLE.VSAConsoleCtrl.1", res);
	IDispatch *DISP_OBJ;
	res = CoCreateInstance(&VSACLSID, NULL, CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER, &IID_IDispatch, (void **)&DISP_OBJ);
	if(res != S_OK)
		printf("If it's failing at this point it usually will mean you forgot to register .ocx control:\nhttps://support.microsoft.com/en-us/kb/146219\n");
	STATUS("CoCreateInstance of IDispatch", res);
	IPersistStreamInit *k = {0};
	res = DISP_OBJ->lpVtbl->QueryInterface(DISP_OBJ, &IID_IPersistStreamInit, (void **)&k);
	STATUS("QueryInterface for IPersistStreamInit", res);
	res = k->lpVtbl->InitNew(k);
	STATUS("InitNew", res);
	res = SetStringProperty(DISP_OBJ, vsaPath, path);
	STATUS("SetStringProperty for vsaPath", res);
	printf("Choose .vsa file to play.");
	res = OpenDialog(path, VSAFILEFILTER);
	if(res != S_OK) {
		printf("No file as selected...");
		DISP_OBJ->lpVtbl->Release(DISP_OBJ);
		STATUS("Release IDispatch", res);
		k->lpVtbl->Release(k);
		STATUS("Release IPersistStreamInit", res);
		OleUninitialize();
	}
	STATUS("Path for *.vsa", res);
	res = SetStringProperty(DISP_OBJ, routinePath, path);
	STATUS("SetStringProprty for routinePath", res);
	//free(path);
	res = SetShortProperty(DISP_OBJ, showWindow, Minimized);
	STATUS("SetShortProperty for showWindow", res);
	BOOL status;
	res = GetBoolFromMethod(DISP_OBJ, Create, &status);
	STATUS("GetBoolFromMethod Create()", res);
#ifdef DEBUG
	printf("Control is owned by us: %s\n", status?"YES":"NO");
#endif
	//Sleep(1000);
	//STATUS("Sleep for 1 second", S_OK);
#ifndef DEBUG
	clrscr();
#endif
	//printf("Apparently everything works!\n");
	printf(HELPSTR);
	long frame_prev;
	BOOL loop = TRUE;
	BOOL showingP = FALSE;
	BOOL redraw = FALSE;
	BOOL showingE = FALSE;
	if(gotMidiDevice) {
		threadStatus = -1;
		threadRun = TRUE;
		_beginthread(MIDIThread, 0, NULL);
	}
	Sleep(3000);
	while(loop) {
		Sleep(50);
		if(_kbhit())
			switch(_getch()) {
				case 'q':
					loop = 0;
					break;
				case ' ':
					TogglePlay(DISP_OBJ);
					break;
				case 's':
					GetNothingFromMethod(DISP_OBJ, Stop);
					break;
				case 'r':
					threadRun = FALSE;
					while(threadStatus != -1);
					Reload(DISP_OBJ);
					printf(HELPSTR);
					redraw = TRUE;
					showingP = FALSE;
					showingE = FALSE;
					if(gotMidiDevice) {
						threadRun = TRUE;
						_beginthread(MIDIThread, 0, NULL);
					}
					break;
				case 'm':
					threadRun = FALSE;
					while(threadStatus != -1);
					gotMidiDevice = !MIDIOutputChooser();
					printf(HELPSTR);
					redraw = TRUE;
					showingP = FALSE;
					showingE = FALSE;
					if(gotMidiDevice) {
						threadRun = TRUE;
						_beginthread(MIDIThread, 0, NULL);
					}
					break;
				case 'n':
					res = OpenDialog(path, VSAFILEFILTER); //TODO: find out why this shit blocks the MIDIThread
					if(res == S_OK) {
						threadRun = FALSE;
						while(threadStatus != -1);
						if(playing)
							GetNothingFromMethod(DISP_OBJ, Pause);
						SetStringProperty(DISP_OBJ, routinePath, path);
						Reload(DISP_OBJ);
						printf(HELPSTR);
						redraw = TRUE;
						showingP = FALSE;
						showingE = FALSE;
						if(gotMidiDevice) {
							threadRun = TRUE;
							_beginthread(MIDIThread, 0, NULL);
						}
					}
				default:
					break;
			}
		GetLongFromMethod(DISP_OBJ, GetPlaybackStatus, &frame);
		if(threadStatus == -1 && !showingE) {
			int wx = wherex();
			int wy = wherey();
			printf("\nConnection to MIDI output lost!");
			showingE = TRUE;
			gotoxy(wy, wy);
		}
		if(threadStatus != -1 && showingE) {
			int wx = wherex();
			int wy = wherey();
			putchar('\n');
			clreol();
			showingE = FALSE;
			gotoxy(wy, wy);
		}
		if(frame == frame_prev) {
			playing = FALSE;
			if(!showingP && frame != -1 && !redraw) {
				printf("\b%c", 186);
				showingP = TRUE;
			}
			if(!redraw)
				continue;
		}
		else {
			playing = TRUE;
			showingP = FALSE;
		}
		redraw = FALSE;
		frame_prev = frame;
		if(frame == -2) {
			printf("Program returned error code -2\r");
			continue; //APPARENTLY IGNORING ERROR CODE FUCKING SOLVES ALL PROBLEMS OF THIS WORLD!
			//break;
		}
		putchar('\r');
		clreol();
		if(frame == -1) {
			printf("00:00:00.00 %c", 254);
		}
		else {
			long cf = frame % 30;
			long sec = frame / 30 % 60;
			long min = frame / (30*60) % 60;
			long hour = frame / (30*60*60);
			printf("%.2li:%.2li:%.2li.%.2li %c", hour, min, sec, cf, 16);
		}
	}
	if(_kbhit())
		getch();
	printf("\n\nShutting down...\n");
	free(path);
	res = GetNothingFromMethod(DISP_OBJ, Destroy);
	STATUS("GetNothingFromMethod Destroy", res);
	DISP_OBJ->lpVtbl->Release(DISP_OBJ);
	STATUS("Release IDispatch", res);
	k->lpVtbl->Release(k);
	STATUS("Release IPersistStreamInit", res);
	OleUninitialize();
	printf("\nMade with ");
	textcolor(RED);
	putchar(3);
	textcolor(LIGHTGRAY);
	printf(" for Laz0r.\nPress any key...\n");
	_setcursortype(_NORMALCURSOR);
	normvideo();
	_getch();
	if(!gotMidiDevice && threadStatus != -1)
		printf("Waiting for thread to exit...");
}

HRESULT GetVSAPath(PWSTR path) {
	HKEY hMykey;
	DWORD disposition;
	wchar_t fPath[MAX_PATH];
	DWORD len = MAX_PATH*sizeof(WCHAR);
	LONG ress = RegCreateKeyEx(HKEY_CURRENT_USER, L"Software\\P4t\\MIDI", 0, NULL, REG_OPTION_NON_VOLATILE,  KEY_WRITE | KEY_READ | KEY_QUERY_VALUE, NULL, &hMykey, &disposition);
	STATUS("CreateKey", ress);
	if(disposition == REG_OPENED_EXISTING_KEY) {
		DWORD type = 0;
		LONG status = RegQueryValueEx(hMykey, VSAEXEREGKEY, NULL, &type, (LPBYTE)fPath, &len);
		if(status == ERROR_SUCCESS && (len/sizeof(WCHAR)) < MAX_PATH) {
			fPath[len/sizeof(WCHAR)+1] = '\0';
			if(PathFileExists(fPath)) {
				RegCloseKey(hMykey);
				wcsncpy(path, fPath, MAX_PATH - 1);
				return 0;
			}
			return 1;
		}
	}
	wchar_t *chpaths[4];
	chpaths[0] = L"%programfiles(x86)%\\Brookshire Software\\Visual Show Automation Ultimate\\VSA.EXE";
	chpaths[1] = L"%programfiles(x86)%\\Brookshire Software\\Visual Show Automation Hobbyist\\VSA.EXE";
	chpaths[2] = L"%programfiles%\\Brookshire Software\\Visual Show Automation Ultimate\\VSA.EXE";
	chpaths[3] = L"%programfiles%\\Brookshire Software\\Visual Show Automation Hobbyist\\VSA.EXE";
	len = 4;
	while(len--) {
		ExpandEnvironmentStrings(chpaths[len], fPath, MAX_PATH-1);
#ifdef  DEBUG
		printf("checking: %.260ls\n", fPath);
#endif
		if(PathFileExists(fPath)) {
#ifdef  DEBUG
			printf("FOUND");
#endif
			len = wcsnlen(fPath, MAX_PATH-1)+1;
			wcsncpy(path, fPath, MAX_PATH-1);
			RegSetValueEx(hMykey, VSAEXEREGKEY, 0, REG_SZ, (LPBYTE)fPath, (DWORD)(len)*sizeof(WCHAR));
			RegCloseKey(hMykey);
			return 0;
		}
	}
	printf("Please point to the VSA.EXE file.\n");
	HRESULT res = OpenDialog(fPath, VSAEXEFILTER);
	if(res == S_OK) {
		wcsncpy(path, fPath, MAX_PATH-1);
		RegSetValueEx(hMykey, VSAEXEREGKEY, 0, REG_SZ, (LPBYTE)fPath, (DWORD)(wcsnlen(fPath, MAX_PATH-1)+1)*sizeof(WCHAR));
		RegCloseKey(hMykey);
		return 0;
	}
	else
		return 1;
}

HRESULT MIDIOutputChooser(void) {
	printf("\n");
	unsigned int cnt = midiOutGetNumDevs();
	if(!cnt) {
		printf("No MIDI Output devices detected!!!");
		_getch();
		clrscr();
		return 1;
	}
	struct text_info term;
	gettextinfo(&term);
	unsigned int pos = 0;
	unsigned int sf = 0;
	char **names;
	fillNames(&names, cnt);
	BOOL loop = TRUE;
	while(loop) {
		unsigned int sv = cnt-sf>term.screenheight-2?(unsigned int)term.screenheight-3:cnt-sf;
		unsigned int counter = 0;
		clrscr();
		gotoxy(1,1);
		printf("Choose output device.\nNavigate with arrow keys and confirm with [enter]. Use [r] to reload.\n");
		gotoxy(1,3);
		while(counter<sv) {
			printf("  %.3u %s\n", counter+sf, names[counter+sf]);
			counter++;
		}
		gotoxy(1, pos-sf+3);
		putchar('>');
		putchar('\r');
		switch(_getch()) {
			case 'r':
				;
				unsigned int prev = cnt;
				cnt = midiOutGetNumDevs();
				if(!cnt) {
					printf("No MIDI Output devices detected!!!");
					_getch();
					return 1;
				}
				freeNames(&names, prev);
				fillNames(&names, cnt);
				pos = 0;
				sf = 0;
				continue;
			case 224:
				;
				char key = (char)_getch();
				if(key == 72) {
					if(pos) {
						pos--;
						if(pos<sf) {
							sf--;
							continue;
						}
						putchar(' ');
						gotoxy(1, pos-sf+3);
						putchar('>');
						putchar('\r');
					}
				}
				else if(key == 80) {
					if(pos < (cnt-1)) {
						pos++;
						if(pos-sf>term.screenheight-3) {
							sf++;
							continue;
						}
						putchar(' ');
						gotoxy(1, pos-sf+3);
						putchar('>');
						putchar('\r');
					}
				}
				break;
			case 13:
				devNo = pos;
				loop = FALSE;
				break;
			default:
				break;
		}
	}
	freeNames(&names, cnt);
	clrscr();
	return S_OK;
}

void freeNames(char ***dev, unsigned int cnt) {
	while(cnt--)
		free(dev[0][cnt]);
	free(dev[0]);
}

void fillNames(char ***dev, unsigned int cnt) {
	dev[0] = malloc(cnt * sizeof(char *));
	while(cnt--) {
		dev[0][cnt] = malloc(sizeof(char) * 32);
		MIDIOUTCAPS devStruct = {0};
		MMRESULT res = midiOutGetDevCaps(cnt, &devStruct, sizeof devStruct);
		if(res == MMSYSERR_NOERROR)
			snprintf(dev[0][cnt], 31, "%ls", devStruct.szPname);
		else
			memcpy(dev[0][cnt], "ERROR", 6);
		//dev[0][cnt][31] = '\0';
	}
}

HRESULT Reload(IDispatch *dispatch) {
	HRESULT res;
	BOOL status;
	printf("\n\nRestarting VSA...\n");
	res = GetNothingFromMethod(dispatch, Destroy);
	STATUS("GetNothingFromMethod Destroy", res);
	res = GetBoolFromMethod(dispatch, Create, &status);
	STATUS("GetBoolFromMethod Create()", res);
	clrscr();
#ifdef DEBUG
	printf("Control is owned by us: %s\n", status?"YES":"NO");
#endif
	return res;
}

HRESULT TogglePlay(IDispatch *dispatch) {
	if(playing)
		return GetNothingFromMethod(dispatch, Pause);
	else
		return CallStupidPlayMethodFuckingHell(dispatch);
}

HRESULT CallStupidPlayMethodFuckingHell(IDispatch *dispatch) {
	VARIANT *result = malloc(sizeof(VARIANT));
	VARIANT *args = malloc(sizeof(VARIANT)*2);
	args[0].vt = VT_BOOL;
	args[0].boolVal = FALSE;
	args[1].vt = VT_I2;
	args[1].iVal = 6;
	DISPPARAMS dp = {0};
	dp.cArgs = 2;
	dp.rgvarg = args;
	HRESULT v = dispatch->lpVtbl->Invoke(dispatch, Play, &IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_METHOD, &dp, result, NULL, NULL);
	free(result);
	free(args);
	return v;
}

HRESULT GetLongFromMethod(IDispatch *dispatch, VSA_IDs dispid, volatile LONG *out){
	VARIANT *result = malloc(sizeof(VARIANT));
	HRESULT v = CallMethod(dispatch, dispid, result);
	if(v != S_OK)
		return v;
	*out = result->lVal; //totally safe guys! totally safe!
	free(result);
	return v;
}

HRESULT GetBoolFromMethod(IDispatch *dispatch, VSA_IDs dispid, BOOL *out) {
	VARIANT *result = malloc(sizeof(VARIANT));
	HRESULT v = CallMethod(dispatch, dispid, result);
	if(v != S_OK)
		return v;
	*out = result->boolVal; //totally safe guys! totally safe!
	free(result);
	return v;
}

HRESULT GetNothingFromMethod(IDispatch *dispatch, VSA_IDs dispid) { //creative af
	VARIANT *result = malloc(sizeof(VARIANT));
	HRESULT v = CallMethod(dispatch, dispid, result);
	free(result);
	return v;
}

HRESULT SetStringProperty(IDispatch* dispatch, VSA_IDs dispid, LPWSTR valueSet) {
	VARIANT *varValue = malloc(sizeof(VARIANT));
	VariantInit(varValue);
	BSTR str = SysAllocString(valueSet);
	varValue->vt = VT_BSTR;
	varValue->bstrVal = str;
	HRESULT v = PutValue(dispatch, dispid, varValue);
	VariantClear(varValue);
	free(varValue);
	return v;
}

HRESULT SetShortProperty(IDispatch *dispatch, VSA_IDs dispid, SHORT valueSet) {
	VARIANT *varValue = malloc(sizeof(VARIANT));
	VariantInit(varValue);
	varValue->vt = VT_I2;
	varValue->iVal = valueSet;
	HRESULT v = PutValue(dispatch, dispid, varValue);
	VariantClear(varValue);
	free(varValue);
	return v;
}

HRESULT PutValue(IDispatch *dispatch, VSA_IDs dispid, VARIANT *varValue) {
	HRESULT v;
	DISPPARAMS dp = {0};
	dp.cArgs = 1;
	dp.rgvarg = varValue;
	dp.cNamedArgs = 1;
	DISPID *dispidNamed = malloc(sizeof(DISPID));
	*dispidNamed = DISPID_PROPERTYPUT;
	dp.rgdispidNamedArgs = dispidNamed;
	v = dispatch->lpVtbl->Invoke(dispatch, dispid, &IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_PROPERTYPUT, &dp, NULL, NULL, NULL);
	free(dispidNamed);
	return v;
}

HRESULT CallMethod(IDispatch *dispatch, VSA_IDs dispid, VARIANT *varValue) {
	DISPPARAMS dp = {0};
	return dispatch->lpVtbl->Invoke(dispatch, dispid, &IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_METHOD, &dp, varValue, NULL, NULL);
}

HRESULT OpenDialog(LPWSTR path, LPCWSTR filter) {

	OPENFILENAME ofn;

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFile = path;
	ofn.lpstrFile[0] = '\0';
	ofn.hwndOwner = NULL; //no owner, null is valid
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 1;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrFileTitle = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if(GetOpenFileName(&ofn))
		return S_OK;
	return 1; //:|
}