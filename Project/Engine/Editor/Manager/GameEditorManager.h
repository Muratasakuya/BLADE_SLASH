#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/Base/IGameEditor.h>

// c++
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <ranges>

//============================================================================
//	GameEditorManager class
//	IGameEditorを継承したエディターの管理を行う
//============================================================================
namespace SakuEngine {

	class GameEditorManager {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		GameEditorManager() = default;
		~GameEditorManager() = default;

		// editorの登録、削除
		void AddEditor(IGameEditor* editor);
		void RemoveEditor(IGameEditor* editor);

		// editorの選択
		void SelectEditor();
		// 選択したeditorの選択
		void EditEditor();

		//--------- accessor -----------------------------------------------------

		// 外部からフォーカスするIDを設定
		void SetSelectObjectID(uint32_t id);

		// singleton
		static GameEditorManager* GetInstance();
		static void Finalize();
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- structure ----------------------------------------------------

		// 選択中の情報
		struct SelectInfo {

			IGameEditor* editor;

			std::string groupName;
			std::optional<uint32_t> index;

			// 選択中のエディターがあるか
			bool Has() { return index.has_value(); }
		};

		//--------- variables ----------------------------------------------------

		static GameEditorManager* instance_;

		// 登録されているエディター
		std::unordered_map<std::string, std::vector<IGameEditor*>> editorMap_;

		// 選択されているエディター情報
		SelectInfo selectInfo_;
	};
}; // SakuEngine