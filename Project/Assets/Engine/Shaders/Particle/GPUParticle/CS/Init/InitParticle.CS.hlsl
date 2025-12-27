//============================================================================
//	include
//============================================================================

#include "../../../Common/ParticleCommonSturctures.hlsli"
#include "../../../../../../../Engine/Editor/Effect/Particle/ParticleConfig.h"

//============================================================================
//	RWStructuredBuffer
//============================================================================

RWStructuredBuffer<Particle> gParticles : register(u0);
RWStructuredBuffer<Transform> gTransform : register(u1);
RWStructuredBuffer<Material> gMaterials : register(u2);

RWStructuredBuffer<int> gFreeListIndex : register(u3);
RWStructuredBuffer<uint> gFreeList : register(u4);

//============================================================================
//	Main
//============================================================================
[numthreads(1024, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID) {
	
	uint particleIndex = DTid.x;
	if (particleIndex < kMaxGPUParticles) {
		
		// ‘S‚Ä0–„‚ß
		gParticles[particleIndex] = (Particle) 0;
		gTransform[particleIndex] = (Transform) 0;
		gMaterials[particleIndex] = (Material) 0;
		
		gFreeList[particleIndex] = particleIndex;
	}
	if (particleIndex == 0) {
		
		gFreeListIndex[0] = kMaxGPUParticles - 1;
	}
}