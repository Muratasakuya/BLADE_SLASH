#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Components/Interface/IUIComponent.h>
#include <Engine/Editor/UI/Components/InputNavigation/Devices/UINavigationAction.h>
#include <Engine/Input/Base/InputMapper.h>
#include <Engine/Input/InputStructures.h>

// c++
#include <cstdint>
#include <vector>
#include <chrono>

namespace SakuEngine {

	//============================================================================
	//	UIInputNavigationComponent class
	//	UI入力ナビゲーションコンポーネント
	//============================================================================
	class UIInputNavigationComponent :
		public IUIComponent {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		UIInputNavigationComponent() = default;
		~UIInputNavigationComponent() = default;

		// 入力マッパーの初期化
		void EnsureMapper();

		// エディター
		void ImGui(const ImVec2& itemSize) override;

		// json
		void FromJson(const Json& data) override;
		void ToJson(Json& data) override;

		// アクティブなアセットのみtrueにする
		bool acceptInput = false;

		// マウス操作でフォーカスを受け取るか
		bool mouseHoverFocus = true;
		bool mouseClickSubmit = true;

		// スティック入力の閾値
		float stickThreshold = 0.5f;

		// 連続入力の設定
		float repeatDelaySecond = 0.25f;
		float repeatIntervalSecond = 0.1f;

		// 入力デバイスごとの配列
		// キーボード
		std::vector<uint8_t> upKeys;
		std::vector<uint8_t> downKeys;
		std::vector<uint8_t> leftKeys;
		std::vector<uint8_t> rightKeys;
		std::vector<uint8_t> submitKeys;
		std::vector<uint8_t> cancelKeys;
		// ゲームパッド
		std::vector<GamePadButtons> upPad;
		std::vector<GamePadButtons> downPad;
		std::vector<GamePadButtons> leftPad;
		std::vector<GamePadButtons> rightPad;
		std::vector<GamePadButtons> submitPad;
		std::vector<GamePadButtons> cancelPad;

		// 現在フォーカスしているUID
		uint32_t focusedUid = 0;

		// リピート状態
		bool holdingDirection = false;
		Direction2D heldDirection = Direction2D::Right;
		std::chrono::steady_clock::time_point nextRepeatTime{};

		// 入力マッパー
		std::unique_ptr<InputMapper<UINavigationAction>> mapper;

		//--------- accessor -----------------------------------------------------

		UIComponentType GetType() const override { return UIComponentType::InputNavigation; }
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		// 追加する予定の入力
		KeyDIKCode addKeyDIKCode = KeyDIKCode::SPACE;
		GamePadButtons addGamePadButton = GamePadButtons::A;
	};
} // SakuEngine