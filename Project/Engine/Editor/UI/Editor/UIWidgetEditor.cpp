#include "UIWidgetEditor.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================

//============================================================================
//	UIWidgetEditor classMethods
//============================================================================

UIWidgetEditor* UIWidgetEditor::instance_ = nullptr;

UIWidgetEditor* UIWidgetEditor::GetInstance() {

	if (instance_ == nullptr) {
		instance_ = new UIWidgetEditor();
	}
	return instance_;
}

void UIWidgetEditor::Finalize() {

	if (instance_ != nullptr) {

		delete instance_;
		instance_ = nullptr;
	}
}

void UIWidgetEditor::Init(const D3D12_GPU_DESCRIPTOR_HANDLE& renderTextureGPUHandle) {

	// エディタ機能初期化
	// パレット
	palette_ = std::make_unique<UIWidgetPalette>();
	// 詳細
	detail_ = std::make_unique<UIWidgetDetail>();
	// ビジュアルデザイナー
	visualDesigner_ = std::make_unique<UIWidgetVisualDesigner>();
	visualDesigner_->Init(renderTextureGPUHandle);
	// ヒエラルキー
	hierarchy_ = std::make_unique<UIWidgetHierarchy>();
	// アニメーション
	animation_ = std::make_unique<UIWidgetAnimation>();
}

void UIWidgetEditor::ImGui() {

	// エディターを開くかどうか
	ImGui::Checkbox("Open Editor", &isAlwaysOpenEditor_);

	if (!isAlwaysOpenEditor_) {
		return;
	}

	// 各機能のImGuiエディタ表示
	palette_->ImGui();
	detail_->ImGui();
	visualDesigner_->ImGui();
	hierarchy_->ImGui();
	animation_->ImGui();
}