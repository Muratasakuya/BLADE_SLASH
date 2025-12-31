#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Base/GameObject3D.h>

// front
class Player;

//============================================================================
//	PlayerReactionAreaChecker enum
//============================================================================

// エリア内に入ったときのリアクションタイプ
enum class AreaReactionType {

	LerpPos,       // 座標補間
	LerpRotate,    // 回転補間
	LerpCamera,    // カメラ補間
	Count
};

//============================================================================
//	PlayerReactionAreaChecker class
//	攻撃対象がリアクションエリア内にいるかをチェックするクラス
//============================================================================
class PlayerReactionAreaChecker {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerReactionAreaChecker() = default;
	~PlayerReactionAreaChecker() = default;

	// 初期化
	void Init();

	// 更新
	void Update();

	// エディター
	void ImGui();

	//--------- accessor -----------------------------------------------------

	// プレイヤーの設定
	void SetPlayer(const Player* player) { player_ = player; }
	// 攻撃対象の設定
	void SetAttackTarget(const SakuEngine::GameObject3D* target) { attackTarget_ = target; }

	// 範囲内にいるかチェック
	bool IsInRange(AreaReactionType reactionType) const;
	// 範囲の取得
	float GetRange(AreaReactionType reactionType) const { return areaParams_[static_cast<uint32_t>(reactionType)].range; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// タイプ
	struct AreaParam {

		// 範囲
		float range;
		// 範囲内にいるかチェック
		bool isInRange;
		// 色
		SakuEngine::Color debugColor;
	};

	//--------- variables ----------------------------------------------------

	// プレイヤー
	const Player* player_;
	// 攻撃対象
	const SakuEngine::GameObject3D* attackTarget_;

	// エリアパラメーターリスト
	std::array<AreaParam, static_cast<uint32_t>(AreaReactionType::Count)> areaParams_;

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson();
	void SaveJson();
};