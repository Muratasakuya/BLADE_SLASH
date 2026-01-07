#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/MathLib/Vector3.h>
#include <Engine/Utility/Enum/Direction.h>

namespace SakuEngine {

	//============================================================================
	//	CollisionCapsule namespace
	//	衝突形状とは別にカプセル形状を提供する
	//============================================================================
	namespace CollisionExt {

		//============================================================================
		//	Capsule
		//	線分を半径で太くした形状
		//============================================================================
		struct Capsule {

			float radius; // 半径
			Vector3 pointA; // 上端
			Vector3 pointB; // 下端
		};

		//============================================================================
		//	CapsuleSettings
		//	アクターの座標からワールドカプセルを生成するためのパラメータ
		//============================================================================
		struct CapsuleSettings {

			Vector3 centerOffset = Vector3::AnyInit(0.0f);
			Vector3 up = Direction::Get(Direction3D::Up);

			// 上端から下端までの距離の半分
			float halfHeight = 1.0f;
			float radius = 0.4f;

			// json
			void FromJson(const Json& data);
			void ToJson(Json& data);
		};

		//============================================================================
		//	関数実装
		//============================================================================

		// ワールド座標系のカプセルを生成
		Capsule MakeWorldCapsule(const Vector3& actorTranslation, const CapsuleSettings& settings);
	}
}