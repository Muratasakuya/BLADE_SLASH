#include "ObjectAreaChecker.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/Line/LineRenderer.h>
#include <Engine/Utility/Enum/EnumAdapter.h>
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Game/Gameplay/Actors/Player/Entity/Player.h>

//============================================================================
//	ObjectAreaChecker classMethods
//============================================================================

void ObjectAreaChecker::Init(const std::string& jsonPath) {

	jsonPath_ = jsonPath;

	// エリアパラメーター初期化
	areaParams_[static_cast<uint32_t>(AreaReactionType::LerpPos)].debugColor = SakuEngine::Color::Cyan();
	areaParams_[static_cast<uint32_t>(AreaReactionType::LerpRotate)].debugColor = SakuEngine::Color::Red();
	areaParams_[static_cast<uint32_t>(AreaReactionType::LerpCamera)].debugColor = SakuEngine::Color::Yellow();

	// json適用
	ApplyJson();
}

void ObjectAreaChecker::Update() {

	// 範囲内チェック
	for (auto& param : areaParams_) {

		// 距離を取得
		float distance = SakuEngine::Math::GetDistance3D(*anchor_, *target_, false, true);
		// 範囲内チェック
		if (distance <= param.range) {

			param.isInRange = true;
		} else {

			param.isInRange = false;
		}
	}
}

void ObjectAreaChecker::ImGui() {

	if (ImGui::Button("Save Json")) {

		SaveJson();
	}
	ImGui::Text("Json Path: %s", jsonPath_.c_str());

	for (uint32_t i = 0; i < static_cast<uint32_t>(AreaReactionType::Count); ++i) {

		auto& param = areaParams_[i];

		const char* typeName = SakuEngine::EnumAdapter<AreaReactionType>::ToString(static_cast<AreaReactionType>(i));
		ImGui::SeparatorText(typeName);
		ImGui::PushID(typeName);

		ImGui::Text(std::format("IsInRange: {}", param.isInRange).c_str());
		ImGui::DragFloat("Range", &param.range, 0.1f);

		SakuEngine::LineRenderer::GetInstance()->Get3D()->DrawCircle(
			12, param.range, anchor_->GetTranslation(), param.debugColor);

		ImGui::PopID();
	}
}

bool ObjectAreaChecker::IsInRange(AreaReactionType reactionType) const {

	uint32_t typeIndex = static_cast<uint32_t>(reactionType);
	return areaParams_[typeIndex].isInRange;
}

void ObjectAreaChecker::ApplyJson() {

	Json data;
	if (!SakuEngine::JsonAdapter::LoadCheck(jsonPath_, data)) {
		return;
	}

	for (uint32_t i = 0; i < static_cast<uint32_t>(AreaReactionType::Count); ++i) {

		auto& param = areaParams_[i];
		const auto& paramJson = data["areaParams"][i];
		param.range = paramJson.value("range", 0.0f);
	}
}

void ObjectAreaChecker::SaveJson() {

	Json data;

	for (uint32_t i = 0; i < static_cast<uint32_t>(AreaReactionType::Count); ++i) {

		const auto& param = areaParams_[i];
		Json paramJson;
		paramJson["range"] = param.range;
		data["areaParams"].push_back(paramJson);
	}

	SakuEngine::JsonAdapter::Save(jsonPath_, data);
}