#include "ObjectAreaChecker.h"

//============================================================================
//	ObjectAreaChecker classMethods
//============================================================================

ObjectArea ObjectAreaChecker::CheckArea(const Vector3& pos, const Vector3& origin) {

	ObjectArea area;
	// 右側、原点よりx座標が大きい
	if (origin.x <= pos.x) {
		if (origin.z <= pos.z) {

			// 前側
			area = ObjectArea::RightFront;
		} else {

			// 後側
			area = ObjectArea::RightBack;
		}
	}
	// 左側、原点よりx座標が小さい
	else {
		if (origin.z <= pos.z) {

			// 前側
			area = ObjectArea::LeftFront;
		} else {

			// 後側
			area = ObjectArea::LeftBack;
		}
	}
	return area;
}

Vector3 ObjectAreaChecker::GetAreaDirection(ObjectArea area) {

	// エリアごとの向きの値を取得
	Vector3 direction{};
	switch (area) {
	case ObjectArea::RightFront:

		direction = Vector3(1.0f, 0.0f, 1.0f);
		break;
	case ObjectArea::LeftFront:

		direction = Vector3(-1.0f, 0.0f, 1.0f);
		break;
	case ObjectArea::RightBack:

		direction = Vector3(1.0f, 0.0f, -1.0f);
		break;
	case ObjectArea::LeftBack:

		direction = Vector3(-1.0f, 0.0f, -1.0f);
		break;
	}
	direction = direction.Normalize();
	return direction;
}