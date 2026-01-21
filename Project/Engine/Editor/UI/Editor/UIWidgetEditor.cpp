#include "UIWidgetEditor.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Editor/Panels/UIWidgetAnimation.h>
#include <Engine/Editor/UI/Editor/Panels/UIWidgetDetail.h>
#include <Engine/Editor/UI/Editor/Panels/UIWidgetHierarchy.h>
#include <Engine/Editor/UI/Editor/Panels/UIWidgetPalette.h>
#include <Engine/Editor/UI/Editor/Panels/UIWidgetVisualDesigner.h>
#include <Engine/Utility/Timer/GameTimer.h>

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

	// ドキュメント初期化
	document_ = std::make_unique<UIWidgetDocument>();
	document_->CreateDocument();
	context_.document = document_.get();
	// ウィジェット型登録初期化
	registry_ = std::make_unique<UIWidgetTypeRegistry>();
	registry_->RegisterDefaults();
	context_.registry = registry_.get();
	// プレビューランタイム初期化
	preview_ = std::make_unique<UIWidgetPreviewRuntime>();
	preview_->Init();
	context_.preview = preview_.get();

	// エディタ機能初期化
	panels_.emplace_back(std::make_unique<UIWidgetPalette>());
	panels_.emplace_back(std::make_unique<UIWidgetDetail>());
	panels_.emplace_back(std::make_unique<UIWidgetHierarchy>());
	panels_.emplace_back(std::make_unique<UIWidgetAnimation>());
	panels_.emplace_back(std::make_unique<UIWidgetVisualDesigner>());

	// ビジュアルデザイナに描画用テクスチャを渡す
	if (auto visualDesigner = static_cast<UIWidgetVisualDesigner*>(panels_.back().get())) {

		visualDesigner->Init(renderTextureGPUHandle);
	}

	// 初回ビルド
	context_.RequestPreviewRebuild();
	ApplyDocumentToPreviewIfNeeded();
}

UIWidgetRefHandle UIWidgetEditor::GetCurrentHandle(const std::string& assetPath) const {

	return document_->ExportHandle(assetPath);
}

void UIWidgetEditor::Create() {

	// ドキュメント新規作成
	document_->CreateDocument();
	context_.selection.Clear();
	context_.RequestPreviewRebuild();
	context_.documentDirty = false;
}

void UIWidgetEditor::Update() {

	// ドキュメントの変更をプレビューに適用
	ApplyDocumentToPreviewIfNeeded();

	// プレビュー表示更新
	preview_->Tick(GameTimer::GetDeltaTime());
}

void UIWidgetEditor::ImGui() {

	// エディターを開くかどうか
	ImGui::Checkbox("Open Editor", &isOpenEditor_);
}

void UIWidgetEditor::EditUIWidget() {

	// エディターが開かれていなければ何もしない
	if (!isOpenEditor_) {
		return;
	}

	ImGui::Begin("UI WidgetEditor");

	// ドキュメントの作成
	if (ImGui::Button("Create")) {

		Create();
	}
	ImGui::Checkbox("documentDirty", &context_.documentDirty);

	ImGui::End();

	// 各パネル描画
	for (const auto& panel : panels_) {
		if (ImGui::Begin(panel->GetPanelName())) {

			panel->Draw(context_);
		}
		ImGui::End();
	}
}

void UIWidgetEditor::ApplyDocumentToPreviewIfNeeded() {

	// プレビュー再構築要求があれば再構築
	if (context_.rebuildPreviewRequested) {

		context_.preview->RebuildFromDocument(*context_.document);
		context_.rebuildPreviewRequested = false;
		context_.syncPreviewRequested = false;
		return;
	}

	// プロパティ同期要求があれば同期
	if (context_.syncPreviewRequested) {

		// プロパティ同期して終了
		context_.preview->SyncPropertiesFromDocument(*context_.document);
		context_.syncPreviewRequested = false;
	}
}

bool UIWidgetEditor::LoadFromJson(const Json& data) {

	bool result = document_->FromJson(data);
	// 読み込み成功したら選択状態クリア
	if (result) {

		context_.selection.Clear();
		context_.RequestPreviewRebuild();
		context_.documentDirty = false;
	}
	return result;
}

void UIWidgetEditor::SaveToJson(Json& data) const {

	document_->ToJson(data);
}