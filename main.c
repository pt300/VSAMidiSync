//
// Created by patryk on 25.03.16.
//

#include <ocidl.h>
#include "main.h"
DEFINE_GUID(IID_VSA, 0xD69EBDF7, 0x494B, 0x11D5, 0x8D, 0x2D, 0x00, 0x20, 0x78, 0x15, 0x1F, 0x21);
int main(void) {
	HRESULT res;
	res = OleInitialize(NULL);
	if(res != S_OK && res != S_FALSE) {
		printf("OleInitialize() failed... 0x%lX\n", res);
		exit(0);
	}
	printf("Success...\nCreating instance\n");
	IDispatch *DISP_OBJ;
	res = CoCreateInstance(&IID_VSA, NULL, CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER, &IID_IDispatch, (void **)&DISP_OBJ);
	if(res != S_OK) {
		printf("Welll... we failed 0x%lX\n", res);
	}
	else {
		printf("Success.\n");
		HRESULT h = SetStringProperty(DISP_OBJ, vsaPath, VSA_PATH);
		printf("Set string finished ");
		if(h == S_OK)
			printf("successfully\n");
		else
			printf("rip 0x%lx\n", h);
		DISP_OBJ->lpVtbl->Release(DISP_OBJ);
	}
	OleUninitialize();
}


HRESULT GetLongProperty(IDispatch* dispatch, DISPID dispid, LONG *valueReceiver){
	HRESULT v;
	DISPPARAMS dp = {0};
	VARIANT vr;
	VariantInit(&vr);
	UINT nErrArg;
	v = dispatch->lpVtbl->Invoke(dispatch, dispid, &IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_PROPERTYGET, &dp, &vr, NULL, &nErrArg);
	valueReceiver = malloc(sizeof(long));
	if (V_VT(&vr) == VT_I4)
		*valueReceiver = V_I4(&vr);
	VariantClear(&vr);
	return v;
}

HRESULT SetStringProperty(IDispatch* dispatch, DISPID dispid, wchar_t *valueSet) {
	HRESULT v;
	VARIANT varValue;
	VariantInit(&varValue);
	V_VT(&varValue) = VT_BSTR;
	V_BSTR(&varValue) = SysAllocString(valueSet);
	DISPPARAMS dp = {0};
	dp.cArgs = 1;
	dp.rgvarg = &varValue;
	VARIANT vr;
	VariantInit(&vr);
	UINT nErrArg;
	v = dispatch->lpVtbl->Invoke(dispatch, dispid, &IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_PROPERTYPUT, &dp, &vr, NULL, &nErrArg);
	VariantClear(&varValue);
	VariantClear(&vr);
	return v;
}