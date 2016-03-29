//
// Created by patryk on 25.03.16.
//

#define INITGUID 1 //force define of GUIDs
#define UNICODE 1
#define _UNICODE 1
#include <ocidl.h>
#include "main.h"
DEFINE_GUID(IID_VSA, 0xD69EBDF7, 0x494B, 0x11D5, 0x8D, 0x2D, 0x00, 0x20, 0x78, 0x15, 0x1F, 0x21);
int main(void) {
	HRESULT res;
	res = OleInitialize(NULL);
	STATUS("OleInitialize", res);
	IDispatch *DISP_OBJ;
	res = CoCreateInstance(&IID_VSA, NULL, CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER, &IID_IDispatch, (void **)&DISP_OBJ);
	STATUS("CoCreateInstance of IDispatch", res);
	IPersistStreamInit *k = {0};
	res = DISP_OBJ->lpVtbl->QueryInterface(DISP_OBJ, &IID_IPersistStreamInit, (void **)&k);
	STATUS("QueryInterface for IPersistStreamInit", res);
	res = k->lpVtbl->InitNew(k);
	STATUS("InitNew", res);
	PWSTR path = malloc(MAX_PATH * sizeof *path);
	res = OpenDialog(path, L"THE SHIT(VSA.EXE)\0VSA.EXE\0");
	STATUS("OpenDialog for VSA.EXE", res);
	res = SetStringProperty(DISP_OBJ, vsaPath, path);
	STATUS("SetStringProperty for vsaPath", res);
	res = OpenDialog(path, L"Pics of horse dicks(*.vsa)\0*.vsa\0");
	STATUS("OpenDialog for *.VSA", res);
	res = SetStringProperty(DISP_OBJ, routinePath, path);
	STATUS("SetStringProprty for routinePath", res);
	free(path);
	res = SetShortProperty(DISP_OBJ, showWindow, Minimized);
	STATUS("SetShortProperty for showWindow", res);
	BOOL status;
	res = GetBoolFromMethod(DISP_OBJ, Create, &status);
	STATUS("GetBoolFromMethod Create()", res);
#ifdef DEBUG
	printf("Control is owned by us: %s\n", status?"YES":"NO");
#endif
	Sleep(1000);
	STATUS("Sleep for 1 second", S_OK);
	printf("Apparently everything works!\n"
			"To exit press [q]. Play/pause [spacebar]. Stop [s]. Reload VSA [r].\n");
	long frame;
	long frame_prev;
	BOOL loop = TRUE;
	BOOL playing = FALSE;
	BOOL showingP = FALSE;
	while(loop) {
		Sleep(50);
		if(_kbhit())
			switch(_getch()) {
				case 'q':
					loop = 0;
					break;
				case ' ':
					TogglePlay(DISP_OBJ, &playing);
					break;
				case 's':
					GetNothingFromMethod(DISP_OBJ, Stop);
					break;
				case 'r':
					Reload(DISP_OBJ);
					break;
					//TODO: Implement MIDI Output change
				default:
					break;
			}
		GetLongFromMethod(DISP_OBJ, GetPlaybackStatus, &frame);
		if(frame == frame_prev) {
			playing = FALSE;
			if(!showingP) {
				printf(" P");
				showingP = TRUE;
			}
			continue;
		}
		else {
			playing = TRUE;
			showingP = FALSE;
		}
		frame_prev = frame;
		if(frame == -2) {
			printf("An error occured\n");
			break;
		}
		printf("\r                   \r");
		if(frame == -1)
			printf("STOPPED");
		else {
			long cf = frame % 30;
			long sec = frame / 30 % 60;
			long min = frame / (30*60) % 60;
			long hour = frame / (30*60*60);
			printf("%.2li:%.2li:%.2li.%.2li", hour, min, sec, cf);
		}
	}
	if(_kbhit())
		getch();
	printf("\nShutting down...\n");
	res = GetNothingFromMethod(DISP_OBJ, Destroy);
	STATUS("GetNothingFromMethod Destroy", res);
	DISP_OBJ->lpVtbl->Release(DISP_OBJ);
	STATUS("Release IDispatch", res);
	k->lpVtbl->Release(k);
	STATUS("Release IPersistStreamInit", res);
	OleUninitialize();
}

HRESULT Reload(IDispatch *dispatch) {
	HRESULT res;
	BOOL status;
	res = GetNothingFromMethod(dispatch, Destroy);
	STATUS("GetNothingFromMethod Destroy", res);
	res = GetBoolFromMethod(dispatch, Create, &status);
	STATUS("GetBoolFromMethod Create()", res);
#ifdef DEBUG
	printf("Control is owned by us: %s\n", status?"YES":"NO");
#endif
	Sleep(1000);
	STATUS("Sleep for 1 second", S_OK);
	return res;
}

HRESULT TogglePlay(IDispatch *dispatch, BOOL *state) {
	if(*state)
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

HRESULT GetLongFromMethod(IDispatch *dispatch, VSA_IDs dispid, LONG *out){
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