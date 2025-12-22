#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/Effect/User/Methods/EffectStructures.h>

// c++
#include <functional>

//============================================================================
//	EffectStopController class
//	停止条件をチェックし、停止すべきかどうかを判定する
//============================================================================
namespace SakuEngine {

class EffectStopController {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	EffectStopController() = default;
	~EffectStopController() = default;

	// 停止条件を満たしているかチェック
	bool ShouldStop(const EffectStopSetting& stop, const EffectEmitSetting& emit,
		const EffectNodeRuntime& runtime, const EffectQueryGroupAliveFn& queryFn) const;
};

}; // SakuEngine
