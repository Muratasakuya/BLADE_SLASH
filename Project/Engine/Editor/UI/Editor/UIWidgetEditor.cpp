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

// メモ
// UEのエディター機能を参考にして作成する
// パレット: ゲームに追加できるウィジェットのリストを表示する。UISlateWidgetを継承したクラスを列挙して表示する
// ヒエラルキー: 現在のUIツリー構造を表示、ここにパレット要素をドラッグアンドドロップして追加できるようにする
// 詳細: 選択されているウィジェットのプロパティを表示、編集できるようにする
// アニメーション処理: ウィジェットのアニメーションを作成、編集できるようにする

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
	ImGui::Checkbox("Open Editor", &isOpenEditor_);

	if (isOpenEditor_) {

		// 各機能のImGuiエディタ表示
		palette_->ImGui();
		detail_->ImGui();
		visualDesigner_->ImGui();
		hierarchy_->ImGui();
		animation_->ImGui();
	}
}