#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Animation/Tracks/Interface/IUIAnimationTrackRuntime.h>
#include <Engine/Editor/UI/Component/Selectable/UISelectableComponent.h>
#include <Engine/Utility/Animation/ValueSource/LerpValueSource.h>

namespace SakuEngine {

	//============================================================================
	//	UIStateAnimationComponent class
	//	UIの状態に合わせてアニメーションを再生するコンポーネント
	//============================================================================
	class UIStateAnimationComponent :
		public IUIComponent {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		UIStateAnimationComponent();
		~UIStateAnimationComponent() = default;

		// エディター、パネル側で処理する
		void ImGui([[maybe_unused]] const ImVec2& itemSize) override {}

		// json
		void FromJson(const Json& data) override;
		void ToJson(Json& data) override;

		//--------- variables ----------------------------------------------------

		// 状態別に再生するアニメーションクリップUID
		std::unordered_map<UIElementState, uint32_t> stateToClipUid;

		// UIの最後の状態
		UIElementState lastState = UIElementState::Hidden;
		// 再生中フラグ
		bool isPlaying = false;
		// 再生中のアニメーションクリップUID
		uint32_t playingClipUid = 0;

		// アニメーションを処理するトラックリスト
		std::vector<std::unique_ptr<IUIAnimationTrackRuntime>> tracks;

		//--------- accessor -----------------------------------------------------

		UIComponentType GetType() const override { return UIComponentType::StateAnimation; }
	};
} // SakuEngine