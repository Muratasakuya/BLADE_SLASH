//============================================================================
//	include
//============================================================================

#include "../../../Common/ParticleCommonSturctures.hlsli"
#include "../../ParticleEmitterStructures.hlsli"
#include "../../../../../../../Engine/Editor/Effect/Particle/ParticleConfig.h"

#include "../../../../Math/Math.hlsli"

//============================================================================
//	CBuffer
//============================================================================

// å`èÛ
struct EmitterBox {
	
	float3 size;
	float3 translation;
	float4x4 rotationMatrix;
};

ConstantBuffer<EmitterCommon> gEmitterCommon : register(b0);
ConstantBuffer<EmitterBox> gEmitterBox : register(b1);
ConstantBuffer<PerFrame> gPerFrame : register(b2);

//============================================================================
//	RWStructuredBuffer
//============================================================================

RWStructuredBuffer<Particle> gParticles : register(u0);
RWStructuredBuffer<Transform> gTransform : register(u1);
RWStructuredBuffer<Material> gMaterials : register(u2);

RWStructuredBuffer<int> gFreeListIndex : register(u3);
RWStructuredBuffer<uint> gFreeList : register(u4);

//============================================================================
//	Functions
//============================================================================

float3 GetRandomPoint(RandomGenerator generator) {
	
	float3 local = (generator.Generate3D() - 0.5f) * gEmitterBox.size;
	return local;
}

//============================================================================
//	Main
//============================================================================
[numthreads(THREAD_EMIT_GROUP, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID) {
	
	// î≠ê∂ãñâ¬Ç™â∫ÇËÇƒÇ¢Ç»ÇØÇÍÇŒèàóùÇµÇ»Ç¢
	if (gEmitterCommon.emit == 0 ||
		gEmitterCommon.count <= DTid.x) {
		return;
	}
	
	// óêêîê∂ê¨
	RandomGenerator generator;
	generator.seed = (DTid + gPerFrame.time) * gPerFrame.time;

	int freeListIndex;
	InterlockedAdd(gFreeListIndex[0], -1, freeListIndex);
		
	if (0 <= freeListIndex && freeListIndex < kMaxGPUParticles) {
			
		uint particleIndex = gFreeList[freeListIndex];
			
		Particle particle;
		particle.currentTime = 0.0f;
		particle.lifeTime = gEmitterCommon.lifeTime;
			
		// å¸Ç¢ÇƒÇ¢ÇÈï˚å¸(+Z)ï˚å¸Ç…îÚÇŒÇ∑
		float3 worldForward = mul(float3(0.0f, 0.0f, 1.0f), (float3x3) gEmitterBox.rotationMatrix);
		particle.velocity = normalize(worldForward) * generator.Generate3D() * gEmitterCommon.moveSpeed;
			
		Transform transform = (Transform) 0;
		transform.translation = gEmitterBox.translation + mul(
			float4(GetRandomPoint(generator), 1.0f), gEmitterBox.rotationMatrix).xyz;
		transform.scale = gEmitterCommon.scale;
			
		Material material = (Material) 0;
		material.color = gEmitterCommon.color;
		material.postProcessMask = gEmitterCommon.postProcessMask;
			
		// ílÇê›íË
		gParticles[particleIndex] = particle;
		gTransform[particleIndex] = transform;
		gMaterials[particleIndex] = material;
	} else {

		InterlockedAdd(gFreeListIndex[0], 1);
	}
}