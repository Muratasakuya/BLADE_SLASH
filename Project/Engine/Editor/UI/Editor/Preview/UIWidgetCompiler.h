#pragma once

//============================================================================
//	include
//============================================================================

// c++
#include <memory>
#include <string>

namespace SakuEngine {

	// front
	class UIWidgetDocument;
	class UIUserWidget;
	class UISlateWidget;

	//============================================================================
	//	UIWidgetCompiler class
	//	プレビュー表示用にドキュメントからランタイムウィジェットを生成する
	//============================================================================
	class UIWidgetCompiler {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		UIWidgetCompiler() = default;
		~UIWidgetCompiler() = default;

		// ドキュメントからランタイムウィジェットツリーを生成
		std::unique_ptr<UISlateWidget> BuildRuntimeTree(const UIWidgetDocument& document) const;
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- functions ----------------------------------------------------

		// ドキュメントからウィジェットツリーを生成
		std::unique_ptr<UISlateWidget> BuildNodeRecursive(const UIWidgetDocument& document, uint32_t nodeId) const;
		// プロパティ適用
		void ApplyCommonProperty(const UIWidgetDocument& document, uint32_t nodeId, UISlateWidget& widget) const;
		void ApplyTypeProperty(const UIWidgetDocument& document, uint32_t nodeId, UISlateWidget& widget) const;
	};
} // SakuEngine