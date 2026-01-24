#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/Base/IGameEditor.h>
#include <Engine/Editor/UI/Runtime/UIDataContext.h>
#include <Engine/Editor/UI/Editor/UIWidgetEditorContext.h>
#include <Engine/Editor/UI/Editor/Document/UIWidgetDocument.h>
#include <Engine/Editor/UI/Editor/Document/UIWidgetTypeRegistry.h>
#include <Engine/Editor/UI/Editor/Preview/UIWidgetPreviewRuntime.h>
#include <Engine/Editor/UI/Editor/Panels/Interface/UIWidgetEditorPanel.h>

// c++
#include <memory>
// directX
#include <d3d12.h>

namespace SakuEngine {

	//============================================================================
	//	UIWidgetEditor class
	//	ゲーム内UIウィジェットを作成するエディター
	//============================================================================
	class UIWidgetEditor :
		public IGameEditor {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		// 初期化
		void Init(const D3D12_GPU_DESCRIPTOR_HANDLE& renderTextureGPUHandle);

		// 更新
		void Update();

		// エディター
		void ImGui() override;
		void EditUIWidget();

		// ドキュメント操作
		bool CreateDocument(const std::string& key);    // 新規追加してアクティブにする
		bool DeleteDocument(const std::string& key);    // 削除
		bool SetActiveDocument(const std::string& key); // アクティブ切替

		//--------- accessor -----------------------------------------------------

		// アクティブドキュメント取得
		UIWidgetDocument* GetActiveDocument() const;
		const std::string& GetActiveKey() const { return activeKey_; }
		const std::unordered_map<std::string, std::unique_ptr<UIWidgetDocument>>& GetDocuments() const { return documents_; }

		// singleton
		static UIWidgetEditor* GetInstance();
		static void Finalize();
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		static UIWidgetEditor* instance_;

		// エディターコンテキスト
		UIWidgetEditorContext context_{};

		// ドキュメント
		std::unordered_map<std::string, std::unique_ptr<UIWidgetDocument>> documents_;
		// 現在操作中のドキュメントのキー
		std::string activeKey_;

		// ウィジェット型登録
		std::unique_ptr<UIWidgetTypeRegistry> registry_;
		// プレビューランタイム
		std::unique_ptr<UIWidgetPreviewRuntime> preview_;

		// エディタ機能
		std::vector<std::unique_ptr<IUIWidgetEditorPanel>> panels_;

		// エディターを開くかどうか
		bool isOpenEditor_ = false;

		//--------- functions ----------------------------------------------------

		void BindActiveDocumentToContext();
		void EnsureAtLeastOneDocument();

		UIWidgetEditor() :IGameEditor("UIWidgetEditor") {}
		~UIWidgetEditor() = default;
		UIWidgetEditor(const UIWidgetEditor&) = delete;
		UIWidgetEditor& operator=(const UIWidgetEditor&) = delete;
	};
} // SakuEngine