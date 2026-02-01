#include "IUISystem.h"

using namespace SakuEngine;

//============================================================================
//	IUISystem classMethods
//============================================================================

void UISystemMethod::RestoreFromJsonCache(IUIComponent& component) {

	// jsonキャッシュが存在する場合は復元
	if (component.HasJsonCache()) {

		// 復元してクリア
		component.FromJson(component.GetJsonCache());
		component.ClearJsonCache();
	}
}