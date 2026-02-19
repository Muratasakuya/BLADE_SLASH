#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/GPUObject/DxConstBuffer.h>
#include <Engine/Core/Graphics/GPUObject/DxStructuredBuffer.h>
#include <Engine/Core/Graphics/GPUObject/IndexBuffer.h>
#include <Engine/Core/Graphics/Mesh/MeshletStructures.h>

//============================================================================
//	IMesh class
//	メッシュ共通インターフェース。GPU頂点/インデックス等の転送IFを定義する。
//============================================================================
namespace SakuEngine {

	class IMesh {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		IMesh() = default;
		virtual ~IMesh() = default;

		void Init(ID3D12Device* device, const ResourceMesh<MeshVertex>& resource,
			bool isSkinned, uint32_t numInstance);

		//--------- accessor -----------------------------------------------------

		bool IsSkinned() const { return isSkinned_; }

		// mesh数
		uint32_t GetMeshCount() const { return static_cast<uint32_t>(meshletCounts_.size()); }

		// meshlet数
		uint32_t GetMeshletCount(uint32_t meshIndex) const { return meshletCounts_[meshIndex]; }
		uint32_t GetVertexCount(uint32_t meshIndex) const { return vertexCounts_[meshIndex]; }
		uint32_t GetIndexCount(uint32_t meshIndex) const { return indexCounts_[meshIndex]; }

		const DxConstBuffer<MeshInstanceData>& GetMeshInstanceData(uint32_t meshIndex) const { return meshInstanceData_[meshIndex]; }

		const DxStructuredBuffer<uint32_t>& GetUniqueVertexIndexBuffer(uint32_t meshIndex) const { return uniqueVertexIndices_[meshIndex]; }

		const DxStructuredBuffer<ResourcePrimitiveIndex>& GetPrimitiveIndexBuffer(uint32_t meshIndex) const { return primitiveIndices_[meshIndex]; }

		const DxStructuredBuffer<ResourceMeshlet>& GetMeshletBuffer(uint32_t meshIndex) const { return meshlets_[meshIndex]; }

		const IndexBuffer& GetIndexBuffer(uint32_t meshIndex) const { return indices_[meshIndex]; }
	protected:
		//========================================================================
		//	protected Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		// staticかskinnedかのフラグ
		bool isSkinned_;

		// meshlet数
		std::vector<uint32_t> meshletCounts_;
		// 頂点数
		std::vector<uint32_t> vertexCounts_;
		std::vector<uint32_t> indexCounts_;

		// buffers
		std::vector<DxConstBuffer<MeshInstanceData>> meshInstanceData_;
		std::vector<DxStructuredBuffer<uint32_t>> uniqueVertexIndices_;
		std::vector<DxStructuredBuffer<ResourcePrimitiveIndex>> primitiveIndices_;
		std::vector<DxStructuredBuffer<ResourceMeshlet>> meshlets_;

		// indexBuffer、描画には使わない
		std::vector<IndexBuffer> indices_;

		//--------- functions ----------------------------------------------------

		void CreateBuffer(ID3D12Device* device, uint32_t meshIndex,
			const ResourceMesh<MeshVertex>& resource);
		void TransferBuffer(uint32_t meshIndex, const ResourceMesh<MeshVertex>& resource,
			bool isSkinned);

		virtual void CreateVertexBuffer(ID3D12Device* device, uint32_t meshIndex,
			const ResourceMesh<MeshVertex>& resource, uint32_t numInstance) = 0;
		// 指定サブメッシュの頂点バッファをGPUへ転送する(リソース/ステート管理込み)
		virtual void TransferVertexBuffer(uint32_t meshIndex, const ResourceMesh<MeshVertex>& resource) = 0;
	};

	//============================================================================
	//	StaticMesh class
	//	静的メッシュ表現。固定頂点を使用し、スキニングを行わない。
	//============================================================================
	class StaticMesh :
		public IMesh {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		StaticMesh() = default;
		~StaticMesh() = default;

		//--------- accessor -----------------------------------------------------

		const DxStructuredBuffer<MeshVertex>& GetVertexBuffer(uint32_t meshIndex) const { return vertices_[meshIndex]; }
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		// vertexBuffer
		std::vector<DxStructuredBuffer<MeshVertex>> vertices_;

		//--------- functions ----------------------------------------------------

		void CreateVertexBuffer(ID3D12Device* device, uint32_t meshIndex,
			const ResourceMesh<MeshVertex>& resource, uint32_t numInstance) override;
		void TransferVertexBuffer(uint32_t meshIndex, const ResourceMesh<MeshVertex>& resource) override;
	};

	//============================================================================
	//	SkinnedMesh class
	//	スキンメッシュ表現。ボーン/ウェイトに基づいて頂点を更新する。
	//============================================================================
	class SkinnedMesh :
		public IMesh {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		SkinnedMesh() = default;
		~SkinnedMesh() = default;

		//--------- accessor -----------------------------------------------------

		const DxStructuredBuffer<MeshVertex>& GetInputVertexBuffer(uint32_t meshIndex) const { return inputVertices_[meshIndex]; }
		const DxStructuredBuffer<MeshVertex>& GetOutputVertexBuffer(uint32_t meshIndex) const { return outputVertices_[meshIndex]; }
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		// vertexBuffer
		std::vector<DxStructuredBuffer<MeshVertex>> inputVertices_;
		std::vector<DxStructuredBuffer<MeshVertex>> outputVertices_;

		//--------- functions ----------------------------------------------------

		void CreateVertexBuffer(ID3D12Device* device, uint32_t meshIndex,
			const ResourceMesh<MeshVertex>& resource, uint32_t numInstance) override;
		void TransferVertexBuffer(uint32_t meshIndex, const ResourceMesh<MeshVertex>& resource) override;
	};
}; // SakuEngine
