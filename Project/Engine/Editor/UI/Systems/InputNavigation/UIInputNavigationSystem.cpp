#include "UIInputNavigationSystem.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Collision/CollisionGeometry.h>
#include <Engine/Input/Input.h>
#include <Engine/Editor/UI/Components/Selectable/UISelectableComponent.h>
#include <Engine/Editor/UI/Components/InputNavigation/UIInputNavigationComponent.h>
#include <Engine/Editor/UI/Components/Transform/UISpriteTransformComponent.h>
#include <Engine/Editor/UI/Components/Transform/UITextTransformComponent.h>

//============================================================================
//	UIInputNavigationSystem classMethods
//============================================================================

namespace {

	// UIアセットから入力モジュールコンポーネントを探す
	UIInputNavigationComponent* FindInputModule(UIAsset& asset) {

		UIInputNavigationComponent* component = nullptr;
		asset.elements.ForEachAlive([&](UIElement::Handle handle, [[maybe_unused]] const UIElement& element) {
			// すでに見つかっていればスキップ
			if (component) {
				return;
			}
			component = static_cast<UIInputNavigationComponent*>(asset.FindComponent(handle, UIComponentType::InputNavigation));
			});
		return component;
	}

	// UIDからハンドルのマップを構築
	void BuildUidMap(UIAsset& asset, std::unordered_map<uint32_t, UIElement::Handle>& out) {

		out.clear();
		asset.elements.ForEachAlive([&](UIElement::Handle handle, const UIElement& element) {

			// UIDが0でなければ登録
			if (element.uid != 0) {
				out[element.uid] = handle;
			}
			});
	}

	// 選択可能かどうか
	bool IsSelectable(UIAsset& asset, UIElement::Handle handle) {

		auto* component = static_cast<UISelectableComponent*>(asset.FindComponent(handle, UIComponentType::Selectable));
		return component && component->interactable && component->wantVisible;
	}

	// 要素のトランスフォームパラメーターを取得する
	bool TryGetElementRectParams(UIAsset& asset, UIElement::Handle handle,
		Vector2& outCenter, Vector2& outSize, Vector2& outAnchor) {

		if (auto* spriteTransform = static_cast<UISpriteTransformComponent*>(asset.FindComponent(handle, UIComponentType::SpriteTransform))) {

			outCenter = spriteTransform->transform->GetWorldPos();
			outSize = spriteTransform->transform->size * spriteTransform->transform->GetWorldScale();
			outAnchor = spriteTransform->transform->anchorPoint;
			return true;
		}
		if (auto* textTransform = static_cast<UITextTransformComponent*>(asset.FindComponent(handle, UIComponentType::TextTransform))) {

			outCenter = textTransform->transform->GetWorldPos();
			outSize = textTransform->transform->textBoxSize * textTransform->transform->GetWorldScale();
			outAnchor = textTransform->transform->anchorPoint;
			return true;
		}
		return false;
	}

	// 移動入力から方向を得る
	bool GetMoveDirection(const UIInputNavigationComponent& navigation, Direction2D& outDirection) {

		if (!navigation.mapper) {
			return false;
		}

		const float moveX = navigation.mapper->GetVector(UINavigationAction::MoveX);
		const float moveY = navigation.mapper->GetVector(UINavigationAction::MoveY);
		const float stickThreshold = navigation.stickThreshold;

		bool has = false;
		// 大きい方の軸で判定
		if (std::fabs(moveX) > std::fabs(moveY)) {
			if (moveX > stickThreshold) {

				outDirection = Direction2D::Right;
				has = true;
			} else if (moveX < -stickThreshold) {

				outDirection = Direction2D::Left;
				has = true;
			}
		} else {
			if (moveY > stickThreshold) {

				outDirection = Direction2D::Up;
				has = true;
			} else if (moveY < -stickThreshold) {

				outDirection = Direction2D::Bottom;
				has = true;
			}
		}
		return has;
	}

	// リピート制御
	bool ConsumeMove(UIInputNavigationComponent& navigation, Direction2D& outDirection) {

		Direction2D direction{};
		// 入力方向を取得、なければリセット
		if (!GetMoveDirection(navigation, direction)) {

			navigation.holdingDirection = false;
			return false;
		}

		using namespace std::chrono;
		const auto now = steady_clock::now();

		// 初回、方向変化は即時反映
		if (!navigation.holdingDirection || navigation.heldDirection != direction) {

			// リピート状態を開始
			navigation.holdingDirection = true;
			navigation.heldDirection = direction;
			navigation.nextRepeatTime = now + duration_cast<steady_clock::duration>(duration<float>(navigation.repeatDelaySecond));
			outDirection = direction;
			return true;
		}

		// リピート
		if (navigation.nextRepeatTime <= now) {

			// 次のリピート時間を設定
			navigation.nextRepeatTime = now + duration_cast<steady_clock::duration>(duration<float>(navigation.repeatIntervalSecond));
			outDirection = navigation.heldDirection;
			return true;
		}
		return false;
	}

	// 明示的ナビゲーションで次の要素を探す
	UIElement::Handle FindNextExplicit(const UISelectableComponent& selectable, Direction2D direction,
		const std::unordered_map<uint32_t, UIElement::Handle>& uidToHandle) {

		uint32_t targetUid = 0;
		// 指定方向のUIDを取得
		switch (direction) {
		case Direction2D::Up: {

			targetUid = selectable.navigation.explicitNavi.up;
			break;
		}
		case Direction2D::Bottom: {

			targetUid = selectable.navigation.explicitNavi.down;
			break;
		}
		case Direction2D::Left: {

			targetUid = selectable.navigation.explicitNavi.left;
			break;
		}
		case Direction2D::Right: {

			targetUid = selectable.navigation.explicitNavi.right;
			break;
		}
		}
		// UIDが0なら無効
		if (targetUid == 0) {
			return {};
		}
		auto it = uidToHandle.find(targetUid);
		if (it == uidToHandle.end()) {
			return {};
		}
		return it->second;
	}

	// 自動ナビゲーションで次の要素を探す
	UIElement::Handle FindNextAutomatic(UIAsset& asset, UIElement::Handle from, Direction2D direction,
		const UISelectableComponent& fromSelectable, const std::vector<UIElement::Handle>& candidates) {

		Vector2 fromCenter, fromSize, fromAnchor;
		if (!TryGetElementRectParams(asset, from, fromCenter, fromSize, fromAnchor)) {
			return {};
		}

		// 方向ベクトル
		const Vector2 dirVector = Direction::Get(direction);

		// 最適候補を探索
		UIElement::Handle best{};
		float bestScore = -std::numeric_limits<float>::infinity();
		for (auto handle : candidates) {

			// 自分自身はスキップ
			if (UIElement::Handle::Equal(handle, from)) {
				continue;
			}
			if (!IsSelectable(asset, handle)) {
				continue;
			}

			Vector2 center, size, anchor;
			// 要素の矩形パラメーターを取得
			if (!TryGetElementRectParams(asset, handle, center, size, anchor)) {
				continue;
			}

			Vector2 distance = center - fromCenter;
			const float length = std::sqrt(distance.x * distance.x + distance.y * distance.y);
			if (length < 1e-5f) {
				continue;
			}

			const Vector2 normalized = distance / length;
			const float dot = (normalized.x * dirVector.x + normalized.y * dirVector.y);
			if (dot < fromSelectable.navigation.minDot) {
				continue;
			}

			// 方向に沿って近いほど高い
			const float score = dot / length;
			// 現在のスコアより良ければ更新
			if (bestScore < score) {

				bestScore = score;
				best = handle;
			}
		}
		return best;
	}

	// マウスホバーしている要素を探す
	UIElement::Handle PickHoveredByMouse(UIAsset& asset, Input* input, InputViewArea viewArea,
		const std::vector<UIElement::Handle>& candidates) {

		UIElement::Handle best{};
		float bestDistance = std::numeric_limits<float>::infinity();
		for (auto handle : candidates) {

			// 選択可能でなければスキップ
			if (!IsSelectable(asset, handle)) {
				continue;
			}

			// 要素の矩形パラメーターを取得
			Vector2 center, size, anchor;
			if (!TryGetElementRectParams(asset, handle, center, size, anchor)) {
				continue;
			}
			// マウスが矩形内にないならスキップ
			if (!Collision::RectToMouse(center, size, anchor, viewArea)) {
				continue;
			}

			// 中心からの距離を計算
			const Vector2 mouse = input->GetMousePos();
			const Vector2 diff = center - mouse;
			const float distance = diff.x * diff.x + diff.y * diff.y;
			// 最も近い要素を選択
			if (distance < bestDistance) {

				bestDistance = distance;
				best = handle;
			}
		}
		return best;
	}
}

void UIInputNavigationSystem::Update([[maybe_unused]] UISystemContext* context, UIAsset& asset) {

	auto* navigation = FindInputModule(asset);
	if (!navigation) {
		return;
	}
	// 入力を受け付けないなら処理しない
	if (!navigation->acceptInput) {
		return;
	}

	// 入力マッパーを確保
	navigation->EnsureMapper();
	Input* input = Input::GetInstance();
	if (!input) {
		return;
	}

	// uidからhandleのマップ
	std::unordered_map<uint32_t, UIElement::Handle> uidToHandle{};
	// マップを構築
	BuildUidMap(asset, uidToHandle);

	// selectable候補
	std::vector<UIElement::Handle> candidates;
	asset.elements.ForEachAlive([&](UIElement::Handle handle, [[maybe_unused]] const UIElement& element) {
		// 選択可能なら候補に追加
		if (IsSelectable(asset, handle)) {

			candidates.emplace_back(handle);
		}
		});

	// フォーカスハンドル
	UIElement::Handle focused{};
	if (navigation->focusedUid != 0) {

		auto it = uidToHandle.find(navigation->focusedUid);
		if (it != uidToHandle.end()) {
			focused = it->second;
		}
	}

	// 無効なら先頭に寄せる
	if (!asset.elements.IsAlive(focused) && !candidates.empty()) {

		focused = candidates.front();
		navigation->focusedUid = asset.elements.Get(focused)->uid;
	}

	// マウスホバーでフォーカス
	if (navigation->mouseHoverFocus) {

		// ホバー判定をして取得してハンドルを得る
		UIElement::Handle hovered = PickHoveredByMouse(asset, input, InputViewArea::UIEditor, candidates);
		// 有効ならフォーカスを移動
		if (asset.elements.IsAlive(hovered)) {

			navigation->focusedUid = asset.elements.Get(hovered)->uid;
			focused = hovered;
		}

		// クリックで決定
		if (navigation->mouseClickSubmit && asset.elements.IsAlive(hovered) && input->TriggerMouseLeft()) {

			navigation->focusedUid = asset.elements.Get(hovered)->uid;
			focused = hovered;
			if (auto* selectable = static_cast<UISelectableComponent*>(asset.FindComponent(focused, UIComponentType::Selectable))) {

				selectable->state = UIElementState::Decided;
			}
		}
	}

	// 決定とキャンセル処理
	if (asset.elements.IsAlive(focused)) {
		// 状態遷移
		// 決定
		if (navigation->mapper->IsTriggered(UINavigationAction::Submit)) {
			if (auto* selectable = static_cast<UISelectableComponent*>(asset.FindComponent(focused, UIComponentType::Selectable))) {

				selectable->state = UIElementState::Decided;
			}
		}
		// キャンセル
	}

	// 移動判定
	if (asset.elements.IsAlive(focused)) {

		Direction2D moveDirection{};
		// 移動入力を取得
		if (ConsumeMove(*navigation, moveDirection)) {

			// 現在の選択要素を取得
			auto* fromSelectable = static_cast<UISelectableComponent*>(asset.FindComponent(focused, UIComponentType::Selectable));
			if (!fromSelectable) {
				return;
			}

			// 次の要素を探す
			UIElement::Handle next{};
			switch (fromSelectable->navigation.mode) {
			case UINavigationMode::None:
				break;
			case UINavigationMode::Explicit:

				// 明示的ナビゲーションで探す
				next = FindNextExplicit(*fromSelectable, moveDirection, uidToHandle);
				break;
			case UINavigationMode::Automatic:

				// 自動ナビゲーションで探す
				next = FindNextAutomatic(asset, focused, moveDirection, *fromSelectable, candidates);
				break;
			}

			// 次の要素が有効ならフォーカスを移動
			if (asset.elements.IsAlive(next) && IsSelectable(asset, next)) {

				navigation->focusedUid = asset.elements.Get(next)->uid;
			}
		}
	}
}