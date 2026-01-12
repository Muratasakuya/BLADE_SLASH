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

	// アトラステクスチャからMSDFをサンプリング
	float3 smple = gTexture.Sample(gSampler, input.texcoord).rgb;

	// 0.5が輪郭、上下が内外
	float signedDistance = median(smple.r, smple.g, smple.b) - 0.5f;

	// screenPxRange:
	// unitRange = pxRange / atlasSize (UV単位での距離レンジ)
	// screenTexSize = 1 / fwidth(uv)  (1pxあたりのUV分解能)
	float2 unitRange = (pxRange / atlasSize);
	float2 screenTexSize = 1.0f / fwidth(input.texcoord);
	float screenPxRange = max(0.5f * dot(unitRange, screenTexSize), 1.0f);

	// signedDistanceをピクセル単位に変換に
	float distancePixel = signedDistance * screenPxRange;

	// アウトライン描画の太さ調整
	distancePixel += boldnessPx;

	// smoothstepの幅
	float softness = max(softnessPx, 0.0001);

	// アルファを計算
	float fillA = smoothstep(-softness, softness, distancePixel);

	// アウトラインのアルファ、内側は0、外側に向かってgOutlineWidthPx分だけ広がる
	float outlineA = 0.0f;
	if (enableOutline != 0 && outlineWidthPx > 0.0f) {
		
		float outer = smoothstep(-softness, softness, distancePixel + outlineWidthPx);
		outlineA = saturate(outer - fillA);
	}

	// 合成
	float a = saturate(fillA + outlineA);

	float3 rgb = 0.0f;
	rgb += outlineColor.rgb * outlineA;
	// fillが優先で上に乗る
	rgb = lerp(rgb, color.rgb, fillA);

	// 色側のalpha乗算
	float finalAlpha = a * color.a;
	rgb *= color.a;

	// アウトラインのalphaも反映
	rgb += outlineColor.rgb * outlineA * (outlineColor.a - color.a);
	finalAlpha = saturate(finalAlpha + outlineA * outlineColor.a);
	
	// 出力設定
	PSOutput output;
	output.color = float4(rgb, finalAlpha);
	output.mask = postProcessMask;
	
	return output;
}