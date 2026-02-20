#pragma once

//============================================================================
//	include
//============================================================================
#include <Game/Gameplay/Actors/Player/ComboAction/Methods/PlayerComboActionModel.h>

//============================================================================
//	IPlayerComboActionEditorPanel class
//	プレイヤーコンボアクションエディターパネルインターフェース
//============================================================================
class IPlayerComboActionEditorPanel {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	IPlayerComboActionEditorPanel() = default;
	virtual ~IPlayerComboActionEditorPanel() = default;

	// エディター
	virtual void Draw(PlayerComboActionModel& model, PlayerComboActionEditorSelection& select) = 0;

	//--------- accessor -----------------------------------------------------

	// パネル名取得
	virtual const char* PanelName() const = 0;
protected:
	//========================================================================
	//	protected Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// アイテムサイズ
	const float imguiItemSize_ = 160.0f;
};

//============================================================================
//	IPlayerComboActionEditorPanel namespace
//	Utility関数
//============================================================================
namespace PlayerComboPanelHelper {

	// 入力テキストのリサイズコールバック
	int InputTextCallback_Resize(ImGuiInputTextCallbackData* data);

	// std::string対応InputText
	bool InputTextStdString(const char* label, std::string* str, ImGuiInputTextFlags flags = 0);
}