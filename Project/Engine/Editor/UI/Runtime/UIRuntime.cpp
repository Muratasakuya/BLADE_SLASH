#include "UIRuntime.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Systems/ObjectSync/UISpriteSyncSystem.h>
#include <Engine/Editor/UI/Systems/ObjectSync/UITextSyncSystem.h>
#include <Engine/Editor/UI/Systems/ParentRect/UIUpdateParentRectTransformSystem.h>
#include <Engine/Editor/UI/Systems/InputNavigation/UIInputNavigationSystem.h>
#include <Engine/Editor/UI/Systems/Animation/UIStateAnimationSystem.h>
#include <Engine/Editor/UI/Systems/Animation/UIAnimationPreviewSystem.h>

//============================================================================
//	UIRuntime classMethods
//============================================================================

void UIRuntime::Init() {

	systems_.clear();
	previewSystems_.clear();

	// 各システムを生成して登録 ↓
	// 入力によるナビゲーション更新
	systems_.emplace_back(std::make_unique<UIInputNavigationSystem>());
	// 状態に応じたアニメーション再生
	systems_.emplace_back(std::make_unique<UIStateAnimationSystem>());
	// 親矩形情報の更新
	systems_.emplace_back(std::make_unique<UIUpdateParentRectTransformSystem>());
	// スプライトオブジェクトの同期
	systems_.emplace_back(std::make_unique<UISpriteSyncSystem>());
	// テキストオブジェクトの同期
	systems_.emplace_back(std::make_unique<UITextSyncSystem>());
	
	// プレビュー登録 ↓
	// 親矩形情報の更新
	previewSystems_.push_back(std::make_unique<UIUpdateParentRectTransformSystem>());
	// スプライトオブジェクトの同期
	previewSystems_.push_back(std::make_unique<UISpriteSyncSystem>());
	// テキストオブジェクトの同期
	previewSystems_.push_back(std::make_unique<UITextSyncSystem>());
	// UIアニメーションプレビュー
	previewSystems_.emplace_back(std::make_unique<UIAnimationPreviewSystem>());
}

void UIRuntime::Update(UISystemContext* context, UIAsset& asset) {

	// 各システムで更新
	for (const auto& system : systems_) {

		system->Update(context, asset);
	}
}

void UIRuntime::UpdatePreview(UISystemContext* context, UIAsset& asset) {

	// 各システムで更新
	for (const auto& system : previewSystems_) {

		system->Update(context, asset);
	}
}