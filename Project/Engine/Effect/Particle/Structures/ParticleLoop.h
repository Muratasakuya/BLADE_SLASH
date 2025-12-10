#pragma once

//============================================================================
//	include
//============================================================================

// c++
#include <algorithm>
#include <cmath>

//============================================================================
//	ParticleLoop class
//============================================================================
namespace ParticleLoop {

	// ループ種類
	enum class Type {

		Repeat,
		PingPong,
	};

	// ループ処理の補間tを返す
	float CalLoopedT(float rawT, int loopCount, Type type);
}