#include "Camera2D.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Config.h>

//============================================================================
//	Camera2D classMethods
//============================================================================

void Camera2D::Init() {

	// 初期値設定
	transform_.Init();

	// 行列更新
	transform_.UpdateMatrix();
	viewMatrix_ = Matrix4x4::Inverse(transform_.matrix.world);

	projectionMatrix_ =
		Matrix4x4::MakeOrthographicMatrix(0.0f, 0.0f, Config::kWindowWidthf,
			Config::kWindowHeightf, 0.0f, 100.0f);

	// 更新1回でいいので、Update関数は実装しない
	viewProjectionMatrix_ = viewMatrix_ * projectionMatrix_;
}

void Camera2D::ImGui() {
}
