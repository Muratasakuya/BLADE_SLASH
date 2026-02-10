#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Components/Interface/IUIComponent.h>
#include <Engine/Utility/Algorithm/HandlePool.h>

// c++
#include <string>
#include <memory>

namespace SakuEngine {

	//============================================================================
	//	UIAsset structure
	//	UIが持つ基底情報
	//============================================================================

	// UIコンポーネントのスロット
	struct UIComponentSlot {

		// ファイルを保存する基底パス
		static inline const std::string kBaseJsonPath = "UIEditor/UIComponent/";

		// コンポーネントの実体
		std::unique_ptr<IUIComponent> component;
	};

	// UIコンポーネント専用ハンドル
	using UIComponentHandle = HandlePool<UIComponentSlot>::Handle;

	// UI要素情報
	struct UIElement {

		// ファイルを保存する基底パス
		static inline const std::string kBaseJsonPath = "UIEditor/UIElement/";

		// UI要素専用ハンドル
		using Handle = HandlePool<UIElement>::Handle;

		// 要素の名前
		uint32_t uid = 0;
		std::string name;

		// 親要素
		Handle parentHandle;
		// 子要素リスト
		std::vector<Handle> children;

		// コンポーネントハンドルリスト
		std::vector<UIComponentHandle> components;
	};

	// UIアセット
	struct UIAsset {

		// ファイルを保存する基底パス
		static inline const std::string kBaseJsonPath = "UIEditor/UIAsset/";

		// 次のUID
		uint32_t nextUid = 1;

		// UI要素プール
		HandlePool<UIElement> elements;
		// UIコンポーネントプール
		HandlePool<UIComponentSlot> components;
		// ルート要素ハンドル
		UIElement::Handle rootHandle;

		//========================================================================
		//	Runtime Methods
		//========================================================================

		// アクティブかどうか
		bool isActive = false;

		// アクティブ化直前の状態保持
		Json runtimeSnapshot{};
		bool hasRuntimeSnapshot = false;

		// アクティブ、非アクティブ化
		void SetActivate(bool active);

		//========================================================================
		//	UIAsset Methods
		//========================================================================

		// 初期化
		void Init();

		// UIDの割り当て
		uint32_t AllocateUid() { return nextUid++; }

		// UIAssetのjson復元、保存
		void FromJson(const Json& data);
		void ToJson(Json& data);
		// UIElementのjson復元、保存
		UIElement::Handle ImportJsonElementPrefab(const Json& data, const UIElement::Handle parent);
		void ExportJsonElementPrefab(Json& data, const UIElement::Handle rootHandle);

		//========================================================================
		//	UIElement Methods
		//========================================================================

		// 親に子を追加、子がすでに他の親を持っている場合は削除して付け替える
		bool AddChild(UIElement::Handle parent, UIElement::Handle child);
		// 親から子を削除
		bool RemoveChild(UIElement::Handle parent, UIElement::Handle child);
		void DestroyRecursive(UIElement::Handle target);

		// 子の親を付け替え
		bool Reparent(UIElement::Handle child, UIElement::Handle newParent);

		// 要素の取得
		UIElement* Get(UIElement::Handle handle) { return elements.Get(handle); }
		const UIElement* Get(UIElement::Handle handle) const { return elements.Get(handle); }

		//========================================================================
		//	UIComponent Methods
		//========================================================================

		// コンポーネントの追加
		UIComponentHandle AddComponentByType(UIElement::Handle owner, UIComponentType type);
		template <typename T, typename ...Args>
		UIComponentHandle AddComponent(UIElement::Handle owner, Args&& ...args) {

			UIElement* element = Get(owner);
			if (!element) {
				return {};
			}

			// コンポーネントスロットを作成してプールに追加
			UIComponentSlot slot;
			slot.component = std::make_unique<T>(std::forward<Args>(args)...);
			UIComponentHandle handle = components.Emplace(std::move(slot));
			element->components.emplace_back(handle);

			// ハンドルを返す
			return handle;
		}

		// 指定タイプのコンポーネントを取得
		IUIComponent* GetComponent(const UIComponentHandle& handle);
		IUIComponent* FindComponent(UIElement::Handle owner, UIComponentType type);
	};

	// UIアセットの登録情報
	struct UIAssetEntry {

		std::string name;
		UIAsset asset;
	};

	// UIアセット破棄関数
	void DestroyObjectRecursive(UIAsset& asset, UIElement::Handle node);
} // SakuEngine