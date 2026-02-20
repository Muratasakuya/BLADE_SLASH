#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Base/GameObject3D.h>

//============================================================================
//	ObjectAreaChecker enum
//============================================================================

// エリア内に入ったときのリアクションタイプ
enum class AreaReactionType {

	LerpPos,       // 座標補間
	LerpRotate,    // 回転補間
	LerpCamera,    // カメラ補間
	Count
};

//============================================================================
//	ObjectAreaChecker class
//	対象からリアクションエリア内にいるかをチェックするクラス
//============================================================================
class ObjectAreaChecker {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	ObjectAreaChecker() = default;
	~ObjectAreaChecker() = default;

	// 初期化
	void Init(const std::string& jsonPath);

	// 更新
	void Update();

	// エディター
	void ImGui();

	//--------- accessor -----------------------------------------------------

	// 基準点設定
	void SetAnchor(const SakuEngine::GameObject3D* anchor) { anchor_ = anchor; }
	// 対象設定
	void SetTarget(const SakuEngine::GameObject3D* target) { target_ = target; }

	// 範囲内にいるかチェック
	bool IsInRange(AreaReactionType reactionType) const;
	// 範囲の取得
	float GetRange(AreaReactionType reactionType) const { return areaParams_[static_cast<uint32_t>(reactionType)].range; }
	// 最小、最大範囲の取得
	float GetMinRange() const;
	float GetMaxRange() const;
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

	// jsonパス
	std::string jsonPath_;

	// 基準点
	const SakuEngine::GameObject3D* anchor_;
	// 対象
	const SakuEngine::GameObject3D* target_;

	// エリアパラメーターリスト
	std::array<AreaParam, static_cast<uint32_t>(AreaReactionType::Count)> areaParams_;

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson();
	void SaveJson();
};