#include "GameObject2DArray.h"

using namespace SakuEngine;

//============================================================================
//	GameObject2DArray classMethods
//============================================================================

void GameObject2DArray::Init() {

	// 親トランスフォーム初期化
	transform_.Init(nullptr);
}

void GameObject2DArray::Add(const std::string& textureName,
	const std::string& name, const std::string& groupName) {

	// 2Dオブジェクトを生成して初期化
	std::unique_ptr<GameObject2D> object = std::make_unique<SakuEngine::GameObject2D>();
	object->Init(textureName, name, groupName);

	// 親トランスフォームを設定
	object->SetParent(transform_);

	// 配列に追加
	objects_.emplace_back(std::move(object));
}

void GameObject2DArray::SetColor(const SakuEngine::Color& color, int32_t index) {

	// 0以上のインデックスが指定された場合、そのインデックスのオブジェクトのみ色を変更
	if (0 <= index) {

		objects_[index]->SetColor(color);
	} else {
		for (const auto& object : objects_) {

			object->SetColor(color);
		}
	}
}

void GameObject2DArray::SetAlpha(float alpha, int32_t index) {

	// 0以上のインデックスが指定された場合、そのインデックスのオブジェクトのみアルファを変更
	if (0 <= index) {

		objects_[index]->SetAlpha(alpha);
	} else {
		for (const auto& object : objects_) {

			object->SetAlpha(alpha);
		}
	}
}

Color GameObject2DArray::GetColor(int32_t index) const {

	if (0 <= index) {

		return objects_[index]->GetColor();
	}
	return objects_.back()->GetColor();
}

void GameObject2DArray::Update() {

	// 親トランスフォーム更新
	transform_.UpdateMatrix();
}

void GameObject2DArray::ImGui() {

	transform_.ImGui(192.0f);

	ImGui::Separator();
	for (const auto& object : objects_) {
		if (ImGui::CollapsingHeader(object->GetTag().name.c_str())) {

			object->ImGui();
		}
	}
}

void GameObject2DArray::FromJson(const Json& data) {

	if (data.empty()) {
		return;
	}

	// 親トランスフォーム読み込み
	transform_.FromJson(data["transform"]);
	// 2Dオブジェクト配列読み込み
	const auto& objectsData = data["objects"];
	uint32_t index = 0;
	for (const auto& objectData : objectsData) {

		objects_[index]->ApplyJson(objectData);
		++index;
	}
}

void GameObject2DArray::ToJson(Json& data) {

	// 親トランスフォーム保存
	transform_.ToJson(data["transform"]);
	// 2Dオブジェクト配列保存
	Json objectsData = Json::array();
	for (const auto& object : objects_) {

		Json objectData;
		object->SaveJson(objectData);
		objectsData.emplace_back(objectData);
	}
	data["objects"] = objectsData;
}
