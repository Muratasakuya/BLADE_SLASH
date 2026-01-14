//============================================================================
//	include
//============================================================================

#include "MSDFText.hlsli"

//============================================================================
//	Output
//============================================================================

struct PSOutput {

	float4 color : SV_TARGET0;
	uint mask : SV_TARGET1;
};

//============================================================================
//	CBuffer
//============================================================================

cbuffer Material : register(b0) {
	
	float4 color;
	float4 outlineColor;

	float2 atlasSize;
	float pxRange;
	float outlineWidthPx;

	float softnessPx;
	float boldnessPx;
	uint enableOutline;
	uint postProcessMask;
};

//============================================================================
//	Texture Sampler
//============================================================================

Texture2D gTexture : register(t0);
SamplerState gSampler : register(s0);

//============================================================================
//	Functions
//============================================================================

float median(float a, float b, float c) {
	return max(min(a, b), min(max(a, b), c));
}

//============================================================================
//	Main
//============================================================================
PSOutput main(VSOutput input) {

	// アトラステクスチャを参照する際のUV補正
	float2 texel = 0.5f / atlasSize;
	float2 uv = clamp(input.texcoord, texel, 1.0f - texel);
	float3 smple = gTexture.SampleLevel(gSampler, uv, 0).rgb;

	// MSDFの符号付き距離を取得
	float signedDistance = median(smple.r, smple.g, smple.b) - 0.5f;
	float2 unitRange = (pxRange / atlasSize);

	// uvと同じ座標でfwidth
	float2 screenTexSize = 1.0f / fwidth(uv);
	// ピクセル単位の距離に変換
	float screenPxRange = max(0.5f * dot(unitRange, screenTexSize), 1.0f);
	float distancePixel = signedDistance * screenPxRange;
	distancePixel += boldnessPx;

	float softness = max(softnessPx, 0.0001);
	float fillA = smoothstep(-softness, softness, distancePixel);
	float outlineA = 0.0f;
	// アウトライン処理
	if (enableOutline != 0 && outlineWidthPx > 0.0f) {

		float outer = smoothstep(-softness, softness, distancePixel + outlineWidthPx);
		outlineA = saturate(outer - fillA);
	}

	// 合成
	float a = saturate(fillA + outlineA);

	float3 rgb = 0.0f;
	rgb += outlineColor.rgb * outlineA;
	rgb = lerp(rgb, color.rgb, fillA);

	float finalAlpha = a * color.a;
	rgb *= color.a;

	rgb += outlineColor.rgb * outlineA * (outlineColor.a - color.a);
	finalAlpha = saturate(finalAlpha + outlineA * outlineColor.a);

	// 出力
	PSOutput output;
	output.color = float4(rgb, finalAlpha);
	output.mask = postProcessMask;
	return output;
}