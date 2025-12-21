#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Scenes/Game/GameState/GameSceneState.h>

// c++
#include <cstdint>

namespace GameHUDEvents {

	// ここにusingの説明が入る
	using EntityId = std::uintptr_t;

	// ここに関数の説明が入る
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
		bool disable{};
	};

	// 目標ナビゲーションのフラスタム内チェックの有効、無効
	struct TargetNavigationInFrustumCheckEvent {

		EntityId owner{};
		bool enable{};
	};

	// ゲームシーンの遷移に応じたイベント
	struct SceneStateChangedEvent {

		GameSceneState prev{};
		GameSceneState next{};
	};
} // GameHUDEvents