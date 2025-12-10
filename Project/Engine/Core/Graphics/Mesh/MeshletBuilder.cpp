#include "MeshletBuilder.h"

using namespace SakuEngine;

//============================================================================
//	MeshletBuilder classMethods
//============================================================================

ResourceMesh<MeshVertex> MeshletBuilder::ParseMesh(const aiScene* scene, bool isSkinned) {

	// 出力
	ResourceMesh<MeshVertex> destinationMesh{};
	destinationMesh.isSkinned = isSkinned;

	// 頂点情報の設定
	SetVertex(destinationMesh, scene);

	// 最適化処理
	Optimize(destinationMesh);

	// meshlet生成処理
	CreateMeshlet(destinationMesh);

	return destinationMesh;
}

ResourceMesh<MeshVertex> MeshletBuilder::ParseMesh(const ModelData& modelData) {

	// 出力
	ResourceMesh<MeshVertex> destinationMesh{};

	// 頂点情報の設定
	SetVertex(destinationMesh, modelData);

	// 最適化処理
	Optimize(destinationMesh);

	// meshlet生成処理
	CreateMeshlet(destinationMesh);

	return destinationMesh;
}

ResourceMesh<EffectMeshVertex> MeshletBuilder::ParseEffectMesh(const ModelData& modelData) {

	// 出力
	ResourceMesh<EffectMeshVertex> destinationMesh{};

	// 頂点情報の設定
	SetEffectVertex(destinationMesh, modelData);

	// 最適化処理
	OptimizeEffectMesh(destinationMesh);

	// meshlet生成処理
	CreateEffectMeshlet(destinationMesh);

	return destinationMesh;
}

void MeshletBuilder::SetVertex(ResourceMesh<MeshVertex>& destinationMesh, const aiScene* scene) {

	// meshの数
	destinationMesh.meshCount_ = scene->mNumMeshes;

	// メモリをmesh数分確保する
	destinationMesh.vertices.resize(scene->mNumMeshes);
	destinationMesh.indices.resize(scene->mNumMeshes);

	// meshの数分
	for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {

		aiMesh* sourceMesh = scene->mMeshes[meshIndex];

		// 頂点データ分メモリを確保する
		destinationMesh.vertices[meshIndex].resize(sourceMesh->mNumVertices);

		aiVector3D zeroVector = aiVector3D(0.0f, 0.0f, 0.0f);
		for (uint32_t i = 0; i < sourceMesh->mNumVertices; ++i) {

			aiVector3D* pos = &(sourceMesh->mVertices[i]);
			aiVector3D* normal = &(sourceMesh->mNormals[i]);

			aiVector3D* texcoord = &zeroVector;
			aiVector3D* tangent = &zeroVector;
			aiVector3D* biNormal = &zeroVector;
			// texcoordがある場合のみ設定
			if (sourceMesh->HasTextureCoords(0)) {

				texcoord = &sourceMesh->mTextureCoords[0][i];
			}
			// tangent、biNormalがある場合のみ設定
			if (sourceMesh->HasTangentsAndBitangents()) {

				tangent = &(sourceMesh->mTangents[i]);
				biNormal = &(sourceMesh->mBitangents[i]);
			}

			// 頂点情報を格納
			destinationMesh.vertices[meshIndex][i] = MeshVertex(

				Vector4(-pos->x, pos->y, pos->z, 1.0f),
				Vector2(texcoord->x, texcoord->y),
				Vector3(-normal->x, normal->y, normal->z),
				Color(1.0f, 1.0f, 1.0f, 1.0f),
				Vector3(-tangent->x, tangent->y, tangent->z),
				Vector3(-biNormal->x, biNormal->y, biNormal->z));
		}

		// 頂点インデックスのメモリを確保する
		destinationMesh.indices[meshIndex].resize(sourceMesh->mNumFaces * 3);

		for (auto i = 0u; i < sourceMesh->mNumFaces; ++i) {

			const auto& face = sourceMesh->mFaces[i];

			destinationMesh.indices[meshIndex][i * 3 + 0] = face.mIndices[0];
			destinationMesh.indices[meshIndex][i * 3 + 1] = face.mIndices[1];
			destinationMesh.indices[meshIndex][i * 3 + 2] = face.mIndices[2];
		}
	}
}

void MeshletBuilder::SetVertex(ResourceMesh<MeshVertex>& destinationMesh, const ModelData& modelData) {

	// meshの数
	destinationMesh.meshCount_ = static_cast<uint32_t>(modelData.meshes.size());

	// メモリをmesh数分確保する
	destinationMesh.vertices.resize(destinationMesh.meshCount_);
	destinationMesh.indices.resize(destinationMesh.meshCount_);

	// meshの数分
	for (uint32_t meshIndex = 0; meshIndex < destinationMesh.meshCount_; ++meshIndex) {

		const MeshModelData& meshData = modelData.meshes[meshIndex];

		// 頂点データ分メモリを確保する
		destinationMesh.vertices[meshIndex].resize(meshData.vertices.size());

		for (uint32_t i = 0; i < meshData.vertices.size(); ++i) {

			Vector4 pos = meshData.vertices[i].pos;
			Vector2 texcoord = meshData.vertices[i].texcoord;
			Vector3 normal = meshData.vertices[i].normal;

			// 頂点情報を格納
			destinationMesh.vertices[meshIndex][i] = MeshVertex(

				Vector4(pos.x, pos.y, pos.z, 1.0f),
				Vector2(texcoord.x, texcoord.y),
				Vector3(normal.x, normal.y, normal.z),
				Color(1.0f, 1.0f, 1.0f, 1.0f));
		}

		// 頂点インデックスのメモリを確保する
		destinationMesh.indices[meshIndex].resize(meshData.indices.size());
		// インデックス情報を格納
		destinationMesh.indices[meshIndex] = meshData.indices;
	}
}

void MeshletBuilder::SetEffectVertex(ResourceMesh<EffectMeshVertex>& destinationMesh,
	const ModelData& modelData) {

	// meshの数
	destinationMesh.meshCount_ = static_cast<uint32_t>(modelData.meshes.size());

	// メモリをmesh数分確保する
	destinationMesh.vertices.resize(destinationMesh.meshCount_);
	destinationMesh.indices.resize(destinationMesh.meshCount_);

	// meshの数分
	for (uint32_t meshIndex = 0; meshIndex < destinationMesh.meshCount_; ++meshIndex) {

		const MeshModelData& meshData = modelData.meshes[meshIndex];

		// 頂点データ分メモリを確保する
		destinationMesh.vertices[meshIndex].resize(meshData.vertices.size());

		for (uint32_t i = 0; i < meshData.vertices.size(); ++i) {

			Vector4 pos = meshData.vertices[i].pos;
			Vector2 texcoord = meshData.vertices[i].texcoord;

			// 頂点情報を格納
			destinationMesh.vertices[meshIndex][i] = EffectMeshVertex(

				Vector4(pos.x, pos.y, pos.z, 1.0f),
				Vector2(texcoord.x, texcoord.y),
				Color(1.0f, 1.0f, 1.0f, 1.0f));
		}

		// 頂点インデックスのメモリを確保する
		destinationMesh.indices[meshIndex].resize(meshData.indices.size());
		// インデックス情報を格納
		destinationMesh.indices[meshIndex] = meshData.indices;
	}
}

void MeshletBuilder::Optimize(ResourceMesh<MeshVertex>& destinationMesh) {

	// skinnedMeshの場合、頂点情報が変わってバグってしまうため
	// 今は最適化処理をスキップ、対応できたらする
	if (destinationMesh.isSkinned) {
		return;
	}

	// meshの数分
	for (uint32_t meshIndex = 0; meshIndex < destinationMesh.meshCount_; ++meshIndex) {

		std::vector<uint32_t> remap(destinationMesh.indices[meshIndex].size());

		// 重複データを削除するための再マッピング用インデックスを生成
		auto vertexCount = meshopt_generateVertexRemap(

			remap.data(),
			destinationMesh.indices[meshIndex].data(),
			destinationMesh.indices[meshIndex].size(),
			destinationMesh.vertices[meshIndex].data(),
			destinationMesh.vertices[meshIndex].size(),
			sizeof(MeshVertex));

		std::vector<MeshVertex> vertices(vertexCount);
		std::vector<uint32_t> indices(destinationMesh.indices[meshIndex].size());

		// 頂点インデックスを再マッピング
		meshopt_remapIndexBuffer(

			indices.data(),
			destinationMesh.indices[meshIndex].data(),
			destinationMesh.indices[meshIndex].size(),
			remap.data());

		// 頂点データを再マッピング
		meshopt_remapVertexBuffer(

			vertices.data(),
			destinationMesh.vertices[meshIndex].data(),
			destinationMesh.vertices[meshIndex].size(),
			sizeof(MeshVertex),
			remap.data());

		// 不要になったメモリを解放
		remap.clear();
		remap.shrink_to_fit();

		// 最適化したサイズにメモリ量を減らす
		destinationMesh.indices[meshIndex].resize(indices.size());
		destinationMesh.vertices[meshIndex].resize(vertices.size());

		// 頂点キャッシュ最適化
		meshopt_optimizeVertexCache(

			destinationMesh.indices[meshIndex].data(),
			indices.data(),
			indices.size(),
			vertexCount);

		// 不要になったメモリを解放
		indices.clear();
		indices.shrink_to_fit();

		// 頂点フェッチ最適化
		meshopt_optimizeVertexFetch(

			destinationMesh.vertices[meshIndex].data(),
			destinationMesh.indices[meshIndex].data(),
			destinationMesh.indices[meshIndex].size(),
			vertices.data(),
			vertices.size(),
			sizeof(MeshVertex));

		// 不要になったメモリを解放
		vertices.clear();
		vertices.shrink_to_fit();
	}
}

void MeshletBuilder::OptimizeEffectMesh(ResourceMesh<EffectMeshVertex>& destinationMesh) {

	// meshの数分
	for (uint32_t meshIndex = 0; meshIndex < destinationMesh.meshCount_; ++meshIndex) {

		std::vector<uint32_t> remap(destinationMesh.indices[meshIndex].size());

		// 重複データを削除するための再マッピング用インデックスを生成
		auto vertexCount = meshopt_generateVertexRemap(

			remap.data(),
			destinationMesh.indices[meshIndex].data(),
			destinationMesh.indices[meshIndex].size(),
			destinationMesh.vertices[meshIndex].data(),
			destinationMesh.vertices[meshIndex].size(),
			sizeof(EffectMeshVertex));

		std::vector<EffectMeshVertex> vertices(vertexCount);
		std::vector<uint32_t> indices(destinationMesh.indices[meshIndex].size());

		// 頂点インデックスを再マッピング
		meshopt_remapIndexBuffer(

			indices.data(),
			destinationMesh.indices[meshIndex].data(),
			destinationMesh.indices[meshIndex].size(),
			remap.data());

		// 頂点データを再マッピング
		meshopt_remapVertexBuffer(

			vertices.data(),
			destinationMesh.vertices[meshIndex].data(),
			destinationMesh.vertices[meshIndex].size(),
			sizeof(EffectMeshVertex),
			remap.data());

		// 不要になったメモリを解放
		remap.clear();
		remap.shrink_to_fit();

		// 最適化したサイズにメモリ量を減らす
		destinationMesh.indices[meshIndex].resize(indices.size());
		destinationMesh.vertices[meshIndex].resize(vertices.size());

		// 頂点キャッシュ最適化
		meshopt_optimizeVertexCache(

			destinationMesh.indices[meshIndex].data(),
			indices.data(),
			indices.size(),
			vertexCount);

		// 不要になったメモリを解放
		indices.clear();
		indices.shrink_to_fit();

		// 頂点フェッチ最適化
		meshopt_optimizeVertexFetch(

			destinationMesh.vertices[meshIndex].data(),
			destinationMesh.indices[meshIndex].data(),
			destinationMesh.indices[meshIndex].size(),
			vertices.data(),
			vertices.size(),
			sizeof(EffectMeshVertex));

		// 不要になったメモリを解放
		vertices.clear();
		vertices.shrink_to_fit();
	}
}

void MeshletBuilder::CreateMeshlet(ResourceMesh<MeshVertex>& destinationMesh) {

	// 頂点、プリミティブ数の最大数
	const size_t kMaxVertices = 64;
	const size_t kMaxPrimitives = 124;

	// メモリをmesh数分確保する
	destinationMesh.uniqueVertexIndices.resize(destinationMesh.meshCount_);
	destinationMesh.primitiveIndices.resize(destinationMesh.meshCount_);
	destinationMesh.meshlets.resize(destinationMesh.meshCount_);

	// meshの数分
	for (uint32_t meshIndex = 0; meshIndex < destinationMesh.meshCount_; ++meshIndex) {

		// メッシュレット最大数の計算
		size_t maxMeshlets = meshopt_buildMeshletsBound(
			destinationMesh.indices[meshIndex].size(),
			kMaxVertices,
			kMaxPrimitives);

		// メッシュレットと補助バッファの確保
		std::vector<meshopt_Meshlet> meshlets(maxMeshlets);
		std::vector<unsigned int> meshletVertices(maxMeshlets * kMaxVertices);
		std::vector<unsigned char> meshletTriangles(maxMeshlets * kMaxPrimitives);

		// メッシュレット構築
		size_t meshletCount = meshopt_buildMeshlets(
			meshlets.data(),
			meshletVertices.data(),
			meshletTriangles.data(),
			destinationMesh.indices[meshIndex].data(),
			destinationMesh.indices[meshIndex].size(),
			reinterpret_cast<const float*>(destinationMesh.vertices[meshIndex].data()),
			destinationMesh.vertices[meshIndex].size(),
			sizeof(MeshVertex),
			kMaxVertices,
			kMaxPrimitives,
			0.0f);

		meshlets.resize(meshletCount);

		// メモリ予約
		destinationMesh.uniqueVertexIndices[meshIndex].reserve(meshletCount * kMaxVertices);
		destinationMesh.primitiveIndices[meshIndex].reserve(meshletCount * kMaxPrimitives);

		// メッシュレットごとにデータを登録
		for (size_t i = 0; i < meshletCount; ++i) {

			const meshopt_Meshlet& meshlet = meshlets[i];

			uint32_t vertexOffset = static_cast<uint32_t>(destinationMesh.uniqueVertexIndices[meshIndex].size());
			uint32_t primitiveOffset = static_cast<uint32_t>(destinationMesh.primitiveIndices[meshIndex].size());

			// 頂点インデックスの登録
			for (size_t j = 0; j < meshlet.vertex_count; ++j) {

				unsigned int index = meshletVertices[meshlet.vertex_offset + j];
				destinationMesh.uniqueVertexIndices[meshIndex].push_back(index);
			}

			// 三角形インデックスの登録、インデックス3つで1三角形
			for (size_t j = 0; j < meshlet.triangle_count; ++j) {

				size_t triBase = meshlet.triangle_offset + j * 3;

				ResourcePrimitiveIndex tris{};
				tris.index0 = meshletTriangles[triBase + 0];
				tris.index1 = meshletTriangles[triBase + 1];
				tris.index2 = meshletTriangles[triBase + 2];

				destinationMesh.primitiveIndices[meshIndex].push_back(tris);
			}

			// メッシュレット情報を登録
			ResourceMeshlet resourceMeshlet = {};
			resourceMeshlet.vertexCount = meshlet.vertex_count;
			resourceMeshlet.vertexOffset = vertexOffset;
			resourceMeshlet.primitiveCount = meshlet.triangle_count;
			resourceMeshlet.primitiveOffset = primitiveOffset;

			std::vector<Color> colors = {

				{1.0f, 1.0f, 1.0f, 1.0f }, // 白
				{ 0.5f, 0.5f, 1.0f, 1.0f }, // 水色
				{ 1.0f, 0.5f, 1.0f, 1.0f }, // ピンク
				{1.0f, 0.0f, 0.0f, 1.0f}, // 赤
				{0.0f, 1.0f, 0.0f, 1.0f}, // 緑
				{0.0f, 0.0f, 1.0f, 1.0f}, // 青
				{1.0f, 1.0f, 0.0f, 1.0f}, // 黄
				{0.0f, 1.0f, 1.0f, 1.0f}, // シアン
				{1.0f, 0.0f, 1.0f, 1.0f}, // マゼンタ
				{1.0f, 0.5f, 0.0f, 1.0f}, // オレンジ
			};

			// meshletの色を指定
			resourceMeshlet.color = colors[i % colors.size()];

			destinationMesh.meshlets[meshIndex].push_back(resourceMeshlet);
		}

		// サイズを実際の使用量に合わせて最適化
		destinationMesh.uniqueVertexIndices.shrink_to_fit();
		destinationMesh.primitiveIndices.shrink_to_fit();
		destinationMesh.meshlets.shrink_to_fit();
	}
}

void MeshletBuilder::CreateEffectMeshlet(ResourceMesh<EffectMeshVertex>& destinationMesh) {

	// 頂点、プリミティブ数の最大数
	const size_t kMaxVertices = 64;
	const size_t kMaxPrimitives = 124;

	// メモリをmesh数分確保する
	destinationMesh.uniqueVertexIndices.resize(destinationMesh.meshCount_);
	destinationMesh.primitiveIndices.resize(destinationMesh.meshCount_);
	destinationMesh.meshlets.resize(destinationMesh.meshCount_);

	// meshの数分
	for (uint32_t meshIndex = 0; meshIndex < destinationMesh.meshCount_; ++meshIndex) {

		// メッシュレット最大数の計算
		size_t maxMeshlets = meshopt_buildMeshletsBound(
			destinationMesh.indices[meshIndex].size(),
			kMaxVertices,
			kMaxPrimitives);

		// メッシュレットと補助バッファの確保
		std::vector<meshopt_Meshlet> meshlets(maxMeshlets);
		std::vector<unsigned int> meshletVertices(maxMeshlets * kMaxVertices);
		std::vector<unsigned char> meshletTriangles(maxMeshlets * kMaxPrimitives);

		// メッシュレット構築
		size_t meshletCount = meshopt_buildMeshlets(
			meshlets.data(),
			meshletVertices.data(),
			meshletTriangles.data(),
			destinationMesh.indices[meshIndex].data(),
			destinationMesh.indices[meshIndex].size(),
			reinterpret_cast<const float*>(destinationMesh.vertices[meshIndex].data()),
			destinationMesh.vertices[meshIndex].size(),
			sizeof(EffectMeshVertex),
			kMaxVertices,
			kMaxPrimitives,
			0.0f);

		meshlets.resize(meshletCount);

		// メモリ予約
		destinationMesh.uniqueVertexIndices[meshIndex].reserve(meshletCount * kMaxVertices);
		destinationMesh.primitiveIndices[meshIndex].reserve(meshletCount * kMaxPrimitives);

		// メッシュレットごとにデータを登録
		for (size_t i = 0; i < meshletCount; ++i) {

			const meshopt_Meshlet& meshlet = meshlets[i];

			uint32_t vertexOffset = static_cast<uint32_t>(destinationMesh.uniqueVertexIndices[meshIndex].size());
			uint32_t primitiveOffset = static_cast<uint32_t>(destinationMesh.primitiveIndices[meshIndex].size());

			// 頂点インデックスの登録
			for (size_t j = 0; j < meshlet.vertex_count; ++j) {

				unsigned int index = meshletVertices[meshlet.vertex_offset + j];
				destinationMesh.uniqueVertexIndices[meshIndex].push_back(index);
			}

			// 三角形インデックスの登録、インデックス3つで1三角形
			for (size_t j = 0; j < meshlet.triangle_count; ++j) {

				size_t triBase = meshlet.triangle_offset + j * 3;

				ResourcePrimitiveIndex tris{};
				tris.index0 = meshletTriangles[triBase + 0];
				tris.index1 = meshletTriangles[triBase + 1];
				tris.index2 = meshletTriangles[triBase + 2];

				destinationMesh.primitiveIndices[meshIndex].push_back(tris);
			}

			// メッシュレット情報を登録
			ResourceMeshlet resourceMeshlet = {};
			resourceMeshlet.vertexCount = meshlet.vertex_count;
			resourceMeshlet.vertexOffset = vertexOffset;
			resourceMeshlet.primitiveCount = meshlet.triangle_count;
			resourceMeshlet.primitiveOffset = primitiveOffset;

			std::vector<Color> colors = {

				{1.0f, 1.0f, 1.0f, 1.0f }, // 白
				{ 0.5f, 0.5f, 1.0f, 1.0f }, // 水色
				{ 1.0f, 0.5f, 1.0f, 1.0f }, // ピンク
				{1.0f, 0.0f, 0.0f, 1.0f}, // 赤
				{0.0f, 1.0f, 0.0f, 1.0f}, // 緑
				{0.0f, 0.0f, 1.0f, 1.0f}, // 青
				{1.0f, 1.0f, 0.0f, 1.0f}, // 黄
				{0.0f, 1.0f, 1.0f, 1.0f}, // シアン
				{1.0f, 0.0f, 1.0f, 1.0f}, // マゼンタ
				{1.0f, 0.5f, 0.0f, 1.0f}, // オレンジ
			};

			// meshletの色を指定
			resourceMeshlet.color = colors[i % colors.size()];

			destinationMesh.meshlets[meshIndex].push_back(resourceMeshlet);
		}

		// サイズを実際の使用量に合わせて最適化
		destinationMesh.uniqueVertexIndices.shrink_to_fit();
		destinationMesh.primitiveIndices.shrink_to_fit();
		destinationMesh.meshlets.shrink_to_fit();
	}
}
