#include "ParticleLoop.h"

//============================================================================
//	ParticleLoop classMethods
//============================================================================

float ParticleLoop::CalLoopedT(float rawT, int loopCount, Type type) {

	if (loopCount <= 1) {

		return std::clamp(rawT, 0.0f, 1.0f);
	}

	float t = rawT * static_cast<float>(loopCount);
	if (type == Type::Repeat) {

		t = std::fmod(t, 1.0f);
	} else if (type == Type::PingPong) {

		t = std::fmod(t, 2.0f);
		if (t > 1.0f) {

			t = 2.0f - t;
		}
	}
	return t;
}
