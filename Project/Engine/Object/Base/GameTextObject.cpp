#include "GameTextObject.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Core/ObjectManager.h>
#include <Engine/Editor/GameObject/ImGuiObjectEditor.h>

//============================================================================
//	GameTextObject classMethods
//============================================================================

void GameTextObject::Init(const std::string& atlasTextureName, const std::string& fontJsonPath,
	const std::string& name, const std::string& groupName) {

	// オブジェクト作成
	objectId_ = objectManager_->CreateTextObject(atlasTextureName, fontJsonPath, name, groupName);

	// データ取得
	transform_ = objectManager_->GetData<TextTransform2D>(objectId_);
	material_ = objectManager_->GetData<MSDFTextMaterial>(objectId_);
	text_ = objectManager_->GetData<MSDFText>(objectId_);
	tag_ = objectManager_->GetData<ObjectTag>(objectId_);

	// ImGui登録
	ImGuiObjectEditor::GetInstance()->Registerobject(objectId_, this);
}

void GameTextObject::ImGui() {

	if (ImGui::BeginTabBar("TextTab")) {
		if (ImGui::BeginTabItem("Text")) {

			text_->ImGui(itemWidth_);
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Transform")) {

			transform_->ImGui(itemWidth_);
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Material")) {

			material_->ImGui(itemWidth_);
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Derived")) {

			DerivedImGui();
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
}