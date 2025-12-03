#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/MathLib/Vector3.h>

//============================================================================
//	ObjectAreaChecker enum class
//============================================================================

// 座標で見たオブジェクトのエリア
enum class ObjectArea {

	RightFront, // 右前(+x,+z)
	LeftFront,  // 左前(-x,+z)
	RightBack,  // 右後(+x,-z)
	LeftBack    // 左後(-x,-z)
};

//============================================================================
//	ObjectAreaChecker class
//	オブジェクトのエリアを判定する
//============================================================================
class ObjectAreaChecker {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	ObjectAreaChecker() = default;
	~ObjectAreaChecker() = default;

	// 座標からエリアを判定
	static ObjectArea CheckArea(const Vector3& pos, const Vector3& origin = Vector3::AnyInit(0.0f));

	// エリアごとの向きの値を取得
	static Vector3 GetAreaDirection(ObjectArea area);
};