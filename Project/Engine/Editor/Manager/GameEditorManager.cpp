#include "GameEditorManager.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================

//============================================================================
//	GameEditorManager classMethods
//============================================================================

GameEditorManager* GameEditorManager::instance_ = nullptr;

GameEditorManager* GameEditorManager::GetInstance() {

	if (instance_ == nullptr) {
		instance_ = new GameEditorManager();
	}
	return instance_;
}

void GameEditorManager::Finalize() {

	if (instance_ != nullptr) {

		delete instance_;
		instance_ = nullptr;
	}
}

void GameEditorManager::AddEditor(IGameEditor* editor) {

	// 同じ名前のエディターが無ければ追加
	std::string name = editor->GetName();

	for (const auto& editors : std::views::values(editorMap_)) {
		for (const auto& registered : editors) {
			// 同じ名前がないかチェック
			if (registered->GetName() == name) {

				// あったら早期リターンして追加させない
				return;
			}
		}
	}
	// 追加
	std::string groupName = editor->GetGroupName();
	editorMap_[groupName].emplace_back(editor);
}

void GameEditorManager::RemoveEditor(IGameEditor* editor) {

	// 選択中のエディターを削除したとき
	if (selectInfo_.editor) {
		if (selectInfo_.editor->GetName() == editor->GetName()) {

			selectInfo_.editor = nullptr;
			selectInfo_.groupName.clear();
			selectInfo_.index = std::nullopt;
		}
	}

	// エディター削除
	for (auto& [groupName, editors] : editorMap_) {
		if (groupName == editor->GetGroupName()) {

			editors.erase(std::remove(editors.begin(), editors.end(), editor), editors.end());
		}
	}
}

void GameEditorManager::SetSelectObjectID(uint32_t id) {

	// 選択されていなければ早期リターン
	if (!selectInfo_.Has()) {
		return;
	}
	selectInfo_.editor->SetSelectObjectID(id);
}

void GameEditorManager::SelectEditor() {

	ImGui::SetWindowFontScale(0.84f);

	// グループ別に表示を行う
	for (const auto& [groupName, editors] : editorMap_) {

		// グループ表示ヘッダー
		if (ImGui::CollapsingHeader(groupName.c_str())) {
			if (editors.empty()) {
				continue;
			}

			ImGui::Indent();

			// 所持エディターを表示
			for (uint32_t index = 0; index < editors.size(); ++index) {
				if (ImGui::Selectable(editors[index]->GetName().c_str(),
					selectInfo_.Has() && selectInfo_.index.value() == index)) {

					selectInfo_.index = index;
					selectInfo_.editor = editors[index];
				}
			}
			ImGui::Unindent();
		}
	}

	ImGui::SetWindowFontScale(1.0f);
}

void GameEditorManager::EditEditor() {

	// 選択されていなければ早期リターン
	if (!selectInfo_.Has()) {
		return;
	}

	// 選択中のグループ内で範囲外アクセスになるなら選択取り消し
	if (editorMap_[selectInfo_.groupName].size() <= static_cast<uint32_t>(selectInfo_.index.value())) {
		selectInfo_.index = std::nullopt;
		selectInfo_.editor = nullptr;
		return;
	}

	// 名前表示
	ImGui::Text("name: %s", selectInfo_.editor->GetName().c_str());

	ImGui::Separator();

	selectInfo_.editor->ImGui();
}