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
	DISP_OBJ->lpVtbl->Release(DISP_OBJ);
	STATUS("Release IDispatch", res);
	k->lpVtbl->Release(k);
	STATUS("Release IPersistStreamInit", res);
	OleUninitialize();
}


HRESULT GetLongProperty(IDispatch* dispatch, VSA_IDs dispid, LONG *valueReceiver){
	HRESULT v;
	DISPPARAMS dp = {0};
	VARIANT vr;
	VariantInit(&vr);
	v = dispatch->lpVtbl->Invoke(dispatch, dispid, &IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_PROPERTYGET, &dp, &vr, NULL, NULL);
	valueReceiver = malloc(sizeof(long));
	if (V_VT(&vr) == VT_I4)
		*valueReceiver = V_I4(&vr);
	VariantClear(&vr);
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