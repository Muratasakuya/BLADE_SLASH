#include "IGameEditor.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/Manager/GameEditorManager.h>
#include <Engine/Utility/Algorithm/Algorithm.h>

//============================================================================
//	IGameEditor classMethods
//============================================================================

IGameEditor::IGameEditor(const std::string& name, const std::string& groupName) {

	name_ = name;
	groupName_ = groupName;
	GameEditorManager::GetInstance()->AddEditor(this);
}

IGameEditor::~IGameEditor() {

	GameEditorManager::GetInstance()->RemoveEditor(this);
}
