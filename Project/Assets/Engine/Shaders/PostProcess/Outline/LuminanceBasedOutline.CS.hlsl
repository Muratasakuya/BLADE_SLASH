//============================================================================
//	include
//============================================================================

#include "../PostProcessCommon.hlsli"

//============================================================================
//	Constant
//============================================================================

// 3x3 のオフセット
static const int2 kIndex3x3[3][3] = {
	{ { -1, -1 }, { 0, -1 }, { 1, -1 } },
	{ { -1, 0 }, { 0, 0 }, { 1, 0 } },
	{ { -1, 1 }, { 0, 1 }, { 1, 1 } }
};

// Prewitt フィルタ (横方向)
static const float kPrewittHorizontalKernel[3][3] = {
	{ -1.0f / 6.0f, 0.0f, 1.0f / 6.0f },
	{ -1.0f / 6.0f, 0.0f, 1.0f / 6.0f },
	{ -1.0f / 6.0f, 0.0f, 1.0f / 6.0f }
};

// Prewitt フィルタ (縦方向)
static const float kPrewittVerticalKernel[3][3] = {
	{ -1.0f / 6.0f, -1.0f / 6.0f, -1.0f / 6.0f },
	{ 0.0f, 0.0f, 0.0f },
	{ 1.0f / 6.0f, 1.0f / 6.0f, 1.0f / 6.0f }
};

//============================================================================
//	CBuffer
//============================================================================

struct EdgeParameter {
	
	float strength; // エッジの強さ
};
ConstantBuffer<EdgeParameter> gEdge : register(b0);

//============================================================================
//	Function
//============================================================================

// RGB → 輝度変換
float Luminance(float3 v) {
	return dot(v, float3(0.2125f, 0.7145f, 0.0721f));
}

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

	// 近傍(3x3)のどこかにBit_LuminanceBasedOutlineが立っていたら処理する
	bool anyMask = false;
	{
		int2 maxP = int2(int(width) - 1, int(height) - 1);
		[unroll]
		for (int y = 0; y < 3; ++y) {
			[unroll]
			for (int x = 0; x < 3; ++x) {

				int2 p = clamp(int2(pixelPos) + kIndex3x3[x][y], int2(0, 0), maxP);

				if (CheckPixelBitMask(Bit_LuminanceBasedOutline, uint2(p))) {
					anyMask = true;
				}
			}
		}
	}

	if (!anyMask) {
		gOutputTexture[pixelPos] = gInputTexture.Load(int3(pixelPos, 0));
		return;
	}

	// UVステップサイズ
	float2 uvStepSize = float2(1.0f / width, 1.0f / height);
	float2 difference = float2(0.0f, 0.0f);
	for (int x = 0; x < 3; ++x) {
		for (int y = 0; y < 3; ++y) {

			int2 offset = kIndex3x3[x][y];
			int2 samplePos = clamp(int2(pixelPos) + offset, int2(0, 0), int2(int(width) - 1, int(height) - 1));

			float3 sampleColor = gInputTexture.Load(int3(samplePos, 0)).rgb;
			float luminance = Luminance(sampleColor);

			difference.x += luminance * kPrewittHorizontalKernel[x][y];
			difference.y += luminance * kPrewittVerticalKernel[x][y];
		}
	}

	// エッジの強さに応じて元画像を減算
	float weight = saturate(length(difference) * gEdge.strength);
	float4 originalColor = gInputTexture.Load(int3(pixelPos, 0));
	float3 finalColor = (1.0f - weight) * originalColor.rgb;

	gOutputTexture[pixelPos] = float4(finalColor, 1.0f);
}