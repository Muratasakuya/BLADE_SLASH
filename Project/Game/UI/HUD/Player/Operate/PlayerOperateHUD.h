#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Base/GameObject2DArray.h>

//============================================================================
//	PlayerOperateHUD class
//	プレイヤーの操作表示HUD
//============================================================================
class PlayerOperateHUD {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerOperateHUD() = default;
	~PlayerOperateHUD() = default;

	// 初期化
	void Init();

	// 更新
	void Update();

	// エディター
	void ImGui();

	// 入力示唆アニメーション開始、終了呼び出し
	void StartInputSuggest();
	void EndInputSuggest();

	//--------- accessor -----------------------------------------------------

	// 表示、非表示設定
	void SetIsDisplay(bool isDisplay);
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// 入力示唆
	struct Suggest {

		// 表示スプライト
		std::unique_ptr<SakuEngine::GameObject2D> sprite;

		// アニメーション
		SakuEngine::SimpleAnimation<SakuEngine::Vector2> sizeAnim;
		SakuEngine::SimpleAnimation<SakuEngine::Color> colorAnim;
		SakuEngine::SimpleAnimation<float> emissiveAnim;
	};

	//--------- variables ----------------------------------------------------

	// 入力アイコン表示
	std::unique_ptr<SakuEngine::GameObject2DArray> operateIcons_;

	// ボタン入力示唆
	static const uint32_t kInputSuggestCount_ = 2;
	std::array<Suggest, kInputSuggestCount_> inputSuggests_;
	// パラメータ、フラグ
	bool isInputSuggestActive_;    // 入力示唆アニメーションが有効かどうか
	bool endDelayInputSuggest_;    // 遅延時間が終わったかどうか
	SakuEngine::StateTimer inputSuggestDelay_; // 表示遅延時間、初期発生時
	SakuEngine::Vector3 inputSuggestEmissionColor_; // 入力示唆の発光色

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson();
	void SaveJson();

	// 入力示唆更新
	void UpdateInputSuggest();
};