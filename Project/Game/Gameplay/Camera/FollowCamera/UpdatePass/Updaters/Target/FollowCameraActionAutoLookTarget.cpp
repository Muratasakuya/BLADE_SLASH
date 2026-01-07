#include "FollowCameraActionAutoLookTarget.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Enum/EnumAdapter.h>
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Engine/Config.h>

//============================================================================
//	include
//============================================================================
#include <Game/Gameplay/Actors/Enemies/Boss/Entity/BossEnemy.h>
#include <Game/Gameplay/Actors/Player/Entity/Player.h>

//============================================================================
//	FollowCameraActionAutoLookTarget classMethods
//============================================================================

void FollowCameraActionAutoLookTarget::Init() {

	// エリアチェッカー初期化
	areaChecker_ = std::make_unique<ObjectAreaChecker>();
	areaChecker_->Init("Camera/Follow/autoLookAreaChecker.json");

	// json適用
	ApplyJson();
}

void FollowCameraActionAutoLookTarget::BindDependencies(
	const FollowCameraDependencies& dependencies) {

	// 依存設定
	dependencies_ = dependencies;
	// エリアチェッカーにオブジェクト設定
	areaChecker_->SetAnchor(dependencies_.player);
	areaChecker_->SetTarget(dependencies_.bossEnemy);
}

void FollowCameraActionAutoLookTarget::Execute(FollowCameraContext& context,
	[[maybe_unused]] const FollowCameraFrameService& service, [[maybe_unused]] float deltaTime) {

	// エリアチェッカー更新
	areaChecker_->Update();

	// 状態チェック
	UpdateStateCheck(context);

	// 現在のパラメータがなければ終了
	if (!currentParameter_.has_value()) {
		return;
	}

	// 注視点へ向く処理
	UpdateLookToTarget(context);
}

//=======================================================================================================================
//	回転処理
//=======================================================================================================================

void FollowCameraActionAutoLookTarget::UpdateLookToTarget(FollowCameraContext& context) {

	// 現在のパラメータ参照
	Parameter& parameter = currentParameter_.value();

	// 目標回転を取得
	Quaternion baseRotation = targetRotation_.has_value() ?
		targetRotation_.value() : GetTargetRotation(parameter);

	// 最短のY軸補間方向を取得
	float lookYawOffset = static_cast<float>(lookYawDirection_) * parameter.lookYawOffset;

	// Y軸にオフセットをかけて目標回転を算出
	Quaternion yawOffsetRotation = Quaternion::MakeAxisAngle(Direction::Get(Direction3D::Up), lookYawOffset);
	Quaternion targetRotation = Quaternion::Normalize(yawOffsetRotation * baseRotation);

	// 時間の更新
	parameter.lookTimer.Update();

	// 回転補間
	context.cameraRotation = SakuEngine::Quaternion::Slerp(startRotation_, targetRotation, parameter.lookTimer.easedT_);

	// 時間経過で終了
	if (parameter.lookTimer.IsReached()) {

		// 回転を固定する
		context.cameraRotation = targetRotation;
		currentParameter_ = std::nullopt;
		targetRotation_ = std::nullopt;
		currentPriority_ = 0;
	}
}

Quaternion FollowCameraActionAutoLookTarget::GetTargetRotation(const Parameter& parameter) const {

	// 向きを取得
	SakuEngine::Vector3 direction = Math::GetDirection3D(*dependencies_.player, *dependencies_.bossEnemy);

	// Y軸の回転
	Quaternion yawRotation = Quaternion::LookRotation(direction, Direction::Get(Direction3D::Up));
	// X軸回転
	SakuEngine::Vector3 rightAxis = (yawRotation * Direction::Get(Direction3D::Right)).Normalize();
	Quaternion pitchRotation = Quaternion::Normalize(Quaternion::MakeAxisAngle(rightAxis, parameter.targetXRotation));

	// 目標回転 
	SakuEngine::Quaternion targetRotation = Quaternion::Normalize(pitchRotation * yawRotation);
	return targetRotation;
}

//=======================================================================================================================
//	状態チェック処理
//=======================================================================================================================

void FollowCameraActionAutoLookTarget::UpdateStateCheck(const FollowCameraContext& context) {

	// エリア内にいるかチェック、範囲外なら何もしない
	if (!areaChecker_->IsInRange(AreaReactionType::LerpCamera)) {
		return;
	}

	// プレイヤーの状態と敵の状態を取得
	PlayerState playerState = dependencies_.player->GetCurrentState();
	BossEnemyState bossState = dependencies_.bossEnemy->GetCurrentState();

	// 未初期化、何も処理をしていないとき
	if (!initializedStates_) {

		initializedStates_ = true;
		prevPlayerState_ = playerState;
		prevBossState_ = bossState;
		// 最初のフレームで状態遷移扱いで起動するか
		if (triggerOnFirstFrame_) {
			if (FindPlayerRule(playerState)) {
				pendingPlayerState_ = playerState;
			}
			if (FindBossRule(bossState)) {
				pendingBossState_ = bossState;
			}
		}
	} else {
		// 状態遷移を検知したらpendingを更新
		if (playerState != prevPlayerState_) {

			prevPlayerState_ = playerState;
			pendingPlayerState_ = FindPlayerRule(playerState) ? std::optional<PlayerState>(playerState) : std::nullopt;
		}
		if (bossState != prevBossState_) {

			prevBossState_ = bossState;
			pendingBossState_ = FindBossRule(bossState) ? std::optional<BossEnemyState>(bossState) : std::nullopt;
		}
	}

	// 候補が無いなら何もしない
	auto best = BuildBestCandidate();
	if (!best.has_value()) {
		return;
	}

	// 現在処理中でないなら開始
	if (!currentParameter_.has_value()) {
		StartCandidate(context, best.value());
		return;
	}
	if (!allowInterrupt_) {
		return;
	}

	// 優先度比較
	int32_t candidatePriority = best->priority;
	int32_t currentPriority = currentPriority_;
	bool shouldInterrupt = false;
	if (candidatePriority > currentPriority) {

		shouldInterrupt = true;
	} else if (candidatePriority == currentPriority && allowInterruptOnEqualPriority_) {

		// 同優先度でも割り込み許可
		shouldInterrupt = true;
	}
	// 割り込み実行
	if (shouldInterrupt) {

		StartCandidate(context, best.value());
	}
}

std::optional<FollowCameraActionAutoLookTarget::Candidate>
FollowCameraActionAutoLookTarget::BuildBestCandidate() const {

	// 候補を構築
	std::optional<Candidate> result = std::nullopt;

	// プレイヤー状態の候補
	if (pendingPlayerState_.has_value()) {

		PlayerState state = pendingPlayerState_.value();
		// 有効なルールがあるか
		if (const PlayerRule* rule = FindPlayerRule(state)) {

			Candidate candidate{};
			candidate.source = Source::Player;
			candidate.priority = rule->priority;
			candidate.playerState = state;
			candidate.parameter = &rule->parameter;
			result = candidate;
		}
	}
	// ボス状態の候補
	if (pendingBossState_.has_value()) {

		BossEnemyState state = pendingBossState_.value();
		// 有効なルールがあるか
		if (const BossRule* rule = FindBossRule(state)) {

			Candidate candidate{};
			candidate.source = Source::Boss;
			candidate.priority = rule->priority;
			candidate.bossState = state;
			candidate.parameter = &rule->parameter;

			// 候補がまだないならそのまま
			if (!result.has_value()) {

				result = candidate;
			} else {
				// 優先度比較
				if (candidate.priority > result->priority) {

					result = candidate;
				} else if (candidate.priority == result->priority) {

					// 同じ優先度の場合、どちらを優先するか
					if (tieBreak_ == TieBreak::PreferBoss) {

						result = candidate;
					}
				}
			}
		}
	}
	return result;
}

void FollowCameraActionAutoLookTarget::StartCandidate(
	const FollowCameraContext& context, const Candidate& candidate) {

	// パラメータがなければ終了
	if (!candidate.parameter) {
		return;
	}

	// 補間開始回転
	startRotation_ = Quaternion::Normalize(context.cameraRotation);

	// 実行パラメータをコピーして開始
	Parameter parameter = *candidate.parameter;
	parameter.lookTimer.Reset();
	currentParameter_ = parameter;

	// 開始時に目標回転を固定するか
	targetRotation_ = std::nullopt;
	Quaternion baseTarget = GetTargetRotation(currentParameter_.value());
	if (currentParameter_->isLockTarget) {

		targetRotation_ = baseTarget;
	}

	// 最短方向を取得
	// yaw方向決定
	const float yawDelta = SakuEngine::Math::YawSignedDelta(startRotation_, dependencies_.bossEnemy->GetRotation());
	if (std::abs(yawDelta) <= Config::kEpsilon) {

		// どちらでも良いので右にする
		lookYawDirection_ = AnchorToDirection2D::Right;
	} else {

		// 最短方向
		lookYawDirection_ = (0.0f < yawDelta) ? AnchorToDirection2D::Right : AnchorToDirection2D::Left;
	}

	// 現在実行中のソースと優先度を保存
	currentSource_ = candidate.source;
	currentPriority_ = candidate.priority;

	// 処理を開始したので候補から消す
	if (candidate.source == Source::Player) {

		pendingPlayerState_ = std::nullopt;
	}
	if (candidate.source == Source::Boss) {

		pendingBossState_ = std::nullopt;
	}
}

void FollowCameraActionAutoLookTarget::Parameter::ImGui(const std::string& label) {

	ImGui::SeparatorText(label.c_str());
	ImGui::PushID(label.c_str());

	ImGui::Checkbox("isLockTarget", &isLockTarget);
	ImGui::DragFloat("targetXRotation", &targetXRotation, 0.01f);
	ImGui::DragFloat("lookYawOffset", &lookYawOffset, 0.001f);

	lookTimer.ImGui("LookTimer", true);

	ImGui::PopID();
}

//=======================================================================================================================
//	エディター
//=======================================================================================================================

void FollowCameraActionAutoLookTarget::ImGui() {

	if (ImGui::Button("Save Json##FollowCameraActionAutoLookTarget")) {

		SaveJson();
	}

	//============================================================================
	//	Runtime,Config
	//============================================================================

	ImGui::SeparatorText("Runtime");

	ImGui::Text(std::format("currentParameter: {}", currentParameter_.has_value()).c_str());
	ImGui::Text(std::format("currentPriority:  {}", currentPriority_).c_str());
	ImGui::Text(std::format("pendingPlayer:    {}", pendingPlayerState_.has_value()).c_str());
	ImGui::Text(std::format("pendingBoss:      {}", pendingBossState_.has_value()).c_str());

	ImGui::SeparatorText("Config");

	ImGui::Checkbox("triggerOnFirstFrame", &triggerOnFirstFrame_);
	ImGui::Checkbox("allowInterrupt", &allowInterrupt_);
	ImGui::Checkbox("allowInterruptOnEqualPriority", &allowInterruptOnEqualPriority_);
	EnumAdapter<TieBreak>::Combo("TieBreak", &tieBreak_);

	//============================================================================
	//	Player Rules
	//============================================================================

	if (ImGui::CollapsingHeader("Player Rules", ImGuiTreeNodeFlags_DefaultOpen)) {

		ImGui::PushID("Player");

		// 追加
		if (ImGui::Button("Add")) {

			PlayerRule rule{};
			rule.enabled = true;
			rule.priority = 0;
			rule.state = PlayerState::Idle;
			rule.parameter = {};
			playerRules_.push_back(rule);
		}
		ImGui::SameLine();
		// 優先度でソート
		if (ImGui::Button("Sort Priority")) {

			std::sort(playerRules_.begin(), playerRules_.end(),
				[](const PlayerRule& a, const PlayerRule& b) { return a.priority > b.priority; });
		}

		for (int32_t i = 0; i < static_cast<int32_t>(playerRules_.size()); ++i) {

			auto& rule = playerRules_[i];
			ImGui::Separator();
			ImGui::PushID(i);

			ImGui::Checkbox("enabled", &rule.enabled);
			ImGui::DragInt("priority", &rule.priority, 1, -999, 999);

			// 状態コンボボックス
			{
				int32_t current = static_cast<int>(rule.state);
				const int32_t count = static_cast<int32_t>(PlayerState::Count);
				if (ImGui::BeginCombo("state", EnumAdapter<PlayerState>::ToString(rule.state))) {
					for (int s = 0; s < count; ++s) {

						PlayerState state = static_cast<PlayerState>(s);
						bool selected = (s == current);
						if (ImGui::Selectable(EnumAdapter<PlayerState>::ToString(state), selected)) {

							rule.state = state;
						}
						if (selected) {
							ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}
			}

			// パラメータの値操作
			rule.parameter.ImGui("Parameter");

			// 削除
			if (ImGui::Button("Remove")) {

				playerRules_.erase(playerRules_.begin() + i);
				ImGui::PopID();
				--i;
				continue;
			}
			ImGui::PopID();
		}
		ImGui::PopID();
	}

	//============================================================================
	//	Boss Rules
	//============================================================================

	if (ImGui::CollapsingHeader("Boss Rules", ImGuiTreeNodeFlags_DefaultOpen)) {

		ImGui::PushID("Boss");

		// 追加
		if (ImGui::Button("Add")) {

			BossRule rule{};
			rule.enabled = true;
			rule.priority = 0;
			rule.state = BossEnemyState::Idle;
			rule.parameter = {};
			bossRules_.push_back(rule);
		}
		ImGui::SameLine();

		// 優先度でソート
		if (ImGui::Button("Sort Priority")) {

			std::sort(bossRules_.begin(), bossRules_.end(),
				[](const BossRule& a, const BossRule& b) { return a.priority > b.priority; });
		}

		for (int32_t i = 0; i < static_cast<int32_t>(bossRules_.size()); ++i) {

			auto& rule = bossRules_[i];
			ImGui::Separator();
			ImGui::PushID(10000 + i);

			ImGui::Checkbox("enabled", &rule.enabled);
			ImGui::DragInt("priority", &rule.priority, 1, -999, 999);

			// 状態コンボボックス
			{
				int32_t current = static_cast<int>(rule.state);
				const int32_t count = static_cast<int32_t>(BossEnemyState::Count);
				if (ImGui::BeginCombo("state", EnumAdapter<BossEnemyState>::ToString(rule.state))) {
					for (int32_t s = 0; s < count; ++s) {

						BossEnemyState state = static_cast<BossEnemyState>(s);
						bool selected = (s == current);
						if (ImGui::Selectable(EnumAdapter<BossEnemyState>::ToString(state), selected)) {

							rule.state = state;
						}
						if (selected) {
							ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}
			}

			// パラメータの値操作
			rule.parameter.ImGui("Parameter");

			// 削除
			if (ImGui::Button("Remove")) {

				bossRules_.erase(bossRules_.begin() + i);
				ImGui::PopID();
				--i;
				continue;
			}
			ImGui::PopID();
		}
		ImGui::PopID();
	}

	if (ImGui::CollapsingHeader("AreaChecker", ImGuiTreeNodeFlags_DefaultOpen)) {

		areaChecker_->ImGui();
	}
}

void FollowCameraActionAutoLookTarget::ApplyJson() {

	Json data;
	if (!JsonAdapter::LoadCheck("Camera/Follow/actionAutoLookTarget.json", data)) {
		playerRules_.clear();
		bossRules_.clear();
		return;
	}

	//============================================================================
	//	Config
	//============================================================================

	triggerOnFirstFrame_ = data.value("triggerOnFirstFrame_", triggerOnFirstFrame_);
	allowInterrupt_ = data.value("allowInterrupt_", allowInterrupt_);
	allowInterruptOnEqualPriority_ = data.value("allowInterruptOnEqualPriority_", allowInterruptOnEqualPriority_);
	tieBreak_ = EnumAdapter<TieBreak>::FromString(data.value("tieBreak_", "PreferPlayer")).value();

	//============================================================================
	//	Player Rules
	//============================================================================

	playerRules_.clear();
	if (data.contains("playerRules_") && data["playerRules_"].is_array()) {
		for (const auto& item : data["playerRules_"]) {

			PlayerRule rule{};
			rule.enabled = item.value("enabled", true);
			rule.priority = item.value("priority", 0);
			rule.state = EnumAdapter<PlayerState>::FromString(item.value("state", std::string("None"))).value();
			rule.parameter.FromJson(item.value("parameter", Json()));
			playerRules_.push_back(rule);
		}
	}

	//============================================================================
	//	Boss Rules
	//============================================================================

	bossRules_.clear();
	if (data.contains("bossRules_") && data["bossRules_"].is_array()) {
		for (const auto& item : data["bossRules_"]) {

			BossRule rule{};
			rule.enabled = item.value("enabled", true);
			rule.priority = item.value("priority", 0);
			rule.state = EnumAdapter<BossEnemyState>::FromString(item.value("state", std::string("Idle"))).value();
			rule.parameter.FromJson(item.value("parameter", Json()));
			bossRules_.push_back(rule);
		}
	}
}

void FollowCameraActionAutoLookTarget::SaveJson() {

	Json data;

	//============================================================================
	//	Config
	//============================================================================

	data["triggerOnFirstFrame_"] = triggerOnFirstFrame_;
	data["allowInterrupt_"] = allowInterrupt_;
	data["allowInterruptOnEqualPriority_"] = allowInterruptOnEqualPriority_;
	data["tieBreak_"] = EnumAdapter<TieBreak>::ToString(tieBreak_);

	//============================================================================
	//	Player Rules
	//============================================================================

	data["playerRules_"] = Json::array();
	for (auto& rule : playerRules_) {

		Json item;
		item["enabled"] = rule.enabled;
		item["priority"] = rule.priority;
		item["state"] = EnumAdapter<PlayerState>::ToString(rule.state);
		rule.parameter.ToJson(item["parameter"]);
		data["playerRules_"].push_back(item);
	}

	//============================================================================
	//	Boss Rules
	//============================================================================

	data["bossRules_"] = Json::array();
	for (auto& rule : bossRules_) {

		Json item;
		item["enabled"] = rule.enabled;
		item["priority"] = rule.priority;
		item["state"] = EnumAdapter<BossEnemyState>::ToString(rule.state);
		rule.parameter.ToJson(item["parameter"]);
		data["bossRules_"].push_back(item);
	}

	JsonAdapter::Save("Camera/Follow/actionAutoLookTarget.json", data);
}

void FollowCameraActionAutoLookTarget::Parameter::FromJson(const Json& data) {

	isLockTarget = data.value("isLockTarget", true);
	targetXRotation = data.value("targetXRotation", 0.0f);
	lookYawOffset = data.value("lookYawOffset", 0.0f);
	lookTimer.FromJson(data.value("lookTimer", Json()));
}

void FollowCameraActionAutoLookTarget::Parameter::ToJson(Json& data) {

	data["isLockTarget"] = isLockTarget;
	data["targetXRotation"] = targetXRotation;
	data["lookYawOffset"] = lookYawOffset;
	lookTimer.ToJson(data["lookTimer"]);
}

const FollowCameraActionAutoLookTarget::PlayerRule*
FollowCameraActionAutoLookTarget::FindPlayerRule(PlayerState state) const {

	for (const auto& rule : playerRules_) {
		if (rule.enabled && rule.state == state) {

			return &rule;
		}
	}
	return nullptr;
}

const FollowCameraActionAutoLookTarget::BossRule*
FollowCameraActionAutoLookTarget::FindBossRule(BossEnemyState state) const {

	for (const auto& rule : bossRules_) {
		if (rule.enabled && rule.state == state) {
			return &rule;
		}
	}
	return nullptr;
}