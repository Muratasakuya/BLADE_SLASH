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

struct NoiseBuffer {
	
	float scale;
	float strength;
	float speed;
};

ConstantBuffer<PerFrame> gPerFrame : register(b0);
ConstantBuffer<ParentBuffer> gParentBuffer : register(b1);
ConstantBuffer<NoiseBuffer> gNoiseBuffer : register(b2);

//============================================================================
//	RWStructuredBuffer
//============================================================================

RWStructuredBuffer<Particle> gParticles : register(u0);
RWStructuredBuffer<Transform> gTransform : register(u1);
RWStructuredBuffer<Material> gMaterials : register(u2);

RWStructuredBuffer<int> gFreeListIndex : register(u3);
RWStructuredBuffer<uint> gFreeList : register(u4);

//============================================================================
//	Texture
//============================================================================

Texture2D<float2> gNoiseTexure : register(t0);
SamplerState gSampler : register(s0);

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
		
		// noise処理、流れていくように動かす
		float2 uv =transform.translation.xy * gNoiseBuffer.scale + gPerFrame.time * gNoiseBuffer.speed;
		float2 noiseVector = gNoiseTexure.SampleLevel(gSampler, uv, 0) * 2.0f - 1.0f;
		
		// 速度にnoise値をXY加算
		particle.velocity.xy += noiseVector * gNoiseBuffer.strength * gPerFrame.deltaTime;
		
		// Z値は別で計算
		float zNoise = gNoiseTexure.SampleLevel(gSampler, uv + float2(37.0f, 17.0f), 0).r * 2.0f - 1.0f;
		particle.velocity.z += zNoise * gNoiseBuffer.strength * gPerFrame.deltaTime;
		
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