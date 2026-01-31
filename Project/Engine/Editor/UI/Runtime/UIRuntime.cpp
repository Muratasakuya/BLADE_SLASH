#include "UIRuntime.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/System/ObjectSync/UISpriteSyncSystem.h>
#include <Engine/Editor/UI/System/ObjectSync/UITextSyncSystem.h>
#include <Engine/Editor/UI/System/ParentRect/UIUpdateParentRectTransformSystem.h>
#include <Engine/Editor/UI/System/InputNavigation/UIInputNavigationSystem.h>

//============================================================================
//	UIRuntime classMethods
//============================================================================

void UIRuntime::Init() {

	// 各システムを生成して登録
	systems_.emplace_back(std::make_unique<UIUpdateParentRectTransformSystem>());
	systems_.emplace_back(std::make_unique<UISpriteSyncSystem>());
	systems_.emplace_back(std::make_unique<UITextSyncSystem>());
	systems_.emplace_back(std::make_unique<UIInputNavigationSystem>());
}

void UIRuntime::Update(UIAsset& asset) {

	// 各システムで更新
	for (const auto& system : systems_) {

		system->Update(asset);
	}
}