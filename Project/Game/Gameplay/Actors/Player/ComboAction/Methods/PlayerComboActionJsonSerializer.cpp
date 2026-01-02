#include "PlayerComboActionJsonSerializer.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Enum/EnumAdapter.h>
#include <Game/Gameplay/Actors/Player/ComboAction/Methods/PlayerActionNodeFactory.h>

//============================================================================
//	PlayerComboActionJsonSerializer classMethods
//============================================================================

namespace {

	template <typename T>
	T GetOr(const Json& j, const char* key, const T& def) {
		if (j.is_object() && j.contains(key)) {
			try {
				return j.at(key).get<T>();
			}
			catch (...) {
				return def;
			}
		}
		return def;
	}

	static Json SerializeStepInput(const PlayerComboActionModel::StepInputSetting& step) {

		Json data;
		data["isUseKeyboard"] = step.isUseKeyboard;
		data["isUseGamePad"] = step.isUseGamePad;
		data["isUseMouse"] = step.isUseMouse;
		data["keyDIKCode"] = EnumAdapter<KeyDIKCode>::ToString(step.keyDIKCode);
		data["mouseButton"] = EnumAdapter<MouseButton>::ToString(step.mouseButton);
		data["gamePadButton"] = EnumAdapter<GamePadButtons>::ToString(step.gamePadButton);
		return data;
	}

	static void DeserializeStepInput(const Json& data, PlayerComboActionModel::StepInputSetting& out) {

		out.isAutoAdvance = GetOr<bool>(data, "isAutoAdvance", out.isAutoAdvance);
		out.isUseKeyboard = GetOr<bool>(data, "isUseKeyboard", out.isUseKeyboard);
		out.isUseGamePad = GetOr<bool>(data, "isUseGamePad", out.isUseGamePad);
		out.isUseMouse = GetOr<bool>(data, "isUseMouse", out.isUseMouse);

		out.keyDIKCode = EnumAdapter<KeyDIKCode>::FromString(data.value("keyDIKCode", "SPACE")).value();
		out.mouseButton = EnumAdapter<MouseButton>::FromString(data.value("mouseButton", "Left")).value();
		out.gamePadButton = EnumAdapter<GamePadButtons>::FromString(data.value("gamePadButton", "A")).value();
	}

	static Json SerializeComboStep(const PlayerComboActionModel::ComboStep& step) {

		Json data;
		data["nodeAssetId"] = step.nodeAssetId;
		data["stepId"] = step.stepId;

		data["input"] = SerializeStepInput(step.input);
		data["inputGraceStartTime"] = step.inputGraceStartTime;
		data["inputGraceTime"] = step.inputGraceTime;

		data["startTime"] = step.startTime;
		data["duration"] = step.duration;
		return data;
	}

	static void DeserializeComboStep(const Json& data, PlayerComboActionModel::ComboStep& out) {

		out.nodeAssetId = GetOr<uint32_t>(data, "nodeAssetId", out.nodeAssetId);
		out.stepId = GetOr<uint32_t>(data, "stepId", out.stepId);

		if (data.contains("input")) {

			DeserializeStepInput(data.at("input"), out.input);
		}
		out.inputGraceStartTime = GetOr<float>(data, "inputGraceStartTime", out.inputGraceStartTime);
		out.inputGraceTime = GetOr<float>(data, "inputGraceTime", out.inputGraceTime);
		out.startTime = GetOr<float>(data, "startTime", out.startTime);
		out.duration = GetOr<float>(data, "duration", out.duration);
	}

	template <typename ComboT>
	void SerializeStartInputIfExists(const ComboT& combo, Json& outCombo) {

		outCombo["startInput"] = SerializeStepInput(combo.startInput);
	}

	template <typename ComboT>
	void DeserializeStartInputIfExists(ComboT& combo, const Json& inCombo) {

		if (inCombo.contains("startInput")) {

			DeserializeStepInput(inCombo.at("startInput"), combo.startInput);
		}
	}

	static Json SerializeActionNodeAsset(const PlayerComboActionModel::ActionNodeAsset& asset) {

		Json data;
		data["id"] = asset.id;
		data["name"] = asset.name;
		data["type"] = EnumAdapter<PlayerActionNodeType>::ToString(asset.type);
		data["isCancelDisabled"] = asset.isCancelDisabled;

		Json impl;
		if (asset.implementation) {

			asset.implementation->ToJson(impl);
		}
		data["implementation"] = impl;
		return data;
	}

	static PlayerComboActionModel::ActionNodeAsset DeserializeActionNodeAssetPreserveId(const Json& data) {

		PlayerComboActionModel::ActionNodeAsset asset;
		asset.id = GetOr<uint32_t>(data, "id", 0);
		asset.name = GetOr<std::string>(data, "name", std::string{});
		asset.type = EnumAdapter<PlayerActionNodeType>::FromString(data.value("type", "MoveFrontAttack")).value();
		asset.isCancelDisabled = GetOr<bool>(data, "isCancelDisabled", false);

		// 実装生成
		asset.implementation = PlayerActionNodeFactory::CreateNode(asset.type);
		if (asset.implementation) {

			asset.implementation->SetIsCancelDisabled(asset.isCancelDisabled);
			if (data.contains("implementation")) {

				asset.implementation->FromJson(data.at("implementation"));
			}
		}
		return asset;
	}
}

Json PlayerComboActionJsonSerializer::SerializeAll(const PlayerComboActionModel& model) {

	Json root;
	root["version"] = 1;

	// ActionNodes
	{
		Json array = Json::array();
		for (const auto& node : model.ActionNodes()) {

			array.push_back(SerializeActionNodeAsset(node));
		}
		root["actionNodes"] = array;
	}

	// Combos
	{
		Json array = Json::array();
		for (const auto& combo : model.Combos()) {

			Json jsonCombo;
			jsonCombo["id"] = combo.id;
			jsonCombo["name"] = combo.name;
			SerializeStartInputIfExists(combo, jsonCombo);

			Json steps = Json::array();
			for (const auto& step : combo.steps) {

				steps.push_back(SerializeComboStep(step));
			}
			jsonCombo["steps"] = steps;
			array.push_back(jsonCombo);
		}
		root["combos"] = array;
	}
	return root;
}

bool PlayerComboActionJsonSerializer::DeserializeAll(const Json& root, PlayerComboActionModel& model) {

	if (!root.is_object()) {
		return false;
	}

	std::vector<PlayerComboActionModel::ActionNodeAsset> nodes;
	std::vector<PlayerComboActionModel::ComboAction> combos;

	// ActionNodes
	if (root.contains("actionNodes") && root.at("actionNodes").is_array()) {
		for (const auto& jsonNode : root.at("actionNodes")) {

			nodes.emplace_back(DeserializeActionNodeAssetPreserveId(jsonNode));
		}
	}

	// Combos
	if (root.contains("combos") && root.at("combos").is_array()) {
		for (const auto& jsonCombo : root.at("combos")) {

			PlayerComboActionModel::ComboAction combo;
			combo.id = GetOr<uint32_t>(jsonCombo, "id", 0u);
			combo.name = GetOr<std::string>(jsonCombo, "name", std::string{});
			DeserializeStartInputIfExists(combo, jsonCombo);
			if (jsonCombo.contains("steps") && jsonCombo.at("steps").is_array()) {
				for (const auto& jsonStep : jsonCombo.at("steps")) {

					PlayerComboActionModel::ComboStep step;
					DeserializeComboStep(jsonStep, step);
					combo.steps.emplace_back(step);
				}
			}
			combos.emplace_back(std::move(combo));
		}
	}

	// 最後にモデルにセット
	model.ReplaceAll(std::move(nodes), std::move(combos));
	return true;
}

Json PlayerComboActionJsonSerializer::SerializeActionNode(const PlayerComboActionModel::ActionNodeAsset& asset) {

	Json root;
	root["version"] = 1;
	root["actionNode"] = SerializeActionNodeAsset(asset);
	return root;
}

bool PlayerComboActionJsonSerializer::DeserializeActionNodeToModel(
	const Json& root, PlayerComboActionModel& model, uint32_t* outNewNodeId) {

	if (!root.is_object() || !root.contains("actionNode")) {
		return false;
	}

	const Json& jsonNode = root.at("actionNode");
	PlayerActionNodeType type = EnumAdapter<PlayerActionNodeType>::FromString(
		jsonNode.value("type", "MoveFrontAttack")).value();
	std::string name = GetOr<std::string>(jsonNode, "name", std::string{});
	bool isCancelDisabled = GetOr<bool>(jsonNode, "isCancelDisabled", false);

	// 新規作成
	uint32_t newId = model.AddActionNode(type, name);

	auto* asset = model.FindNodeAssetById(newId);
	if (!asset) {
		return false;
	}
	asset->isCancelDisabled = isCancelDisabled;
	if (asset->implementation) {

		asset->implementation->SetIsCancelDisabled(isCancelDisabled);
		if (jsonNode.contains("implementation")) {

			asset->implementation->FromJson(jsonNode.at("implementation"));
		}
	}

	if (outNewNodeId) {
		*outNewNodeId = newId;
	}

	return true;
}

bool PlayerComboActionJsonSerializer::SaveAllToFile(const std::string& relPath, const PlayerComboActionModel& model) {

	Json data = SerializeAll(model);
	SakuEngine::JsonAdapter::Save(relPath, data);
	return true;
}

bool PlayerComboActionJsonSerializer::LoadAllFromFile(const std::string& relPath, PlayerComboActionModel& model) {

	Json data = SakuEngine::JsonAdapter::Load(relPath);
	if (data.empty()) {
		return false;
	}
	return DeserializeAll(data, model);
}

bool PlayerComboActionJsonSerializer::SaveActionNodeToFile(const std::string& relPath,
	const PlayerComboActionModel::ActionNodeAsset& asset) {

	Json data = SerializeActionNode(asset);
	SakuEngine::JsonAdapter::Save(relPath, data);
	return true;
}

bool PlayerComboActionJsonSerializer::LoadActionNodeFromFileToModel(const std::string& relPath,
	PlayerComboActionModel& model, uint32_t* outNewNodeId) {

	Json data = SakuEngine::JsonAdapter::Load(relPath);
	if (data.empty()) {
		return false;
	}
	return DeserializeActionNodeToModel(data, model, outNewNodeId);
}