#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/Base/IGameEditor.h>
#include <Engine/Editor/UI/AssetData/UIAssetLibrary.h>
#include <Engine/Editor/UI/Tools/Interface/IUIToolPanel.h>
#include <Engine/Editor/UI/Tools/Registry/UIPaletteRegistry.h>
#include <Engine/Editor/UI/Runtime/UIRuntime.h>

// directX
#include <d3d12.h>
// c++
#include <memory>

namespace SakuEngine {

	//============================================================================
	//	UIEditor class
	//	UIを作成して編集するエディター
	//============================================================================
	class UIEditor :
		public IGameEditor {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		// 初期化
		void Init(Asset* asset, const D3D12_GPU_DESCRIPTOR_HANDLE& handle);

		// 更新処理
		void Update();

		// エディター
		void ImGui() override;
		void EditPanels();

		//--------- accessor -----------------------------------------------------

		// singleton
		static UIEditor* GetInstance();
		static void Finalize();
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		static UIEditor* instance_;

		// UI要素のIDを管理するライブラリ
		std::unique_ptr<UIAssetLibrary> assetLibrary_;
		// UIパレットの登録を行うレジストリ
		std::unique_ptr<UIPaletteRegistry> paletteRegistry_;
		// UIアセットの更新を行うランタイム
		std::unique_ptr<UIRuntime> runtime_;

		// ツールコンテキスト
		std::unique_ptr<UIToolContext> toolContext_;
		// パネル群
		std::vector<std::unique_ptr<IUIToolPanel>> panels_;

		// エディター
		bool isOpen_ = false; // エディターが開いているか

		//--------- functions ----------------------------------------------------

		UIEditor() :IGameEditor("UIEditor") {}
		~UIEditor() = default;
		UIEditor(const UIEditor&) = delete;
		UIEditor& operator=(const UIEditor&) = delete;
	};
} // SakuEngine