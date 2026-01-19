//============================================================================
//	include
//============================================================================

#include "../PostProcessCommon.hlsli"

//============================================================================
//	CBuffer
//============================================================================

struct Material {

	// 露出度
	float exposure;
};
ConstantBuffer<Material> gMaterial : register(b0);

//============================================================================
//	Functions
//============================================================================

float3 RRTAndODTFit(float3 v) {
	
	float3 a = v * (v + 0.0245786f) - 0.000090537f;
	float3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
	return a / b;
}

float3 ACESFitted(float3 color) {

	// sRGB/Rec709 -> ACES 的な変換
	const float3x3 ACESInputMat = float3x3(
		0.59719f, 0.35458f, 0.04823f,
		0.07600f, 0.90834f, 0.01566f,
		0.02840f, 0.13383f, 0.83777f);
	const float3x3 ACESOutputMat = float3x3(
		1.60475f, -0.53108f, -0.07367f,
		-0.10208f, 1.10813f, -0.00605f,
		-0.00327f, -0.07276f, 1.07602f);

	// ACESフィッティング
	color = mul(ACESInputMat, color);
	color = RRTAndODTFit(color);
	color = mul(ACESOutputMat, color);
	return saturate(color);
}

float3 LinearToSRGB(float3 x) {

	x = saturate(x);
	float3 lo = x * 12.92f;
	float3 hi = 1.055f * pow(x, 1.0f / 2.4f) - 0.055f;
	return lerp(lo, hi, step(0.0031308f, x));
}

//============================================================================
//	main
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

	// HDRテクスチャをサンプリング
	float3 hdrColor = gInputTexture.Load(int3(pixelPos, 0)).rgb;

	// 露出
	hdrColor *= gMaterial.exposure;
	
	// ACESフィッティング
	float3 ldrLinear = ACESFitted(hdrColor);
	
	// LinearからsRGBへ変換
	float3 ldrSRGB = LinearToSRGB(ldrLinear);
	
	// 最終的な色を出力
	gOutputTexture[pixelPos] = float4(ldrSRGB, 1.0f);
}