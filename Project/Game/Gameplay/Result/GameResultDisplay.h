#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/Base/IGameEditor.h>
#include <Engine/Utility/Timer/StateTimer.h>
#include <Engine/Utility/Animation/SimpleAnimation.h>
#include <Game/UI/Navigation/GamecButtonFocusNavigator.h>
#include <Game/UI/Widgets/TimerDisplay/GameTimerDisplay.h>
#include <Game/UI/Widgets/Button/GameButton.h>

//============================================================================
//	GameResultDisplay enum class
//============================================================================

// リザルト画面の選択
enum class ResultSelect {

	None,  // 選択無し
	Title, // タイトルに戻る
	Retry  // リトライ
};

//============================================================================
//	GameResultDisplay class
//	リザルト画面のスプライト表示
//============================================================================
class GameResultDisplay :
	public SakuEngine::IGameEditor {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	GameResultDisplay() :IGameEditor("GameResultDisplay") {}
	~GameResultDisplay() = default;

	void Init();

	void Update();
	void Measurement();

	void ImGui() override;

	//--------- accessor -----------------------------------------------------

	// 表示開始
	void StartDisplay();

	ResultSelect GetResultSelect() const { return resultSelect_; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// 現在の状態
	enum class State {

		None,
		BeginTime, // 時間表示
		Result,    // クリア表示
		Select     // セレクト
	};

	//--------- variables ----------------------------------------------------

	// 現在の状態
	State currentState_;
	ResultSelect resultSelect_;

	// 経過時間
	SakuEngine::StateTimer resultTimer_;

	// 使用するオブジェクト
	std::unique_ptr<SakuEngine::GameObject2D> background_; // 背景
	std::unique_ptr<GameTimerDisplay> resultTime_; // 時間表示
	std::unique_ptr<SakuEngine::GameObject2D> glitchArea_; // グリッチ適応エリア
	std::unique_ptr<SakuEngine::GameObject2D> clearText_; // クリア文字表示
	std::unique_ptr<GameButton> leftButton_;       // 左ボタン
	std::unique_ptr<GameButton> rightButton_;      // 右ボタン

	// 入力管理
	std::unique_ptr<GamecButtonFocusNavigator> buttonFocusNavigator_;
	bool wasGamepad_ = false;

	// parameters
	// BeginTime
	SakuEngine::StateTimer randomDisplayTimer_; // 時間をランダムで表示する時間
	SakuEngine::StateTimer displayWaitTimer_;   // 表示後の待ち時間
	float randomTimeMax_;    // 乱数の最大値
	int randomSwitchCount_;  // 数字が切り替わる回数
	int randomSwitchIndex_;  // 現在の切り替え回数
	float randomSwitchBias_; // 切り替え間隔
	float nextSwitchT_;      // 次の切り替え閾値
	float lastRandomTime_;   // 直近に表示している乱数

	// タイマー
	float timeResultSizeScale_;   // リザルト表示の際のサイズスケーリング値
	float timeResultOffsetScale_; // リザルト表示の際のサイズスケーリング値
	SakuEngine::Vector2 timerTranslation_;    // タイマーの座標
	float timerOffsetX_;          // タイマーの間の間隔X
	SakuEngine::Vector2 timerSize_;           // タイマーのサイズ
	SakuEngine::Vector2 timerSymbolSize_;     // タイマーの記号のサイズ

	// グリッチ適応エリア
	SakuEngine::Vector2 glitchAreaSize_;

	// クリア文字
	std::unique_ptr<SakuEngine::SimpleAnimation<SakuEngine::Vector2>> clearPosAnim_;
	std::unique_ptr<SakuEngine::SimpleAnimation<SakuEngine::Vector2>> clearSizeAnim_;
	// クリアタイム
	std::unique_ptr<SakuEngine::SimpleAnimation<SakuEngine::Vector2>> clearTimePosAnim_;
	// ボタン
	std::unique_ptr<SakuEngine::SimpleAnimation<SakuEngine::Vector2>> leftButtonAnim_;
	std::unique_ptr<SakuEngine::SimpleAnimation<SakuEngine::Vector2>> rightButtonAnim_;
	SakuEngine::Vector2 buttonSize_;

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson();
	void SaveJson();

	// init
	void InitAnimations();
	void InitNavigator();

	// update
	void UpdateBeginTime();
	void UpdateResult();
	void UpdateInputGamepad();
	void ConfirmLeftByPad();
	void ConfirmRightByPad();
};