#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/Base/IGameEditor.h>

// 機能クラス
#include <Engine/Editor/UI/Editor/VisualDesigner/UIWidgetVisualDesigner.h>
#include <Engine/Editor/UI/Editor/Detail/UIWidgetDetail.h>
#include <Engine/Editor/UI/Editor/Hierarchy/UIWidgetHierarchy.h>
#include <Engine/Editor/UI/Editor/Animation/UIWidgetAnimation.h>
#include <Engine/Editor/UI/Editor/Palette/UIWidgetPalette.h>

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

		// エディター
		void ImGui() override;
		void EditUIWidget();

		//--------- accessor -----------------------------------------------------

		// singleton
		static UIWidgetEditor* GetInstance();
		static void Finalize();
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		static UIWidgetEditor* instance_;

		// エディタ機能
		// ウィジェットのパレットを表示、提供する
		std::unique_ptr<UIWidgetPalette> palette_;
		// ウィジェットの詳細を確認、編集する
		std::unique_ptr<UIWidgetDetail> detail_;
		// 作成したウィジェットの見た目を確認、編集する
		std::unique_ptr<UIWidgetVisualDesigner> visualDesigner_;
		// ウィジェットのヒエラルキーを確認、編集する
		std::unique_ptr<UIWidgetHierarchy> hierarchy_;
		// ウィジェットのアニメーションを作成、編集する
		std::unique_ptr<UIWidgetAnimation> animation_;

		// エディターを開くかどうか
		bool isOpenEditor_ = false;

		//--------- functions ----------------------------------------------------

		UIWidgetEditor() :IGameEditor("UIWidgetEditor") {}
		~UIWidgetEditor() = default;
		UIWidgetEditor(const UIWidgetEditor&) = delete;
		UIWidgetEditor& operator=(const UIWidgetEditor&) = delete;
	};
} // SakuEngine