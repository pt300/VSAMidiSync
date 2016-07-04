//
// Created by patryk on 24.06.16.
//

#include <shlwapi.h>
#include "Util.h"

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
	return 1; //TODO: think about it
}

HRESULT GetVSAPath(LPWSTR path) {
	HKEY key;
	DWORD disposition;
	WCHAR file_path[MAX_PATH];
	DWORD len, type;
	HRESULT res;

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

	WCHAR *chpaths[] = {
			TEXT("%programfiles(x86)%\\Brookshire Software\\Visual Show Automation Ultimate\\VSA.EXE"),
			TEXT("%programfiles(x86)%\\Brookshire Software\\Visual Show Automation Hobbyist\\VSA.EXE"),
			TEXT("%programfiles%\\Brookshire Software\\Visual Show Automation Ultimate\\VSA.EXE"),
			TEXT("%programfiles%\\Brookshire Software\\Visual Show Automation Hobbyist\\VSA.EXE")
	};
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