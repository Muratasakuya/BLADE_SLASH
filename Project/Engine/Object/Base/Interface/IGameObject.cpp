#include "IGameObject.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Core/ObjectManager.h>

//============================================================================
//	IGameObject classMethods
//============================================================================

IGameObject::IGameObject() {

	objectManager_ = ObjectManager::GetInstance();
}

IGameObject::~IGameObject() {

	// object削除
	objectManager_->Destroy(objectId_);
}
