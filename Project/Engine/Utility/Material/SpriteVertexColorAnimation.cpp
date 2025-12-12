#include "SpriteVertexColorAnimation.h"

using namespace SakuEngine;

//============================================================================
//	SpriteVertexColorAnimation classMethods
//============================================================================

void SpriteVertexColorAnimation::Update(GameObject2D& object) {

	// アニメーションが無効なら何もしない
	if (!isEnable_) {
		// 頂点カラー無効
		object.SetUseVertexColor(false);
		return;
	}

	// 頂点カラー有効
	object.SetUseVertexColor(true);

	// 頂点カラーアニメーション更新
	SakuEngine::Color leftBottomColor = Color::White();
	SakuEngine::Color leftTopColor = Color::White();
	SakuEngine::Color rightBottomColor = Color::White();
	SakuEngine::Color rightTopColor = Color::White();
	leftBottomColorAnim_.LerpValue(leftBottomColor);
	leftTopColorAnim_.LerpValue(leftTopColor);
	rightBottomColorAnim_.LerpValue(rightBottomColor);
	rightTopColorAnim_.LerpValue(rightTopColor);

	// 頂点カラー設定
	object.SetVertexColor(SpriteVertexPos::LeftBottom, leftBottomColor);
	object.SetVertexColor(SpriteVertexPos::LeftTop, leftTopColor);
	object.SetVertexColor(SpriteVertexPos::RightBottom, rightBottomColor);
	object.SetVertexColor(SpriteVertexPos::RightTop, rightTopColor);
}

void SakuEngine::SpriteVertexColorAnimation::Start() {

	isEnable_ = true;

	// アニメーション開始
	leftBottomColorAnim_.Start();
	leftTopColorAnim_.Start();
	rightBottomColorAnim_.Start();
	rightTopColorAnim_.Start();
}

void SakuEngine::SpriteVertexColorAnimation::Reset() {

	isEnable_ = false;

	// アニメーション停止
	leftBottomColorAnim_.Reset();
	leftTopColorAnim_.Reset();
	rightBottomColorAnim_.Reset();
	rightTopColorAnim_.Reset();
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

			leftBottomColorAnim_.ImGuiParam("LeftBottom", true);
			leftTopColorAnim_.ImGuiParam("LeftTop", true);
			rightBottomColorAnim_.ImGuiParam("RightBottom", true);
			rightTopColorAnim_.ImGuiParam("RightTop", true);
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Timer")) {

			leftBottomColorAnim_.ImGuiTimer("LeftBottom", true);
			leftTopColorAnim_.ImGuiTimer("LeftTop", true);
			rightBottomColorAnim_.ImGuiTimer("RightBottom", true);
			rightTopColorAnim_.ImGuiTimer("RightTop", true);
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Loop")) {

			leftBottomColorAnim_.ImGuiLoop("LeftBottom", true);
			leftTopColorAnim_.ImGuiLoop("LeftTop", true);
			rightBottomColorAnim_.ImGuiLoop("RightBottom", true);
			rightTopColorAnim_.ImGuiLoop("RightTop", true);
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}

	// 左下
	if (ImGui::CollapsingHeader("LeftBottom")) {

		leftBottomColorAnim_.ImGui("LeftBottom");
	}
	ImGui::Spacing();
	// 左上
	if (ImGui::CollapsingHeader("LeftTop")) {

		leftTopColorAnim_.ImGui("LeftTop");
	}
	ImGui::Spacing();
	// 右下
	if (ImGui::CollapsingHeader("RightBottom")) {

		rightBottomColorAnim_.ImGui("RightBottom");
	}
	ImGui::Spacing();
	// 右上
	if (ImGui::CollapsingHeader("RightTop")) {

		rightTopColorAnim_.ImGui("RightTop");
	}

	ImGui::PopID();
}

void SpriteVertexColorAnimation::ToJson(Json& data) {

	leftBottomColorAnim_.ToJson(data["leftBottomColorAnim_"]);
	leftTopColorAnim_.ToJson(data["leftTopColorAnim_"]);
	rightBottomColorAnim_.ToJson(data["rightBottomColorAnim_"]);
	rightTopColorAnim_.ToJson(data["rightTopColorAnim_"]);
}

void SpriteVertexColorAnimation::FromJson(const Json& data) {

	if (data.empty()) {
		return;
	}

	leftBottomColorAnim_.FromJson(data["leftBottomColorAnim_"]);
	leftTopColorAnim_.FromJson(data["leftTopColorAnim_"]);
	rightBottomColorAnim_.FromJson(data["rightBottomColorAnim_"]);
	rightTopColorAnim_.FromJson(data["rightTopColorAnim_"]);
}