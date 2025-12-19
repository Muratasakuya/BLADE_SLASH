#include "Player.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/PostProcess/Core/PostProcessSystem.h>
#include <Engine/Input/Input.h>
#include <Engine/Utility/Random/RandomGenerator.h>
#include <Engine/Utility/Enum/EnumAdapter.h>
#include <Engine/Utility/Enum/Direction.h>
#include <Game/Objects/GameScene/Enemy/Boss/Entity/BossEnemy.h>

//============================================================================
//	Player classMethods
//============================================================================

void Player::InitWeapon() {

	// 右手
	rightWeapon_ = std::make_unique<PlayerWeapon>();
	rightWeapon_->Init("playerRightWeapon", "playerRightWeapon", "Player");

	// 左手
	leftWeapon_ = std::make_unique<PlayerWeapon>();
	leftWeapon_->Init("playerLeftWeapon", "playerLeftWeapon", "Player");

	// 武器を手を親として動かす
	if (const auto& hand = GetJointTransform("rightHand")) {

		rightWeapon_->SetParent(*hand);
	}
	if (const auto& hand = GetJointTransform("leftHand")) {

		leftWeapon_->SetParent(*hand);
	}
}

void Player::InitAnimations() {

	// 最初は待機状態で初期化
	animation_->SetPlayAnimation("player_idle", true);

	// animationのデータを設定
	animation_->SetAnimationData("player_walk");
	animation_->SetAnimationData("player_dash");
	animation_->SetAnimationData("player_avoid");
	animation_->SetAnimationData("player_attack_1st");
	animation_->SetAnimationData("player_attack_2nd");
	animation_->SetAnimationData("player_attack_3rd");
	animation_->SetAnimationData("player_attack_4th");
	animation_->SetAnimationData("player_skilAttack_1st");
	animation_->SetAnimationData("player_skilAttack_2nd");
	animation_->SetAnimationData("player_stunAttack");
	animation_->SetAnimationData("player_parry");
	animation_->SetAnimationData("player_falter");

	// 両手を親として更新させる
	animation_->SetParentJoint("rightHand");
	animation_->SetParentJoint("leftHand");

	// keyEventを設定
	animation_->SetKeyframeEvent("Player/animationEffectKey.json");
	animation_->Update(transform_->matrix.world);
}

void Player::InitCollision() {

	// OBBで設定
	SakuEngine::CollisionBody* body = bodies_.emplace_back(Collider::AddCollider(SakuEngine::CollisionShape::OBB().Default()));
	bodyOffsets_.emplace_back(SakuEngine::CollisionShape::OBB().Default());

	// タイプ設定
	body->SetType(ColliderType::Type_Player);
	body->SetTargetType(ColliderType::Type_BossEnemy);

	// 衝突を管理するクラスを初期化
	attackCollision_ = std::make_unique<PlayerAttackCollision>();
	attackCollision_->Init();
}

void Player::InitState() {

	// 初期化、ここで初期状態も設定
	stateController_ = std::make_unique<PlayerStateController>();
	stateController_->Init(this);
}

void Player::InitHUD() {

	// HUDの初期化
	hudSprites_ = std::make_unique<PlayerHUD>();
	hudSprites_->Init();

	stunHudSprites_ = std::make_unique<PlayerStunHUD>();
	stunHudSprites_->Init();

	targetNavigation_ = std::make_unique<TargetNavigation>();
	targetNavigation_->Init();

	isCanParryBossEnemy_ = false;
}

void Player::InitEffects() {

	// 回避エフェクトの初期化
	avoidEffect_ = std::make_unique<SakuEngine::EffectGroup>();
	avoidEffect_->Init("avoidEffect", "PlayerEffect");
	avoidEffect_->LoadJson("GameEffectGroup/Player/playerAvoidEffect.json");
}

void Player::SetInitTransform() {

	transform_->scale = initTransform_.scale;
	transform_->eulerRotate = initTransform_.eulerRotate;
	transform_->rotation = initTransform_.rotation;
	transform_->translation = initTransform_.translation;

	// 常に更新を行わせる
	transform_->isCompulsion_ = true;
}

void Player::DerivedInit() {

	// 使用する武器を初期化
	InitWeapon();

	// animation初期化、設定
	InitAnimations();

	// collision初期化、設定
	InitCollision();

	// 状態初期化
	InitState();

	// HUD初期化
	InitHUD();

	// エフェクト初期化
	InitEffects();

	// json適応
	ApplyJson();

	// ポストエフェクト設定
	uint32_t postProcessBit = Bit_Bloom | Bit_DepthBasedOutline |
		Bit_Glitch | Bit_Grayscale | Bit_PlayerAfterImage;
	SetPostProcessMask(postProcessBit);
	leftWeapon_->SetPostProcessMask(postProcessBit);
	rightWeapon_->SetPostProcessMask(postProcessBit);
}

void Player::SetBossEnemy(BossEnemy* bossEnemy) {

	bossEnemy_ = nullptr;
	bossEnemy_ = bossEnemy;

	stateController_->SetBossEnemy(bossEnemy);
	hudSprites_->SetBossEnemy(bossEnemy);
	attackCollision_->SetBossEnemy(bossEnemy);
}

void Player::SetFollowCamera(FollowCamera* followCamera) {

	stateController_->SetFollowCamera(followCamera);
	hudSprites_->SetFollowCamera(followCamera);
	targetNavigation_->SetCamera(followCamera);
}

void Player::SetReverseWeapon(bool isReverse, PlayerWeaponType type) {

	// 剣の持ち方設定
	if (isReverse) {
		if (type == PlayerWeaponType::Left) {

			leftWeapon_->SetRotation(SakuEngine::Quaternion::MakeAxisAngle(Direction::Get(Direction3D::Right), SakuEngine::pi));
		} else {

			rightWeapon_->SetRotation(SakuEngine::Quaternion::MakeAxisAngle(Direction::Get(Direction3D::Right), SakuEngine::pi));
		}
	} else {
		if (type == PlayerWeaponType::Left) {

			leftWeapon_->SetRotation(SakuEngine::Quaternion::MakeAxisAngle(Direction::Get(Direction3D::Right), 0.0f));
		} else {

			rightWeapon_->SetRotation(SakuEngine::Quaternion::MakeAxisAngle(Direction::Get(Direction3D::Right), 0.0f));
		}
	}
}

void Player::ResetWeaponTransform(PlayerWeaponType type) {

	// 武器の位置を元に戻す
	if (type == PlayerWeaponType::Left) {

		leftWeapon_->ApplyJson(cacheJsonData_["LeftWeapon"]);
		if (const auto& hand = GetJointTransform("leftHand")) {

			leftWeapon_->SetParent(*hand);
		}
	} else if (type == PlayerWeaponType::Right) {

		rightWeapon_->ApplyJson(cacheJsonData_["RightWeapon"]);
		if (const auto& hand = GetJointTransform("rightHand")) {

			rightWeapon_->SetParent(*hand);
		}
	}
}

PlayerWeapon* Player::GetWeapon(PlayerWeaponType type) const {

	switch (type) {
	case PlayerWeaponType::Left:

		return leftWeapon_.get();
	case PlayerWeaponType::Right:

		return rightWeapon_.get();
	default:
		return nullptr;
	}
}

int Player::GetDamage() const {

	// 現在の状態に応じたダメージを取得
	PlayerState currentState = stateController_->GetCurrentState();

	// ダメージを与えられる状態か確認してから設定
	if (SakuEngine::Algorithm::Find(stats_.damages, currentState)) {

		int damage = stats_.damages.at(currentState);
		// ランダムでダメージを設定
		damage = SakuEngine::RandomGenerator::Generate(damage - stats_.damageRandomRange,
			damage + stats_.damageRandomRange);
		return damage;
	}
	return 0;
}

void Player::Update() {

	// 更新モードがNotの時は更新しない
	if (updateMode_ == ObjectUpdateMode::Not) {
		return;
	}

	// パリィ可能状態のチェック
	CheckBossEnemyParry();

	// 方向UIの更新
	UpdateTargetNavigation();

	// 状態の更新
	stateController_->Update();

	// 武器の更新
	rightWeapon_->Update();
	leftWeapon_->Update();

	// スキルポイントの更新
	UpdateSKilPoint();

	// HUDの更新
	hudSprites_->SetStatas(stats_);
	hudSprites_->Update(*this);
	stunHudSprites_->Update();

	// 衝突情報更新
	Collider::UpdateAllBodies(*transform_);
	attackCollision_->Update(*transform_);

	// エフェクトの更新
	avoidEffect_->Update();

	// 入力で攻撃を無効化できるようにする
	if (SakuEngine::Input::GetInstance()->TriggerKey(DIK_F9)) {

		isInvincible_ = !isInvincible_;
	}
}

void Player::UpdateTargetNavigation() {

	// 座標を設定
	targetNavigation_->SetTargetPos(bossEnemy_->GetTranslation());
	targetNavigation_->SetPivotPos(transform_->translation);

	// オブジェクト更新
	targetNavigation_->Update();
}

void Player::UpdateSKilPoint() {

	// 時間経過で回復
	recoverSkilPointTimer_.Update();
	if (recoverSkilPointTimer_.IsReached()) {

		// スキルポイントの回復
		stats_.currentSkilPoint = (std::min)(stats_.maxSkilPoint, stats_.currentSkilPoint + stats_.incrementSkilPoint);
		// タイマーリセット
		recoverSkilPointTimer_.Reset();
	}

	// 状態に応じたスキルポイントの消費
	// スキル攻撃の時かつ状態が切り替わったとき
	if (stateController_->GetCurrentState() == PlayerState::SkilAttack) {

		// スキルポイントを消費
		stats_.currentSkilPoint = (std::max)(0, stats_.currentSkilPoint - stats_.skilCost);
	}
}

void Player::CheckBossEnemyParry() {

	// ボスの状態が切り替わったら、パリィ可能状態をリセット
	if (exitParryBossEnemyState_.has_value()) {
		if (exitParryBossEnemyState_.value() != bossEnemy_->GetCurrentState()) {

			exitParryBossEnemyState_ = std::nullopt;
		}
	}

	// パリィ可能状態になったかどうか
	const ParryParameter& parryParam = bossEnemy_->GetParryParam();
	if (!exitParryBossEnemyState_.has_value() &&
		!isCanParryBossEnemy_ && parryParam.canParry) {

		isCanParryBossEnemy_ = true;
		// パリィ可能状態になったら入力示唆を開始
		hudSprites_->StartInputSuggest();
		targetNavigation_->SetIsBlink(true);
	}
	// 入力不可になったら入力示唆を終了
	if (isCanParryBossEnemy_ && !parryParam.canParry) {

		// この時点のボスの状態を取得
		exitParryBossEnemyState_ = bossEnemy_->GetCurrentState();
		isCanParryBossEnemy_ = false;

		hudSprites_->EndInputSuggest();
		targetNavigation_->SetIsBlink(false);
	}
}

void Player::OnCollisionEnter(const SakuEngine::CollisionBody* collisionBody) {

	// パリィ処理中なら攻撃を受けない
	if (stateController_->IsActiveParry()) {
		return;
	}

	// 敵から攻撃を受けた時のみ
	if ((collisionBody->GetType() & (ColliderType::Type_BossWeapon | ColliderType::Type_BossBlade))
		!= ColliderType::Type_None) {

		// 攻撃を受けた瞬間に回避行動をしていれば攻撃を受けない
		if (stateController_->IsAvoidance()) {

			// 回避エフェクトを出す
			SakuEngine::PostProcessSystem::GetInstance()->Start(PostProcessType::Grayscale);

			SakuEngine::Vector3 playerPos = transform_->translation;
			avoidEffect_->Emit(playerPos);
			return;
		}

		// ダメージを受ける、無敵状態の時は0
		const int damage = isInvincible_ ? 0 : bossEnemy_->GetDamage();
		stats_.currentHP = (std::max)(0, stats_.currentHP - damage);

		// HUDに通知
		hudSprites_->SetDamage(damage);

		// 怯み状態遷移へリクエスト
		stateController_->RequestFalterState();
	}
}

void Player::DerivedImGui() {

	ImGui::PushItemWidth(itemWidth_);
	ImGui::SetWindowFontScale(0.8f);

	if (ImGui::BeginTabBar("PlayerTabs")) {

		// ---- Stats ---------------------------------------------------
		if (ImGui::BeginTabItem("Stats")) {

			ImGui::Text(std::format("isStunUpdate: {}", isStunUpdate_).c_str());
			ImGui::Text(std::format("isInvincible: {}", isInvincible_).c_str());

			ImGui::Text("HP : %d / %d", stats_.currentHP, stats_.maxHP);
			ImGui::Text("SP : %d / %d", stats_.currentSkilPoint, stats_.maxSkilPoint);

			ImGui::DragInt("Max HP", &stats_.maxHP, 1, 0);
			ImGui::DragInt("Cur HP", &stats_.currentHP, 1, 0, stats_.maxHP);
			ImGui::DragInt("Max SP", &stats_.maxSkilPoint, 1, 0);
			ImGui::DragInt("Cur SP", &stats_.currentSkilPoint, 1, 0, stats_.maxSkilPoint);
			ImGui::DragInt("Skil Cost", &stats_.skilCost, 1, 0, stats_.maxSkilPoint);
			ImGui::DragInt("Increment SP", &stats_.incrementSkilPoint, 1, 0);

			recoverSkilPointTimer_.ImGui("Recover SP Timer");

			// 各stateダメージの値を調整
			SakuEngine::EnumAdapter<PlayerState>::Combo("EditDamage", &editingState_);

			ImGui::SeparatorText(SakuEngine::EnumAdapter<PlayerState>::ToString(editingState_));
			ImGui::DragInt("Damage", &stats_.damages[editingState_], 1, 0);
			ImGui::DragInt("DamageRange", &stats_.damageRandomRange, 1, 0);
			ImGui::DragInt("toughness", &stats_.toughness, 1, 0);

			if (ImGui::Button("Reset HP")) {
				stats_.currentHP = stats_.maxHP;
			}
			ImGui::SameLine();
			if (ImGui::Button("Reset SP")) {
				stats_.currentSkilPoint = stats_.maxSkilPoint / 2;
			}
			ImGui::EndTabItem();
		}

		// ---- Init ----------------------------------------------------
		if (ImGui::BeginTabItem("Init")) {

			if (ImGui::Button("Save##InitJson")) {
				SaveJson();
			}

			initTransform_.ImGui(itemWidth_);
			Collider::ImGui(itemWidth_);
			ImGui::EndTabItem();
		}

		// ---- State ---------------------------------------------------
		if (ImGui::BeginTabItem("State")) {

			stateController_->ImGui();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("AttackCollision")) {
			attackCollision_->ImGui();
			ImGui::EndTabItem();
		}

		// ---- HUD -----------------------------------------------------
		if (ImGui::BeginTabItem("HUD")) {
			hudSprites_->ImGui();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("StunHUD")) {
			stunHudSprites_->ImGui();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Navigation")) {
			targetNavigation_->ImGui();
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}

	ImGui::PopItemWidth();
	ImGui::SetWindowFontScale(1.0f);
}

void Player::ClampInitPosY() {

	// y座標が初期化時のY座標より下に行かないようにする
	transform_->translation.y = (std::max)(transform_->translation.y,
		initTransform_.translation.y);
}

void Player::ApplyJson() {

	if (!SakuEngine::JsonAdapter::LoadCheck("Player/initParameter.json", cacheJsonData_)) {
		return;
	}

	initTransform_.FromJson(cacheJsonData_["Transform"]);
	SetInitTransform();

	GameObject3D::ApplyMaterial(cacheJsonData_);
	Collider::ApplyBodyOffset(cacheJsonData_);

	// 武器
	rightWeapon_->ApplyJson(cacheJsonData_["RightWeapon"]);
	leftWeapon_->ApplyJson(cacheJsonData_["LeftWeapon"]);

	// 衝突
	attackCollision_->ApplyJson(cacheJsonData_["AttackCollision"]);

	stats_.maxHP = SakuEngine::JsonAdapter::GetValue<int>(cacheJsonData_, "maxHP");
	stats_.maxSkilPoint = SakuEngine::JsonAdapter::GetValue<int>(cacheJsonData_, "maxSkilPoint");
	stats_.skilCost = cacheJsonData_.value("skilCost", 50);
	stats_.incrementSkilPoint = cacheJsonData_.value("incrementSkilPoint", 1);
	// 初期化時は最大と同じ値にする
	stats_.currentHP = stats_.maxHP;
	stats_.currentSkilPoint = 0;

	recoverSkilPointTimer_.FromJson(cacheJsonData_.value("RecoverSkilPointTimer", Json()));

	for (const auto& [key, value] : cacheJsonData_["Damages"].items()) {

		PlayerState state = static_cast<PlayerState>(std::stoi(key));
		stats_.damages[state] = value.get<int>();
	}

	stats_.damageRandomRange = SakuEngine::JsonAdapter::GetValue<int>(cacheJsonData_, "DamageRandomRange");
	stats_.toughness = SakuEngine::JsonAdapter::GetValue<int>(cacheJsonData_, "toughness");
}

void Player::SaveJson() {

	Json data;

	initTransform_.ToJson(data["Transform"]);
	GameObject3D::SaveMaterial(data);
	Collider::SaveBodyOffset(data);

	// 武器
	rightWeapon_->SaveJson(data["RightWeapon"]);
	leftWeapon_->SaveJson(data["LeftWeapon"]);

	// 衝突
	attackCollision_->SaveJson(data["AttackCollision"]);

	data["maxHP"] = stats_.maxHP;
	data["maxSkilPoint"] = stats_.maxSkilPoint;
	data["skilCost"] = stats_.skilCost;
	data["incrementSkilPoint"] = stats_.incrementSkilPoint;

	recoverSkilPointTimer_.ToJson(data["RecoverSkilPointTimer"]);

	for (const auto& [state, value] : stats_.damages) {

		data["Damages"][std::to_string(static_cast<int>(state))] = value;
	}
	data["DamageRandomRange"] = stats_.damageRandomRange;
	data["toughness"] = stats_.toughness;

	SakuEngine::JsonAdapter::Save("Player/initParameter.json", data);
}