#include "UIAnimationTrackFactory.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================


//============================================================================
//	UIAnimationTrackFactory classMethods
//============================================================================

std::unique_ptr<IUIAnimationTrackRuntime> UIAnimationTrackFactory::Create(const UIAnimationTrackDefinition& definition) {

	if (definition.valueSourceType != "Lerp") {
		return nullptr;
	}

	switch (definition.type) {
	case UIAnimationDataType::Translation:

		return nullptr;
	case UIAnimationDataType::Rotation:

		return nullptr;
	case UIAnimationDataType::Scale:

		return nullptr;
	case UIAnimationDataType::Color:

		return nullptr;
	}
	return nullptr;
}