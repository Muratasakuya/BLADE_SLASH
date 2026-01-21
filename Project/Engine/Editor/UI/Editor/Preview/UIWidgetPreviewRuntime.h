#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Editor/Preview/UIWidgetCompiler.h>
#include <Engine/Editor/UI/Runtime/UIUserWidget.h>
#include <Engine/Editor/UI/Runtime/UIApplication.h>
#include <Engine/MathLib/Vector2.h>

// c++
#include <memory>
#include <optional>
#include <cstdint>
// directX
#include <d3d12.h>

namespace SakuEngine {

	// front
	class UIWidgetDocument;
	class UISlateWidget;

	//============================================================================
	//	UIWidgetPreviewRuntime structures
	//============================================================================

	// UIピック結果構造体
	struct UIPickResult {

		uint32_t widgetId = 0;
		bool valid = false;
	};

	//============================================================================
	//	UIWidgetPreviewRuntime class
	//	UIウィジェットのプレビュー用ランタイムクラス
	//============================================================================
	class UIWidgetPreviewRuntime {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		UIWidgetPreviewRuntime();
		~UIWidgetPreviewRuntime() = default;

		// 初期化
		void Init();

		// フレーム更新
		void Tick(float deltaTime);

		// ドキュメントからウィジェットを構築
		void RebuildFromDocument(const UIWidgetDocument& document);
		// プロパティを同期
		void SyncPropertiesFromDocument(const UIWidgetDocument& document);

		// 指定位置にあるウィジェットをピックする
		UIPickResult PickWidgetAt(const Vector2& screenPos) const;

		//--------- accessor -----------------------------------------------------

		// ビューポートサイズの設定
		void SetViewportSize(const Vector2& size);

		// ウィジェットの位置とサイズを取得
		bool GetWidgetRect(uint32_t widgetId, Vector2& outPos, Vector2& outSize) const;
		// ビューポートサイズの取得
		const Vector2& GetViewportSize() const { return viewportSize_; }

		// 外部からプレビューのDataContextに値を入れる用
		UIUserWidget* GetPreviewUserWidget() const { return userWidget_.get(); }
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------
		
		// 描画ビューポートサイズ
		Vector2 viewportSize_;

		// ランタイムで動かすウィジェットのルート
		std::unique_ptr<UISlateWidget> runtimeRoot_;
		// ウィジェットを動かす
		std::unique_ptr<UIApplication> app_;

		// ユーザウィジェット
		std::unique_ptr<UIUserWidget> userWidget_;
		// ウィジェットを構築するコンパイラ
		std::unique_ptr<UIWidgetCompiler> compiler_;

		//--------- functions ----------------------------------------------------

		// 指定位置にあるウィジェットを再帰的にヒットテストする
		uint32_t HitTestRecursive(UISlateWidget* widget, const Vector2& pos) const;
	};
} // SakuEngine