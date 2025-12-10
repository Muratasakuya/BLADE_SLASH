#include "WinApp.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Debug/Assert.h>
#include <Engine/Core/Debug/SpdLogger.h>
#include <Engine/Config.h>

// imgui
#include "imgui.h"
#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
	HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
#pragma comment(lib,"winmm.lib")

//============================================================================
//	WinApp classMethods
//============================================================================

HWND WinApp::hwnd_ = nullptr;

void WinApp::Create() {

	timeBeginPeriod(1);
	RegisterWindowClass();

	windowRect_.right = Config::kWindowWidth;
	windowRect_.bottom = Config::kWindowHeight;

	windowStyle_ = WS_OVERLAPPEDWINDOW;

	AdjustWindowRect(&windowRect_, windowStyle_, false);

	hwnd_ = CreateWindow(
		L"WindowClass",
		Config::kWindowTitle,
		windowStyle_,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		windowRect_.right - windowRect_.left,
		windowRect_.bottom - windowRect_.top,
		nullptr,
		nullptr,
		GetModuleHandle(nullptr),
		nullptr);

	ShowWindow(hwnd_, SW_MAXIMIZE);
}

bool WinApp::ProcessMessage() {

	MSG msg{};

	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	if (msg.message == WM_QUIT) {

		return true;
	}

	return false;
}

void WinApp::SetFullscreen(bool fullscreen) {

	if (fullscreen) {

		// 現在のウィンドウ情報を保存（復元用）
		GetWindowRect(hwnd_, &windowRect_);

		// ウィンドウがあるモニターの領域を取得
		HMONITOR hMon = MonitorFromWindow(hwnd_, MONITOR_DEFAULTTONEAREST);
		MONITORINFO mi{};
		mi.cbSize = sizeof(MONITORINFO);
		GetMonitorInfo(hMon, &mi);

		// 境界線なしスタイルに変更
		SetWindowLong(hwnd_, GWL_STYLE, WS_POPUP);
		SetWindowLong(hwnd_, GWL_EXSTYLE, 0);

		// モニターサイズに合わせて最大化
		SetWindowPos(
			hwnd_,
			HWND_TOP,
			mi.rcMonitor.left,
			mi.rcMonitor.top,
			mi.rcMonitor.right - mi.rcMonitor.left,
			mi.rcMonitor.bottom - mi.rcMonitor.top,
			SWP_FRAMECHANGED | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_SHOWWINDOW
		);

		ShowWindow(hwnd_, SW_SHOWNORMAL);
		SetForegroundWindow(hwnd_);
		SetFocus(hwnd_);
	} else {

		// 通常のウィンドウスタイルに戻す
		SetWindowLong(hwnd_, GWL_STYLE, WS_OVERLAPPEDWINDOW);
		SetWindowLong(hwnd_, GWL_EXSTYLE, 0);

		// 保存していたウィンドウの位置とサイズに復元
		SetWindowPos(
			hwnd_,
			HWND_NOTOPMOST,
			windowRect_.left,
			windowRect_.top,
			windowRect_.right - windowRect_.left,
			windowRect_.bottom - windowRect_.top,
			SWP_FRAMECHANGED | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_SHOWWINDOW);

		ShowWindow(hwnd_, SW_SHOWNORMAL);
		SetForegroundWindow(hwnd_);
		SetFocus(hwnd_);
	}
}

LRESULT WinApp::WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {

	// ImGuiマウス有効
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {
		return true;
	}

	switch (msg) {
	case WM_DESTROY:

		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

void WinApp::RegisterWindowClass() {

	// Window Procedure
	WNDCLASS wc{};
	// ウィンドウプロシージャ(Window Procedure)
	wc.lpfnWndProc = WindowProc;
	// ウィンドウクラス名
	wc.lpszClassName = L"WindowClass";
	// インスタンスハンドル
	wc.hInstance = GetModuleHandle(nullptr);
	// カーソル
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

	// ウィンドウクラスを登録する
	RegisterClass(&wc);
}
