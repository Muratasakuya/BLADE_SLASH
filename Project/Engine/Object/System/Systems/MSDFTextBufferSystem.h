#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/System/Base/ISystem.h>
#include <Engine/Object/Data/Text/MSDFText.h>

namespace SakuEngine {

	// front
	class Transform2D;
	class MSDFTextMaterial;

	//============================================================================
	//	structure
	//============================================================================

	struct MSDFTextData {

		Transform2D* transform;
		MSDFTextMaterial* material;
		MSDFText* text;
	};

	//============================================================================
	//	MSDFTextBufferSystem class
	//	MSDFテキストのバッファを管理するシステム
	//============================================================================
	class MSDFTextBufferSystem :
		public ISystem {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		MSDFTextBufferSystem() = default;
		~MSDFTextBufferSystem() = default;

		Archetype Signature() const override;

		void Update(ObjectPoolManager& ObjectPoolManager) override;

		//--------- accessor -----------------------------------------------------

		const std::vector<MSDFTextData>& GetTextData() const { return textData_; }
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		std::vector<MSDFTextData> textData_;
	};
}; // SakuEngine