#include "Fullscreen.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput {
	float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input) {
	
	PixelShaderOutput output;

	// テクスチャサンプリング
	output.color = gTexture.Sample(gSampler, input.texcoord);
	// アルファ値を1.0fに設定
	output.color.a = 1.0f;
	return output;
}