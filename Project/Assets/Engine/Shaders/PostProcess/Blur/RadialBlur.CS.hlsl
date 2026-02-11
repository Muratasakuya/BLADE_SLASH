//============================================================================
//	include
//============================================================================

#include "../PostProcessCommon.hlsli"

//============================================================================
//	CBuffer
//============================================================================

struct BlurParameter {
	
	float2 center; // ブラーの中心
	int numSamples; // サンプリング数
	float width; // ぼかしの幅
};
ConstantBuffer<BlurParameter> gBlur : register(b0);

//============================================================================
//	buffer
//============================================================================

SamplerState gSampler : register(s0);

//============================================================================
//	Main
//============================================================================
[numthreads(THREAD_POSTPROCESS_GROUP, THREAD_POSTPROCESS_GROUP, 1)]
void main(uint3 DTid : SV_DispatchThreadID) {
	
	uint width, height;
	gInputTexture.GetDimensions(width, height);

	// ピクセル位置
	uint2 pixelPos = DTid.xy;

	// 範囲外
	if (pixelPos.x >= width || pixelPos.y >= height) {
		return;
	}
	
	// フラグが立っていなければ処理しない
	if (!CheckPixelBitMask(Bit_RadialBlur, pixelPos)) {
		
		gOutputTexture[pixelPos] = gInputTexture.Load(int3(pixelPos, 0));
		return;
	}
	
	// サンプリング回数が0ならそのままの色で返す
	if (gBlur.numSamples == 0) {

		gOutputTexture[pixelPos] = gInputTexture.Load(int3(pixelPos, 0));
		return;
	}

	// UV座標計算
	float2 uv = (float2(pixelPos) + 0.5f) / float2(width, height);

	// 放射ブラー方向
	float2 direction = normalize(uv - gBlur.center);
	float distance = length(uv - gBlur.center);

	// ブラー適用
	float3 outputColor = float3(0.0f, 0.0f, 0.0f);
	float sampleCount = 0.0f;
	for (int i = 0; i < gBlur.numSamples; ++i) {

		float t = float(i) / max(1.0f, float(gBlur.numSamples - 1));
		float2 sampleUV = uv + direction * gBlur.width * distance * t;

		sampleUV = clamp(sampleUV, 0.001f, 0.999f);

		// UV → ピクセル座標
		uint2 samplePixel =
		uint2(sampleUV * float2(width, height));

		// サンプル元もマスク判定を行って影響を受けないようにする
		if (!CheckPixelBitMask(Bit_RadialBlur, samplePixel)) {
			continue;
		}
		outputColor += gInputTexture.SampleLevel(gSampler, sampleUV, 0).rgb;
		sampleCount += 1.0f;
	}

	if (sampleCount > 0.0f) {

		outputColor /= sampleCount;
	} else {
		
		// 全部弾かれたら元の色
		outputColor = gInputTexture.Load(int3(pixelPos, 0)).rgb;
	}
	gOutputTexture[pixelPos] = float4(outputColor, 1.0f);
}