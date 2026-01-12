//============================================================================
//	include
//============================================================================

#include "MSDFText.hlsli"

//============================================================================
//	Input
//============================================================================

struct VSInput {
	
	float2 position : POSITION0;
	float2 texcoord : TEXCOORD0;
};

//============================================================================
//	CBuffer
//============================================================================

cbuffer TransformationMatrix : register(b0) {
	
	float4x4 world;
};

cbuffer CameraData : register(b1) {
	
	float4x4 viewProjection;
};

//============================================================================
//	Main
//============================================================================
VSOutput main(VSInput input) {
	
	VSOutput output;
	
	float4x4 wvp = mul(world, viewProjection);
	output.position = mul(float4(input.position, 0.0f, 1.0f), wvp);
	output.texcoord = input.texcoord;

	return output;
}