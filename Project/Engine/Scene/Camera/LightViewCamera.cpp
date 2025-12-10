#include "LightViewCamera.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================

// directX
#include <DirectXMath.h>

//============================================================================
//	LightViewCamera classMethods
//============================================================================

void LightViewCamera::Init() {

	// 初期値設定
	translation_ = Vector3(0.0f, 14.0f, 0.0f);
	target_ = Vector3(0.0f, 0.0f, 0.0f);
	upDirection_ = Vector3(0.0f, 0.0f, -1.0f);

	orthoSize_ = 128.0f;
	nearPlane_ = 1.0f;
	farPlane_ = 640.0f;

	Update();
}

void LightViewCamera::Update() {

	DirectX::XMMATRIX dxProjectionMatrix = DirectX::XMMatrixOrthographicLH(
		orthoSize_,
		orthoSize_,
		nearPlane_,
		farPlane_
	);

	DirectX::XMFLOAT3 eye(translation_.x, translation_.y, translation_.z);
	DirectX::XMFLOAT3 target(target_.x, target_.y, target_.z);
	DirectX::XMFLOAT3 up(upDirection_.x, upDirection_.y, upDirection_.z);

	// light視点のView行列の計算
	DirectX::XMMATRIX dxViewMatrix = DirectX::XMMatrixLookAtLH(
		DirectX::XMLoadFloat3(&eye),
		DirectX::XMLoadFloat3(&target),
		DirectX::XMLoadFloat3(&up));

	// 元のMatrix4x4に戻す
	DirectX::XMStoreFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4*>(&projectionMatrix_), dxProjectionMatrix);
	DirectX::XMStoreFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4*>(&viewMatrix_), dxViewMatrix);

	viewProjectionMatrix_ = viewMatrix_ * projectionMatrix_;
}

void LightViewCamera::ImGui() {

	ImGui::PushItemWidth(itemWidth_);

	ImGui::DragFloat3("translation##LightViewCamera", &transform_.translation.x, 0.01f);
	ImGui::DragFloat3("target##LightViewCamera", &target_.x, 0.01f);
	ImGui::DragFloat3("upDirection##LightViewCamera", &upDirection_.x, 0.01f);

	ImGui::DragFloat("orthoSize##LightViewCamera", &orthoSize_, 0.5f);
	ImGui::DragFloat("nearPlane##LightViewCamera", &nearPlane_, 1.0f);
	ImGui::DragFloat("farPlane##LightViewCamera", &farPlane_, 1.0f);

	ImGui::PopItemWidth();
}
