/*
 * Created by patryk on 24.06.16.
 */

#include <shlwapi.h>
#include <commctrl.h>
#include <stdio.h>
#include "util.h"
#include "MIDI.h"
HRESULT OpenDialog(HWND parent, LPWSTR path, LPCWSTR filter) {
	OPENFILENAME ofn;

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFile = path;
	ofn.lpstrFile[0] = '\0';
	ofn.hwndOwner = parent;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 1;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrFileTitle = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if(GetOpenFileName(&ofn))
		return S_OK;
	return 1;
}

/*
 * I could put it in one function, but hell, who cares?
 */
HRESULT SaveDialog(HWND parent, LPWSTR path, LPCWSTR filter, LPCWSTR ext) {
	OPENFILENAME ofn;

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFile = path;
	ofn.lpstrFile[0] = '\0';
	ofn.hwndOwner = parent;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 1;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrFileTitle = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt = ext;

	if(GetSaveFileName(&ofn))
		return S_OK;
	return 1;
}

HRESULT GetVSAPath(LPWSTR path) {
	HKEY key;
	DWORD disposition;
	WCHAR file_path[MAX_PATH];
	DWORD len, type;
	HRESULT res;
	WCHAR *chpaths[] = {
			TEXT("%programfiles(x86)%\\Brookshire Software\\Visual Show Automation Ultimate\\VSA.EXE"),
			TEXT("%programfiles(x86)%\\Brookshire Software\\Visual Show Automation Hobbyist\\VSA.EXE"),
			TEXT("%programfiles%\\Brookshire Software\\Visual Show Automation Ultimate\\VSA.EXE"),
			TEXT("%programfiles%\\Brookshire Software\\Visual Show Automation Hobbyist\\VSA.EXE")
	};

	if((res = RegCreateKeyEx(HKEY_CURRENT_USER, REGROOT, 0, NULL, REG_OPTION_NON_VOLATILE,
							 KEY_WRITE | KEY_READ | KEY_QUERY_VALUE, NULL, &key, &disposition)) != ERROR_SUCCESS) {
		return res;
	}

	if(disposition == REG_OPENED_EXISTING_KEY) {
		type = 0;
		len = MAX_PATH * sizeof(WCHAR);
		res = RegQueryValueEx(key, VSAEXEREGKEY, NULL, &type, (LPBYTE) file_path, &len);
		if(res == ERROR_SUCCESS) {
			file_path[MAX_PATH - 1] = '\0';
			if(PathFileExists(file_path)) {
				RegCloseKey(key);
				wcsncpy(path, file_path, MAX_PATH - 1);
				return 0;
			}
			return 1;
		}
	}

	len = 4;
	while(len--) {
		ExpandEnvironmentStrings(chpaths[len], file_path, MAX_PATH - 1);
		if(PathFileExists(file_path)) {
			len = wcsnlen(file_path, MAX_PATH - 1) + 1;
			wcsncpy(path, file_path, MAX_PATH - 1);
			RegSetValueEx(key, VSAEXEREGKEY, 0, REG_SZ, (LPBYTE) file_path, (DWORD) len * sizeof(WCHAR));
			RegCloseKey(key);
			return 0;
		}
	}

	MessageBox(NULL, TEXT("VSA.EXE could not be found.\nPlease point to the valid VSA.EXE file."),
			   TEXT("VSAMidiSync"), MB_OK | MB_ICONINFORMATION);
	res = OpenDialog(NULL, file_path, VSAEXEFILTER);
	if(res == S_OK) {
		wcsncpy(path, file_path, MAX_PATH - 1);
		RegSetValueEx(key, VSAEXEREGKEY, 0, REG_SZ, (LPBYTE) file_path,
					  (DWORD) (wcsnlen(file_path, MAX_PATH - 1) + 1) * sizeof(WCHAR));
		RegCloseKey(key);
		return 0;
	}
	else {
		return 1;
	}
}

HRESULT GetMIDIName(LPWSTR name) {
	HKEY key;
	DWORD disposition;
	WCHAR midi_name[32];
	DWORD len, type;
	HRESULT res;

	if((res = RegCreateKeyEx(HKEY_CURRENT_USER, REGROOT, 0, NULL, REG_OPTION_NON_VOLATILE,
							 KEY_WRITE | KEY_READ | KEY_QUERY_VALUE, NULL, &key, &disposition)) != ERROR_SUCCESS) {
		return res;
	}
	if(disposition == REG_OPENED_EXISTING_KEY) {
		type = 0;
		len = 32 * sizeof(WCHAR);
		res = RegQueryValueEx(key, MIDIKEY, NULL, &type, (LPBYTE) midi_name, &len);
		if(res == ERROR_SUCCESS) {
			midi_name[31] = '\0';
			RegCloseKey(key);
			wcsncpy(name, midi_name, 32);
			return 0;
		}
	}
	return 1;
}

HRESULT SetMIDIName(LPWSTR name) {
	HKEY key;
	DWORD disposition;
	HRESULT res;

	if((res = RegCreateKeyEx(HKEY_CURRENT_USER, REGROOT, 0, NULL, REG_OPTION_NON_VOLATILE,
							 KEY_WRITE | KEY_READ | KEY_QUERY_VALUE, NULL, &key, &disposition)) != ERROR_SUCCESS) {
		return res;
	}

	res = RegSetValueEx(key, MIDIKEY, 0, REG_SZ, (LPBYTE) name, (DWORD) lstrlen(name) * sizeof(WCHAR));

	RegCloseKey(key);

	return res;
}

void FillList(HWND list, subtracks_list *obj) {
	LVITEM item;
	UINT i;
	WCHAR text[12];

	item.mask = LVIF_TEXT;
	item.pszText = STATE_PLAYING;
	item.iItem = 0;
	item.iSubItem = STATE;

	ListView_DeleteAllItems(list);

	ListView_InsertItem(list, &item);

	ListView_SetItemText(list, 0, NAME, TEXT("Entire"));

	ListView_SetItemText(list, 0, START_TEXT, TEXT("00:00:00.00"));
	ListView_SetItemText(list, 0, STOP_TEXT, TEXT("30:59:59.29"));

	LongIntoString(MIDI_BOTTOM, text);
	ListView_SetItemText(list, 0, START, text);

	LongIntoString(MIDI_TOP, text);
	ListView_SetItemText(list, 0, STOP, text);

	ListView_SetItemState(list, 0, LVIS_FOCUSED | LVIS_SELECTED, 0x000F);


	if(obj == NULL) {
		return;
	}

	for(i = 0; i < obj->length; i++) {
		item.pszText = TEXT("");
		item.iItem = i + 1;
		item.iSubItem = STATE;

		ListView_InsertItem(list, &item);

		ListView_SetItemText(list, i + 1, NAME, obj->tracks[i]->name);

		wsprintf(text, L"%.2li:%.2li:%.2li.%.2li", obj->tracks[i]->start / (30 * 60 * 60),
				 obj->tracks[i]->start / (30 * 60) % 60, obj->tracks[i]->start / 30 % 60, obj->tracks[i]->start % 30);
		ListView_SetItemText(list, i + 1, START_TEXT, text);
		wsprintf(text, L"%.2li:%.2li:%.2li.%.2li", obj->tracks[i]->stop / (30 * 60 * 60),
				 obj->tracks[i]->stop / (30 * 60) % 60, obj->tracks[i]->stop / 30 % 60, obj->tracks[i]->stop % 30);
		ListView_SetItemText(list, i + 1, STOP_TEXT, text);

		LongIntoString(obj->tracks[i]->start, text);
		ListView_SetItemText(list, i + 1, START, text);

		LongIntoString(obj->tracks[i]->stop, text);
		ListView_SetItemText(list, i + 1, STOP, text);
	}
}

void SetStates(HWND list, INT playing, INT looping) {
	int i;

	i = ListView_GetItemCount(list);
	while(i--) {
		ListView_SetItemText(list, i, STATE, i == playing ? STATE_PLAYING : i == looping ? STATE_LOOPING : TEXT(""));
	}
}

void LongIntoString(LONG number, LPWSTR str) {
	/*
	 * TODO: Think about compacting it again
	 * since problem with retrieving value back from ListView
	 * is resolved now.
	 */
	wsprintf(str, L"%li", number);
}

LONG LongFromString(LPWSTR str) {
	LONG penis;

	swscanf(str, L"%li", &penis);

	return penis;
}

void SetVSARange(AX_object *ctrl, long start, long stop) {
	AX_callMethod(ctrl, TEXT("SetStartMarker"), VT_NULL, NULL, VT_I4, start, NULL);
	AX_callMethod(ctrl, TEXT("SetStopMarker"), VT_NULL, NULL, VT_I4, stop, NULL);
	AX_callMethod(ctrl, TEXT("SetStartMarker"), VT_NULL, NULL, VT_I4, start, NULL);
	AX_callMethod(ctrl, TEXT("SetStopMarker"), VT_NULL, NULL, VT_I4, stop, NULL);
}