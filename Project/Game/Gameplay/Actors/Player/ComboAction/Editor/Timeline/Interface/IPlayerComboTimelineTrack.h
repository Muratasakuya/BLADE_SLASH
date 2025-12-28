#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/MathLib/Vector2.h>
#include <Engine/Utility/Helper/ImGuiHelper.h>
#include <Game/Gameplay/Actors/Player/ComboAction/Methods/PlayerComboActionModel.h>

// imgui
#include <imgui.h>

//============================================================================
//	PlayerComboActionTimelinePanel structures
//============================================================================

// タイムラインのランタイム情報
struct PlayerComboTimelineRuntime {

	// 再生状態
	bool isPlaying = false;

	// 現在時間
	float currentTime = 0.0f;
	// コマ送り時間
	float stepTime = 0.1f;
	// 再生速度
	float playSpeed = 0.1f;
};

// タイムラインの表示設定
struct PlayerComboTimelineView {

	// 1秒あたりのピクセル
	float pixelsPerSecond = 200.0f;
	// マウスドラッグ感度
	float dragsSensitivity = 190.0f;

	// グリッド間隔
	float gridStep = 0.2f;
	// 大グリッド間隔
	float majorGridStep = 0.2f;
	// ルーラーの数字だけフォントスケール
	float rulerFontScale = 0.8f;
	// 左のトラック名領域幅
	float trackNameWidth = 190.0f;
	// トラック高さ
	float trackHeight = 40.0f;
	// ルーラー高さ
	float rulerHeight = 30.0f;

	// 余白
	SakuEngine::Vector2 contentPadding = SakuEngine::Vector2::AnyInit(0.0f);
};

// タイムライン描画コンテキスト
struct PlayerComboTimelineDrawContext {

	// コンボエディター情報
	PlayerComboActionModel& model;
	PlayerComboActionEditorSelection& select;
	// タイムライン、表示設定
	PlayerComboTimelineRuntime& runtime;
	PlayerComboTimelineView& view;

	// 選択中コンボ
	size_t comboIndex = 0;

	// 描画領域
	ImDrawList* drawList = nullptr;

	// スクリーン座標基準
	ImVec2 contentScreenPos = {};
	ImVec2 contentScreenSize = {};

	// スクロール
	SakuEngine::Vector2 scroll = SakuEngine::Vector2::AnyInit(0.0f);

	// タイムラインの総時間
	float totalTime = 0.0f;
	// 表示範囲の時間
	float visibleTimeStart = 0.0f;
	float visibleTimeEnd = 8.0f;
};

//============================================================================
//	IPlayerComboTimelineTrack class
//	プレイヤーコンボタイムラインのトラックインターフェース
//============================================================================
class IPlayerComboTimelineTrack {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	IPlayerComboTimelineTrack() = default;
	virtual ~IPlayerComboTimelineTrack() = default;

	// トラック描画
	virtual void DrawTrack(PlayerComboTimelineDrawContext& context, float trackTopY) = 0;

	//--------- accessor -----------------------------------------------------

	// トラック名
	virtual const char* TrackName() const = 0;
};

//============================================================================
//	IPlayerComboTimelineHelper namespace
//	Utility関数
//============================================================================
namespace PlayerComboTimelineHelper {

	// 時間をグリッドにスナップ
	float SnapTime(float t);
	// 時間からローカルX座標を取得
	float TimeToLocalX(const PlayerComboTimelineView& view, float time);
	// ローカルX座標から時間を取得
	float LocalXToTime(const PlayerComboTimelineView& view, float localX);

	// トラックグリッド縦線描画
	void DrawTrackGridLines(const PlayerComboTimelineDrawContext& context, float trackTopY);
}

//============================================================================
//	PlayerComboTimelinePayload namespace
//	Drag&Dropのペイロード識別子
//============================================================================
namespace PlayerComboTimelinePayload {

	inline constexpr char kActionNodeAssetId[] = "ActionNodeAssetId";
	// ペイロードサイズチェック
	static_assert(sizeof(kActionNodeAssetId) <= 32, "Payload type must be <= 31 chars");
}