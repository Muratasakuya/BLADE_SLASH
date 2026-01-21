#include "UIWidgetEditorContext.h"

using namespace SakuEngine;

//============================================================================
//	UIWidgetEditorContext classMethods
//============================================================================

void UIWidgetEditorContext::MarkDocumentDirty(bool requestPreviewSync) {

	documentDirty = true;
	syncPreviewRequested = requestPreviewSync;
}

void UIWidgetEditorContext::RequestPreviewRebuild() {

	rebuildPreviewRequested = true;
	syncPreviewRequested = true;
}