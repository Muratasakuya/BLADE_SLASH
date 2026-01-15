#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/System/Base/ISystem.h>
#include <Engine/Object/Data/Canvas/CanvasCommon.h>

namespace SakuEngine {

	//============================================================================
	//	CanvasBufferSystem class
	//	キャンバス描画用バッファ管理システム
	//============================================================================
	class CanvasBufferSystem :
		public ISystem {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		CanvasBufferSystem() = default;
		~CanvasBufferSystem() = default;

		Archetype Signature() const override { return Archetype{}; }

		void Update(ObjectPoolManager& ObjectPoolManager) override;

		//--------- accessor -----------------------------------------------------

		// レイヤー毎のキャンバスデータ取得
		const std::vector<BaseCanvas*>& GetCanvasData(CanvasLayer layer) { return canvasDataMap_[layer]; }
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		// レイヤー毎のキャンバスデータ
		std::unordered_map<CanvasLayer, std::vector<BaseCanvas*>> canvasDataMap_;
	};
}