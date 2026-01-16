#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/UISlateWidget/Core/UISlateWidget.h>

namespace SakuEngine {

	//============================================================================
	//	UISlatePanelWidget class
	//	
	//============================================================================
	class UISlatePanelWidget :
		public UISlateWidget {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		UISlatePanelWidget() = default;
		virtual ~UISlatePanelWidget() = default;
		 
		// 子の追加
		void AddChild(std::unique_ptr<UISlateWidget> child);

		// フレーム更新
		void Tick(UIUserWidget& owner, float deltaTime) override;

		// 子供を持つか
		bool IsPanel() const override { return true; }

		// ツリーに入った後に呼ばれる
		void OnAddedToTree(UIUserWidget& owner) override;
		// プロパティ同期
		void SynchProperties(UIUserWidget& owner) override;

		// json
		void FromJson(const Json& data) override;
		void ToJson(Json& data) override;

		//--------- accessor -----------------------------------------------------

		// 子の列挙
		void GetChildren(std::vector<UISlateWidget*>& outChildren) override;
	protected:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		// 子ウィジェット群
		std::vector<std::unique_ptr<UISlateWidget>> children_{};
	};
} // SakuEngine