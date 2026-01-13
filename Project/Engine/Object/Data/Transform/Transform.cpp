#include "Transform.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Config.h>
#include <Engine/Core/Graphics/Renderer/Line/LineRenderer.h>
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Engine/Utility/Helper/ImGuiHelper.h>
#include <Engine/Utility/Enum/EnumAdapter.h>
#include <Engine/Object/Data/Text/MSDFText.h>
#include <Engine/Editor/GameObject/ImGuiObjectEditor.h>

// imgui
#include <imgui.h>

//============================================================================
//	Transform classMethods
//============================================================================

//============================================================================
// 3D
//============================================================================

void BaseTransform3D::Init() {

	scale = Vector3::AnyInit(1.0f);
	rotation.Init();
	translation.Init();

	eulerRotate.Init();
	prevScale = Vector3::AnyInit(1.0f);
}

void BaseTransform3D::UpdateMatrix() {

	// 値に変更がなければ更新しない
	bool selfUnchanged =
		(scale == prevScale &&
			rotation == prevRotation &&
			translation == prevTranslation &&
			offsetTranslation == prevOffsetTranslation);

	// 親と自分の値が変わっていなければ更新しない
	if (selfUnchanged && !isCompulsion_) {

		// 親の変更をチェック
		if (parent) {
			// 親も変更がなければ更新しない
			if (!parent->isDirty_) {

				isDirty_ = false;
				return;
			}
		}
		// 親がいなければ更新しない
		else {

			isDirty_ = false;
			return;
		}
	}
	// どちらかに変更があれば更新
	isDirty_ = true;

	// 行列を更新
	matrix.Update(parent, scale, rotation, offsetTranslation + translation, isIgnoreParentScale);

	// 値を保存
	prevScale = scale;
	prevRotation = rotation;
	prevTranslation = translation;
	prevOffsetTranslation = offsetTranslation;
}

bool BaseTransform3D::ImGui(float itemSize) {

	bool edited = false;

	ImGui::PushItemWidth(itemSize);

	edited = ImGui::Button("Reset");
	if (edited) {

		scale = Vector3::AnyInit(1.0f);
		rotation.Init();
		translation.Init();

		eulerRotate.Init();
	}
	ImGui::Separator();

	ImGui::Text(std::format("isDirty: {}", isDirty_).c_str());
	edited |= ImGui::Checkbox("isCompulsion", &isCompulsion_);
	edited |= ImGui::Checkbox("isIgnoreParentScale", &isIgnoreParentScale);

	edited |= ImGui::DragFloat3("translation", &translation.x, 0.01f);
	if (ImGui::DragFloat3("rotation", &eulerRotate.x, 0.01f)) {

		rotation = Quaternion::EulerToQuaternion(eulerRotate);
	}
	ImGui::Text("quaternion(%4.3f, %4.3f, %4.3f, %4.3f)",
		rotation.x, rotation.y, rotation.z, rotation.w);
	edited |= ImGui::DragFloat3("scale", &scale.x, 0.01f);

	ImGui::SeparatorText("Offset");

	ImGuiHelper::ValueText<Vector3>("translation", offsetTranslation);

	ImGui::SeparatorText("World Matrix");
	if (ImGui::BeginTable("WorldMatrix", 4,
		ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit)) {

		const Matrix4x4& world = matrix.world;
		for (int row = 0; row < 4; ++row) {

			ImGui::TableNextRow();
			for (int col = 0; col < 4; ++col) {

				ImGui::TableSetColumnIndex(col);
				ImGui::Text("%.3f", world.m[row][col]);
			}
		}
		ImGui::EndTable();
	}
	// 親がいる場合
	if (parent) {

		ImGui::SeparatorText("Parent World Matrix");

		ImGui::Text(std::format("isDirty {}", parent->isDirty_).c_str());
		ImGui::Text(std::format("isCompulsion {}", parent->isCompulsion_).c_str());
		ImGui::Text("name: %s", static_cast<const Transform3D*>(parent)->GetInstancingName().c_str());
		if (ImGui::BeginTable("Parent WorldMatrix", 4,
			ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit)) {

			const Matrix4x4& world = parent->matrix.world;
			for (int row = 0; row < 4; ++row) {

				ImGui::TableNextRow();
				for (int col = 0; col < 4; ++col) {

					ImGui::TableSetColumnIndex(col);
					ImGui::Text("%.3f", world.m[row][col]);
				}
			}
			ImGui::EndTable();
		}
	}

	ImGui::PopItemWidth();

	return edited;
}

void BaseTransform3D::ToJson(Json& data) {

	data["isCompulsion_"] = isCompulsion_;
	data["isIgnoreParentScale"] = isIgnoreParentScale;
	data["scale"] = scale.ToJson();

	// 正規化してから保存
	rotation = Quaternion::Normalize(rotation);
	data["rotation"] = rotation.ToJson();
	data["translation"] = translation.ToJson();
}

void BaseTransform3D::FromJson(const Json& data) {

	if (data.empty()) {
		return;
	}

	isCompulsion_ = data.value("isCompulsion_", false);
	isIgnoreParentScale = data.value("isIgnoreParentScale", false);
	scale = JsonAdapter::ToObject<Vector3>(data["scale"]);
	rotation = JsonAdapter::ToObject<Quaternion>(data["rotation"]);
	translation = JsonAdapter::ToObject<Vector3>(data["translation"]);
}

Vector3 BaseTransform3D::GetWorldScale() const {

	Vector3 right(matrix.world.m[0][0], matrix.world.m[0][1], matrix.world.m[0][2]);
	Vector3 up(matrix.world.m[1][0], matrix.world.m[1][1], matrix.world.m[1][2]);
	Vector3 forward(matrix.world.m[2][0], matrix.world.m[2][1], matrix.world.m[2][2]);

	Vector3 worldScale{};
	worldScale.x = right.Length();
	worldScale.y = up.Length();
	worldScale.z = forward.Length();
	return worldScale;
}

Quaternion BaseTransform3D::GetWorldRotation() const {

	// 親も含めたForward、Upベクトルを取得
	Vector3 forward = GetForward();
	Vector3 up = GetUp();

	// 回転を作成
	Quaternion worldRotation = Quaternion::LookRotation(forward, up);
	return Quaternion::Normalize(worldRotation);
}

Vector3 BaseTransform3D::GetWorldPos() const {

	Vector3 worldPos{};
	worldPos.x = matrix.world.m[3][0];
	worldPos.y = matrix.world.m[3][1];
	worldPos.z = matrix.world.m[3][2];

	return worldPos;
}

Vector3 BaseTransform3D::GetForward() const {
	return Vector3(matrix.world.m[2][0], matrix.world.m[2][1], matrix.world.m[2][2]).Normalize();
}

Vector3 BaseTransform3D::GetBack() const {
	return Vector3(-GetForward().x, -GetForward().y, -GetForward().z);
}

Vector3 BaseTransform3D::GetRight() const {
	return Vector3(matrix.world.m[0][0], matrix.world.m[0][1], matrix.world.m[0][2]).Normalize();
}

Vector3 BaseTransform3D::GetLeft() const {
	return Vector3(-GetRight().x, -GetRight().y, -GetRight().z);
}

Vector3 BaseTransform3D::GetUp() const {
	return Vector3(matrix.world.m[1][0], matrix.world.m[1][1], matrix.world.m[1][2]).Normalize();
}

Vector3 BaseTransform3D::GetDown() const {
	return Vector3(-GetUp().x, -GetUp().y, -GetUp().z);
}

void BaseTransform3D::SetIsDirty(bool isDirty) {

	isDirty_ = isDirty;
	if (isDirty_) {

		// 値を強制的に変化させる
		prevScale += 1.0f;
	}
}

//============================================================================
// Effect
//============================================================================

void EffectTransform::Init() {

	rotation.Init();
	eulerRotate.Init();

	translation.Init();
}

void EffectTransform::ImGui(float itemSize) {

	ImGui::PushItemWidth(itemSize);

	if (ImGui::Button("Reset")) {

		rotation.Init();
		translation.Init();

		eulerRotate.Init();
	}
	ImGui::Separator();

	ImGui::DragFloat3("translation", &translation.x, 0.01f);
	if (ImGui::DragFloat3("rotation", &eulerRotate.x, 0.01f)) {

		rotation = Quaternion::Normalize(Quaternion::EulerToQuaternion(eulerRotate));
	}
	ImGui::Text("quaternion(%4.3f, %4.3f, %4.3f, %4.3f)",
		rotation.x, rotation.y, rotation.z, rotation.w);

	ImGui::PopItemWidth();
}

//============================================================================
// 2D
//============================================================================

Vector2 BaseTransform2D::GetWorldPos() const {

	Vector2 worldPos{};
	worldPos.x = matrix.m[3][0];
	worldPos.y = matrix.m[3][1];
	return worldPos;
}

void BaseTransform2D::SetCenterPos() {

	translation.x = Config::kWindowWidthf / 2.0f;
	translation.y = Config::kWindowHeightf / 2.0f;
}

void BaseTransform2D::Init(ID3D12Device* device) {

	translation = Vector2::AnyInit(0.0f);
	rotation = 0.0f;
	sizeScale = Vector2::AnyInit(1.0f);
	// 中心設定
	anchorPoint = Vector2::AnyInit(0.5f);

	// deviceがnullptrの場合はバッファを作成しない
	if (!device) {
		return;
	}

	// buffer初期化
	buffer_.CreateBuffer(device);
}

void BaseTransform2D::UpdateMatrix() {

	// ローカル行列の計算
	Matrix4x4 local = Matrix4x4::MakeAffineMatrix(Vector3(sizeScale.x, sizeScale.y, 1.0f),
		Vector3(0.0f, 0.0f, rotation), Vector3(translation.x, translation.y, 0.0f));

	if (parent) {
		// その場で回転させる
		if (rotateAroundSelfWhenParented) {

			matrix = Matrix4x4::Multiply(local, parent->matrix);
		}
		// 親に合わせて回転させる
		else {

			matrix = Matrix4x4::Multiply(parent->matrix, local);
		}
	} else {

		// 親がいない場合はローカル行列をそのまま設定
		matrix = local;
	}

	// 作成されたときのみ
	if (buffer_.IsCreatedResource()) {

		// buffer転送
		buffer_.TransferData(matrix);
	}
}

void BaseTransform2D::ImGuiCommon(float itemSize) {

	ImGui::SeparatorText("Common");

	if (ImGui::Button("Set CenterPos", ImVec2(itemSize, 28.0f))) {

		SetCenterPos();
	}
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::PushItemWidth(itemSize);

	ImGui::DragFloat2("translation", &translation.x, 1.0f);
	ImGui::SliderAngle("rotation", &rotation);
	ImGui::DragFloat2("sizeScale", &sizeScale.x, 0.01f);
	ImGui::DragFloat2("anchorPoint", &anchorPoint.x, 0.01f, -1.0f, 1.0f);
	ImGui::Checkbox("rotateAroundSelfWhenParented", &rotateAroundSelfWhenParented);

	ImGui::PopItemWidth();
}

void BaseTransform2D::FromJsonCommon(const Json& data) {

	if (data.empty()) {
		return;
	}
	translation = Vector2::FromJson(data["translation"]);
	rotation = data["rotation"].get<float>();
	sizeScale = Vector2::FromJson(data["sizeScale"]);
	anchorPoint = Vector2::FromJson(data["anchorPoint"]);
	rotateAroundSelfWhenParented = data.value("rotateAroundSelfWhenParented", true);
}

void BaseTransform2D::ToJsonCommon(Json& data) {

	data["translation"] = translation.ToJson();
	data["rotation"] = rotation;
	data["sizeScale"] = sizeScale.ToJson();
	data["anchorPoint"] = anchorPoint.ToJson();
	data["rotateAroundSelfWhenParented"] = rotateAroundSelfWhenParented;
}

void Transform2D::Init(ID3D12Device* device) {

	// 親クラス初期化
	BaseTransform2D::Init(device);

	size = Vector2::AnyInit(0.0f);
	// 左上設定
	textureLeftTop = Vector2::AnyInit(0.0f);
	textureSize = Vector2::AnyInit(0.0f);
}

void Transform2D::ImGui(float itemSize) {

	BaseTransform2D::ImGuiCommon(itemSize);

	ImGui::PushItemWidth(itemSize);

	ImGui::DragFloat2("size", &size.x, 1.0f);
	ImGui::DragFloat2("textureLeftTop", &textureLeftTop.x, 1.0f);
	ImGui::DragFloat2("textureSize", &textureSize.x, 1.0f);

	ImGui::SeparatorText("VertexOffset");

	// 左下
	ImGui::DragFloat2("leftBottom", &vertexOffset[0].x, 0.1f);
	// 左上
	ImGui::DragFloat2("leftTop", &vertexOffset[1].x, 0.1f);
	// 右下
	ImGui::DragFloat2("rightBottom", &vertexOffset[2].x, 0.1f);
	// 右上
	ImGui::DragFloat2("rightTop", &vertexOffset[3].x, 0.1f);

	ImGui::PopItemWidth();
}

void Transform2D::ToJson(Json& data) {

	BaseTransform2D::ToJsonCommon(data);

	data["size"] = size.ToJson();
	data["textureLeftTop"] = textureLeftTop.ToJson();
	data["textureSize"] = textureSize.ToJson();

	data["vertexOffset"] = Json::array();
	for (const auto& offset : vertexOffset) {

		data["vertexOffset"].push_back(offset.ToJson());
	}
}

void Transform2D::FromJson(const Json& data) {

	BaseTransform2D::FromJsonCommon(data);

	size = Vector2::FromJson(data["size"]);
	textureLeftTop = Vector2::FromJson(data["textureLeftTop"]);
	textureSize = Vector2::FromJson(data["textureSize"]);

	if (data.contains("vertexOffset")) {
		for (uint32_t i = 0; i < vertexOffset.size(); ++i) {

			vertexOffset[i] = Vector2::FromJson(data["vertexOffset"][i]);
		}
	}
}

void TextTransform2D::Secure(ID3D12Device* device, uint32_t maxGlyphs) {

	// すでに確保されている場合は何もしない
	if (!charTransforms_.empty()) {
		return;
	}

	maxGlyphs_ = maxGlyphs;

	charTransforms_.clear();
	matrices_.clear();
	for (uint32_t i = 0; i < maxGlyphs; ++i) {

		charTransforms_.emplace_back();
		matrices_.emplace_back();
		charTransforms_[i].Init(nullptr);
		matrices_[i] = Matrix4x4::MakeIdentity4x4();
	}

	// 文字ごとの行列バッファ作成
	charMatrixBuffer_.CreateSRVBuffer(device, maxGlyphs_);
}

void TextTransform2D::UpdateAllMatrix(const MSDFText& text) {

	// 現在の文字を取得
	currentText_ = &text.GetText();
	currentCodepoints_ = text.GetCodepoints();

	if (charTransforms_.empty()) {
		return;
	}

	// 全てのトランスフォームを更新
	BaseTransform2D::UpdateMatrix();

	// 描画される文字数分
	uint32_t renderCount = (std::min)(maxGlyphs_, text.GetRenderedGlyphCount());

	// 文字分更新
	for (uint32_t i = 0; i < maxGlyphs_; ++i) {

		// 描画されない文字は単位行列を設定
		if (renderCount <= i) {
			matrices_[i] = Matrix4x4::MakeIdentity4x4();
			continue;
		}

		const Vector2 pivot = text.GetGlyphPivot(i);
		const auto& transform = charTransforms_[i];

		// ピボット位置まで移動させた行列
		Matrix4x4 toOrigin = Matrix4x4::MakeAffineMatrix(Vector3(1.0f, 1.0f, 1.0f),
			Vector3(0.0f, 0.0f, 0.0f), Vector3(-pivot.x, -pivot.y, 0.0f));

		// ピボット+オフセット位置でのSRT行列
		Matrix4x4 worldMatrix = Matrix4x4::MakeAffineMatrix(Vector3(transform.sizeScale.x, transform.sizeScale.y, 1.0f),
			Vector3(0.0f, 0.0f, transform.rotation),
			Vector3(pivot.x + transform.translation.x, pivot.y + transform.translation.y, 0.0f));
		// 最終行列を計算
		matrices_[i] = Matrix4x4::Multiply(toOrigin, worldMatrix);
	}
	// バッファ転送
	charMatrixBuffer_.TransferData(matrices_);
}

void TextTransform2D::ImGui(float itemSize) {

	BaseTransform2D::ImGuiCommon(itemSize);

	ImGui::PushItemWidth(itemSize);

	ImGui::Text("CurrentText: %s", currentText_ ? currentText_->c_str() : "null");
	ImGui::Text("MaxGlyphs:   %d", maxGlyphs_);
	ImGui::Checkbox("drawTextBox_", &drawTextBox_);

	ImGui::SeparatorText("TextBox");

	ImGui::Checkbox("enableTextBox", &enableTextBox);
	ImGui::DragFloat2("textBoxSize", &textBoxSize.x, 1.0f);
	ImGui::DragFloat2("textBoxPadding", &textBoxPadding.x, 1.0f);
	ImGui::DragFloat("lineSpacing", &lineSpacing, 0.1f);

	EnumAdapter<TextWrapMode>::Combo("wrapMode", &wrapMode);
	EnumAdapter<TextVerticalAlign>::Combo("verticalAlign", &verticalAlign);

	ImGui::SeparatorText("CharTransforms");

	// 文字ごとのトランスフォーム表示
	for (uint32_t i = 0; i < maxGlyphs_; ++i) {

		bool enable = i < currentCodepoints_.size();
		if (!enable) {
			continue;
		}

		// 表示する文字がある場合はその文字を表示
		std::string label = "CharTransform: " + Algorithm::CodepointToUtf8(currentCodepoints_[i]);

		ImGui::PushID(i);
		if (ImGui::TreeNode(label.c_str())) {

			auto& charTransform = charTransforms_[i];

			ImGui::DragFloat2("translation", &charTransform.translation.x, 0.1f);
			ImGui::SliderAngle("rotation", &charTransform.rotation);
			ImGui::DragFloat2("sizeScale", &charTransform.sizeScale.x, 0.01f);
			ImGui::TreePop();
		}
		ImGui::PopID();
	}
	ImGui::PopItemWidth();

	LineRenderer* lineRenderer = LineRenderer::GetInstance();

	// テキストボックス描画
	if (drawTextBox_) {

		Vector2 baseAnchor = Vector2::AnyInit(0.5f);
		Vector2 boxCenter = translation + (baseAnchor - anchorPoint) * textBoxSize;
		lineRenderer->Get2D()->DrawRect(boxCenter, textBoxSize, baseAnchor, Color::Magenta());
	}
}