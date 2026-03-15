#include "BossEnemyAttackCollision.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Debug/SpdLogger.h>
#include <Engine/Object/Data/Transform/Transform.h>
#include <Engine/Utility/Timer/GameTimer.h>
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Engine/Utility/Enum/EnumAdapter.h>
#include <Engine/Utility/Algorithm/Algorithm.h>

// imgui
#include <imgui.h>

//============================================================================
//	BossEnemyAttackCollision classMethods
//============================================================================

void BossEnemyAttackCollision::Init() {

	// 形状初期化
	weaponBody_ = bodies_.emplace_back(Collider::AddCollider(CollisionShape::OBB().Default()));
	bodyOffsets_.emplace_back(CollisionShape::OBB().Default());

	// タイプ設定
	// 最初は無効状態
	weaponBody_->SetType(ColliderType::Type_None);
	weaponBody_->SetTargetType(ColliderType::Type_Player);

	// ヒットエフェクト
	attackHitEffect_ = std::make_unique<SakuEngine::EffectGroup>();
	attackHitEffect_->Init("bossAttackHitEffect", "BossEnemyEffect");
	attackHitEffect_->LoadJson("GameEffectGroup/BossEnemy/bossEnemyAttackHitEffect.json");
}

void BossEnemyAttackCollision::Update(const Transform3D& transform) {

	// ヒットエフェクトは常に更新する
	attackHitEffect_->Update();

	auto it = table_.find(currentState_);
	if (it == table_.end()) {
		return;
	}
	AttackParameter& parameter = it->second;

	// 時間を進める
	currentTimer_ += GameTimer::GetScaledDeltaTime();

	// 攻撃中かどうか
	bool isAttack = std::any_of(parameter.windows.begin(),
		parameter.windows.end(),
		[t = currentTimer_](const TimeWindow& window) {
			return window.on <= t && t < window.off; });

	// 遷移可能な状態の時のみ武器状態にする
	if (isAttack) {

		weaponBody_->SetType(ColliderType::Type_BossWeapon);

		// 状態別で形状の値を設定
		auto& offset = std::get<CollisionShape::OBB>(bodyOffsets_.front());
		const Vector3 offsetWorld =
			transform.GetRight() * parameter.centerOffset.x +
			transform.GetUp() * parameter.centerOffset.y +
			transform.GetForward() * parameter.centerOffset.z;
		offset.center = offsetWorld;
		offset.size = parameter.size;

		// デバッグログ
		// 状態と時間を表示
		LOG_INFO("BossAttackCollision::Attack On State: {}, Time: {:.3f}",
			EnumAdapter<BossEnemyState>::ToString(currentState_), currentTimer_);
		// 当たり判定の中心とサイズを表示
		LOG_INFO("  Center: ({:.3f}, {:.3f}, {:.3f}), Size: ({:.3f}, {:.3f}, {:.3f})",
			offset.center.x, offset.center.y, offset.center.z, offset.size.x, offset.size.y, offset.size.z);
	} else {

		weaponBody_->SetType(ColliderType::Type_None);

		// 当たらないようにする
		auto& offset = std::get<CollisionShape::OBB>(bodyOffsets_.front());
		offset.center = Vector3(0.0f, -128.0f, 0.0f);
		offset.size = Vector3::AnyInit(0.0f);
	}


	// 衝突情報更新
	Collider::UpdateAllBodies(transform);
}

void BossEnemyAttackCollision::SetEnterState(BossEnemyState state) {

	currentState_ = state;
	currentTimer_ = 0.0f;
	// 無効状態を設定
	weaponBody_->SetType(ColliderType::Type_None);
}

void BossEnemyAttackCollision::ImGui() {

	ImGui::Text("currentType: %s", EnumAdapter<ColliderType>::ToString(weaponBody_->GetType()));
	ImGui::Text("currentTimer: %.3f", currentTimer_);

	EnumAdapter<BossEnemyState>::Combo("State", &editingState_);
	AttackParameter& parameter = table_[editingState_];

	ImGui::Separator();

	bool edit = false;
	edit |= ImGui::DragFloat3("centerOffset", &parameter.centerOffset.x, 0.01f);
	edit |= ImGui::DragFloat3("size", &parameter.size.x, 0.01f);

	EditWindowParameter("hitWindow", parameter.windows);

	if (edit) {

		auto& offset = std::get<CollisionShape::OBB>(bodyOffsets_.front());
		offset.center = parameter.centerOffset;
		offset.size = parameter.size;
	}
}

void BossEnemyAttackCollision::ApplyJson(const Json& data) {

	for (const auto& [key, value] : data.items()) {

		BossEnemyState state = GetBossEnemyStateFromName(key);
		AttackParameter parameter;

		parameter.centerOffset = JsonAdapter::ToObject<Vector3>(value["centerOffset"]);
		parameter.size = JsonAdapter::ToObject<Vector3>(value["size"]);
		if (value.contains("hitWindows")) {
			for (const auto& w : value["hitWindows"]) {

				TimeWindow time;
				time.on = w.value("onTime", 0.0f);
				time.off = w.value("offTime", 0.0f);
				parameter.windows.emplace_back(time);
			}
		}
		table_[state] = parameter;

		// デバッグログ
		// 読み込んだ内容を表示
		LOG_INFO("BossEnemyAttackCollision::Load State: {}", key);
		LOG_INFO("  CenterOffset: ({:.3f}, {:.3f}, {:.3f}), Size: ({:.3f}, {:.3f}, {:.3f})",
			parameter.centerOffset.x, parameter.centerOffset.y, parameter.centerOffset.z,
			parameter.size.x, parameter.size.y, parameter.size.z);
		for (const auto& w : parameter.windows) {
			LOG_INFO("  HitWindow: onTime: {:.3f}, offTime: {:.3f}", w.on, w.off);
		}
	}
}

void BossEnemyAttackCollision::SaveJson(Json& data) {

	for (auto& [state, parameter] : table_) {

		Json& value = data[EnumAdapter<BossEnemyState>::ToString(state)];
		value["centerOffset"] = JsonAdapter::FromObject(parameter.centerOffset);
		value["size"] = JsonAdapter::FromObject(parameter.size);

		{
			Json windowData = Json::array();
			for (auto& w : parameter.windows) {

				Json j;
				j["onTime"] = w.on;
				j["offTime"] = w.off;
				windowData.push_back(j);
			}
			value["hitWindows"] = windowData;
		}
	}
}

void BossEnemyAttackCollision::OnCollisionEnter(const SakuEngine::CollisionBody* collisionBody) {

	// プレイヤーに衝突したか
	if ((collisionBody->GetType() & ColliderType::Type_Player) != ColliderType::Type_None) {
		if (const auto* obb = std::get_if<CollisionShape::OBB>(&collisionBody->GetShape())) {

			// 発生座標
			Vector3 emitPos = obb->center;
			emitPos.y += obb->size.y / 2.0f;

			// ヒットエフェクト発生
			attackHitEffect_->Emit(emitPos);
		}
	}
}

BossEnemyState BossEnemyAttackCollision::GetBossEnemyStateFromName(const std::string& name) {

	return EnumAdapter<BossEnemyState>::FromString(name).value();
}

void BossEnemyAttackCollision::EditWindowParameter(
	const std::string& label, std::vector<TimeWindow>& windows) {

	ImGui::PushID(label.c_str());

	ImGui::SeparatorText(label.c_str());

	for (size_t i = 0; i < windows.size(); ++i) {

		ImGui::PushID(static_cast<int>(i));
		ImGui::DragFloat("onTime", &windows[i].on, 0.01f, 0.0f);
		ImGui::DragFloat("offTime", &windows[i].off, 0.01f, windows[i].on);
		ImGui::PopID();
	}
	if (ImGui::Button(("AddOnTime" + label).c_str())) {

		windows.emplace_back();
	}
	if (!windows.empty() && ImGui::Button(("RemoveOnTime" + label).c_str())) {

		windows.pop_back();
	}

	ImGui::PopID();
}