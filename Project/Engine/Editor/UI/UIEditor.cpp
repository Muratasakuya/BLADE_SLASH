#include "UIEditor.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Tools/Panels/UIAssetPanel.h>
#include <Engine/Editor/UI/Tools/Panels/UIHierarchyPanel.h>
#include <Engine/Editor/UI/Tools/Panels/UIVisualDesignerPanel.h>
#include <Engine/Editor/UI/Tools/Panels/UIAnimationPanel.h>
#include <Engine/Editor/UI/Tools/Panels/UIDetailPanel.h>
#include <Engine/Editor/UI/Tools/Panels/UIPalettePanel.h>

//============================================================================
//	UIEditor classMethods
//============================================================================

UIEditor* UIEditor::instance_ = nullptr;

UIEditor* UIEditor::GetInstance() {

	if (instance_ == nullptr) {
		instance_ = new UIEditor();
	}
	return instance_;
}

void UIEditor::Finalize() {

	if (instance_ != nullptr) {

		delete instance_;
		instance_ = nullptr;
	}
}

void UIEditor::Init(const D3D12_GPU_DESCRIPTOR_HANDLE& handle) {

	// パレットの初期化
	paletteRegistry_.RegisterDefaultItems();

	// ツールコンテキストの作成
	toolContext_ = std::make_unique<UIToolContext>();

	// パネル群の作成
	panels_.emplace_back(std::make_unique<UIAssetPanel>());
	panels_.emplace_back(std::make_unique<UIPalettePanel>());
	panels_.emplace_back(std::make_unique<UIHierarchyPanel>());
	panels_.emplace_back(std::make_unique<UIVisualDesignerPanel>());
	panels_.emplace_back(std::make_unique<UIAnimationPanel>());
	panels_.emplace_back(std::make_unique<UIDetailPanel>());

	// 描画用テクスチャのGPUハンドルを設定
	for (const auto& panel : panels_) {
		if (auto visualDesignerPanel = dynamic_cast<UIVisualDesignerPanel*>(panel.get())) {

			visualDesignerPanel->SetTextureGPUHandle(handle);
			break;
		}
	}
}

void UIEditor::ImGui() {

	ImGui::Checkbox("isOpen", &isOpen_);
}

void UIEditor::EditPanels() {

	if (!isOpen_) {
		return;
	}

	// パネル群の表示
	for (const auto& panel : panels_) {

		ImGui::Begin(panel->GetName());

		panel->ImGui(*toolContext_.get());

		ImGui::End();
	}
}