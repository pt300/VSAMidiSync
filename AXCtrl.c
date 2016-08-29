/*
 * Created by patryk on 23.06.16.
 */

#include "AXCtrl.h"

HRESULT AX_init(void) {
	HRESULT res;

	res = OleInitialize(NULL);

	return res <= S_FALSE ? S_OK : res;
}

void AX_deinit(void) {
	OleUninitialize();
}

HRESULT AX_getControl(AX_object *obj, LPCOLESTR name) {
	HRESULT res;
	CLSID id;
	/*TODO: error specific to init step*/
	if((res = CLSIDFromProgID(name, &id)) ||
	   (res = CoCreateInstance(&id, NULL, CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER, &IID_IDispatch, (void **) &obj->dispatch)) ||
	   (res = obj->dispatch->lpVtbl->QueryInterface(obj->dispatch, &IID_IPersistStreamInit, (void **) &obj->stream)) ||
	   (res = obj->stream->lpVtbl->InitNew(obj->stream))) {
		return res;
	}

	return S_OK;
}

HRESULT AX_destroyControl(AX_object *obj) {
	HRESULT res;

	if((res = obj->stream->lpVtbl->Release(obj->stream)) ||
	   (res = obj->dispatch->lpVtbl->Release(obj->dispatch))) {
		return res;
	}

	return S_OK;
}

/*TODO: enum with return codes*/
HRESULT AX_callMethod(AX_object *obj, OLECHAR *name, enum VARENUM ret_type, void *ret_val, ...) {
	va_list args;
	enum VARENUM type;
	VARIANT *var, tmp, ret;
	DISPPARAMS params;
	size_t allocated, length, i;
	HRESULT res;
	DISPID id;

	res = obj->dispatch->lpVtbl->GetIDsOfNames(obj->dispatch, &IID_NULL, &name, 1, LOCALE_SYSTEM_DEFAULT, &id);

	if(res) {
		return res;
	}

	length = 0;
	allocated = 5;
	var = calloc(allocated, sizeof *var);

	va_start(args, ret_val);

	while((type = va_arg(args, enum VARENUM))) {
		var[length].vt = type;
		switch(type) {
			case VT_I1:
				var[length++].cVal = (CHAR)va_arg(args, int);
				break;
			case VT_I2:
				var[length++].iVal = (SHORT)va_arg(args, int);
				break;
			case VT_I4:
				var[length++].lVal = va_arg(args, LONG);
				break;
			case VT_I8:
				var[length++].llVal = va_arg(args, LONGLONG);
				break;
			case VT_UI1:
				var[length++].bVal = (BYTE)va_arg(args, int);
				break;
			case VT_UI2:
				var[length++].uiVal = (USHORT)va_arg(args, int);
				break;
			case VT_UI4:
				var[length++].ulVal = va_arg(args, ULONG);
				break;
			case VT_UI8:
				var[length++].ullVal = va_arg(args, ULONGLONG);
				break;
			case VT_R4:
				var[length++].fltVal = (float)va_arg(args, double);
				break;
			case VT_R8:
				var[length++].dblVal = va_arg(args, DOUBLE);
				break;
			case VT_BSTR:
				var[length++].bstrVal = SysAllocString(va_arg(args, LPWSTR));
				break;
			case VT_BOOL:
				var[length++].boolVal = va_arg(args, BOOL) ? TRUE : FALSE;
				break;
			default:
				while(length--) {
					VariantClear(var + length * sizeof *var);
				}
				free(var);
				return -1;
		}
		if(length == allocated) {
			allocated += 5;
			var = realloc(var, allocated);
		}
	}

	va_end(args);

	for(i = 0; i < length / 2; i++) {
		tmp = var[i];
		var[i] = var[length - 1 - i];
		var[length - 1 - i] = tmp;
	}

	params.cArgs = length;
	params.cNamedArgs = 0;
	params.rgvarg = var;

	res = obj->dispatch->lpVtbl->Invoke(obj->dispatch, id, &IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_METHOD, &params,
										&ret, NULL, NULL);

	while(length--) {
		VariantClear(var + length * sizeof *var);
	}
	free(var);

	if(res) {
		return res;
	}

	switch(ret_type) {
		case VT_NULL:
		case VT_EMPTY:
			VariantClear(&ret);
			break;
		case VT_I1:
			*(CHAR *)ret_val = ret.cVal;
			break;
		case VT_I2:
			*(SHORT *)ret_val = ret.iVal;
			break;
		case VT_I4:
			*(LONG *)ret_val = ret.lVal;
			break;
		case VT_I8:
			*(LONGLONG *)ret_val = ret.llVal;
			break;
		case VT_UI1:
			*(BYTE *)ret_val = ret.bVal;
			break;
		case VT_UI2:
			*(USHORT *)ret_val = ret.uiVal;
			break;
		case VT_UI4:
			*(ULONG *)ret_val = ret.ulVal;
			break;
		case VT_UI8:
			*(ULONGLONG *)ret_val = ret.ullVal;
			break;
		case VT_R4:
			*(FLOAT *)ret_val = ret.fltVal;
			break;
		case VT_R8:
			*(DOUBLE *)ret_val = ret.dblVal;
			break;
		case VT_BSTR:
			*(BSTR *)ret_val = ret.bstrVal;
			break;
		case VT_BOOL:
			*(BOOL *)ret_val = ret.boolVal;
			break;
		default:
			return -3;
	}

	return S_OK;
}

HRESULT AX_setValue(AX_object *obj, OLECHAR *name, enum VARENUM type, void *data) {
	VARIANT var;
	DISPPARAMS params;
	DISPID id, disp;
	HRESULT res;

	res = obj->dispatch->lpVtbl->GetIDsOfNames(obj->dispatch, &IID_NULL, &name, 1, LOCALE_SYSTEM_DEFAULT, &id);

	if(res) {
		return res;
	}

	var.vt = type;

	switch(type) {
		case VT_I1:
			var.cVal = *(CHAR *)data;
			break;
		case VT_I2:
			var.iVal = *(SHORT *)data;
			break;
		case VT_I4:
			var.lVal = *(LONG *)data;
			break;
		case VT_I8:
			var.llVal = *(LONGLONG *)data;
			break;
		case VT_UI1:
			var.bVal = *(BYTE *)data;
			break;
		case VT_UI2:
			var.uiVal = *(USHORT *)data;
			break;
		case VT_UI4:
			var.ulVal = *(ULONG *)data;
			break;
		case VT_UI8:
			var.ullVal = *(ULONGLONG *)data;
			break;
		case VT_R4:
			var.fltVal = *(FLOAT *)data;
			break;
		case VT_R8:
			var.dblVal = *(DOUBLE *)data;
			break;
		case VT_BSTR:
			var.bstrVal = SysAllocString((LPWSTR)data);
			break;
		case VT_BOOL:
			var.boolVal = *(BOOL *)data ? TRUE : FALSE;
			break;
		default:
			return -1;
	}

	disp = DISPID_PROPERTYPUT;

	params.cArgs = 1;
	params.rgvarg = &var;
	params.cNamedArgs = 1;
	params.rgdispidNamedArgs = &disp;

	res = obj->dispatch->lpVtbl->Invoke(obj->dispatch, id, &IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_PROPERTYPUT, &params, NULL, NULL, NULL);

	VariantClear(&var);

	return res;
}