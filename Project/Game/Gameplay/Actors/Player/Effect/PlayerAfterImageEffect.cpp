#include "PlayerAfterImageEffect.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/PostProcess/Core/PostProcessSystem.h>
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Game/Effects/PostProcess/DepthOutlineUpdater.h>
#include <Game/Effects/PostProcess/PlayerAfterImageUpdater.h>

//============================================================================
//	PlayerAfterImageEffect classMethods
//============================================================================

PlayerAfterImageEffect::~PlayerAfterImageEffect() {

	// 完全に終了させる
	// エフェクトリセット
	SakuEngine::PostProcessSystem* postProcessSystem = SakuEngine::PostProcessSystem::GetInstance();

	// 残像表現エフェクト
	postProcessSystem->GetUpdater<PlayerAfterImageUpdater>(PostProcessType::PlayerAfterImage)->Reset();
	// アウトライン
	postProcessSystem->GetUpdater<DepthOutlineUpdater>(PostProcessType::DepthBasedOutline)->Reset();
}

void PlayerAfterImageEffect::Init(const std::string& fileName) {

	// ファイル名保存
	fileName_ = fileName;
	// json適用
	ApplyJson();
}

void PlayerAfterImageEffect::Start(std::vector<SakuEngine::GameObject3D*>& objects) {

	// α値を0.0f、加算合成にする
	for (auto& object : objects) {

		object->SetAlpha(0.0f);
		object->SetBlendMode(SakuEngine::BlendMode::kBlendModeAdd);
	}

	// エフェクト開始
	SakuEngine::PostProcessSystem* postProcessSystem = SakuEngine::PostProcessSystem::GetInstance();

	// 残像表現エフェクト
	postProcessSystem->GetUpdater<PlayerAfterImageUpdater>(PostProcessType::PlayerAfterImage)->Start(color_);
	// アウトライン
	postProcessSystem->GetUpdater<DepthOutlineUpdater>(PostProcessType::DepthBasedOutline)->Start(color_, edgeScale_);
}

void PlayerAfterImageEffect::End(std::vector<SakuEngine::GameObject3D*>& objects) {

	// α値を1.0fにする、通常の合成に戻す
	for (auto& object : objects) {

		object->SetAlpha(1.0f);
		object->SetBlendMode(SakuEngine::BlendMode::kBlendModeNormal);
	}

	// エフェクトリセット
	SakuEngine::PostProcessSystem* postProcessSystem = SakuEngine::PostProcessSystem::GetInstance();

	// 残像表現エフェクト
	postProcessSystem->GetUpdater<PlayerAfterImageUpdater>(PostProcessType::PlayerAfterImage)->Reset();
	// アウトライン
	postProcessSystem->GetUpdater<DepthOutlineUpdater>(PostProcessType::DepthBasedOutline)->Reset();
}

void PlayerAfterImageEffect::ImGui(bool isSeparate) {

	if (isSeparate) {

		ImGui::SeparatorText("PlayerAfterImageEffect");
	}

	ImGui::PushID(fileName_.c_str());

	if (ImGui::Button("Save Json")) {

		SaveJson();
	}

	ImGui::ColorEdit4("color", &color_.r);
	ImGui::DragFloat("edgeScale", &edgeScale_, 0.01f);

	ImGui::PopID();
}

void PlayerAfterImageEffect::ApplyJson() {

	Json data;
	if (!SakuEngine::JsonAdapter::LoadCheck("Player/AfterImageEffect/" + fileName_ + ".json", data)) {
		return;
	}

	color_ = SakuEngine::Color::FromJson(data.value("color", Json()));
	edgeScale_ = data.value("edgeScale", 1.0f);
}

void PlayerAfterImageEffect::SaveJson() {

	Json data;

	data["color"] = color_.ToJson();
	data["edgeScale"] = edgeScale_;

	SakuEngine::JsonAdapter::Save("Player/AfterImageEffect/" + fileName_, data);
}