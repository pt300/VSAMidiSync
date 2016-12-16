#include <stdio.h>
#include <windowsx.h>
#include "callbacks.h"
#include "resource.h"
#include "MIDI.h"
#include "util.h"

BOOLEAN dragging;
HIMAGELIST imageList;

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	static HINSTANCE hInstance;
	static HDC hdcStatic = NULL;
	INT pos, i, r;
	POINT p;
	LV_ITEM lvi;
	LPWSTR buf;
	LVHITTESTINFO lvhti;

	switch(msg) {
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case ID_HELP_ABOUT:
					DialogBox(hInstance, MAKEINTRESOURCE(IDD_ABOUTDIALOG), hWnd, &AboutDialogProc);
					return 0;
				default:
					PostMessage(NULL, msg, wParam, lParam);
					break;
			}
			break;
		case WM_MOUSEMOVE:
			if(!dragging)
				break;

			p.x = LOWORD(lParam);
			p.y = HIWORD(lParam);

			ClientToScreen(hWnd, &p);
			ImageList_DragMove(p.x, p.y);
			break;
		case WM_LBUTTONUP:
			if(!dragging)
				break;

			dragging = FALSE;
			ImageList_DragLeave(tlist);
			ImageList_EndDrag();
			ImageList_Destroy(imageList);

			ReleaseCapture();

			lvhti.pt.x = LOWORD(lParam);
			lvhti.pt.y = HIWORD(lParam);
			ClientToScreen(hWnd, &lvhti.pt);
			ScreenToClient(tlist, &lvhti.pt);
			ListView_HitTest(tlist, &lvhti);

			if(lvhti.iItem <= 0 ||
			   (lvhti.flags & (LVHT_ONITEMLABEL | LVHT_ONITEMSTATEICON)) == 0)
				break;

			lvi.iItem = lvhti.iItem;
			lvi.iSubItem = 0;
			lvi.mask = LVIF_STATE;
			lvi.stateMask = LVIS_SELECTED;
			ListView_GetItem(tlist, &lvi);

			if(lvi.state & LVIS_SELECTED)
				break;

			pos = ListView_GetNextItem(tlist, -1, LVNI_SELECTED);
			if(pos != -1) {
				buf = malloc(52 * sizeof *buf);

				lvi.iItem = pos;
				lvi.iSubItem = 0;
				lvi.cchTextMax = 51;
				lvi.pszText = buf;
				lvi.stateMask = (UINT) ~LVIS_SELECTED;
				lvi.mask = LVIF_STATE | LVIF_IMAGE
						   | LVIF_INDENT | LVIF_PARAM | LVIF_TEXT;
				ListView_GetItem(tlist, &lvi);
				lvi.iItem = lvhti.iItem;
				r = ListView_InsertItem(tlist, &lvi);

				if(r <= pos)
					pos++;

				for(i = 1; i < COL_LEN; i++) {
					ListView_GetItemText(tlist, pos, i, buf, 51);
					ListView_SetItemText(tlist, r, i, buf);
				}

				ListView_DeleteItem(tlist, pos);
				free(buf);
				PostMessage(NULL, SWAP_STRACKS, (WPARAM) --lvhti.iItem, (LPARAM) --pos);
			}
			break;
		case WM_NOTIFY:
			if(((LPNMHDR) lParam)->code == HDN_BEGINTRACK) {
				return TRUE;
			}
			if(wParam == ID_TRACKS) {
				switch(((LPNMHDR) lParam)->code) {
					case LVN_ITEMACTIVATE:
						break;
					case LVN_BEGINDRAG:
						p.x = 8;
						p.y = 8;

						pos = ListView_GetNextItem(tlist, -1, LVNI_SELECTED);
						if(pos > 0) {
							imageList = ListView_CreateDragImage(tlist, pos, &p);
							ImageList_BeginDrag(imageList, 0, 0, 0);
							p = ((NM_LISTVIEW *) ((LPNMHDR) lParam))->ptAction;
							ClientToScreen(tlist, &p);
							ImageList_DragEnter(GetDesktopWindow(), p.x, p.y);
							dragging = TRUE;

							SetCapture(hWnd);
						}
						break;
					default:
						PostMessage(NULL, msg, wParam, lParam);
						break;
				}
			}
			else {
				PostMessage(NULL, msg, wParam, lParam);
			}
			break;

		case WM_CLOSE:
			if(!AreThereAnyFuckingChanges()) {
				DestroyWindow(hWnd);
			}
			else {
				switch(MessageBox(hWnd,
								  TEXT("There are unsaved changes in tracks list.\n Would you like to save them now?"),
								  TEXT("Unsaved changes"), MB_ICONWARNING | MB_YESNOCANCEL)) {
					case IDYES:
						PostMessage(NULL, WM_COMMAND, ID_FILE_SAVE_T, 1);
					case IDCANCEL:
						return 0;
					case IDNO:
						DestroyWindow(hWnd);
						break;
				}
			}
			break;
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
		default:
			break;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

INT_PTR CALLBACK AboutDialogProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	HDC screen;
	INT cx, cy;

	switch(msg) {
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDOK:
				case IDCANCEL:
					EndDialog(hWnd, (INT_PTR) LOWORD(wParam));
					return (INT_PTR) TRUE;
				case 40066:
					screen = GetDC(NULL);
					cx = GetSystemMetrics(SM_CXSCREEN);
					cy = GetSystemMetrics(SM_CYSCREEN);

					StretchBlt(screen, cx * 2 / 100, cy * 2 / 100, cx * 96 / 100, cy * 96 / 100, screen, 0, 0, cx, cy,
							   SRCCOPY);

					ReleaseDC(NULL, screen);
			}
			break;
		case WM_INITDIALOG:
			return (INT_PTR) TRUE;
	}

	return (INT_PTR) FALSE;
}

INT_PTR CALLBACK MIDIChooserProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	HWND control;
	LRESULT pos;
	LPWSTR *dev;

	switch(msg) {
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case ID_REFRESH:
					control = GetDlgItem(hWnd, ID_DEVICELIST);
					SendMessage(control, LB_RESETCONTENT, 0, 0);
					if(!MIDI_listDevices(control)) {
						control = GetDlgItem(hWnd, IDOK);
						EnableWindow(control, FALSE);
						control = GetDlgItem(hWnd, ID_MIDI_DEF);
						EnableWindow(control, FALSE);
					}
					else {
						SendMessage(control, LB_SETCURSEL, 0, 0);
						control = GetDlgItem(hWnd, IDOK);
						EnableWindow(control, TRUE);
						control = GetDlgItem(hWnd, ID_MIDI_DEF);
						EnableWindow(control, TRUE);
					}
					break;
				case ID_MIDI_DEF:
					control = GetDlgItem(hWnd, ID_DEVICELIST);
					pos = SendMessage(control, LB_GETCURSEL, 0, 0);
					dev = malloc(32 * sizeof(*dev));
					if(SendMessage(control, LB_GETTEXT, (WPARAM) pos, (LPARAM) dev) != LB_ERR) {
						PostMessage(NULL, DEF_CANCER, 0, (LPARAM) dev);
					}
				case IDOK:
					control = GetDlgItem(hWnd, ID_DEVICELIST);
					pos = SendMessage(control, LB_GETCURSEL, 0, 0);
					if(pos != LB_ERR) {
						pos = SendMessage(control, LB_GETITEMDATA, (WPARAM) pos, 0);
						if(pos != LB_ERR) {
							PostMessage(NULL, CANCER, 0, (LPARAM) pos);
						}
					}
				case IDCANCEL:
					EndDialog(hWnd, (INT_PTR) LOWORD(wParam));
					return (INT_PTR) TRUE;
				default:
					break;
			}
			break;
		case WM_INITDIALOG:
			control = GetDlgItem(hWnd, ID_DEVICELIST);
			if(!MIDI_listDevices(control)) {
				control = GetDlgItem(hWnd, IDOK);
				EnableWindow(control, FALSE);
				control = GetDlgItem(hWnd, ID_MIDI_DEF);
				EnableWindow(control, FALSE);
			}
			else {
				SendMessage(control, LB_SETCURSEL, 0, 0);
			}
			return (INT_PTR) TRUE;
		default:
			break;
	}

	return (INT_PTR) FALSE;
}

INT_PTR CALLBACK ListViewHeaderProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch(msg) {
		/*
		 * Stop the header from changing to the "change column width mouse cursor"
		 * Without this it will look like the user can resize, but he won't be able to do it
		 */
		case WM_SETCURSOR:
			return TRUE;
			/*
			 * Stop the user from resizing by double clicking on the header
			 */
		case WM_LBUTTONDBLCLK:
			return 0;
		default:
			break;
	}

	/*
	 * Send all other messages to the ListViews original WndProc - THIS IS IMPORTANT
	 */
	return CallWindowProc(ListViewHeaderOriginalProc, hWnd, msg, wParam, lParam);
}

INT_PTR CALLBACK AddSubtrackProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	subtrack *t;

	switch(msg) {
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDOK:
					t = malloc(sizeof *t);
					if(Edit_GetTextLength(GetDlgItem(hWnd, IDI_TEXT_NAME)) == 0) {
						MessageBox(hWnd, TEXT("Length of name can't be zero."),
								   TEXT("Add track"), MB_OK | MB_ICONERROR);
						free(t);
						break;
					}
					if(Edit_GetTextLength(GetDlgItem(hWnd, IDI_TEXT_START)) == 0) {
						t->start = 0;
					}
					else {
						Edit_GetText(GetDlgItem(hWnd, IDI_TEXT_START), t->name, 8);
						swscanf(t->name, L"%li", &t->start);
						if(t->start >= MIDI_TOP) {
							MessageBox(hWnd, TEXT("Start frame must be smaller than 3347999."),
									   TEXT("Add track"), MB_OK | MB_ICONERROR);
							free(t);
							break;

						}
					}
					if(Edit_GetTextLength(GetDlgItem(hWnd, IDI_TEXT_STOP)) == 0) {
						t->stop = 0;
					}
					else {
						Edit_GetText(GetDlgItem(hWnd, IDI_TEXT_STOP), t->name, 8);
						swscanf(t->name, L"%li", &t->stop);
						if(t->start >= MIDI_TOP) {
							MessageBox(hWnd, TEXT("Stop frame must be smaller than 3348000."),
									   TEXT("Add track"), MB_OK | MB_ICONERROR);
							free(t);
							break;
						}
					}
					if(t->start >= t->stop) {
						MessageBox(hWnd, TEXT("Start frame must be smaller than stop frame."),
								   TEXT("Add track"), MB_OK | MB_ICONERROR);
						free(t);
						break;
					}
					Edit_GetText(GetDlgItem(hWnd, IDI_TEXT_NAME), t->name, S_NAMELEN + 1);
					PostMessage(NULL, ADD_STRACK, 0, (LPARAM) t);
				case IDCANCEL:
					EndDialog(hWnd, (INT_PTR) LOWORD(wParam));
					break;
			}
			break;
		case WM_INITDIALOG:
			Edit_LimitText(GetDlgItem(hWnd, IDI_TEXT_NAME), S_NAMELEN);
			Edit_LimitText(GetDlgItem(hWnd, IDI_TEXT_START), 7);
			Edit_LimitText(GetDlgItem(hWnd, IDI_TEXT_STOP), 7);
			return (INT_PTR) TRUE;
		default:
			break;
	}

	return (INT_PTR) FALSE;
}