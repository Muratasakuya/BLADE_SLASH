#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/UISlateWidget/Core/UISlateWidget.h>

// c++
#include <functional>

namespace SakuEngine {

	//============================================================================
	//	UIWidgetFactory class
	//	UIWidgetを生成するファクトリクラス
	//============================================================================
	class UIWidgetFactory {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		UIWidgetFactory() = default;
		~UIWidgetFactory() = default;

		//---------- using ------------------------------------------------------

		using CreateFn = std::function<std::unique_ptr<UISlateWidget>()>;

		//--------- functions ----------------------------------------------------

		// ウィジェットの登録
		std::unique_ptr<UISlateWidget> Create(const std::string& typeName) const;

		// ウィジェットの登録
		void Register(std::string typeName, CreateFn fn);

		// デフォルト型登録
		void RegisterDefaults();

		//--------- accessor -----------------------------------------------------

		static UIWidgetFactory& GetInstance();
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		std::unordered_map<std::string, CreateFn> registry_{};
	};

	//============================================================================
	//	UIWidgetAutoRegister structure
	//============================================================================
	template<typename T>
	struct UIWidgetAutoRegister {

		// コンストラクタ
		UIWidgetAutoRegister(const char* typeName) {

			UIWidgetFactory::GetInstance().Register(typeName, []() { return std::make_unique<T>(); });
		}
	};
}