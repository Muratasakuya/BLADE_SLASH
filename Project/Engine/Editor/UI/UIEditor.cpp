#include "UIEditor.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Tools/Panels/AssetLibrary/UIAssetLibraryPanel.h>
#include <Engine/Editor/UI/Tools/Panels/CoreAsset/UICoreAssetPanel.h>
#include <Engine/Editor/UI/Tools/Panels/Hierarchy/UIHierarchyPanel.h>
#include <Engine/Editor/UI/Tools/Panels/VisualDesigner/UIVisualDesignerPanel.h>
#include <Engine/Editor/UI/Tools/Panels/Animation/UIAnimationPanel.h>
#include <Engine/Editor/UI/Tools/Panels/Detail/UIDetailPanel.h>
#include <Engine/Editor/UI/Tools/Panels/Palette/UIPalettePanel.h>
#include <Engine/Editor/UI/Tools/Panels/Runtime/UIRuntimePanel.h>

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

	// ランタイムの初期化
	runtime_ = std::make_unique<UIRuntime>();
	runtime_->Init();

	// ツールコンテキストの作成
	toolContext_ = std::make_unique<UIToolContext>();
	toolContext_->asset = asset;
	toolContext_->assetLibrary = assetLibrary_.get();
	toolContext_->animationLibrary = animationLibrary_.get();
	toolContext_->paletteRegistry = paletteRegistry_.get();
	// システムコンテキストの作成
	systemContext_ = std::make_unique<UISystemContext>();
	systemContext_->animationLibrary = animationLibrary_.get();

#if defined(_DEBUG) || defined(_DEVELOPBUILD)

	// パネル群の作成
	panels_.emplace_back(std::make_unique<UIAssetLibraryPanel>());
	panels_.emplace_back(std::make_unique<UICoreAssetPanel>());
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
#endif
}

void UIEditor::Update() {

	// コンテキスト更新
	UpdateContext();

	// 全てのアセットを更新
	assetLibrary_->ForEachAsset([this]([[maybe_unused]] UIAssetHandle handle, UIAssetEntry& entry) {

		// 選択中のアセットかどうかを判定
		bool isSelected = UIAssetHandle::Equal(handle, toolContext_->selectedAsset);
		// アセットの更新
		// プレビュー中かつ選択中のアセットの場合はプレビュー更新を行う
		if (isSelected && systemContext_->preview.enabled && systemContext_->preview.clipUid != 0) {

			runtime_->UpdatePreview(systemContext_.get(), entry.asset);
		}
		// それ以外は通常更新を行う
		else {

			runtime_->Update(systemContext_.get(), entry.asset);
		}
		});
}

void UIEditor::UpdateContext() {

	// プレビュー情報をシステムコンテキストに反映
	// ツールパネルで変更する
	systemContext_->preview.enabled = toolContext_->previewEnabled;
	systemContext_->preview.clipUid = toolContext_->previewClipUid;
	systemContext_->preview.element = toolContext_->selectedElement;
	systemContext_->preview.requestStart = toolContext_->previewStart;
	toolContext_->previewStart = false;
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