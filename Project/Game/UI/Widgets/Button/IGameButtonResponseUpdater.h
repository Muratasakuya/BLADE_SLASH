#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Base/GameObject2D.h>

//============================================================================
//	IGameButtonResponseUpdater structure
//============================================================================

// 現在の入力状態
struct GameButtonFrameContext {

	bool hoverAtRelease = false; // 離した瞬間にホバーしていたか
};

//============================================================================
//	IGameButtonResponseUpdater class
//	ボタンの反応更新処理インターフェース
//============================================================================
class IGameButtonResponseUpdater {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	IGameButtonResponseUpdater() = default;
	virtual ~IGameButtonResponseUpdater() = default;

	// 更新開始
	virtual void Begin(SakuEngine::GameObject2D& object) = 0;

	// 更新中
	virtual void ActiveUpdate(SakuEngine::GameObject2D& object) = 0;
	// 非アクティブになったときの更新
	virtual void InactiveUpdate(SakuEngine::GameObject2D& object) = 0;

	// リセット
	virtual void End(SakuEngine::GameObject2D& object) = 0;

	// 常に行う更新処理
	virtual void IdleUpdate(SakuEngine::GameObject2D&) = 0;

	// editor
	virtual void ImGui() = 0;

	// json
	virtual void FromJson(const Json& data) = 0;
	virtual void ToJson(Json& data) = 0;

	//--------- accessor -----------------------------------------------------

	void SetFrameContext(GameButtonFrameContext& context) { context_ = context; }

	virtual bool IsInactiveFinished() const = 0;
protected:
	//========================================================================
	//	protected Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	GameButtonFrameContext context_;
};