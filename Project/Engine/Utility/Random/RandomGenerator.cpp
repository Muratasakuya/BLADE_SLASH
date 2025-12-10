#include "RandomGenerator.h"

using namespace SakuEngine;

//============================================================================
//	RandomGenerator classMethods
//============================================================================

Vector3 RandomGenerator::Generate(const SakuEngine::Vector3& min, const SakuEngine::Vector3& max) {

	return Vector3{
	Generate(min.x, max.x),
	Generate(min.y, max.y),
	Generate(min.z, max.z) };
}

Color RandomGenerator::Generate(const SakuEngine::Color& min, const SakuEngine::Color& max) {

	return Color{
		Generate(min.r, max.r),
		Generate(min.g, max.g),
		Generate(min.b, max.b),
		Generate(min.a, max.a) };
}
