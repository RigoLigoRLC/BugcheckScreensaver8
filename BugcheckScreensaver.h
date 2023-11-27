#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>

class BugcheckScreensaver
{
public:
	BugcheckScreensaver(HWND hWnd, HINSTANCE hInst, bool inPreviewBox);
	~BugcheckScreensaver();
	LRESULT Proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
	void FetchConfig(HWND hWnd);
	void PainterDispatch(HWND hWnd);

	void PainterBackground(HWND hWnd);
	void PainterSadFace(HWND hWnd);
	void PainterPrompt(HWND hWnd);
	void PainterForMoreInfo(HWND hWnd);
	void PainterDetails(HWND hWnd);
	void PainterQrCode(HWND hWnd);
	void PainterCollection(HWND hWnd);

	void NextTask();

	HWND m_hWnd;
	HINSTANCE m_hInst;

	// State
	HDC m_dc; // The final place where the painted stuff goes
	HBITMAP m_fakeScreen; // Fake screen we first paint into when running in preview
	enum DrawStatus {
		InvalidStatus = -1,

		PaintBackground = 0,
		PaintSadFace,
		PaintPrompt,
		PaintForMoreInfo,
		PaintDetails,
		PaintQrCode,
		PaintCollection,
		AwaitRestart,

		MaxStages
	} m_Status;
	DrawStatus m_TaskList[MaxStages];
	int m_TaskIndex = 0;
	DWORD m_Tick;
	UINT m_CollectInfoTextY = 0;
	UINT m_AdvancingY = 0;
	std::wstring m_logStr;
	

	// Config
	int m_cfStagePreDelay[MaxStages];
	BOOL m_cfLaggyBackgroundFill;
	float m_cfScale;
	COLORREF m_cfBgColor;
	UINT m_cfProblemStr;
	int m_cfBugTextIndex;
	BOOL m_cfBugCodeIsCommonBugCode;
	BOOL m_cfBugCodeIsRandom;
	BOOL m_cfShowLogs;

	BOOL m_cfAutoRestart, m_cfCollectInfo;


	// Background painter
	HBRUSH m_chunkBrush = NULL;
	int m_chunkX = 0, m_chunkY = 0, m_smallChunkSize = 16, m_chunkSize = 8 * m_smallChunkSize, m_nChunkX = -1, m_nChunkY = -1;
};

