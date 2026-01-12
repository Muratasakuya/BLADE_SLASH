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
//	StructuredBuffer
//============================================================================

// インスタンスごとの行列
StructuredBuffer<float4x4> gChildMatrices : register(t0);

//============================================================================
//	Main
//============================================================================
VSOutput main(VSInput input, uint vertexID : SV_VertexID) {
	
	VSOutput output;
	
	// インスタンスごとの設定データを取得、4頂点で1文字
	uint glyphIndex = vertexID / 4;
	// 行列を取得してワールド行列を計算
	float4x4 childMatrix = gChildMatrices[glyphIndex];
	float4x4 worldMatrix = mul(childMatrix, world);
	
	// ワールドビュー射影行列を計算して頂点変換
	float4x4 wvp = mul(worldMatrix, viewProjection);
	output.position = mul(float4(input.position, 0.0f, 1.0f), wvp);
	output.texcoord = input.texcoord;

	return output;
}