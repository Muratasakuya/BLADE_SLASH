#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Scenes/Game/GameState/GameSceneState.h>

// c++
#include <cstdint>

namespace GameHUDEvents {

	// Entityをイベント上で識別するためのID型
	using EntityId = std::uintptr_t;

	// ポインタをEntityIdに変換するヘルパー
	EntityId MakeEntityId(const void* ptr) noexcept;

	// ダメージを受けた
	struct DamageTakenEvent {

		EntityId victim;   // ダメージを受けたエンティティ
		EntityId attacker; // 攻撃を与えたエンティティ
		int damage;        // ダメージ量
	};

	// HUDの表示、非表示
	struct VisibilityChangedEvent {

		EntityId target;  // 対象エンティティ
		bool visible;     // 表示(true)/非表示(false)
	};

	// 目標ナビゲーションの有効、無効
	struct TargetNavigationDisableEvent {

		EntityId owner{};
		bool disable;
	};

	// 目標ナビゲーションのフラスタム内チェックの有効、無効
	struct TargetNavigationInFrustumCheckEvent {

		EntityId owner{};
		bool enable;
	};

	// パリィ入力示唆の表示、非表示
	struct ParrySuggestEvent {

		EntityId owner{};
		bool visible;
	};

	// ゲームシーンの遷移に応じたイベント
	struct SceneStateChangedEvent {

		GameSceneState prev{};
		GameSceneState next{};
	};
} // GameHUDEvents