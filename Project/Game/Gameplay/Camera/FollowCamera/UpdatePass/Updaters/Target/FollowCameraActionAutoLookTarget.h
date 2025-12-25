#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Timer/StateTimer.h>
#include <Game/Gameplay/Camera/FollowCamera/UpdatePass/Interface/IFollowCameraUpdatePass.h>
#include <Game/Gameplay/Actors/Enemies/Boss/Structures/BossEnemyStructures.h>
#include <Game/Gameplay/Actors/Player/Structure/PlayerStructures.h>

//============================================================================
//	FollowCameraActionAutoLookTarget class
//	ゲーム内アクションに応じて自動で注視点を向かせる処理
//============================================================================
class FollowCameraActionAutoLookTarget :
	public IFollowCameraUpdatePass {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	FollowCameraActionAutoLookTarget() = default;
	~FollowCameraActionAutoLookTarget() = default;

	// 初期化
	void Init() override;

	// 更新
	void Execute(FollowCameraContext& context, const FollowCameraFrameService& service, float deltaTime) override;

	// エディター
	void ImGui() override;

	//--------- accessor -----------------------------------------------------

	// 識別IDの取得
	static constexpr FollowCameraUpdatePassID ID = FollowCameraUpdatePassID::ActionAutoLookTarget;
	virtual FollowCameraUpdatePassID GetID() const override { return ID; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// 左か右か
	enum class AnchorToDirection {

		Right = -1,
		Left = 1,
	};
	// 処理の起点
	enum class Source {
		None,
		Player,
		Boss,
	};
	// 同優先度時の処理方法
	enum class TieBreak {

		PreferPlayer,
		PreferBoss,
	};


	// パラメータ
	struct Parameter {

		// 処理を開始した時点で注視点を固定するか
		bool isLockTarget = true;

		// 目標X軸回転
		float targetXRotation = 0.0f;
		// Y軸オフセット角度
		float lookYawOffset = 0.0f;

		// 注視点へ向くまでの時間
		SakuEngine::StateTimer lookTimer;

		// エディター
		void ImGui(const std::string& label);

		// json
		void FromJson(const Json& data);
		void ToJson(Json& data);
	};

	// ルール
	template <typename Enum>
	struct RuleEntry {

		bool enabled = true;
		int32_t priority = 0;  // 大きいほど優先して処理するようにする
		Enum state;            // 監視対象の状態
		Parameter parameter{}; // 実行パラメータ
	};
	using PlayerRule = RuleEntry<PlayerState>;
	using BossRule = RuleEntry<BossEnemyState>;

	// 処理候補
	struct Candidate {

		Source source = Source::None;
		int32_t priority = 0;
		// どちらか片方だけが入る
		std::optional<PlayerState> playerState = std::nullopt;
		std::optional<BossEnemyState> bossState = std::nullopt;
		const Parameter* parameter = nullptr;
	};

	//--------- variables ----------------------------------------------------

	// 補間に使用する値
	SakuEngine::Quaternion startRotation_;                 // 補間開始時の回転
	std::optional<SakuEngine::Quaternion> targetRotation_; // 補間目標の回転

	// プレイヤーと敵の状態ごとのパラメータ
	std::unordered_map<PlayerState, Parameter> playerParameters_;
	std::unordered_map<BossEnemyState, Parameter> bossParameters_;
	// 目標回転が基準点から見て左か右か
	AnchorToDirection lookYawDirection_;

	// ルール
	std::vector<PlayerRule> playerRules_;
	std::vector<BossRule> bossRules_;

	// 状態監視
	bool initializedStates_ = false;
	PlayerState prevPlayerState_;
	BossEnemyState prevBossState_;

	// 状態が変わったが未処理の候補
	std::optional<PlayerState> pendingPlayerState_ = std::nullopt;
	std::optional<BossEnemyState> pendingBossState_ = std::nullopt;

	// 現在処理中のパラメータ
	std::optional<Parameter> currentParameter_ = std::nullopt;
	Source currentSource_ = Source::None;
	int32_t currentPriority_ = 0;

	// ランタイム設定
	bool triggerOnFirstFrame_ = true;            // 状態遷移扱いで起動するか
	bool allowInterrupt_ = true;                 // 処理中の割り込みを許可
	bool allowInterruptOnEqualPriority_ = false; // 同優先度でも割り込みを許可
	TieBreak tieBreak_ = TieBreak::PreferBoss;   // 同優先度時の選択

	//--------- functions ----------------------------------------------------

	// json
	void ApplyJson();
	void SaveJson();

	// 最適な候補を構築して取得する
	std::optional<Candidate> BuildBestCandidate() const;
	// 候補の処理を開始する
	void StartCandidate(const FollowCameraContext& context, const Candidate& candidate);

	// 状態をチェックして処理を行うか判断する
	void UpdateStateCheck(const FollowCameraContext& context);

	// 回転を注視点に向ける処理
	void UpdateLookToTarget(FollowCameraContext& context);

	// 目標回転を計算して取得する
	SakuEngine::Quaternion GetTargetRotation(const Parameter& parameter) const;

	// ルール検索
	const PlayerRule* FindPlayerRule(PlayerState state) const;
	const BossRule* FindBossRule(BossEnemyState state) const;
};