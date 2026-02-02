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
#include <Engine/Editor/UI/Tools/Panels/UIRuntimePanel.h>

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

void UIEditor::Init(Asset* asset, const D3D12_GPU_DESCRIPTOR_HANDLE& handle) {

	// アセットライブラリの初期化
	assetLibrary_ = std::make_unique<UIAssetLibrary>();

	// アニメーションライブラリの初期化
	animationLibrary_ = std::make_unique<UIAnimationLibrary>();
	// 全てのアニメーションを読み込み
	animationLibrary_->LoadAllAnimations();

	// パレットの初期化
	paletteRegistry_ = std::make_unique<UIPaletteRegistry>();
	paletteRegistry_->RegisterDefaultItems();

	// ツールコンテキストの作成
	toolContext_ = std::make_unique<UIToolContext>();
	toolContext_->asset = asset;
	toolContext_->assetLibrary = assetLibrary_.get();
	toolContext_->animationLibrary = animationLibrary_.get();
	toolContext_->paletteRegistry = paletteRegistry_.get();
	// システムコンテキストの作成
	systemContext_ = std::make_unique<UISystemContext>();
	systemContext_->animationLibrary = animationLibrary_.get();

	// ランタイムの初期化
	runtime_ = std::make_unique<UIRuntime>();
	runtime_->Init();

	// パネル群の作成
	panels_.emplace_back(std::make_unique<UIAssetPanel>());
	panels_.emplace_back(std::make_unique<UIPalettePanel>());
	panels_.emplace_back(std::make_unique<UIHierarchyPanel>());
	panels_.emplace_back(std::make_unique<UIVisualDesignerPanel>());
	panels_.emplace_back(std::make_unique<UIAnimationPanel>());
	panels_.emplace_back(std::make_unique<UIDetailPanel>());
	panels_.emplace_back(std::make_unique<UIRuntimePanel>());

	// 描画用テクスチャのGPUハンドルを設定
	for (const auto& panel : panels_) {
		if (auto visualDesignerPanel = dynamic_cast<UIVisualDesignerPanel*>(panel.get())) {

			visualDesignerPanel->SetTextureGPUHandle(handle);
			break;
		}
	}
}

void UIEditor::Update() {

	// 全てのアセットを更新
	assetLibrary_->ForEachAsset([this]([[maybe_unused]] UIAssetHandle handle, UIAssetEntry& entry) {
		runtime_->Update(systemContext_.get(), entry.asset);
		});
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