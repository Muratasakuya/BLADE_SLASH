#pragma once

//============================================================================
//	include
//============================================================================

// c++
#include <string>
#include <vector>
#include <unordered_map>

namespace SakuEngine {

	//============================================================================
	//	UIWidgetTypeRegistry structures
	//============================================================================

	// UIウィジェット型情報
	struct UIWidgetTypeInfo {

		std::string type;        // タイプ名
		std::string displayName; // 表示名
		std::string category;    // カテゴリー名
		bool isPanel = false;    // 子を持てるか
	};

	//============================================================================
	//	UIWidgetTypeRegistry class
	//	
	//============================================================================
	class UIWidgetTypeRegistry {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		UIWidgetTypeRegistry() = default;
		~UIWidgetTypeRegistry() = default;

		// ウィジェット型登録
		void Register(const UIWidgetTypeInfo& info);

		// デフォルト型登録
		void RegisterDefaults();

		//--------- accessor -----------------------------------------------------

		// 全ウィジェット型情報取得
		const std::vector<UIWidgetTypeInfo>& GetAll() const { return all_; }
		// カテゴリー別ウィジェット型情報取得
		std::vector<UIWidgetTypeInfo> GetByCategory(const std::string& category) const;

		// ウィジェット型情報取得
		const UIWidgetTypeInfo* Find(const std::string& type) const;
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		// 登録されている全ウィジェット型情報
		std::vector<UIWidgetTypeInfo> all_;
		// タイプ名からインデックスへのマップ
		std::unordered_map<std::string, size_t> map_;
	};
} // SakuEngine