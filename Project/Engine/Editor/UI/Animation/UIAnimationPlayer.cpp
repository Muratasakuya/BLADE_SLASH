#include "UIAnimationPlayer.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Animation/Track/UIPropertyTrackRuntime.h>

//============================================================================
//	UIAnimationPlayer classMethods
//============================================================================

void UIAnimationPlayer::Play(const UIAnimationClip& clip, UIAsset& asset, const UIElement::Handle& handle) {

	// トラックランタイムの生成と開始
	tracks_.clear();
	tracks_.reserve(clip.tracks.size());
	for (auto& track : clip.tracks) {

		tracks_.emplace_back(CreateRuntimeFromAsset(track));
	}
	// 開始処理
	for (auto& track : tracks_) {

		track->Start(asset, handle);
	}
	// 再生中フラグをオン
	playing_ = true;
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
		auto build = [=, &track](UIAsset& asset, const UIElement::Handle& handle) {
			return BuildVec2Access(asset, handle, track.target, track.property);
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
		auto build = [=, &track](UIAsset& asset, const UIElement::Handle& handle) {
			return BuildFloatAccess(asset, handle, track.target, track.property);
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
		auto build = [=, &track](UIAsset& asset, const UIElement::Handle& handle) {
			return BuildColorAccess(asset, handle, track.target, track.property);
			};
		return std::make_unique<UIPropertyTrackRuntime<Color>>(track.applyMode, std::move(runtime), build);
	}
	}
	return nullptr;
}