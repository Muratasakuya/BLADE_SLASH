#include "UIApplication.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Input/Input.h>

//============================================================================
//	UIApplication classMethods
//============================================================================

void UIApplication::Update(float deltaTime) {

	// 更新出来るウィジェットが無ければ終了
	if (!userWidget_ || !userWidget_->GetRoot()) {
		return;
	}

	// マウス位置
	Input* input = Input::GetInstance();
	// ビュー内のマウス位置を取得
	const auto mouseInView = input->GetMousePosInView(viewArea_);
	Vector2 mousePos = mouseInView.has_value() ? mouseInView.value() : input->GetMousePos();
	// 前回位置保存
	const Vector2 prevMousePos = prevMousePos_;
	prevMousePos_ = mousePos;

	// マウスイベント処理
	{
		UIPointerEvent moveEvent{};
		moveEvent.type = UIEventType::PointerMove;
		moveEvent.screenPos = mousePos;
		moveEvent.delta = mousePos - prevMousePos;
		moveEvent.inputDevice = input->GetType();
		DispatchPointerEvent(moveEvent);
	}

	// マウスホイールイベント処理
	{
		float wheel = input->GetMouseWheel();
		if (wheel != 0.0f) {

			UIPointerEvent wheelEvent{};
			wheelEvent.type = UIEventType::PointerWheel;
			wheelEvent.screenPos = mousePos;
			wheelEvent.wheelDelta = wheel;
			wheelEvent.inputDevice = input->GetType();
			DispatchPointerEvent(wheelEvent);
		}
	}

	// マウスボタンイベント処理
	{
		bool down = input->TriggerMouseLeft();
		bool up = input->ReleaseMouse(MouseButton::Left);
		if (down) {

			UIPointerEvent event{};
			event.type = UIEventType::PointerDown;
			event.screenPos = mousePos;
			event.button = UIPointerButton::Left;
			event.inputDevice = input->GetType();
			DispatchPointerEvent(event);
		}
		if (up) {

			UIPointerEvent event{};
			event.type = UIEventType::PointerUp;
			event.screenPos = mousePos;
			event.button = UIPointerButton::Left;
			event.inputDevice = input->GetType();
			DispatchPointerEvent(event);
		}
	}

	// 値同期
	userWidget_->SynchronizeProperties();
	// ユーザーTick
	userWidget_->Tick(deltaTime);
	// レイアウト
	Arrange();

	// 描画データ更新
	userWidget_->PaintDrawData();
}

void UIApplication::SetViewportRect(const UIRect& rect) {

	viewportRect_ = rect;
	if (userWidget_) {

		userWidget_->SetViewportRect(rect);
	}
}

void UIApplication::Arrange() {

	auto* root = userWidget_->GetRoot();
	if (!root) {
		return;
	}

	// rootの割り当て矩形
	const UIRect rootRect = userWidget_->GetViewportRect();
	// rootのキャッシュ矩形設定
	root->SetCachedRect(rootRect);
	// 配置計算
	std::vector<UIArrangedWidget> arranged;
	root->ArrangeChildren(rootRect, arranged);
}

UISlateWidget* UIApplication::HitTest(UISlateWidget* widget, const Vector2& pos) {

	// ウィジェットが無ければ終了
	if (!widget) {
		return nullptr;
	}

	const auto visibility = widget->GetVisibility();
	if (visibility == UIVisibility::Collapsed || visibility == UIVisibility::Hidden) {
		return nullptr;
	}
	if (visibility == UIVisibility::HitTestInvisible) {
	} else {
		// ウィジェットの矩形内か
		if (!widget->GetCachedRect().Contains(pos)) {

			return nullptr;
		}
	}

	// ウィジェットのローカル座標に変換
	if (widget->IsPanel()) {

		// 子を逆順にチェックする
		std::vector<UISlateWidget*> children;
		widget->GetChildren(children);
		for (int32_t i = static_cast<int32_t>(children.size()) - 1; 0 <= i; --i) {
			if (auto* hit = HitTest(children[i], pos)) {
				return hit;
			}
		}
	}

	// HitTestInvisibleなら自分は返さない
	if (visibility == UIVisibility::HitTestInvisible) {
		return nullptr;
	}
	return widget;
}

void UIApplication::DispatchPointerEvent(const UIPointerEvent& event) {

	auto* root = userWidget_->GetRoot();
	if (!root) return;

	// キャプチャーがあればそれを優先
	UISlateWidget* target = captureWidget_;
	// なければHitTest
	if (!target) {
		target = HitTest(root, event.screenPos);
	}
	// ターゲットが無ければ終了
	if (!target) {
		return;
	}

	// Disabledは何もしない
	if (!target->IsEnabled()) {
		return;
	}

	// イベント配信
	UIReply reply = target->OnPointerEvent(*userWidget_, event);

	// マウスキャプチャー設定
	if (reply.captureMouse) {
		captureWidget_ = target;
	}
	if (reply.releaseMouse) {
		captureWidget_ = nullptr;
	}
	// キーボードフォーカス設定
	if (reply.setKeyboardFocus && target->SupportsKeyboardFocus()) {

		focusWidget_ = target;
	}
}