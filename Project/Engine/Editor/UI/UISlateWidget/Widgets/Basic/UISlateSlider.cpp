#include "UISlateSlider.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Runtime/UIUserWidget.h>

//============================================================================
//	UISlateSlider classMethods
//============================================================================

void UISlateSlider::OnAddedToTree(UIUserWidget& owner) {

	// 表示用イメージを自動挿入
	auto EnsureChildImage = [&](UISlateImage*& outPtr, const std::string& suffix) {
		// 既に存在するなら何もしない
		if (outPtr) {
			return;
		}
		// 作成して追加
		auto image = std::make_unique<UISlateImage>();
		image->SetName(GetName() + suffix);
		outPtr = image.get();
		// 子に追加
		AddChild(std::move(image));
		};

	// 子の作成
	EnsureChildImage(background_, "_BG");
	EnsureChildImage(fill_, "_Fill");
	EnsureChildImage(knob_, "_Knob");

	// レイアウト
	// 背景はスライダー全体にストレッチ
	background_->GetLayout().anchors.min = Vector2::AnyInit(0.0f);
	background_->GetLayout().anchors.max = Vector2::AnyInit(1.0f);
	background_->GetLayout().offsets = UIMargin{ 0.0f,0.0f,0.0f,0.0f };
	background_->GetLayout().alignment = Vector2::AnyInit(0.0f);

	// スライダーの中身
	fill_->GetLayout().anchors.min = Vector2::AnyInit(0.0f);
	fill_->GetLayout().anchors.max = Vector2(0.0f, 1.0f);
	fill_->GetLayout().offsets = UIMargin{ 0.0f,0.0f,100.0f,0.0f };
	fill_->GetLayout().alignment = Vector2::AnyInit(0.0f);

	// 持ち手
	knob_->GetLayout().anchors.min = Vector2(0, 0.5f);
	knob_->GetLayout().anchors.max = Vector2(0, 0.5f);
	knob_->GetLayout().offsets = UIMargin{ 0.0f,0.0f,20.0f,20.0f };
	knob_->GetLayout().alignment = Vector2(0.5f, 0.5f);

	// 子のツリー登録
	UISlatePanelWidget::OnAddedToTree(owner);
}

void UISlateSlider::SynchProperties(UIUserWidget& owner) {

	// バインディングキーが設定されていれば値を同期
	if (!valueBindingKey_.empty()) {

		value_ = std::clamp(owner.Data().GetValue<float>(valueBindingKey_), 0.0f, 1.0f);
	}
}

UIReply UISlateSlider::OnPointerEvent(UIUserWidget& owner, const UIPointerEvent& event) {

	// ポインタ位置が内部か
	bool isInsideRect = GetCachedRect().Contains(event.screenPos);

	switch (event.type) {
	case UIEventType::PointerMove: {

		// 内部にあるときのみ
		if (isInsideRect) {

			// ドラッグ開始
			isDragging_ = true;
			UpdateFromPointer(event.screenPos);
			return UIReply::Handled().CaptureMouse().SetKeyboardFocus();
		}
		return UIReply::Unhandled();
		break;
	}
	case UIEventType::PointerDown: {

		// ドラッグ中なら位置更新
		if (isDragging_) {

			UpdateFromPointer(event.screenPos);
			return UIReply::Handled();
		}
		return UIReply::Unhandled();
		break;
	}
	case UIEventType::PointerUp: {

		// ドラッグ中に話したら終了
		if (isDragging_) {

			isDragging_ = false;
			return UIReply::Handled().ReleaseMouse();
		}
		return UIReply::Unhandled();
		break;
	}
	}
	return UIReply::Unhandled();
}

void UISlateSlider::PaintDrawData(UIUserWidget& owner) {

	// レイアウトをvalueに合わせる
	UIRect rect = GetCachedRect();

	// fill幅
	const float fillW = (std::max)(0.0f, rect.size.x * value_);
	if (fill_) {

		// 右端をvalueに合わせる
		fill_->GetLayout().offsets.right = fillW;
		fill_->GetLayout().offsets.bottom = rect.size.y;
	}

	// 手持ち位置
	if (knob_) {

		// x位置をvalueに合わせる
		float knobX = rect.size.x * value_;
		knob_->GetLayout().offsets.left = knobX;
	}

	// 描画データ更新
	UISlatePanelWidget::PaintDrawData(owner);
}

void UISlateSlider::UpdateFromPointer(const Vector2& screenPos) {

	// スライダー矩形内の位置から値を計算
	UIRect rect = GetCachedRect();
	float localX = screenPos.x - rect.pos.x;
	float value = (rect.size.x <= 0.0f) ? 0.0f : (localX / rect.size.x);
	float clamped = std::clamp(value, 0.0f, 1.0f);
	// 値が変わったらコールバック
	if (clamped != value_) {

		value_ = clamped;
		// 登録されている関数を実行
		if (onValueChanged_) {
			onValueChanged_(value_);
		}
	}
}

void UISlateSlider::FromJson(const Json& data) {

	UISlatePanelWidget::FromJson(data);
	value_ = data.value("value", value_);
	valueBindingKey_ = data.value("valueBindingKey", valueBindingKey_);
}

void UISlateSlider::ToJson(Json& data) {

	UISlatePanelWidget::ToJson(data);
	data["value"] = value_;
	data["valueBindingKey"] = valueBindingKey_;
}