#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/System/Base/ISystem.h>
#include <Engine/Object/Data/Sprite/Sprite.h>

namespace SakuEngine {

	//============================================================================
	//	SpriteBufferSystem class
	//	2Dスプライトのバッファを管理するシステム
	//============================================================================
	class SpriteBufferSystem :
		public ISystem {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		SpriteBufferSystem() = default;
		~SpriteBufferSystem() = default;

		Archetype Signature() const override;

		void Update(ObjectPoolManager& ObjectPoolManager) override;
	};

}; // SakuEngine
