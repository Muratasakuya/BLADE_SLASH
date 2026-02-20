#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Gameplay/Actors/Player/ComboAction/Action/Interface/PlayerIActionNode.h>

//============================================================================
//	PlayerThrowWeaponAttack class
//	プレイヤーの投擲武器攻撃ノード
//============================================================================
class PlayerThrowWeaponAttack :
	public PlayerIActionNode {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerThrowWeaponAttack() = default;
	~PlayerThrowWeaponAttack() = default;

	void Enter() override;

	void Update() override;

	void Exit() override;

	void ImGui() override;

	void FromJson(const Json& data) override;
	void ToJson(Json& data) override;

	//--------- accessor -----------------------------------------------------

	void SetProgress(float progress) override;
	bool IsFinished() const override;
	float GetTotalTime() const override;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// 武器のパラメータ
	struct WeaponParam {

		bool isMoveStart;      // 移動開始フラグ
		SakuEngine::StateTimer moveTimer; // 共通から値を受け取る
		float moveValue;       // 移動量
		float rotateSpeed;     // 回転速度
		float offsetRotationY; // 進行方向のオフセットY回転
		SakuEngine::Vector3 startPos;  // 開始座標(親子付けを外した瞬間のワールド座標)
		SakuEngine::Vector3 targetPos; // 目標座標
		SakuEngine::Vector3 rotation;  // 回転
	};

	//--------- variables ----------------------------------------------------

	// 敵が状態開始時に攻撃可能範囲にいるかチェックした結果
	bool isInRange_;

	// アニメーションの遷移時間
	float nextAnimDuration_;

	// 後ずさりする距離
	float backMoveDistance_;

	// 座標移動補間
	SakuEngine::LerpValue<SakuEngine::Vector3> lerpPos_;
	// 回転補間
	SakuEngine::LerpValue<SakuEngine::Quaternion> lerpRotate_;

	// 剣
	static const uint32_t kWeaponCount = 2;
	std::array<WeaponParam, kWeaponCount> weaponParams_;
	// 共通
	SakuEngine::StateTimer weaponMoveTimer_; // 剣が目標座標に行くまでの時間
	float bossEnemyDistance_;
	float weaponPosY_;

	//--------- functions ----------------------------------------------------

	// アニメーションキーイベントに応じた更新
	void UpdateAnimKeyEvent();

	// 投擲武器の移動開始
	void StartMoveWeapon(PlayerWeaponType type);
	// 投擲武器の補間処理
	void LerpWeapon(PlayerWeaponType type);

	// Y軸回転オフセットをかけたベクトルを返す
	SakuEngine::Vector3 RotateYOffset(const SakuEngine::Vector3& direction, float offsetRotationY);
};