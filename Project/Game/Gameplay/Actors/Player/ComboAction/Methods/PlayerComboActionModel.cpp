#include "PlayerComboActionModel.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Input/Input.h>
#include <Game/Gameplay/Actors/Player/ComboAction/Methods/PlayerActionNodeFactory.h>

//============================================================================
//	PlayerComboActionModel classMethods
//============================================================================

PlayerComboActionModel::ComboStep PlayerComboActionModel::CreateDefaultStep(uint32_t nodeAssetId) {

	ComboStep step;
	// ID割り当て
	step.stepId = AllocateStepId();
	step.nodeAssetId = nodeAssetId;

	// 時間の設定
	step.startTime = 0.0f;
	step.duration = 0.3f;
	if (auto* asset = FindNodeAssetById(nodeAssetId)) {
		// ノードの合計時間を取得して設定
		if (asset->implementation) {
			step.duration = (std::max)(0.0f, asset->implementation->GetTotalTime());
		}
	}
	// 入力設定
	step.inputGraceStartTime = step.startTime + step.duration;
	step.inputGraceTime = 0.2f;
	step.input.isUseKeyboard = true;
	step.input.isUseGamePad = true;
	step.input.keyDIKCode = KeyDIKCode::SPACE;
	step.input.gamePadButton = GamePadButtons::A;
	return step;
}

uint32_t PlayerComboActionModel::AddActionNode(PlayerActionNodeType type, const std::string& nameOverride) {

	// アセット作成
	ActionNodeAsset asset;
	asset.id = AllocateId();
	asset.type = type;
	asset.name = nameOverride.empty() ? SakuEngine::EnumAdapter<PlayerActionNodeType>::ToString(type) : nameOverride;
	asset.implementation = PlayerActionNodeFactory::CreateNode(type);
	// 必要なポインタを設定
	asset.implementation->SetPlayer(player_);
	asset.implementation->SetAttackTarget(attackTarget_);
	asset.implementation->SetAreaChecker(areaChecker_);

	// 初期値
	asset.isCancelDisabled = false;
	if (asset.implementation) {

		asset.implementation->SetIsCancelDisabled(asset.isCancelDisabled);
	}

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

		auto& steps = combo.steps;
		steps.erase(std::remove_if(steps.begin(), steps.end(),
			[&](const ComboStep& step) { return step.nodeAssetId == removedId; }), steps.end());
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
	uint32_t newId = AddActionNode(source.type, source.name);

	// 複製したノードにキャンセル不可フラグを反映
	auto* newAsset = FindNodeAssetById(newId);
	if (newAsset) {
		newAsset->isCancelDisabled = source.isCancelDisabled;
		if (newAsset->implementation) {

			newAsset->implementation->SetIsCancelDisabled(newAsset->isCancelDisabled);
		}
	}
	return true;
}

uint32_t PlayerComboActionModel::CreateCombo(const std::string& nameOverride) {


	// コンボ作成
	ComboAction combo;
	combo.id = AllocateId();
	combo.name = nameOverride.empty() ? "Action" : nameOverride;
	combo.steps.clear();

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

	// 末尾に追加
	auto& combo = combos_[comboIndex];
	ComboStep newStep = CreateDefaultStep(nodeAssetId);

	// 末尾に繋がる開始時間
	float lastEnd = 0.0f;
	for (const auto& step : combo.steps) {
		// 各ステップの終了時間を計算
		float end = step.startTime + step.duration;
		if (lastEnd < end) {

			lastEnd = end;
		}
	}
	newStep.startTime = lastEnd;
	newStep.inputGraceStartTime = newStep.startTime + newStep.duration;
	combo.steps.emplace_back(newStep);
	return true;
}

bool PlayerComboActionModel::InsertNodeToCombo(size_t comboIndex, size_t insertPos, uint32_t nodeAssetId) {

	// 範囲外参照チェック
	if (combos_.size() <= comboIndex) {
		return false;
	}

	auto& steps = combos_[comboIndex].steps;

	// 範囲外参照チェック
	if (steps.size() < insertPos) {
		insertPos = steps.size();
	}

	// 指定位置に挿入
	ComboStep step = CreateDefaultStep(nodeAssetId);
	steps.insert(steps.begin() + static_cast<std::ptrdiff_t>(insertPos), step);
	return true;
}

bool PlayerComboActionModel::RemoveComboStep(size_t comboIndex, size_t stepIndex) {

	// 範囲外参照チェック
	if (combos_.size() <= comboIndex) {
		return false;
	}

	auto& steps = combos_[comboIndex].steps;
	// 範囲外参照チェック
	if (steps.size() <= stepIndex) {
		return false;
	}
	steps.erase(steps.begin() + static_cast<std::ptrdiff_t>(stepIndex));
	return true;
}

bool PlayerComboActionModel::SwapComboSteps(size_t comboIndex, size_t nodeAIndex, size_t nodeBIndex) {

	// 範囲外参照チェック
	if (combos_.size() <= comboIndex) {
		return false;
	}

	auto& steps = combos_[comboIndex].steps;
	// 範囲外参照チェック
	if (steps.size() <= nodeAIndex || steps.size() <= nodeBIndex) {
		return false;
	}
	// 同じインデックスは処理しない
	if (nodeAIndex == nodeBIndex) {
		return true;
	}
	std::swap(steps[nodeAIndex], steps[nodeBIndex]);
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

	// 複製作成して追加
	ComboAction copy;
	copy.id = AllocateId();
	copy.name = source.name + "_Copy";
	copy.steps = source.steps;

	// stepIdは重複しないように振り直す
	for (auto& step : copy.steps) {

		step.stepId = AllocateStepId();
	}

	combos_.emplace_back(std::move(copy));
	return true;
}

bool PlayerComboActionModel::ClearComboSteps(size_t comboIndex) {

	// 範囲外参照チェック
	if (combos_.size() <= comboIndex) {
		return false;
	}
	// ステップ全消し
	combos_[comboIndex].steps.clear();
	return true;
}

bool PlayerComboActionModel::AddStepAtTime(size_t comboIndex, uint32_t nodeAssetId, float startTime, uint32_t* outStepId) {

	// 範囲外参照チェック
	if (combos_.size() <= comboIndex) {
		return false;
	}

	auto& combo = combos_[comboIndex];

	// 指定時間位置に追加
	ComboStep step = CreateDefaultStep(nodeAssetId);
	step.startTime = (std::max)(0.0f, startTime);
	// デフォルトは、ノード終了から入力猶予開始
	step.inputGraceStartTime = step.startTime + step.duration;
	if (outStepId) {
		*outStepId = step.stepId;
	}
	combo.steps.emplace_back(step);
	return true;
}

void PlayerComboActionModel::SortStepsByStartTime(size_t comboIndex) {

	// 範囲外参照チェック
	if (combos_.size() <= comboIndex) {
		return;
	}

	// 開始時間が早い順にソート、同じ場合はstepIdでソートする
	auto& steps = combos_[comboIndex].steps;
	std::sort(steps.begin(), steps.end(),
		[](const ComboStep& a, const ComboStep& b) {
			if (a.startTime == b.startTime) {
				return a.stepId < b.stepId;
			}
			return a.startTime < b.startTime;
		});
}

float PlayerComboActionModel::CalculateTotalTime(size_t comboIndex) const {

	if (combos_.size() <= comboIndex) {
		return 0.0f;
	}

	const auto& steps = combos_[comboIndex].steps;
	float total = 0.0f;
	for (const auto& step : steps) {

		// ノードの合計時間を取得
		float duration = (std::max)(0.0f, step.duration);
		if (const auto* asset = FindNodeAssetById(step.nodeAssetId)) {
			// 実装がある場合は実装の合計時間を優先
			if (asset->implementation) {

				duration = (std::max)(0.0f, asset->implementation->GetTotalTime());
			}
		}

		float nodeEnd = step.startTime + duration;
		float graceEnd = step.inputGraceStartTime + (std::max)(0.0f, step.inputGraceTime);
		float end = (std::max)(nodeEnd, graceEnd);
		// 合計時間を更新
		if (total < end) {
			total = end;
		}
	}
	return total;
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

const PlayerComboActionModel::ActionNodeAsset* PlayerComboActionModel::FindNodeAssetById(uint32_t id) const {

	// 一致するIDを探して返す
	for (const auto& node : actionNodes_) {
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

int32_t PlayerComboActionModel::FindStepIndexById(size_t comboIndex, uint32_t stepId) const {

	// 範囲外参照チェック
	if (combos_.size() <= comboIndex) {
		return -1;
	}

	const auto& steps = combos_[comboIndex].steps;
	for (int32_t i = 0; i < static_cast<int32_t>(steps.size()); ++i) {
		if (steps[static_cast<size_t>(i)].stepId == stepId) {

			return i;
		}
	}
	return -1;
}