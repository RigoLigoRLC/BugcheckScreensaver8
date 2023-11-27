
#include "consts.h"
#include "BugcheckScreensaver.h"
#include "windows.h"
#include "scrnsave.h"
#include "stdlib.h"
#include "math.h"

UINT uTimer;
BugcheckScreensaver* inst = nullptr;

LRESULT WINAPI ScreenSaverProc(
	HWND   hWnd,
	UINT   message,
	WPARAM wParam,
	LPARAM lParam
)
{
	switch (message)
	{
		case WM_CREATE:
		{
			auto cs = (LPCREATESTRUCTW)lParam;
			bool inPreviewBox = true;

			// If screensaver is running in preview box, do not hide cursor,
			// and should also use a zoomed fake screen DC
			if (!(cs->style & WS_CHILDWINDOW))
			{
				ShowCursor(FALSE);
				inPreviewBox = false;
			}

			inst = new BugcheckScreensaver(hWnd, GetModuleHandleW(NULL), inPreviewBox);
			uTimer = SetTimer(hWnd, 1, 20, NULL); // 20ms Event

			break;
		}

		// Ignored mouse events
		case WM_MOUSEMOVE:
		case WM_MBUTTONDOWN:
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_KEYUP:
			break;

		// Intended Exit Path
		case WM_KEYDOWN:
			if (wParam == VK_ESCAPE)
				PostQuitMessage(0);
			break;

		case WM_DESTROY:
			KillTimer(hWnd, uTimer);
			delete inst;
			break;

		default:
			if (inst)
				return inst->Proc(hWnd, message, wParam, lParam);
			else
				return DefScreenSaverProc(hWnd, message, wParam, lParam);
	}
	return TRUE;
}

BOOL WINAPI ScreenSaverConfigureDialog(
	HWND hDlg,
	UINT message,
	WPARAM wParam,
	LPARAM lParam
)
{
	return FALSE;
}

BOOL WINAPI RegisterDialogClasses(
	HANDLE hInst
)
{
	return TRUE;
}
