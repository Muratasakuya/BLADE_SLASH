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
		void Init() {

			// クリアしてルートのみ作成
			elements.Clear();
			rootHandle = elements.Emplace(UIElement{ .name = "Root" });
		}

		// 要素を子要素として追加
		void AddChild(UIElement::Handle parent, UIElement::Handle child) {

			UIElement* parentElement = elements.Get(parent);
			UIElement* childElement = elements.Get(child);
			if (parentElement && childElement) {

				// 親子関係を設定
				childElement->parentHandle = parent;
				parentElement->children.emplace_back(child);
			}
		}
	};

	// UIアセットの登録情報
	struct UIAssetEntry {

		std::string name;
		UIAsset asset;
	};
} // SakuEngine