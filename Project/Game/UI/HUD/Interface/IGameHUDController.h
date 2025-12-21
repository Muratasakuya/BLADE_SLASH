#pragma once

//============================================================================
//	IGameHUDController class
//	ゲーム内表示HUDのコントローラーインターフェース
//============================================================================
class IGameHUDController {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	IGameHUDController() = default;
	virtual ~IGameHUDController() = default;

	// 初期化
	virtual void Init() = 0;

	// 更新
	virtual void Update() = 0;

	// エディター
	virtual void ImGui() = 0;
};