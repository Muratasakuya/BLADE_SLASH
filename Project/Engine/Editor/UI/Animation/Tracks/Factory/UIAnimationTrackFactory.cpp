#include "UIAnimationTrackFactory.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Animation/Tracks/Factory/UIAnimationValueSourceFactory.h>
#include <Engine/Editor/UI/Animation/Tracks/UIPropertyTrackRuntime.h>

//============================================================================
//	UIAnimationTrackFactory classMethods
//============================================================================

std::unique_ptr<IUIAnimationTrackRuntime> UIAnimationTrackFactory::Create(const UIAnimationTrackDefinition& definition) {

	// タイプ別に生成
	switch (definition.type) {
	case UIAnimationDataType::Translation: {

		auto valueSource = CreateValueSource<Vector2>(definition.valueSourceType);
		if (!valueSource) {
			return nullptr;
		}
		// jsonから値ソースを復元
		valueSource->FromJson(definition.valueSource);

		// トラックランタイムを生成して返す
		return std::make_unique<UIPropertyTrackRuntime<Vector2>>(UIAnimationDataType::Translation,
			definition.applyMode, std::move(valueSource), [](UIAsset& asset, UIElement::Handle handle, CanvasType canvasType) {

				return BuildTranslationAccess(asset, handle, canvasType);
			}
		);
	}
	case UIAnimationDataType::Rotation: {

		auto valueSource = CreateValueSource<float>(definition.valueSourceType);
		if (!valueSource) {
			return nullptr;
		}
		// jsonから値ソースを復元
		valueSource->FromJson(definition.valueSource);

		// トラックランタイムを生成して返す
		return std::make_unique<UIPropertyTrackRuntime<float>>(UIAnimationDataType::Rotation,
			definition.applyMode, std::move(valueSource), [](UIAsset& asset, UIElement::Handle handle, CanvasType canvasType) {

				return BuildRotationAccess(asset, handle, canvasType);
			}
		);
	}

	case UIAnimationDataType::Scale: {

		auto valueSource = CreateValueSource<Vector2>(definition.valueSourceType);
		if (!valueSource) {
			return nullptr;
		}
		// jsonから値ソースを復元
		valueSource->FromJson(definition.valueSource);

		// トラックランタイムを生成して返す
		return std::make_unique<UIPropertyTrackRuntime<Vector2>>(UIAnimationDataType::Scale,
			definition.applyMode, std::move(valueSource), [](UIAsset& asset, UIElement::Handle handle, CanvasType canvasType) {

				return BuildScaleAccess(asset, handle, canvasType);
			}
		);
	}
	case UIAnimationDataType::Color: {

		auto valueSource = CreateValueSource<Color>(definition.valueSourceType);
		if (!valueSource) {
			return nullptr;
		}
		// jsonから値ソースを復元
		valueSource->FromJson(definition.valueSource);

		// トラックランタイムを生成して返す
		return std::make_unique<UIPropertyTrackRuntime<Color>>(UIAnimationDataType::Color,
			definition.applyMode, std::move(valueSource), [](UIAsset& asset, UIElement::Handle handle, CanvasType canvasType) {

				return BuildColorAccess(asset, handle, canvasType);
			}
		);
	}
	}
	return nullptr;
}