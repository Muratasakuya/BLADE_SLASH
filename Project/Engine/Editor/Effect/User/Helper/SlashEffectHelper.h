#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/Effect/User/EffectGroup.h>
#include <Engine/Object/Base/GameObject3D.h>

namespace SakuEngine {

	//============================================================================
	//	SlashEffectHelper class
	//	3Dオブジェクトに追従する剣エフェクトの補助クラス
	//============================================================================
	struct SlashEffectHelper {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		SlashEffectHelper() = default;
		~SlashEffectHelper() = default;

		//--------- variables ----------------------------------------------------

		// エフェクト本体
		std::unique_ptr<SakuEngine::EffectGroup> effect;
		// ノードの名前
		std::string effectNodeName;

		// 発生位置のオフセット
		SakuEngine::Vector3 effectOffset;

		//--------- functions ----------------------------------------------------

		// 親の設定
		void SetParent(const std::string& nodeName, const Transform3D& transform);

		// 発生
		void Emit(const GameObject3D& object);
		// 更新
		void Update(const GameObject3D& object);

		// エディター
		void EditOffset(const std::string& label = "slashEffectOffset");

		// json
		void FromJson(const Json& data, const std::string& label);
		void ToJson(Json& data, const std::string& label) const;
	};
} // SakuEngine