#include "callbacks.h"
#include "resource.h"
#include "MIDI.h"

// Window procedure for our main window.
LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	static HINSTANCE hInstance;
	static HDC hdcStatic = NULL;

	switch(msg) {
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case ID_HELP_ABOUT:
					DialogBox(hInstance, MAKEINTRESOURCE(IDD_ABOUTDIALOG), hWnd, &AboutDialogProc);
					return 0;

				case ID_FILE_EXIT:
					DestroyWindow(hWnd);
					return 0;
				default:
					PostMessage(NULL, msg, wParam, lParam);
					break;
			}
			break;

//		case WM_SYSCOMMAND:
//			switch(LOWORD(wParam)) {
//				case ID_HELP_ABOUT:
//					DialogBox(hInstance, MAKEINTRESOURCE(IDD_ABOUTDIALOG), hWnd, &AboutDialogProc);
//					return 0;
//
//			}
//			break;

		case WM_CREATE:
			hInstance = ((LPCREATESTRUCT) lParam)->hInstance;
			return 0;

		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

		case WM_CTLCOLORSTATIC:
			hdcStatic = (HDC) wParam;
			SetTextColor(hdcStatic, RGB(0, 0, 0));
			SetBkMode(hdcStatic, TRANSPARENT);
			return (LRESULT) GetStockObject(NULL_BRUSH);
		default:break;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

// Dialog procedure for our "about" dialog.
INT_PTR CALLBACK AboutDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch(uMsg) {
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDOK:
				case IDCANCEL:
					EndDialog(hwndDlg, (INT_PTR) LOWORD(wParam));
					return (INT_PTR) TRUE;
			}
			break;
		case WM_INITDIALOG:
			return (INT_PTR) TRUE;
	}

	return (INT_PTR) FALSE;
}

INT_PTR CALLBACK MIDIChooserProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	HWND control;
	LRESULT pos;
	LPWSTR *dev;

	switch(uMsg) {
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case ID_REFRESH:
					control = GetDlgItem(hwndDlg, ID_DEVICELIST);
					SendMessage(control, LB_RESETCONTENT, 0, 0);
					if(!MIDI_listDevices(control)) {
						control = GetDlgItem(hwndDlg, IDOK);
						EnableWindow(control, FALSE);
						control = GetDlgItem(hwndDlg, ID_MIDI_DEF);
						EnableWindow(control, FALSE);
					}
					else {
						SendMessage(control, LB_SETCURSEL, 0, 0);
						control = GetDlgItem(hwndDlg, IDOK);
						EnableWindow(control, TRUE);
						control = GetDlgItem(hwndDlg, ID_MIDI_DEF);
						EnableWindow(control, TRUE);
					}
					break;
				case ID_MIDI_DEF:
					control = GetDlgItem(hwndDlg, ID_DEVICELIST);
					pos = SendMessage(control, LB_GETCURSEL, 0, 0);
					dev = malloc(32 * sizeof(* dev));
					if(SendMessage(control, LB_GETTEXT, (WPARAM) pos, (LPARAM) dev) != LB_ERR) {
						PostMessage(NULL, DEF_CANCER, 0, (LPARAM) dev);
					}
				case IDOK:
					control = GetDlgItem(hwndDlg, ID_DEVICELIST);
					pos = SendMessage(control, LB_GETCURSEL, 0, 0);
					if(pos != LB_ERR) {
						pos = SendMessage(control, LB_GETITEMDATA, (WPARAM) pos, 0);
						if(pos != LB_ERR) {
							PostMessage(NULL, CANCER, 0, (LPARAM) pos);
						}
					}
				case IDCANCEL:
					EndDialog(hwndDlg, (INT_PTR) LOWORD(wParam));
					return (INT_PTR) TRUE;
				default:break;
			}
			break;
		case WM_INITDIALOG:
			control = GetDlgItem(hwndDlg, ID_DEVICELIST);
			if(!MIDI_listDevices(control)) {
				control = GetDlgItem(hwndDlg, IDOK);
				EnableWindow(control, FALSE);
				control = GetDlgItem(hwndDlg, ID_MIDI_DEF);
				EnableWindow(control, FALSE);
			}
			else {
				SendMessage(control, LB_SETCURSEL, 0, 0);
			}
			return (INT_PTR) TRUE;
		default:break;
	}

	return (INT_PTR) FALSE;
}