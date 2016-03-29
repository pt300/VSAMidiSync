//
// Created by patryk on 25.03.16.
//

#define INITGUID 1 //force define of GUIDs
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
	res = SetStringProperty(DISP_OBJ, vsaPath, VSA_PATH);
	STATUS("SetStringProperty", res);
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



HRESULT SetStringProperty(IDispatch* dispatch, VSA_IDs dispid, wchar_t *valueSet) {
	HRESULT v;
	VARIANT *varValue = malloc(sizeof(VARIANT));
	VariantInit(varValue);
	BSTR str = SysAllocString(valueSet);
	varValue->vt = VT_BSTR;
	varValue->bstrVal = str;
	DISPPARAMS dp = {0};
	dp.cArgs = 1;
	dp.rgvarg = varValue;
	dp.cNamedArgs = 1;
	DISPID *dispidNamed = malloc(sizeof(DISPID));
	*dispidNamed = DISPID_PROPERTYPUT;
	dp.rgdispidNamedArgs = dispidNamed;
	v = dispatch->lpVtbl->Invoke(dispatch, dispid, &IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_PROPERTYPUT, &dp, NULL, NULL, NULL);
	VariantClear(varValue);
	free(varValue);
	free(dispidNamed);
	return v;
}