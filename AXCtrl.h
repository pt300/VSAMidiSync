/*
 * Created by patryk on 23.06.16.
 */

#ifndef AXCTRL_H
#define AXCTRL_H

#include <windows.h>
#include <ocidl.h>

typedef struct {
	IPersistStreamInit *stream;
	IDispatch *dispatch;
} AX_object;

HRESULT AX_init(void);
void AX_deinit(void);
HRESULT AX_getControl(AX_object *obj, LPCOLESTR name);
HRESULT AX_destroyControl(AX_object *obj);
HRESULT AX_callMethod(AX_object *obj, OLECHAR *name, enum VARENUM ret_type, void *ret_val, ...) __attribute__((__sentinel__));
HRESULT AX_setValue(AX_object *obj, OLECHAR *name, enum VARENUM type, void *var);

#endif //AXCTRL_H
