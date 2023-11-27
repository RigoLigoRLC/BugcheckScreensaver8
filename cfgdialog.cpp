
#include "cfgdialog.h"

#include <cassert>

ATOM CfgDialog::m_wndClass = NULL;
HINSTANCE CfgDialog::m_Instance = NULL;

CfgDialog::CfgDialog()
{
	if (!m_wndClass)
		return;
	m_hWnd = CreateWindowW(reinterpret_cast<LPCWSTR>(m_wndClass),
						   L"BugCheck Screensaver Configuration",
						   WS_OVERLAPPEDWINDOW,
						   CW_USEDEFAULT, CW_USEDEFAULT,
						   500, 300,
						   NULL, NULL, m_Instance,
						   this);
	assert(m_hWnd);
	ShowWindow(m_hWnd, SW_SHOWNORMAL);
}

LRESULT
CALLBACK
CfgDialog::StaticWndProc(
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
)
{
	constexpr static LPCWSTR winInstanceProp = L"WindowObjectInstance";

	if (uMsg == WM_CREATE)
	{
		CREATESTRUCTW* cs = (CREATESTRUCTW*)lParam;
		SetPropW(hWnd, winInstanceProp, cs->lpCreateParams);
	}

	CfgDialog* inst = static_cast<CfgDialog*>(GetPropW(hWnd, winInstanceProp));
	if (inst)
		return inst->WindowProc(uMsg, wParam, lParam);
	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

LRESULT
WINAPI
CfgDialog::RegisterWindowClass(HINSTANCE hInst)
{
	if (!m_Instance)
	{
		m_Instance = hInst;
	}
	if (!m_wndClass)
	{
		WNDCLASSW wndClass;
		wndClass.hInstance = hInst;
		wndClass.lpfnWndProc = &CfgDialog::StaticWndProc;
		wndClass.lpszClassName = L"BugCheckScreenSaver8_ConfigDialogClass";
		wndClass.hIcon = NULL;

		m_wndClass = RegisterClassW(&wndClass);
		assert(m_wndClass);
	}
	return GetLastError();
}

LRESULT CALLBACK CfgDialog::WindowProc(UINT uMsg,
									   WPARAM wParam,
									   LPARAM lParam)
{
	return DefWindowProcW(m_hWnd, uMsg, wParam, lParam);
	switch (uMsg)
	{
	case WM_CREATE:
		{

		break;
		}
	}
}

