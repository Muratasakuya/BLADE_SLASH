//============================================================================
//	include
//============================================================================

#include "../../../Common/ParticleCommonSturctures.hlsli"
#include "../../../../../../../Engine/Editor/Effect/Particle/ParticleConfig.h"

//============================================================================
//	CBuffer
//============================================================================

struct ParentBuffer {
	
	float4x4 parentMatrix;
	uint aliveParent;
};

ConstantBuffer<PerFrame> gPerFrame : register(b0);
ConstantBuffer<ParentBuffer> gParentBuffer : register(b1);

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
[numthreads(THREAD_UPDATE_GROUP, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID) {
	
	uint particleIndex = DTid.x;
	if (kMaxGPUParticles <= particleIndex) {
		return;
	}
	
	Particle particle = gParticles[particleIndex];
	Transform transform = gTransform[particleIndex];
	Material material = gMaterials[particleIndex];
	if (particle.currentTime < particle.lifeTime) {
		
		// 時間経過の更新
		particle.currentTime += gPerFrame.deltaTime;
		
		// transform
		transform.translation += particle.velocity * gPerFrame.deltaTime;
		// 親の設定
		transform.aliveParent = gParentBuffer.aliveParent;
		transform.parentMatrix = gParentBuffer.parentMatrix;
		
		// material
		material.color.a = saturate(1.0f - particle.currentTime / particle.lifeTime);

		// 値を設定
		gParticles[particleIndex] = particle;
		gTransform[particleIndex] = transform;
		gMaterials[particleIndex] = material;
		return;
	}

	gParticles[particleIndex] = (Particle) 0;

	int posPrev;
	InterlockedAdd(gFreeListIndex[0], 1, posPrev);
	uint writePos = posPrev + 1;

	if (writePos < kMaxGPUParticles) {
		
		gFreeList[writePos] = particleIndex;
	} else {
		
		InterlockedAdd(gFreeListIndex[0], -1);
	}
}