#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Component/Interface/IUIComponent.h>
#include <Engine/Utility/Algorithm/HandlePool.h>

// c++
#include <string>

namespace SakuEngine {

	//============================================================================
	//	UIAsset structure
	//	UIが持つ基底情報
	//============================================================================

	// UI要素情報
	struct UIElement {

		// UI要素専用ハンドル
		using Handle = HandlePool<UIElement>::Handle;

		// 要素の名前
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

		// UI要素プール
		HandlePool<UIElement> elements;
		// UIコンポーネントプール
		HandlePool<IUIComponent> components;
		// ルート要素ハンドル
		UIElement::Handle rootHandle;

		// 初期化
		void Init();

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
	};

	// UIアセットの登録情報
	struct UIAssetEntry {

		std::string name;
		UIAsset asset;
	};
} // SakuEngine