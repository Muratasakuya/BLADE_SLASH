#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Runtime/UIDataContext.h>
#include <Engine/Editor/UI/UISlateWidget/Core/UISlateWidget.h>
#include <Engine/Config.h>

// c++
#include <string>

namespace SakuEngine {

	//============================================================================
	//	UIUserWidget class
	//	
	//============================================================================
	class UIUserWidget {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		UIUserWidget() = default;
		~UIUserWidget() = default;

		// コピー禁止
		UIUserWidget(const UIUserWidget&) = delete;
		UIUserWidget& operator=(const UIUserWidget&) = delete;

		// 初期化
		void Init(std::string uiGroupName = "UI");

		// ウィジェット生成
		std::unique_ptr<UISlateWidget> CreateWidgetFromJson(const Json& node);
		// ウィジェットID割り当て
		uint64_t AllocateWidgetId(const std::string& nameHint);

		// フレーム更新
		void Tick(float deltaTime);
		//
		void OnAddedToTree();
		// 
		void SynchronizeProperties();

		// json
		bool LoadFromJson(const Json& data);
		void SaveToJson(Json& data) const;

		//--------- accessor -----------------------------------------------------

		// ウィジェットを検索して取得
		UISlateWidget* FindWidgetByName(const std::string& name) const;
		UIWidgetHandle FindHandleByName(const std::string& name) const;
		// ハンドルからウィジェット参照を取得
		UISlateWidget* Resolve(const UIWidgetHandle& handle) const;

		// ルート設定
		void SetRoot(std::unique_ptr<UISlateWidget> root);
		// 矩形サイズ設定
		void SetViewportRect(const UIRect& rect) { viewportRect_ = rect; }

		// UIグループ名取得
		const std::string& GetUIGroupName() const { return uiGroupName_; }
		// ルート取得
		UISlateWidget* GetRoot() const { return root_.get(); }
		// 矩形サイズ取得
		const UIRect& GetViewportRect() const { return viewportRect_; }
		// コンテキスト取得
		const UIDataContext& Data() const { return data_; }
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		// UIグループ名
		std::string uiGroupName_ = "UI";

		// ルートウィジェット
		std::unique_ptr<UISlateWidget> root_{};
		// 矩形サイズ
		UIRect viewportRect_{ Vector2::AnyInit(0.0f), Vector2(Config::kWindowWidthf, Config::kWindowHeightf) };

		// データコンテキスト
		UIDataContext data_{};

		// 名前、IDからウィジェット参照マップ
		std::unordered_map<std::string, UISlateWidget*> nameToWidget_{};
		std::unordered_map<uint64_t, UISlateWidget*> idToWidget_{};

		//--------- functions ----------------------------------------------------

		// ウィジェットIDカウンタ
		void RebuildIndex();
	};
} // SakuEngine