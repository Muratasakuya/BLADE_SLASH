#include "UIAnimationPlayer.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Animation/Track/UIPropertyTrackRuntime.h>

//============================================================================
//	UIAnimationPlayer classMethods
//============================================================================

namespace {

	// UIアニメーションターゲットタイプをUIコンポーネントタイプに変換
	UIComponentType ToComponentType(UIAnimationTargetType type) {

		switch (type) {
		case UIAnimationTargetType::ParentRectTransform: return UIComponentType::ParentRectTransform;
		case UIAnimationTargetType::SpriteTransform:     return UIComponentType::SpriteTransform;
		case UIAnimationTargetType::TextTransform:       return UIComponentType::TextTransform;
		case UIAnimationTargetType::SpriteMaterial:      return UIComponentType::SpriteMaterial;
		case UIAnimationTargetType::TextMaterial:        return UIComponentType::TextMaterial;
		}
		// フォロースルー
		return UIComponentType::ParentRectTransform;
	}

	// 指定タイプのコンポーネントを所持しているか
	bool HasTarget(UIAsset& asset, const UIElement::Handle& handle, UIAnimationTargetType targetType) {

		return asset.FindComponent(handle, ToComponentType(targetType)) != nullptr;
	}

	// トランスフォーム対象プロパティかどうか
	bool IsTransformProperty(UIAnimationProperty property) {
		return (property == UIAnimationProperty::Translation ||
			property == UIAnimationProperty::Scale ||
			property == UIAnimationProperty::Rotation);
	}

	// UI要素に対して適切なトランスフォームターゲットを解決する
	UIAnimationTargetType ResolveTransformTargetForElement(UIAsset& asset, const UIElement::Handle& handle) {
		if (HasTarget(asset, handle, UIAnimationTargetType::SpriteTransform)) {

			return UIAnimationTargetType::SpriteTransform;
		}
		if (HasTarget(asset, handle, UIAnimationTargetType::TextTransform)) {

			return UIAnimationTargetType::TextTransform;
		}
		return UIAnimationTargetType::ParentRectTransform;
	}

	// 空のプロパティアクセスを生成
	template <typename AccessT, typename ValueT>
	AccessT MakeNoopAccess() {
		AccessT access{};
		access.getter = []() -> ValueT { return ValueT{}; };
		access.setter = [](const ValueT&) {};
		return access;
	}
}

void UIAnimationPlayer::Play(const UIAnimationClip& clip, UIAsset& asset, const UIElement::Handle& handle) {

	// トラックランタイムの生成と開始
	tracks_.clear();
	tracks_.reserve(clip.tracks.size());
	for (auto& track : clip.tracks) {

		auto runtime = CreateRuntimeFromAsset(track);
		if (runtime) {

			tracks_.emplace_back(std::move(runtime));
		}
	}
	// 開始処理
	for (auto& track : tracks_) {

		track->Start(asset, handle);
	}
	// 再生中フラグをオン
	playing_ = !tracks_.empty();
}

void UIAnimationPlayer::Update(UIAsset& asset, const UIElement::Handle& handle) {

	if (!playing_) {
		return;
	}

	bool allFinished = true;
	for (auto& track : tracks_) {

		// 更新処理
		track->Update(asset, handle);

		// 一つでも終了していなければ再生中フラグを維持
		if (!track->IsFinished()) {
			allFinished = false;
		}
	}

	// 全て終了していれば再生中フラグをオフ
	if (allFinished) {
		playing_ = false;
	}
}

std::unique_ptr<IUIAnimationTrackRuntime> UIAnimationPlayer::CreateRuntimeFromAsset(const UIAnimationTrackAsset& track) {

	// トラックアセットのプロパティに応じてランタイムを生成
	switch (track.property) {
	case UIAnimationProperty::Translation:
	case UIAnimationProperty::Scale: {

		// Vector2値ソースランタイムを生成
		auto runtime = track.valueVec2 ? track.valueVec2->CreateRuntime() : nullptr;
		if (!runtime) {
			return nullptr;
		}
		// プロパティアクセス構築関数を生成
		UIAnimationTargetType targetType = track.target;
		UIAnimationProperty propertyType = track.property;
		auto build = [targetType, propertyType](UIAsset& asset, const UIElement::Handle& handle) {

			// 選択要素に合わせてターゲットを自動解決
			UIAnimationTargetType resolved = targetType;
			if (IsTransformProperty(propertyType)) {

				// 未所持コンポーネントを避けるため
				resolved = ResolveTransformTargetForElement(asset, handle);
			}

			// アクセス型を得る
			using AccessT = decltype(BuildVec2Access(std::declval<UIAsset&>(),
				std::declval<const UIElement::Handle&>(), UIAnimationTargetType{}, UIAnimationProperty{}));
			// ターゲットを所持していなければ空アクセスを返す
			if (!HasTarget(asset, handle, resolved)) {

				return MakeNoopAccess<AccessT, Vector2>();
			}
			return BuildVec2Access(asset, handle, resolved, propertyType);
			};
		return std::make_unique<UIPropertyTrackRuntime<Vector2>>(track.applyMode, std::move(runtime), build);
	}
	case UIAnimationProperty::Rotation: {

		// float値ソースランタイムを生成
		auto runtime = track.valueFloat ? track.valueFloat->CreateRuntime() : nullptr;
		if (!runtime) {
			return nullptr;
		}
		// プロパティアクセス構築関数を生成
		UIAnimationTargetType targetType = track.target;
		UIAnimationProperty propertyType = track.property;
		auto build = [targetType, propertyType](UIAsset& asset, const UIElement::Handle& handle) {

			// 選択要素に合わせてターゲットを自動解決
			UIAnimationTargetType resolved = targetType;
			if (IsTransformProperty(propertyType)) {

				// 未所持コンポーネントを避けるため
				resolved = ResolveTransformTargetForElement(asset, handle);
			}

			// アクセス型を得る
			using AccessT = decltype(BuildFloatAccess(std::declval<UIAsset&>(),
				std::declval<const UIElement::Handle&>(), UIAnimationTargetType{}, UIAnimationProperty{}));
			// ターゲットを所持していなければ空アクセスを返す
			if (!HasTarget(asset, handle, resolved)) {

				return MakeNoopAccess<AccessT, float>();
			}
			return BuildFloatAccess(asset, handle, resolved, propertyType);
			};
		return std::make_unique<UIPropertyTrackRuntime<float>>(track.applyMode, std::move(runtime), build);
	}
	case UIAnimationProperty::Color: {

		// Color値ソースランタイムを生成
		auto runtime = track.valueColor ? track.valueColor->CreateRuntime() : nullptr;
		if (!runtime) {
			return nullptr;
		}
		// プロパティアクセス構築関数を生成
		UIAnimationTargetType targetType = track.target;
		UIAnimationProperty propertyType = track.property;
		auto build = [targetType, propertyType](UIAsset& asset, const UIElement::Handle& handle) {

			UIAnimationTargetType resolved = targetType;

			// アクセス型を得る
			using AccessT = decltype(BuildColorAccess(std::declval<UIAsset&>(),
				std::declval<const UIElement::Handle&>(), UIAnimationTargetType{}, UIAnimationProperty{}));
			// ターゲットを所持していなければ空アクセスを返す
			if (!HasTarget(asset, handle, resolved)) {

				return MakeNoopAccess<AccessT, Color>();
			}
			return BuildColorAccess(asset, handle, resolved, propertyType);
			};
		return std::make_unique<UIPropertyTrackRuntime<Color>>(track.applyMode, std::move(runtime), build);
	}
	}
	return nullptr;
}