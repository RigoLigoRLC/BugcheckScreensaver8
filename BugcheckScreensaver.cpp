#include "BugcheckScreensaver.h"
#include "consts.h"
#include "scrnsave.h"
#include "resource.h"
#include <cmath>
#include <cassert>
#include <ctime>

#pragma warning(disable : 4996, "Fuck wcscat_s \"Secure\" routines")

BugcheckScreensaver::BugcheckScreensaver(HWND hWnd, HINSTANCE hInst, bool inPreviewBox) :
	m_hWnd(hWnd),
	m_hInst(hInst)
{
	FetchConfig(m_hWnd);
	m_Tick = GetTickCount();

	if (inPreviewBox) {
		m_logStr += L"In Preview\n";
		auto windowDc = GetDC(hWnd);
		m_fakeScreen = CreateCompatibleBitmap(windowDc, 1920, 1080);
		m_dc = CreateCompatibleDC(windowDc);
		SelectObject(m_dc, m_fakeScreen);

		ReleaseDC(hWnd, windowDc);
	}
	else {
		m_logStr += L"Not Preview\n";
		m_dc = GetDC(hWnd);
		SetStretchBltMode(m_dc, HALFTONE); // Set high quality scaling
		m_fakeScreen = NULL;
	}
}

BugcheckScreensaver::~BugcheckScreensaver()
{
	ReleaseDC(m_hWnd, m_dc);
	if (m_fakeScreen)
		DeleteObject(m_fakeScreen);
}

LRESULT BugcheckScreensaver::Proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
		case WM_ERASEBKGND:
		{
			if (m_cfLaggyBackgroundFill) {
				// Grab screen and fill it to screensaver's DC
				HDC hDCDesktop = GetDC(NULL), hDC = m_dc;
				int height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
				int width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
				BitBlt(hDC, 0, 0, width, height, hDCDesktop, 0, 0, SRCCOPY);
				ReleaseDC(NULL, hDCDesktop);
			}
			break;
		}

		case WM_TIMER:
		{
			// Tick event, dispatch to painters of corresponding stages
			DWORD tickNow = GetTickCount(), tickElapsed = tickNow - m_Tick;

			if (m_TaskList[m_TaskIndex] == InvalidStatus)
			{
				return TRUE; // TODO: Restart :p
			}

			if (tickElapsed > m_cfStagePreDelay[m_TaskList[m_TaskIndex]])
			{
				PainterDispatch(hWnd);
			}
		}

		default:
			return DefScreenSaverProc(hWnd, uMsg, wParam, lParam);
	}
	return TRUE;
}

void BugcheckScreensaver::FetchConfig(HWND hWnd)
{
	srand(time(0));

	RtlZeroMemory(m_cfStagePreDelay, sizeof(m_cfStagePreDelay));
	m_cfLaggyBackgroundFill = FALSE;
	m_cfCollectInfo = TRUE;
	m_cfAutoRestart = TRUE;
	m_cfBgColor = BCCOLOR_TEAL;
	m_cfProblemStr = IDS_DEVICE_PROBLEM;
	m_cfBugCodeIsCommonBugCode = TRUE;
	m_cfBugCodeIsRandom = TRUE;
	m_cfBugTextIndex = (m_cfBugCodeIsCommonBugCode ? (
		m_cfBugCodeIsRandom ? (rand() % ARRAYSIZE(CommonBugCode)) : 1
		) : (
			m_cfBugCodeIsRandom ? (rand() % ARRAYSIZE(AllBugCode)) : 9
			));

	m_cfShowLogs = FALSE;

	// Default task list
	m_TaskList[0] = PaintBackground;
	m_TaskList[1] = PaintSadFace;
	m_TaskList[2] = PaintPrompt;
	m_TaskList[3] = PaintForMoreInfo;
	m_TaskList[4] = PaintDetails;
	m_TaskList[5] = PaintQrCode;
	m_TaskList[6] = PaintCollection;
	m_TaskList[7] = InvalidStatus;

	m_cfStagePreDelay[PaintBackground] = 1200;
	m_cfStagePreDelay[PaintSadFace] = 200;
	m_cfStagePreDelay[PaintPrompt] = 40;
	m_cfStagePreDelay[PaintForMoreInfo] = 40;

	m_TaskIndex = 0;

	auto hDC = GetDC(hWnd);
	auto dpi = GetDeviceCaps(hDC, LOGPIXELSY);
	ReleaseDC(hWnd, hDC);
	m_cfScale = dpi / 96.0;
}

void BugcheckScreensaver::PainterDispatch(HWND hWnd)
{
	switch (m_TaskList[m_TaskIndex])
	{
		case InvalidStatus:
		case MaxStages:
		default:
			break;

		case PaintBackground:
			PainterBackground(hWnd);
			break;

		case PaintSadFace:
			PainterSadFace(hWnd);
			break;

		case PaintPrompt:
			PainterPrompt(hWnd);
			break;

		case PaintForMoreInfo:
			PainterForMoreInfo(hWnd);
			break;

		case PaintDetails:
			PainterDetails(hWnd);
			break;

		case PaintQrCode:
			PainterQrCode(hWnd);
			break;

		case PaintCollection:
			PainterCollection(hWnd);
			break;
	}

	// Write logs
	if (m_cfShowLogs) {
		HFONT font;
		font = CreateFontW(20, 0, 0, 0,
						   FW_NORMAL,
						   FALSE, FALSE, FALSE,
						   DEFAULT_CHARSET,
						   OUT_TT_ONLY_PRECIS,
						   CLIP_DEFAULT_PRECIS,
						   PROOF_QUALITY,
						   DEFAULT_PITCH | FF_DONTCARE,
						   L"Segoe UI");
		auto hDC = GetDC(hWnd);
		RECT cr;
		GetClientRect(hWnd, &cr);
		WCHAR TickCount[24];
		wsprintfW(TickCount, L"%08x", GetTickCount());
		SelectObject(hDC, font);
		TextOutW(hDC, 0, 0, TickCount, wcslen(TickCount));
		cr.top += 20;
		DrawTextW(hDC, m_logStr.c_str(), m_logStr.size(), &cr, DT_LEFT | DT_TOP);
		cr.top -= 20;
		DeleteObject(font);
		ReleaseDC(hWnd, hDC);
	}

	// Transfer fake screen onto preview box
	if (m_fakeScreen) {
		auto hDC = GetDC(hWnd);
		RECT cr;
		GetClientRect(hWnd, &cr);
		SetStretchBltMode(hDC, HALFTONE); // Set high quality scaling
		auto status = StretchBlt(hDC,
				   cr.left,
				   cr.top,
				   cr.right - cr.left,
				   cr.bottom - cr.top,
				   m_dc,
				   0,
				   0,
				   1920,
				   1080,
				   SRCCOPY);
		if (status == FALSE) {
			WCHAR statusHex[16];
			wsprintfW(statusHex, L"%08X\n", GetLastError());
			m_logStr += L"Xfer fail: ";
			m_logStr += statusHex;
		}
		ReleaseDC(hWnd, hDC);
	}
}

void BugcheckScreensaver::PainterBackground(HWND hWnd)
{
	if (m_cfLaggyBackgroundFill) {
		// Once called, random paint one chunk of progress, and fill last chunk solid
		if (m_nChunkX == -1 || m_nChunkY == -1) {
			// Init
			RECT cr{ 0, 0, 0, 0 };
			cr.right = GetDeviceCaps(m_dc, HORZRES);
			cr.bottom = GetDeviceCaps(m_dc, VERTRES);
			m_nChunkX = std::ceil((cr.right - cr.left) / (double)m_chunkSize);
			m_nChunkY = std::ceil((cr.bottom - cr.top) / (double)m_chunkSize);
			m_chunkBrush = CreateSolidBrush(m_cfBgColor);
		}
		RECT fr;

		// Fill last chunk
		fr.left = m_chunkX * m_chunkSize;
		fr.right = (m_chunkX + 1) * m_chunkSize;
		fr.top = m_chunkY * m_chunkSize;
		fr.bottom = (m_chunkY + 1) * m_chunkSize;
		FillRect(m_dc, &fr, m_chunkBrush);

		// Fill background finished
		if (m_chunkY >= m_nChunkY) {
			DeleteObject(m_chunkBrush);
			NextTask();
			return;
		}

		// Randomly fill next chunk
		if (++m_chunkX >= m_nChunkX) {
			m_chunkY++;
			m_chunkX = 0;
		}
		int maxSubChunks, subChunkPerChunk = maxSubChunks = m_chunkSize / m_smallChunkSize;
		maxSubChunks *= maxSubChunks;
		int subChunk = rand() % maxSubChunks;

		// Sub chunk lines
		fr.left = m_chunkX * m_chunkSize;
		fr.right = (m_chunkX + 1) * m_chunkSize;
		fr.top = m_chunkY * m_chunkSize;
		fr.bottom = m_chunkY * m_chunkSize + (subChunk / subChunkPerChunk) * m_smallChunkSize;
		FillRect(m_dc, &fr, m_chunkBrush);

		// Incomplete line of sub chunks
		fr.top = fr.bottom;
		fr.bottom += m_smallChunkSize;
		fr.right -= m_chunkSize - (subChunk % subChunkPerChunk) * m_smallChunkSize;
		FillRect(m_dc, &fr, m_chunkBrush);
	}
	else {
		// Direct fill without lag painting
		RECT rect{0, 0, 0, 0};
		rect.right = GetDeviceCaps(m_dc, HORZRES);
		rect.bottom = GetDeviceCaps(m_dc, VERTRES);
		HBRUSH blueBrush = CreateSolidBrush(m_cfBgColor);
		FillRect(m_dc, &rect, blueBrush);
		DeleteObject(blueBrush);

		NextTask();
	}
}

void BugcheckScreensaver::PainterSadFace(HWND hWnd)
{
	m_logStr += L"SadFace proc\n";
	constexpr WCHAR SadFaceText[] = L":(";
	HFONT font;
	font = CreateFontW(SadFacePx * m_cfScale, 0, 0, 0,
					   FW_NORMAL,
					   FALSE, FALSE, FALSE,
					   DEFAULT_CHARSET,
					   OUT_TT_ONLY_PRECIS,
					   CLIP_DEFAULT_PRECIS,
					   PROOF_QUALITY,
					   DEFAULT_PITCH | FF_DONTCARE,
					   L"Segoe UI");
	SetBkColor(m_dc, m_cfBgColor);

	SelectObject(m_dc, font);
	SetTextColor(m_dc, RGB(255, 255, 255));
	TextOutW(m_dc, SadFaceX * m_cfScale, SadFaceY * m_cfScale, SadFaceText, ARRAYSIZE(SadFaceText));

	DeleteObject(font);
	NextTask();
}

void BugcheckScreensaver::PainterPrompt(HWND hWnd)
{
	m_logStr += L"Prompt proc\n";
	static WCHAR PromptText[1024] = {};
	HFONT font;
	font = CreateFontW(PromptPx * m_cfScale, 0, 0, 0,
					   FW_NORMAL,
					   FALSE, FALSE, FALSE,
					   DEFAULT_CHARSET,
					   OUT_TT_ONLY_PRECIS,
					   CLIP_DEFAULT_PRECIS,
					   PROOF_QUALITY,
					   DEFAULT_PITCH | FF_DONTCARE,
					   L"Segoe UI");
	SetBkColor(m_dc, m_cfBgColor);

	// Get strings
	{
		static WCHAR concat[256] = {};
		LoadStringW(m_hInst, m_cfProblemStr, PromptText, ARRAYSIZE(PromptText));
		if (m_cfCollectInfo) {
			wcscat(PromptText, L"\r\n");
			LoadStringW(m_hInst, IDS_COLLECT_INFO, concat, ARRAYSIZE(concat));
			wcscat(PromptText, concat);
			LoadStringW(m_hInst, m_cfAutoRestart ? IDS_COLLECT_AUTO_RESTART : IDS_COLLECT_USER_RESTART, concat, ARRAYSIZE(concat));
			wcscat(PromptText, concat);
		}
		else {
			LoadStringW(m_hInst, m_cfAutoRestart ? IDS_AUTO_RESTART : IDS_USER_RESTART, concat, ARRAYSIZE(concat));
			wcscat(PromptText, concat);
		}

	}

	SelectObject(m_dc, font);

	SetTextColor(m_dc, RGB(255, 255, 255));
	RECT textRect, clientRect{ 0, 0, 0, 0 };
	clientRect.right = GetDeviceCaps(m_dc, HORZRES);
	clientRect.bottom = GetDeviceCaps(m_dc, VERTRES);
	textRect.left = m_cfScale * PromptX;
	textRect.right = m_cfScale * (clientRect.right - PromptRightMargin);
	textRect.top = m_cfScale * PromptY;
	textRect.bottom = m_cfScale * QrCodeY;

	// Should accumulate Y now
	m_AdvancingY = textRect.top;

	m_AdvancingY += DrawTextW(m_dc, PromptText, wcslen(PromptText), &textRect, DT_LEFT | DT_TOP | DT_WORDBREAK);

	DeleteObject(font);
	NextTask();
}

void BugcheckScreensaver::PainterForMoreInfo(HWND hWnd)
{
	m_logStr += L"MoreInfo proc\n";
	static WCHAR PromptText[1024] = {};
	HFONT font;
	font = CreateFontW(ForMoreInfoPx * m_cfScale, 0, 0, 0,
					   FW_NORMAL,
					   FALSE, FALSE, FALSE,
					   DEFAULT_CHARSET,
					   OUT_TT_ONLY_PRECIS,
					   CLIP_DEFAULT_PRECIS,
					   PROOF_QUALITY,
					   DEFAULT_PITCH | FF_DONTCARE,
					   L"Segoe UI");
	SetBkColor(m_dc, m_cfBgColor);

	// Get strings
	LoadStringW(m_hInst, IDS_MORE_INFO, PromptText, ARRAYSIZE(PromptText));

	SelectObject(m_dc, font);

	SetTextColor(m_dc, RGB(255, 255, 255));
	RECT textRect, clientRect{ 0, 0, 0, 0 };
	clientRect.right = GetDeviceCaps(m_dc, HORZRES);
	clientRect.bottom = GetDeviceCaps(m_dc, VERTRES);
	textRect.left = m_cfScale * ForMoreInfoX;
	textRect.right = m_cfScale * (clientRect.right - PromptRightMargin);
	textRect.top = m_cfScale * ForMoreInfoY;
	textRect.bottom = m_cfScale * clientRect.bottom;

	DrawTextW(m_dc, PromptText, wcslen(PromptText), &textRect, DT_LEFT | DT_TOP | DT_WORDBREAK);

	DeleteObject(font);
	NextTask();
}

void BugcheckScreensaver::PainterDetails(HWND hWnd)
{
	m_logStr += L"Details proc\n";
	static WCHAR PromptText[1024] = {};
	HFONT font;
	font = CreateFontW(DetailsPx * m_cfScale, 0, 0, 0,
					   FW_NORMAL,
					   FALSE, FALSE, FALSE,
					   DEFAULT_CHARSET,
					   OUT_TT_ONLY_PRECIS,
					   CLIP_DEFAULT_PRECIS,
					   PROOF_QUALITY,
					   DEFAULT_PITCH | FF_DONTCARE,
					   L"Segoe UI");
	SetBkColor(m_dc, m_cfBgColor);

	SelectObject(m_dc, font);
	
	TEXTMETRICW metrics;
	GetTextMetricsW(m_dc, &metrics);
	metrics.tmExternalLeading += 5 * m_cfScale;
	

	SetTextColor(m_dc, RGB(255, 255, 255));
	RECT textRect, clientRect{ 0, 0, 0, 0 };
	clientRect.right = GetDeviceCaps(m_dc, HORZRES);
	clientRect.bottom = GetDeviceCaps(m_dc, VERTRES);
	textRect.left = m_cfScale * DetailsX;
	textRect.right = m_cfScale * (clientRect.right - PromptRightMargin);
	textRect.top = m_cfScale * DetailsY;
	textRect.bottom = m_cfScale * clientRect.bottom;

	// If you call a support person, ...
	LoadStringW(m_hInst, IDS_CALL_SUPPORT, PromptText, ARRAYSIZE(PromptText));
	textRect.top += DrawTextW(m_dc, PromptText, wcslen(PromptText), &textRect, DT_LEFT | DT_TOP | DT_WORDBREAK);
	textRect.top += DetailsAdvance * m_cfScale;

	// Stop code: ...
	LoadStringW(m_hInst, IDS_STOPCODE, PromptText, ARRAYSIZE(PromptText));
	wcscat(PromptText, (m_cfBugCodeIsCommonBugCode ? CommonBugCode : AllBugCode)[m_cfBugTextIndex]);
	textRect.top += DrawTextW(m_dc, PromptText, wcslen(PromptText), &textRect, DT_LEFT | DT_TOP | DT_WORDBREAK);
	textRect.top += DetailsAdvance * m_cfScale;

	DeleteObject(font);
	NextTask();
}

void BugcheckScreensaver::PainterQrCode(HWND hWnd)
{
	m_logStr += L"QR proc\n";
	HBITMAP qrcode = LoadBitmapW(m_hInst, (LPCWSTR)IDB_QRCODE);
	HDC hDCQrcode = CreateCompatibleDC(m_dc);
	BITMAP qrcodeObj;
	
	GetObjectW(qrcode, sizeof(BITMAP), &qrcodeObj);
	SelectObject(hDCQrcode, qrcode);
	StretchBlt(m_dc,
			   QrCodeX * m_cfScale, QrCodeY * m_cfScale,
			   QrCodeSize * m_cfScale, QrCodeSize * m_cfScale,
			   hDCQrcode,
			   0, 0,
			   qrcodeObj.bmWidth, qrcodeObj.bmHeight,
			   SRCCOPY);

	DeleteDC(hDCQrcode);
	DeleteObject(qrcode);
	NextTask();
}

void BugcheckScreensaver::PainterCollection(HWND hWnd)
{
	NextTask();
}

void BugcheckScreensaver::NextTask()
{
	m_TaskIndex++;
	m_Tick = GetTickCount();
}


