#include "UIWidgetPreviewRuntime.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Runtime/UIUserWidget.h>
#include <Engine/Editor/UI/Runtime/UIApplication.h>
#include <Engine/Editor/UI/Editor/Preview/UIWidgetCompiler.h>

//============================================================================
//	UIWidgetPreviewRuntime classMethods
//============================================================================

void UIWidgetPreviewRuntime::Init() {

	app_ = std::make_unique<UIApplication>();
	userWidget_ = std::make_unique<UIUserWidget>();
	userWidget_->Init("UI_Preview");

	// viewport 設定
	SetViewportSize(viewportSize_);
}

void UIWidgetPreviewRuntime::SetViewportSize(const Vector2& size) {

	viewportSize_ = size;
	// ユーザウィジェットに設定
	if (userWidget_) {

		UIRect rect{};
		rect.pos = Vector2::AnyInit(0.0f);
		rect.size = viewportSize_;
		userWidget_->SetViewportRect(rect);
	}
}

void UIWidgetPreviewRuntime::Tick(float deltaTime) {

	if (!userWidget_ || !userWidget_->GetRoot()) {
		return;
	}

	// フレーム更新
	app_->Update(deltaTime);
}

void UIWidgetPreviewRuntime::RebuildFromDocument(const UIWidgetDocument& document) {

	// ウィジェットを作成
	UIWidgetCompiler compiler{};
	auto root = compiler.BuildRuntimeTree(document);

	// ユーザウィジェットに設定
	userWidget_->SetRoot(std::move(root));
	app_->SetUserWidget(userWidget_.get());
}

void UIWidgetPreviewRuntime::SyncPropertiesFromDocument(const UIWidgetDocument& document) {

	// 構築し直し
	RebuildFromDocument(document);
}

bool UIWidgetPreviewRuntime::GetWidgetRect(uint32_t widgetId, Vector2& outPos, Vector2& outSize) const {

	if (!userWidget_ || !userWidget_->GetRoot()) {
		return false;
	}

	// 総当たりで探す
	std::vector<UISlateWidget*> stack{};
	stack.push_back(userWidget_->GetRoot());
	while (!stack.empty()) {

		UISlateWidget* widget = stack.back();
		stack.pop_back();
		if (!widget) {
			continue;
		}

		// ID一致チェック
		if (widget->GetId() == widgetId) {

			const auto& rect = widget->GetCachedRect();
			outPos = rect.pos;
			outSize = rect.size;
			return true;
		}
		// パネルなら子供を追加
		if (widget->IsPanel()) {

			std::vector<UISlateWidget*> children{};
			widget->GetChildren(children);
			for (auto* child : children) {

				stack.push_back(child);
			}
		}
	}
	return false;
}

UIPickResult UIWidgetPreviewRuntime::PickWidgetAt(const Vector2& screenPos) const {

	UIPickResult rect{};
	if (!userWidget_ || !userWidget_->GetRoot()) {
		return rect;
	}
	// ヒットテスト
	uint32_t id = HitTestRecursive(userWidget_->GetRoot(), screenPos);
	// 0以外なら有効
	if (id != 0) {
		rect.widgetId = id;
		rect.valid = true;
	}
	return rect;
}

uint32_t UIWidgetPreviewRuntime::HitTestRecursive(UISlateWidget* widget, const Vector2& pos) const {

	if (!widget) {
		return 0;
	}

	const auto visibility = widget->GetVisibility();
	// 入力を受け付けないなら無視
	if (visibility == UIVisibility::Collapsed || visibility == UIVisibility::Hidden) {
		return 0;
	}

	// 自分は無視して子供だけヒットテストするか
	const bool selfHitTest = (visibility != UIVisibility::HitTestInvisible);

	// パネルのみ
	if (widget->IsPanel()) {

		std::vector<UISlateWidget*> children{};
		widget->GetChildren(children);
		// 子供を逆順でヒットテスト
		for (int32_t i = static_cast<int32_t>(children.size()) - 1; 0 <= i; --i) {

			uint32_t hit = HitTestRecursive(children[i], pos);
			// 0以外ならヒットしたIDを返す
			if (hit != 0) {
				return hit;
			}
		}
	}

	// 自分自身のヒットテストを行わないなら無視
	if (!selfHitTest) {
		return 0;
	}

	// 矩形内かチェック
	if (widget->GetCachedRect().Contains(pos)) {
		return widget->GetId();
	}
	return 0;
}