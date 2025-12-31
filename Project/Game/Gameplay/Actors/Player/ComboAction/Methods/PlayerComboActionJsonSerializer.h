#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Gameplay/Actors/Player/ComboAction/Methods/PlayerComboActionModel.h>
#include <Engine/Utility/Json/JsonAdapter.h> 

//============================================================================
//	PlayerComboActionJsonSerializer class
//	コンボ全体、アクションノード単体のJson保存、読込を行う
//============================================================================
class PlayerComboActionJsonSerializer {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerComboActionJsonSerializer() = default;
	~PlayerComboActionJsonSerializer() = default;

	// 全体をJson化
	static Json SerializeAll(const PlayerComboActionModel& model);
	// 全体をJsonから復元
	static bool DeserializeAll(const Json& root, PlayerComboActionModel& model);

	// アクションノード単体をJson化
	static Json SerializeActionNode(const PlayerComboActionModel::ActionNodeAsset& asset);
	// アクションノード単体をJsonから復元してmodelに追加
	static bool DeserializeActionNodeToModel(const Json& root, PlayerComboActionModel& model, uint32_t* outNewNodeId = nullptr);

	// ファイル保存/読込
	static bool SaveAllToFile(const std::string& relPath, const PlayerComboActionModel& model);
	static bool LoadAllFromFile(const std::string& relPath, PlayerComboActionModel& model);

	static bool SaveActionNodeToFile(const std::string& relPath, const PlayerComboActionModel::ActionNodeAsset& asset);
	static bool LoadActionNodeFromFileToModel(const std::string& relPath, PlayerComboActionModel& model, uint32_t* outNewNodeId = nullptr);
};