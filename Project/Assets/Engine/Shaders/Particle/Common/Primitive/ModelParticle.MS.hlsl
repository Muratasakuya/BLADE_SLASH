//============================================================================
//	include
//============================================================================

#include "../ParticleOutput.hlsli"
#include "../ParticleCommonSturctures.hlsli"

#include "../../../Math/Math.hlsli"

//============================================================================
//	Input
//============================================================================

struct MSInput {
	
	float4 position;
	float2 texcoord;
	float3 normal;
	float4 color;
	float3 tangent;
	float3 biNormal;
};

//============================================================================
//	CBuffer
//============================================================================

cbuffer InstanceData : register(b0) {
	
	uint meshletCount;
	uint numVertices;
	int isSkinned;
};

ConstantBuffer<PerView> gPerView : register(b1);

//============================================================================
//	StructuredBuffer
//============================================================================

struct Meshlet {
	
	uint vertexOffset;
	uint vertexCount;
	uint primitiveOffset;
	uint primitiveCount;
	float4 color;
};

StructuredBuffer<MSInput> gVertices : register(t0);
StructuredBuffer<uint> gIndices : register(t1);
StructuredBuffer<Meshlet> gMeshlets : register(t2);
StructuredBuffer<uint> gPrimitives : register(t3);
StructuredBuffer<Transform> gTransform : register(t4);

//============================================================================
//	Function
//============================================================================

// 10bit * 3
uint3 UnpackPrimitiveIndex(uint packedIndex) {
	
	// 下位10bit、1つ目の頂点インデックス
	// 中間10bit、2つ目の頂点インデックス
	// 上位10bit、3つ目の頂点インデックス
	
	return uint3(
	packedIndex & 0x3FF,
	(packedIndex >> 10) & 0x3FF,
	(packedIndex >> 20) & 0x3FF);
}

//============================================================================
//	Main
//============================================================================
[numthreads(128, 1, 1)]
[outputtopology("triangle")]
void main(
uint groupThreadId : SV_GroupThreadID, uint groupId : SV_GroupID,
out vertices MSOutput verts[64], out indices uint3 polys[126]) {
	
	// バッファアクセス
	// DispatchMesh での1次元グループID
	uint groupIdx = groupId.x;
	uint meshletIndex = groupIdx % meshletCount;
	uint instanceIndex = groupIdx / meshletCount;
	
	// 現在のグループに対応するmeshletを取得
	Meshlet meshlet = gMeshlets[meshletIndex];
	Transform transform = gTransform[instanceIndex];
	
	// メッシュシェーダーの出力数、頂点数、プリミティブ数を設定
	SetMeshOutputCounts(meshlet.vertexCount, meshlet.primitiveCount);
	
	// 各スレッドがプリミティブインデックスをアンパックして出力配列に格納
	if (groupThreadId < meshlet.primitiveCount) {
		
		uint packedIndex = gPrimitives[meshlet.primitiveOffset + groupThreadId];
		polys[groupThreadId] = UnpackPrimitiveIndex(packedIndex);
	}
	
	// 三角形インデックスは最初の2スレッドで書き込み
	if (groupThreadId < 2) {

		const uint3 index[2] = { uint3(0, 1, 2), uint3(2, 1, 3) };
		polys[groupThreadId] = index[groupThreadId];
	}
	
	// 各スレッドで頂点データを処理して出力
	if (groupThreadId < meshlet.vertexCount) {
		
		uint index = gIndices[meshlet.vertexOffset + groupThreadId];
		// skinnedMeshだったらindexをインスタンスインデックス * 頂点分足す
		if (isSkinned == 1) {
			
			index += instanceIndex * numVertices;
		}
		MSInput input = gVertices[index];
		MSOutput vertex = (MSOutput) 0;
		
		// world行列を作成
		float4x4 worldMatrix = MakeWorldMatrix(transform, gPerView.billboardMatrix, gPerView.cameraPos);
		worldMatrix[3].xyz = transform.translation;
		
		// 親の設定
		worldMatrix = SetParent(transform, worldMatrix);

		// 行列計算
		float4x4 wvp = mul(worldMatrix, gPerView.viewProjection);
		
		// 頂点位置をスクリーン座標に変換
		vertex.position = mul(input.position, wvp);

		// テクスチャ座標
		vertex.texcoord = input.texcoord;
		
		// instanceIdを取得
		vertex.instanceID = instanceIndex;
		vertex.vertexColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
		verts[groupThreadId] = vertex;
	}
}