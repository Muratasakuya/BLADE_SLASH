#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Gameplay/Actors/Player/ComboAction/Action/Interface/PlayerIActionNode.h>

//============================================================================
// PlayerComboActionEditorSelection
//============================================================================

// エディター選択情報
struct PlayerComboActionEditorSelection {

	int32_t selectedNodeIndex = -1;  // アクションノードリスト
	int32_t selectedComboIndex = -1; // コンボアクションリスト
	int32_t selectedStepIndex = -1;  // コンボアクション内ステップリスト
};

//============================================================================
//	PlayerComboActionModel class
//============================================================================
class PlayerComboActionModel {
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// アクションノード
	struct ActionNodeAsset {

		// 識別子
		uint32_t id = 0;
		std::string name;
		// ノードタイプ
		PlayerActionNodeType type;
		// ノード実装
		std::unique_ptr<PlayerIActionNode> implementation;
	};

	// 1アクション
	struct ComboAction {

		// 識別子
		uint32_t id = 0;
		std::string name;
		// アクションノードIDリスト
		std::vector<uint32_t> nodeAssetIds;
	};
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerComboActionModel() = default;
	~PlayerComboActionModel() = default;

	// IDを割り当てて次に進める
	uint32_t AllocateId() { return ++nextId_; }

	//========================================================================
	//	ActionNode Methods
	//========================================================================

	// アクションノード追加
	uint32_t AddActionNode(PlayerActionNodeType type, const std::string& nameOverride = std::string());
	// アクションノード削除
	bool RemoveActionNode(size_t index);
	// アクションノード入れ替え
	bool SwapActionNodes(size_t nodeAIndex, size_t nodeBIndex);
	// アクションノード複製
	bool DuplicateActionNode(size_t index);

	// IDからアクションノードを探す
	ActionNodeAsset* FindNodeAssetById(uint32_t id);

	//========================================================================
	//	ComboAction Methods
	//========================================================================

	// コンボアクション作成
	uint32_t CreateCombo(const std::string& nameOverride = std::string());
	// コンボアクション削除
	bool RemoveCombo(size_t comboIndex);
	// 選択ノードを末尾に追加
	bool AppendNodeToCombo(size_t comboIndex, uint32_t nodeAssetId);
	// 指定位置に挿入
	bool InsertNodeToCombo(size_t comboIndex, size_t insertPos, uint32_t nodeAssetId);
	// ステップ削除
	bool RemoveComboStep(size_t comboIndex, size_t stepIndex);
	// ステップ入替
	bool SwapComboSteps(size_t comboIndex, size_t nodeAIndex, size_t nodeBIndex);
	bool SwapCombos(size_t comboAIndex, size_t comboBIndex);
	bool DuplicateCombo(size_t comboIndex);
	// ステップ全消し
	bool ClearComboSteps(size_t comboIndex);

	// IDからコンボアクションを探す
	ComboAction* FindComboById(uint32_t id);

	//--------- accessor -----------------------------------------------------

	// アクションノードリスト取得
	std::vector<ActionNodeAsset>& ActionNodes() { return actionNodes_; }
	const std::vector<ActionNodeAsset>& ActionNodes() const { return actionNodes_; }
	// コンボアクションリスト取得
	std::vector<ComboAction>& Combos() { return combos_; }
	const std::vector<ComboAction>& Combos() const { return combos_; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// 次のID
	uint32_t nextId_ = 1;
	// アクションノードリスト
	std::vector<ActionNodeAsset> actionNodes_;
	// コンボアクションリスト
	std::vector<ComboAction> combos_;
};