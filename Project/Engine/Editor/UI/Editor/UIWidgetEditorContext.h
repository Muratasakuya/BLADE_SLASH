#pragma once

//============================================================================
//	include
//============================================================================

// c++
#include <memory>
#include <string>
#include <optional>
#include <cstdint>

namespace SakuEngine {

	// front
	class UIWidgetDocument;
	class UIWidgetTypeRegistry;
	class UIWidgetPreviewRuntime;

	// エディタ上の選択状態
	struct UIWidgetEditorSelection {

		uint32_t selectedNodeId = 0;
		uint32_t hoveredNodeId = 0;

		// 何かが選択されているか
		bool HasSelection() const { return selectedNodeId != 0; }
		// 選択状態をクリア
		void Clear() { selectedNodeId = 0; hoveredNodeId = 0; }
	};

	//============================================================================
	//	UIWidgetEditorContext class
	//	UIウィジェットエディターの値共有コンテキスト
	//============================================================================
	class UIWidgetEditorContext {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		UIWidgetEditorContext() = default;
		~UIWidgetEditorContext() = default;

		//--------- variables ----------------------------------------------------

		// ドキュメント
		UIWidgetDocument* document = nullptr;
		// ウィジェットタイプのレジストリ
		UIWidgetTypeRegistry* registry = nullptr;
		// プレビューランタイム
		UIWidgetPreviewRuntime* preview = nullptr;

		// エディタ上の選択状態
		UIWidgetEditorSelection selection{};

		// 変更が入ったことを通知するフラグ
		bool documentDirty = false;           // 保存が必要
		bool rebuildPreviewRequested = false; // 構造変更でRebuild
		bool syncPreviewRequested = false;    // プロパティ変更で同期

		//--------- functions ----------------------------------------------------

		// ドキュメントが変更されたことをマーク
		void MarkDocumentDirty(bool requestPreviewSync);
		// プレビューの再構築をリクエスト
		void RequestPreviewRebuild();
	};
}