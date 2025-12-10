#include "SystemManager.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Object/System/Systems/InstancedMeshSystem.h>

//============================================================================
//	SystemManager classMethods
//============================================================================

void SystemManager::UpdateData(ObjectPoolManager& ObjectPoolManager) {

	// 各systemを更新
	for (const auto& [type, system] : systems_) {

		// bufferの転送処理は別で行う
		if (type == std::type_index(typeid(InstancedMeshSystem))) {
			continue;
		}
		system->Update(ObjectPoolManager);
	}
}

void SystemManager::UpdateBuffer(ObjectPoolManager& ObjectPoolManager) {

	// buffer転送処理
	this->GetSystem<InstancedMeshSystem>()->Update(ObjectPoolManager);
}