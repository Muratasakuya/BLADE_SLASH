#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Animation/ValueSource/UILerpValueSource.h>

namespace SakuEngine {

	//============================================================================
	//	UIValueSourceFactory class
	//	UI値ソースを生成するファクトリー
	//============================================================================
	class UIValueSourceFactory {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		UIValueSourceFactory() = default;
		~UIValueSourceFactory() = default;

		template <typename T>
		static std::unique_ptr<IUIValueSourceAsset<T>> Create(UIValueSourceType type);
	};

	//============================================================================
	//	UIValueSourceFactory templateMethods
	//============================================================================

	template<typename T>
	inline std::unique_ptr<IUIValueSourceAsset<T>> UIValueSourceFactory::Create(UIValueSourceType type) {

		switch (type) {
		case UIValueSourceType::Lerp:

			return std::make_unique<UILerpValueSourceAsset<T>>();
		}
		return nullptr;
	}
} // SakuEngine