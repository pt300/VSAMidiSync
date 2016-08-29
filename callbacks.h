/*
 * Created by someone smoe time ago
 * geez
 */

#ifndef CALLBACKS_H
#define CALLBACKS_H

#include <windows.h>
#include <commctrl.h>

WNDPROC ListViewOriginalProc; //this is horrible
HWND tlist; //ehhhhhh

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK AboutDialogProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK MIDIChooserProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ListViewProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK AddSubtrackProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#endif //CALLBACKS_H