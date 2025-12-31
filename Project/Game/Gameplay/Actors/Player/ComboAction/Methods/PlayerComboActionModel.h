#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Input/InputStructures.h>
#include <Game/Gameplay/Actors/Player/ComboAction/Action/Interface/PlayerIActionNode.h>

//============================================================================
// PlayerComboActionEditorSelection
//============================================================================

// エディター選択情報
struct PlayerComboActionEditorSelection {

	int32_t selectedNodeIndex = -1;  // アクションノードリスト
	int32_t selectedComboIndex = -1; // コンボアクションリスト
	int32_t selectedStepIndex = -1;  // コンボアクション内ステップリスト

	// タイムライン操作中の追跡用ステップID
	uint32_t selectedStepId = 0;
};

//============================================================================
//	PlayerComboActionModel class
//============================================================================
class PlayerComboActionModel {
public:
	//========================================================================
	//	public Methods
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

		// キャンセル不可フラグ
		bool isCancelDisabled = false;
	};
	// ステップ入力設定
	struct StepInputSetting {

		// キーボード入力を使うか
		bool isUseKeyboard = true;
		// マウス入力を使うか
		bool isUseMouse = true;
		// ゲームパッド入力を使うか
		bool isUseGamePad = true;

		// DIKコード
		KeyDIKCode keyDIKCode = KeyDIKCode::SPACE;
		// Mouse
		MouseButton mouseButton = MouseButton::Left;
		// GamePadButtons
		GamePadButtons gamePadButton = GamePadButtons::A;
	};

	// コンボ内のステップ
	struct ComboStep {

		// アクションノードID
		uint32_t nodeAssetId = 0;
		// ステップ識別子
		uint32_t stepId = 0;

		// 入力
		StepInputSetting input;
		// 入力猶予開始時間
		float inputGraceStartTime = 0.0f;
		// 入力猶予
		float inputGraceTime = 0.4f;

		// タイムライン
		float startTime = 0.0f; // 開始時間
		float duration = 0.4f;  // 処理時間
	};

	// 1アクション
	struct ComboAction {

		// 識別子
		uint32_t id = 0;
		std::string name;

		// コンボ開始入力設定
		StepInputSetting startInput;

		// ステップリスト
		std::vector<ComboStep> steps;
	};
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PlayerComboActionModel() = default;
	~PlayerComboActionModel() = default;

	// IDを割り当てて次に進める
	uint32_t AllocateId() { return ++nextId_; }
	uint32_t AllocateStepId() { return ++nextStepId_; }

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
	const ActionNodeAsset* FindNodeAssetById(uint32_t id) const;

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
	// コンボ複製
	bool DuplicateCombo(size_t comboIndex);
	// ステップ全消し
	bool ClearComboSteps(size_t comboIndex);
	// ステップを指定時間に追加
	bool AddStepAtTime(size_t comboIndex, uint32_t nodeAssetId, float startTime, uint32_t* outStepId);

	// 開始時間でソート
	void SortStepsByStartTime(size_t comboIndex);
	// コンボの総時間を計算
	float CalculateTotalTime(size_t comboIndex) const;

	// IDからコンボアクションを探す
	ComboAction* FindComboById(uint32_t id);
	// ステップIDからステップインデックスを探す
	int32_t FindStepIndexById(size_t comboIndex, uint32_t stepId) const;

	//========================================================================
	//	Json Methods
	//========================================================================

	// 全クリア
	void ClearAll();
	// 全置換
	void ReplaceAll(std::vector<ActionNodeAsset>&& nodes, std::vector<ComboAction>&& combos);
	// 次のIDを再計算
	void RecalculateNextIds();

	//--------- accessor -----------------------------------------------------

	// 処理対象のプレイヤーを設定
	void SetPlayer(Player* player) { player_ = player; }
	// 攻撃対象を設定
	void SetAttackTarget(const SakuEngine::GameObject3D* target) { attackTarget_ = target; }
	// リアクションエリアチェッカーを設定
	void SetAreaChecker(const PlayerReactionAreaChecker* checker) { areaChecker_ = checker; }

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

	// 処理対象
	Player* player_;
	// 攻撃対象
	const SakuEngine::GameObject3D* attackTarget_;
	// リアクションエリアチェッカー
	const PlayerReactionAreaChecker* areaChecker_;

	// 次のID
	uint32_t nextId_ = 1;
	// 次のStepID
	uint32_t nextStepId_ = 1;

	// アクションノードリスト
	std::vector<ActionNodeAsset> actionNodes_;
	// コンボアクションリスト
	std::vector<ComboAction> combos_;

	//--------- functions ----------------------------------------------------

	// ステップのデフォルト値
	ComboStep CreateDefaultStep(uint32_t nodeAssetId);
};