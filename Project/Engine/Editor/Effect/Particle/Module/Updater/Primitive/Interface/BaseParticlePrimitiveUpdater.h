#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/Effect/Particle/Module/Updater/Primitive/Interface/IParticlePrimitiveUpdater.h>

//============================================================================
//	BaseParticlePrimitiveUpdater class
//	パーティクルのプリミティブ更新基底クラス
//============================================================================
namespace SakuEngine {

template <typename T>
class BaseParticlePrimitiveUpdater :
	public IParticlePrimitiveUpdater {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	BaseParticlePrimitiveUpdater() = default;
	virtual ~BaseParticlePrimitiveUpdater() = default;
protected:
	//========================================================================
	//	protected Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// 補間値
	T start_;
	T target_;
};

}; // SakuEngine
