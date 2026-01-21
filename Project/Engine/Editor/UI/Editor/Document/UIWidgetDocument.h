#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/UISlateWidget/Core/UISlateWidget.h>

// c++
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <variant>
#include <optional>

namespace SakuEngine {

	//============================================================================
	//	UIWidgetDocument structures
	//============================================================================

	// プロパティ値の型
	using UIPropertyValue = std::variant<bool, int32_t, uint32_t,
		float, std::string, Vector2>;

	// UIウィジェットノード
	struct UIWidgetNode {

		// 固有情報
		uint32_t id = 0;
		uint32_t parentId = 0;          // 親ノードID
		std::vector<uint32_t> children; // 子ノードIDリスト
		std::string name;      // ウィジェットの名前
		std::string typeName;  // ウィジェットの型名

		// ウィジェット表示設定
		UIVisibility visibility = UIVisibility::Visible;
		bool enabled = true;

		// レイアウト情報
		UIAnchorData layout{};

		// プロパティ値の型
		std::unordered_map<std::string, UIPropertyValue> properties;
	};

	// エディターからゲームへ渡す参照ハンドル
	struct UIWidgetRefHandle {

		// ウィジェット固有ID
		std::string assetPath;
	};

	//============================================================================
	//	UIWidgetDocument class
	//	
	//============================================================================
	class UIWidgetDocument {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		UIWidgetDocument() = default;
		~UIWidgetDocument() = default;

		// ドキュメント新規作成
		void CreateDocument();

		// ノードの操作
		// 追加
		uint32_t AddNode(const std::string& type, uint32_t parentId, const std::string& nameHint = "");
		// 削除
		bool RemoveNode(uint32_t id);

		// 親変更
		bool ReparentNode(uint32_t id, uint32_t newParentId, int32_t insertIndex = -1);
		// 名前変更
		bool RenameNode(uint32_t id, const std::string& newName);

		// json
		bool FromJson(const Json& data);
		void ToJson(Json& data) const;

		// パスからハンドルを取得
		UIWidgetRefHandle ExportHandle(const std::string& assetPath) const;

		//--------- accessor -----------------------------------------------------

		// プロパティ設定
		template<typename T>
		bool SetProperty(uint32_t id, const std::string& key, const T& value);

		// ルートノード取得
		uint32_t GetRootId() const { return rootId_; }
		const UIWidgetNode* GetRootNode() const;
		// ノード検索
		UIWidgetNode* FindNode(uint32_t id);
		const UIWidgetNode* FindNode(uint32_t id) const;

		// プロパティ取得
		template<typename T>
		T GetProperty(uint32_t id, const std::string& key, const T& fallback) const;
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		// ルートノードID
		uint32_t rootId_ = 0;
		uint32_t nextId_ = 1;

		// ノードリスト
		std::unordered_map<uint32_t, UIWidgetNode> nodes_;

		//--------- functions ----------------------------------------------------

		// ID割り当て、次に進める
		uint32_t AllocateId();
		// 被らない名前を作成
		std::string MakeUniqueName(const std::string& base) const;

		// サブツリー削除、収集
		void RemoveSubtree(uint32_t id);
		void CollectSubtree(uint32_t id, std::vector<uint32_t>& out) const;
	};

	//============================================================================
	//	UIWidgetDocument templateMethods
	//============================================================================

	template<typename T>
	inline bool UIWidgetDocument::SetProperty(uint32_t id, const std::string& key, const T& value) {

		// ノード取得
		UIWidgetNode* node = FindNode(id);
		if (!node) {
			return false;
		}
		node->properties[key] = value;
		return true;
	}

	template<typename T>
	inline T UIWidgetDocument::GetProperty(uint32_t id, const std::string& key, const T& fallback) const {

		// ノード取得
		const UIWidgetNode* node = FindNode(id);
		if (!node) {
			return fallback;
		}
		auto it = node->properties.find(key);
		if (it == node->properties.end()) {
			return fallback;
		}
		// 型チェックと取得
		if (auto property = std::get_if<T>(&it->second)) {

			return *property;
		}
		return fallback;
	}
} // SakuEngine