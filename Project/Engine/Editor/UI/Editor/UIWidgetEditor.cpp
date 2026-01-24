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
#include <Engine/Editor/UI/Editor/Panels/UIWidgetDocumentsPanel.h>
#include <Engine/Editor/UI/UISlateWidget/Mehtods/UIWidgetFactory.h>
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

	// レジストリ初期化
	registry_ = std::make_unique<UIWidgetTypeRegistry>();
	registry_->RegisterDefaults();
	UIWidgetFactory::GetInstance().RegisterDefaults();
	// プレビュー表示初期化
	preview_ = std::make_unique<UIWidgetPreviewRuntime>();

	// ドキュメントを1つ確保
	EnsureAtLeastOneDocument();

	// コンテキストを初期バインド
	context_.registry = registry_.get();
	context_.preview = preview_.get();
	context_.editor = this;
	BindActiveDocumentToContext();

	// プレビュー表示初期化
	preview_->Init();
	context_.RequestPreviewRebuild();

	// パネル群初期化
	panels_.emplace_back(std::make_unique<UIWidgetHierarchy>());
	panels_.emplace_back(std::make_unique<UIWidgetPalette>());
	panels_.emplace_back(std::make_unique<UIWidgetDetail>());
	panels_.emplace_back(std::make_unique<UIWidgetAnimation>());
	panels_.emplace_back(std::make_unique<UIWidgetDocumentsPanel>());
	panels_.emplace_back(std::make_unique<UIWidgetVisualDesigner>());

	// ビジュアルデザイナに描画用テクスチャを渡す
	if (auto visualDesigner = dynamic_cast<UIWidgetVisualDesigner*>(panels_.back().get())) {

		visualDesigner->Init(renderTextureGPUHandle);
	}
}

bool UIWidgetEditor::CreateDocument(const std::string& key) {

	// キーが空、もしくは存在しない場合は失敗
	if (key.empty()) {
		return false;
	}
	if (documents_.find(key) != documents_.end()) {
		return false;
	}

	// 新規作成してアクティブにする
	std::unique_ptr<UIWidgetDocument> document = std::make_unique<UIWidgetDocument>();
	document->CreateDocument();

	// 登録とアクティブ化
	documents_[key] = std::move(document);
	activeKey_ = key;
	BindActiveDocumentToContext();

	// 選択状態クリア
	context_.selection.Clear();
	context_.RequestPreviewRebuild();
	return true;
}

bool UIWidgetEditor::DeleteDocument(const std::string& key) {

	// キーが存在しない場合はfalseを返す
	auto it = documents_.find(key);
	if (it == documents_.end()) {
		return false;
	}

	// 削除処理
	bool deletingActive = (key == activeKey_);
	documents_.erase(it);

	// ドキュメントが1つもなくなった場合は新規作成
	if (documents_.empty()) {

		EnsureAtLeastOneDocument();
	}
	// アクティブドキュメントが削除された場合は先頭のドキュメントをアクティブにする
	else if (deletingActive) {

		activeKey_ = documents_.begin()->first;
	}

	// コンテキスト再バインド
	BindActiveDocumentToContext();
	context_.selection.Clear();
	context_.RequestPreviewRebuild();
	return true;
}

bool UIWidgetEditor::SetActiveDocument(const std::string& key) {

	// キーが存在しない場合はfalseを返す
	if (activeKey_ == key) {
		return true;
	}
	auto it = documents_.find(key);
	if (it == documents_.end()) {
		return false;
	}

	// アクティブ切替
	activeKey_ = key;
	// コンテキスト再バインド
	BindActiveDocumentToContext();
	context_.selection.Clear();
	context_.RequestPreviewRebuild();
	return true;
}

void UIWidgetEditor::BindActiveDocumentToContext() {

	// コンテキストにアクティブドキュメントをバインド
	context_.document = GetActiveDocument();
}

void UIWidgetEditor::EnsureAtLeastOneDocument() {

	// ドキュメントが1つもない場合はデフォルトドキュメントを作成
	if (!documents_.empty()) {
		return;
	}
	CreateDocument("Default");
}

UIWidgetDocument* UIWidgetEditor::GetActiveDocument() const {

	// アクティブドキュメント取得、存在しなければnullptrを返す
	auto it = documents_.find(activeKey_);
	return (it != documents_.end()) ? it->second.get() : nullptr;
}

void UIWidgetEditor::Update() {

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

	// 各パネル描画
	for (const auto& panel : panels_) {
		if (ImGui::Begin(panel->GetPanelName())) {

			panel->Draw(context_);
		}
		ImGui::End();
	}
}