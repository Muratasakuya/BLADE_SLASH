#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Base/GameObject3D.h>
#include <Game/Gameplay/Actors/Player/ComboAction/Runtime/Execute/PlayerComboExecuteMode.h>
#include <Game/Gameplay/Actors/Player/ComboAction/Methods/PlayerComboActionModel.h>

// front
class Player;
class PlayerComboActionModel;
class PlayerReactionAreaChecker;

//============================================================================
//	IPlayerComboExecuteMode class
//	プレイヤーのコンボ実行を処理するインターフェース
//============================================================================
class IPlayerComboExecuteMode {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	IPlayerComboExecuteMode() = default;
	virtual ~IPlayerComboExecuteMode() = default;

	// 初期化
	virtual void Init(Player* player, PlayerComboActionModel* model,
		const PlayerReactionAreaChecker* areaChecker, const SakuEngine::GameObject3D* attackTarget) = 0;

	// 更新
	virtual void Update() = 0;

	// コンボ開始
	virtual void StartCombo(uint32_t comboId) = 0;
	// 外部からのキャンセル要求
	virtual void RequestCancel(bool force) = 0;

	//--------- accessor -----------------------------------------------------

	// モードを取得
	virtual PlayerComboExecuteMode GetMode() const = 0;
	// コンボ実行中か
	virtual bool IsPlaying() const = 0;
	// 現在再生中のコンボID取得
	virtual uint32_t GetPlayingComboId() const = 0;
};