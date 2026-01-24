#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Input/InputStructures.h>
#include <Engine/Editor/UI/Runtime/UIUserWidget.h>

namespace SakuEngine {

	//============================================================================
	//	UIApplication class
	//	UIウィジェットのルート管理、更新を行うクラス
	//============================================================================
	class UIApplication {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		UIApplication() = default;
		~UIApplication() = default;

		// 更新
		void Update(float deltaTime);

		//--------- accessor -----------------------------------------------------

		// ユーザーウィジェット設定
		void SetUserWidget(UIUserWidget* userWidget) { userWidget_ = userWidget; }

		// 入力ビューエリア設定
		void SetViewArea(InputViewArea area) { viewArea_ = area; }
		// 矩形設定
		void SetViewportRect(const UIRect& rect);
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		// 入力ビューエリア
		InputViewArea viewArea_ = InputViewArea::Game;

		// ユーザーウィジェット
		UIUserWidget* userWidget_{};

		// 矩形サイズ
		UIRect viewportRect_{ Vector2::AnyInit(0.0f), Vector2(Config::kWindowWidthf, Config::kWindowHeightf) };
		// 前回のマウス位置
		Vector2 prevMousePos_ = Vector2::AnyInit(0.0f);

		// キャプチャー、フォーカスウィジェット
		UISlateWidget* captureWidget_ = nullptr;
		UISlateWidget* focusWidget_ = nullptr;

		//--------- functions ----------------------------------------------------

		// rootからcachedRectを確定し、panelは子へ再帰
		void Arrange();
		//深い子を優先してヒットテスト
		UISlateWidget* HitTest(UISlateWidget* widget, const Vector2& pos);
		// ポインターイベント配信
		void DispatchPointerEvent(const UIPointerEvent& event);
	};
} // SakuEngine