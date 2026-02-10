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

namespace {

	// 要素のUIDを取得
	uint32_t GetElementUid(UIAsset& asset, UIElement::Handle handle) {
		if (auto* element = asset.Get(handle)) {

			return element->uid;
		}
		return 0;
	}
}

void UIRuntime::Init() {

	baseSystems_.clear();
	activeSystems_.clear();
	previewSystems_.clear();

	// 各システムを生成して登録 ↓
	// 親矩形情報の更新
	baseSystems_.emplace_back(std::make_unique<UIUpdateParentRectTransformSystem>());
	// スプライトオブジェクトの同期
	baseSystems_.emplace_back(std::make_unique<UISpriteSyncSystem>());
	// テキストオブジェクトの同期
	baseSystems_.emplace_back(std::make_unique<UITextSyncSystem>());
	// 入力ナビゲーション更新
	activeSystems_.emplace_back(std::make_unique<UIInputNavigationSystem>());
	// 状態アニメーション更新
	activeSystems_.emplace_back(std::make_unique<UIStateAnimationSystem>());

	// プレビュー登録 ↓
	// 親矩形情報の更新
	previewSystems_.push_back(std::make_unique<UIUpdateParentRectTransformSystem>());
	// スプライトオブジェクトの同期
	previewSystems_.push_back(std::make_unique<UISpriteSyncSystem>());
	// テキストオブジェクトの同期
	previewSystems_.push_back(std::make_unique<UITextSyncSystem>());
	// UIアニメーションプレビュー
	previewSystems_.emplace_back(std::make_unique<UIAnimationPreviewSystem>());

	// プレビューアセットをクリア
	ClearPreviewAsset();
}

void UIRuntime::Update(UISystemContext* context, UIAsset& asset) {

	// プレビューが無効ならプレビュー用要素をクリア
	if (!context->preview.enabled || context->preview.clipUid == 0) {
		if (previewAlive_) {

			ClearPreviewAsset();
		}
		context->preview.previewElement = {};
	}

	// 通常更新
	context->preview.isPreviewPass = false;
	context->preview.suppressOriginalSubtree = false;

	// 各システムで更新
	for (const auto& system : baseSystems_) {

		system->Update(context, asset);
	}
	if (asset.isActive) {
		for (const auto& system : activeSystems_) {

			system->Update(context, asset);
		}
	}
}

void UIRuntime::UpdatePreview(UISystemContext* context, UIAsset& originalAsset) {

	// プレビュー条件が揃ってないなら通常更新へ
	if (!context->preview.enabled || context->preview.clipUid == 0 ||
		!context->preview.element.IsValid() || !originalAsset.elements.IsAlive(context->preview.element)) {

		// 既存プレビューアセットが残ってたら消す
		if (previewAlive_) {

			ClearPreviewAsset();
		}
		context->preview.previewElement = {};

		// 普通に更新を行う
		Update(context, originalAsset);
		return;
	}

	// 元アセットの更新、プレビューアセットの更新を分けて行う
	context->preview.isPreviewPass = false;
	context->preview.suppressOriginalSubtree = true;

	// 各システムで更新
	for (const auto& system : baseSystems_) {

		system->Update(context, originalAsset);
	}
	if (originalAsset.isActive) {
		for (const auto& system : activeSystems_) {

			system->Update(context, originalAsset);
		}
	}

	// 選択要素が変わったらプレビューアセットを作り直す
	const uint32_t uid = GetElementUid(originalAsset, context->preview.element);
	if (!previewAlive_ || previewSourceElementUid_ != uid) {

		// 構築
		RebuildPreviewAsset(originalAsset, context->preview.element);
		previewSourceElementUid_ = uid;

		// 作り直したら再生し直させる
		context->preview.requestStart = true;
	}

	// プレビューアセットの更新
	context->preview.isPreviewPass = true;
	context->preview.suppressOriginalSubtree = false;
	context->preview.previewElement = previewRoot_;
	for (const auto& system : previewSystems_) {

		system->Update(context, previewAsset_);
	}

	// プレビュー終了
	context->preview.isPreviewPass = false;
}

void UIRuntime::ClearPreviewAsset() {

	if (previewAlive_) {

		// 生成済みオブジェクトを破棄
		if (previewAsset_.elements.IsAlive(previewRoot_)) {

			DestroyObjectRecursive(previewAsset_, previewRoot_);
			previewAsset_.DestroyRecursive(previewRoot_);
		}
	}

	// プレビュー情報をリセット
	previewAsset_.Init();
	previewRoot_ = {};
	previewSourceElementUid_ = 0;
	previewAlive_ = false;
}

void UIRuntime::RebuildPreviewAsset(UIAsset& originalAsset, UIElement::Handle originalElement) {

	// プレビューアセットが未初期化なら初期化
	if (!previewAlive_) {

		previewAsset_.Init();
		previewAlive_ = true;
	}
	// 既存要素があれば破棄
	else if (previewAsset_.elements.IsAlive(previewRoot_)) {

		DestroyObjectRecursive(previewAsset_, previewRoot_);
		previewAsset_.DestroyRecursive(previewRoot_);
		previewRoot_ = {};
	}

	// プレビューアセットのルート要素に親矩形トランスフォームを追加
	previewAsset_.AddComponentByType(previewAsset_.rootHandle, UIComponentType::ParentRectTransform);
	// トランスフォームを初期化
	auto* transform = static_cast<UIParentRectTransform*>(previewAsset_.FindComponent(previewAsset_.rootHandle, UIComponentType::ParentRectTransform));
	transform->transform.Init(nullptr);
	transform->transform.UpdateMatrix();

	// 元要素をプレファブとして書き出してインポート
	Json prefab{};
	originalAsset.ExportJsonElementPrefab(prefab, originalElement);
	previewRoot_ = previewAsset_.ImportJsonElementPrefab(prefab, previewAsset_.rootHandle);
}