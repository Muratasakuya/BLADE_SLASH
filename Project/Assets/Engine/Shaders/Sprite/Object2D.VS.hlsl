//============================================================================
//	include
//============================================================================

#include "Object2D.hlsli"

//============================================================================
//	Input
//============================================================================

struct VSInput {
	
	float2 position : POSITION0;
	float2 texcoord : TEXCOORD0;
	float4 color : COLOR0;
};

//============================================================================
//	CBuffer
//============================================================================

cbuffer CameraData : register(b0) {
	
	float4x4 viewProjection;
};

cbuffer TransformationMatrix : register(b1) {
	
	float4x4 world;
};

//============================================================================
//	Main
//============================================================================
VSOutput main(VSInput input) {
	
	VSOutput output;
	
	float4x4 wvp = mul(world, viewProjection);
	output.position = mul(float4(input.position, 0.0f, 1.0f), wvp);
	output.texcoord = input.texcoord;
	output.color = input.color;

	return output;
}