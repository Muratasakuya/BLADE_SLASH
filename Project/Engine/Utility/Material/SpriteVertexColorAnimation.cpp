#include "SpriteVertexColorAnimation.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Enum/EnumAdapter.h>

//============================================================================
//	SpriteVertexColorAnimation classMethods
//============================================================================

SakuEngine::SpriteVertexColorAnimation::SpriteVertexColorAnimation() {

	for (uint32_t i = 0; i < kSpriteVertexPosNum; ++i) {
		currentColors_[i] = Color::White();
	}
}

void SpriteVertexColorAnimation::Update(GameObject2D& object) {

	// アニメーションが無効なら何もしない
	if (!isEnable_) {
		// 頂点カラー無効
		object.SetUseVertexColor(false);
		return;
	}

	// 頂点カラー有効
	object.SetUseVertexColor(true);

	for (uint32_t i = 0; i < kSpriteVertexPosNum; ++i) {

		// 頂点カラーアニメーション更新
		colorAnims_[i].LerpValue(currentColors_[i]);

		// 頂点カラー設定
		object.SetVertexColor(static_cast<SpriteVertexPos>(i), currentColors_[i]);
	}
}

void SakuEngine::SpriteVertexColorAnimation::Update() {

	// アニメーションが無効なら何もしない
	if (!isEnable_) {
		return;
	}

	for (uint32_t i = 0; i < kSpriteVertexPosNum; ++i) {

		// 頂点カラーアニメーション更新
		colorAnims_[i].LerpValue(currentColors_[i]);
	}
}

void SakuEngine::SpriteVertexColorAnimation::Start() {

	if (!isEnable_) {
		for (uint32_t i = 0; i < kSpriteVertexPosNum; ++i) {

			// アニメーション開始
			colorAnims_[i].Start();
		}
	}
	isEnable_ = true;
}

void SakuEngine::SpriteVertexColorAnimation::Reset() {

	if (isEnable_) {

		for (uint32_t i = 0; i < kSpriteVertexPosNum; ++i) {

			// アニメーション停止
			colorAnims_[i].Reset();
		}
	}
	isEnable_ = false;
}

void SpriteVertexColorAnimation::ImGui(const std::string& label) {

	ImGui::SeparatorText(label.c_str());
	ImGui::PushID(label.c_str());

	if (ImGui::Checkbox("Enable", &isEnable_)) {
		if (!isEnable_) {
			Reset();
		}
		if (isEnable_) {
			Start();
		}
	}

	if (ImGui::BeginTabBar("ConsoleTabBar")) {

		if (ImGui::BeginTabItem("Color")) {
			for (uint32_t i = 0; i < kSpriteVertexPosNum; ++i) {

				SpriteVertexPos pos = static_cast<SpriteVertexPos>(i);
				colorAnims_[i].ImGuiParam(EnumAdapter<SpriteVertexPos>::ToString(pos), true);
			}
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Timer")) {
			for (uint32_t i = 0; i < kSpriteVertexPosNum; ++i) {

				SpriteVertexPos pos = static_cast<SpriteVertexPos>(i);
				colorAnims_[i].ImGuiTimer(EnumAdapter<SpriteVertexPos>::ToString(pos), true);
			}
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Loop")) {
			for (uint32_t i = 0; i < kSpriteVertexPosNum; ++i) {

				SpriteVertexPos pos = static_cast<SpriteVertexPos>(i);
				colorAnims_[i].ImGuiLoop(EnumAdapter<SpriteVertexPos>::ToString(pos), true);
			}
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}

	for (uint32_t i = 0; i < kSpriteVertexPosNum; ++i) {

		SpriteVertexPos pos = static_cast<SpriteVertexPos>(i);
		std::string posLabel = EnumAdapter<SpriteVertexPos>::ToString(pos);
		if (ImGui::CollapsingHeader(posLabel.c_str())) {

			colorAnims_[i].ImGuiLoop(posLabel, true);
		}
		ImGui::Spacing();
	}
	ImGui::PopID();
}

void SpriteVertexColorAnimation::ToJson(Json& data) {

	colorAnims_[0].ToJson(data["leftBottomColorAnim_"]);
	colorAnims_[1].ToJson(data["leftTopColorAnim_"]);
	colorAnims_[2].ToJson(data["rightBottomColorAnim_"]);
	colorAnims_[3].ToJson(data["rightTopColorAnim_"]);
}

void SpriteVertexColorAnimation::FromJson(const Json& data) {

	if (data.empty()) {
		return;
	}

	colorAnims_[0].FromJson(data["leftBottomColorAnim_"]);
	colorAnims_[1].FromJson(data["leftTopColorAnim_"]);
	colorAnims_[2].FromJson(data["rightBottomColorAnim_"]);
	colorAnims_[3].FromJson(data["rightTopColorAnim_"]);
}