#include "PlayerComboActionModel.h"

//============================================================================
//	include
//============================================================================
#include <Game/Gameplay/Actors/Player/ComboAction/Methods/PlayerActionNodeFactory.h>

//============================================================================
//	PlayerComboActionModel classMethods
//============================================================================

uint32_t PlayerComboActionModel::AddActionNode(PlayerActionNodeType type, const std::string& nameOverride) {

	// アセット作成
	ActionNodeAsset asset;
	asset.id = AllocateId();
	asset.type = type;
	asset.name = nameOverride.empty() ? SakuEngine::EnumAdapter<PlayerActionNodeType>::ToString(type) : nameOverride;
	asset.implementation = PlayerActionNodeFactory::CreateNode(type);

	// リストに追加
	actionNodes_.emplace_back(std::move(asset));
	return actionNodes_.back().id;
}

bool PlayerComboActionModel::RemoveActionNode(size_t index) {

	// 範囲外参照チェック
	if (actionNodes_.size() <= index) {
		return false;
	}

	// ノード削除
	uint32_t removedId = actionNodes_[index].id;
	actionNodes_.erase(actionNodes_.begin() + static_cast<std::ptrdiff_t>(index));

	// コンボ内の参照除去
	for (auto& combo : combos_) {

		auto& nodes = combo.nodeAssetIds;
		nodes.erase(std::remove(nodes.begin(), nodes.end(), removedId), nodes.end());
	}
	return true;
}

bool PlayerComboActionModel::SwapActionNodes(size_t nodeAIndex, size_t nodeBIndex) {

	// 範囲外参照チェック
	if (actionNodes_.size() <= nodeAIndex || actionNodes_.size() <= nodeBIndex) {
		return false;
	}
	// 同じインデックスは処理しない
	if (nodeAIndex == nodeBIndex) {
		return true;
	}
	std::swap(actionNodes_[nodeAIndex], actionNodes_[nodeBIndex]);
	return true;
}

bool PlayerComboActionModel::DuplicateActionNode(size_t index) {

	// 範囲外参照チェック
	if (actionNodes_.size() <= index) {
		return false;
	}
	// 複製元取得
	const auto& source = actionNodes_[index];
	// 複製作成して追加
	AddActionNode(source.type, source.name);
	return true;
}

uint32_t PlayerComboActionModel::CreateCombo(const std::string& nameOverride) {

	// コンボ作成
	ComboAction combo;
	combo.id = AllocateId();
	combo.name = nameOverride.empty() ? "Action" : nameOverride;
	combo.nodeAssetIds.clear();

	// リストに追加
	combos_.emplace_back(std::move(combo));
	return combos_.back().id;
}

bool PlayerComboActionModel::RemoveCombo(size_t comboIndex) {

	// 範囲外参照チェック
	if (combos_.size() <= comboIndex) {
		return false;
	}
	combos_.erase(combos_.begin() + static_cast<std::ptrdiff_t>(comboIndex));
	return true;
}

bool PlayerComboActionModel::AppendNodeToCombo(size_t comboIndex, uint32_t nodeAssetId) {

	// 範囲外参照チェック
	if (combos_.size() <= comboIndex) {
		return false;
	}

	// ノード追加
	combos_[comboIndex].nodeAssetIds.emplace_back(nodeAssetId);
	return true;
}

bool PlayerComboActionModel::InsertNodeToCombo(size_t comboIndex, size_t insertPos, uint32_t nodeAssetId) {

	// 範囲外参照チェック
	if (combos_.size() <= comboIndex) {
		return false;
	}

	auto& nodes = combos_[comboIndex].nodeAssetIds;
	// 範囲外参照チェック
	if (nodes.size() < insertPos) {
		// 末尾に追加
		insertPos = nodes.size();
	}
	nodes.insert(nodes.begin() + static_cast<std::ptrdiff_t>(insertPos), nodeAssetId);
	return true;
}

bool PlayerComboActionModel::RemoveComboStep(size_t comboIndex, size_t stepIndex) {

	// 範囲外参照チェック
	if (combos_.size() <= comboIndex) {
		return false;
	}

	auto& nodes = combos_[comboIndex].nodeAssetIds;
	// 範囲外参照チェック
	if (nodes.size() < stepIndex) {
		return false;
	}
	nodes.erase(nodes.begin() + static_cast<std::ptrdiff_t>(stepIndex));
	return true;
}

bool PlayerComboActionModel::SwapComboSteps(size_t comboIndex, size_t nodeAIndex, size_t nodeBIndex) {

	// 範囲外参照チェック
	if (combos_.size() <= comboIndex) {
		return false;
	}

	auto& nodes = combos_[comboIndex].nodeAssetIds;
	// 範囲外参照チェック
	if (nodes.size() <= nodeAIndex || nodes.size() <= nodeBIndex) {
		return false;
	}
	// 同じインデックスは処理しない
	if (nodeAIndex == nodeBIndex) {
		return true;
	}
	std::swap(nodes[nodeAIndex], nodes[nodeBIndex]);
	return true;
}

bool PlayerComboActionModel::SwapCombos(size_t comboAIndex, size_t comboBIndex) {

	// 範囲外参照チェック
	if (combos_.size() <= comboAIndex || combos_.size() <= comboBIndex) {
		return false;
	}
	// 同じインデックスは処理しない
	if (comboAIndex == comboBIndex) {
		return true;
	}
	std::swap(combos_[comboAIndex], combos_[comboBIndex]);
	return true;
}

bool PlayerComboActionModel::DuplicateCombo(size_t comboIndex) {

	// 範囲外参照チェック
	if (combos_.size() <= comboIndex) {
		return false;
	}
	const auto& source = combos_[comboIndex];

	ComboAction combo{};
	combo.id = AllocateId();
	combo.name = source.name + "_Copy";
	combo.nodeAssetIds = source.nodeAssetIds;
	combos_.emplace_back(std::move(combo));
	return true;
}

bool PlayerComboActionModel::ClearComboSteps(size_t comboIndex) {

	// 範囲外参照チェック
	if (combos_.size() <= comboIndex) {
		return false;
	}
	// ステップ全消し
	combos_[comboIndex].nodeAssetIds.clear();
	return true;
}

PlayerComboActionModel::ActionNodeAsset* PlayerComboActionModel::FindNodeAssetById(uint32_t id) {

	// 一致するIDを探して返す
	for (auto& node : actionNodes_) {
		if (node.id == id) {
			return &node;
		}
	}
	return nullptr;
}

PlayerComboActionModel::ComboAction* PlayerComboActionModel::FindComboById(uint32_t id) {

	// 一致するIDを探して返す
	for (auto& combo : combos_) {
		if (combo.id == id) {
			return &combo;
		}
	}
	return nullptr;
}