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
	
	// 最終的な色を出力
	gOutputTexture[pixelPos] = float4(gInputTexture.Load(int3(pixelPos, 0)).rgb, 1.0f);
}