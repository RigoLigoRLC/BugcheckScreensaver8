#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

class CfgDialog
{
public:
	CfgDialog();
	~CfgDialog();

	void Show();

private:
	static LRESULT WINAPI StaticWndProc(HWND hWnd, // Dispatch to individual instances
										UINT uMsg,
										WPARAM wParam,
										LPARAM lParam);
	static LRESULT WINAPI RegisterWindowClass(HINSTANCE hInst);
	static ATOM m_wndClass;
	static HINSTANCE m_Instance;

	LRESULT CALLBACK WindowProc(UINT uMsg,
								WPARAM wParam,
								LPARAM lParam);

	HWND
		m_hWnd, // Main window handle

		m_chkRandomBugCode, // Bug code selection
		m_staticBugCode,
		m_comboBugCode,

		m_chkQrCode, // QR Code

		m_radioBlueScreen,
		m_radioGreenScreen,
		m_radioBlackScreen,
		m_radioDarkBlueScreen,
		m_radio;
};
