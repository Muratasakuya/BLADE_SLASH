#include "UISlateButton.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================

//============================================================================
//	UISlateButton classMethods
//============================================================================

void UISlateButton::OnAddedToTree(UIUserWidget& owner) {

	// 背景を最初の子に自動挿入
	if (!children_.empty()) {

		background_ = static_cast<UISlateImage*>(children_[0].get());
	}
	if (!background_) {

		auto background = std::make_unique<UISlateImage>();
		background->SetName(GetName() + "_BG");
		// Button自身のRectにストレッチさせる
		background->GetLayout().anchors.min = Vector2::AnyInit(0.0f);
		background->GetLayout().anchors.max = Vector2::AnyInit(1.0f);
		background->GetLayout().offsets = UIMargin{ 0.0f,0.0f,0.0f,0.0f };
		background->GetLayout().alignment = Vector2::AnyInit(0.0f);
		background_ = background.get();

		// 先頭に入れる
		children_.insert(children_.begin(), std::move(background));
	}

	// 子のツリー登録
	UISlatePanelWidget::OnAddedToTree(owner);
}

UIReply UISlateButton::OnPointerEvent(UIUserWidget& owner, const UIPointerEvent& event) {

	// 無効ならDisableで無視
	if (!IsEnabled()) {

		state_ = UIEventButtonState::Disabled;
		return UIReply::Unhandled();
	}

	// ポインタ位置が内部か
	bool isInsideRect = GetCachedRect().Contains(event.screenPos);

	switch (event.type) {
	case UIEventType::PointerMove: {

		// 押下中でなければホバー状態更新
		if (!isPressedInside_) {

			state_ = isInsideRect ? UIEventButtonState::Hovered : UIEventButtonState::Normal;
		}
		return UIReply::Unhandled();
		break;
	}
	case UIEventType::PointerDown: {

		// 内部なら押下状態に
		if (isInsideRect) {

			isPressedInside_ = true;
			state_ = UIEventButtonState::Pressed;
			return UIReply::Handled().CaptureMouse().SetKeyboardFocus();
		}
		return UIReply::Unhandled();
		break;
	}
	case UIEventType::PointerUp: {

		// 押下中であれば
		if (isPressedInside_) {

			isPressedInside_ = false;
			state_ = isInsideRect ? UIEventButtonState::Hovered : UIEventButtonState::Normal;
			if (isInsideRect) {
				// クリック処理があったか
				if (onClicked_) {

					onClicked_();
				}
			}
			return UIReply::Handled().ReleaseMouse();
		}
		return UIReply::Unhandled();
		break;
	}
	}
	return UIReply::Unhandled();
}

void UISlateButton::PaintDrawData(UIUserWidget& owner) {

	// 背景テクスチャ切替
	if (background_) {

		// 状態別
		switch (state_) {
		case SakuEngine::UIEventButtonState::Normal:

			background_->SetTextureName(style_.normalTexture);
			break;
		case SakuEngine::UIEventButtonState::Hovered:

			background_->SetTextureName(style_.hoveredTexture.empty() ? style_.normalTexture : style_.hoveredTexture);
			break;
		case SakuEngine::UIEventButtonState::Pressed:

			background_->SetTextureName(style_.pressedTexture.empty() ? style_.normalTexture : style_.pressedTexture);
			break;
		case SakuEngine::UIEventButtonState::Disabled:

			background_->SetTextureName(style_.disabledTexture.empty() ? style_.normalTexture : style_.disabledTexture);
			break;
		}
	}

	// 子を描画更新
	UISlatePanelWidget::PaintDrawData(owner);
}

void UISlateButton::FromJson(const Json& data) {

	UISlatePanelWidget::FromJson(data);
	style_.normalTexture = data["buttonStyle"].value("normal", style_.normalTexture);
	style_.hoveredTexture = data["buttonStyle"].value("hovered", style_.hoveredTexture);
	style_.pressedTexture = data["buttonStyle"].value("pressed", style_.pressedTexture);
	style_.disabledTexture = data["buttonStyle"].value("disabled", style_.disabledTexture);
	clickActionKey_ = data.value("clickActionKey", clickActionKey_);
}

void UISlateButton::ToJson(Json& data) {

	UISlatePanelWidget::ToJson(data);
	Json style{};
	style["normal"] = style_.normalTexture;
	style["hovered"] = style_.hoveredTexture;
	style["pressed"] = style_.pressedTexture;
	style["disabled"] = style_.disabledTexture;
	data["buttonStyle"] = style;
	data["clickActionKey"] = clickActionKey_;
}