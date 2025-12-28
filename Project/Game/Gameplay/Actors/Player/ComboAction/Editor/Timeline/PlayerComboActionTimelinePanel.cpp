#include "PlayerComboActionTimelinePanel.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Timer/GameTimer.h>

// タイムライントラック
#include <Game/Gameplay/Actors/Player/ComboAction/Editor/Timeline/Tracks/PlayerActionNodeAssetTimelineTrack.h>
#include <Game/Gameplay/Actors/Player/ComboAction/Editor/Timeline/Tracks/PlayerInputGraceTimelineTrack.h>

//============================================================================
//	PlayerComboActionTimelinePanel classMethods
//============================================================================

PlayerComboActionTimelinePanel::PlayerComboActionTimelinePanel() {

	// トラック作成
	tracks_.emplace_back(std::make_unique<PlayerActionNodeAssetTimelineTrack>());
	tracks_.emplace_back(std::make_unique<PlayerInputGraceTimelineTrack>());
}

void PlayerComboActionTimelinePanel::Draw(PlayerComboActionModel& model,
	PlayerComboActionEditorSelection& select) {

	// コンボ未選択なら表示しない
	if (select.selectedComboIndex < 0 || static_cast<int32_t>(model.Combos().size()) <= select.selectedComboIndex) {
		ImGui::TextUnformatted("Select an action from the middle panel.");
		return;
	}

	// 描画コンテキスト作成
	PlayerComboTimelineDrawContext context{
		.model = model,
		.select = select,
		.runtime = runtime_,
		.view = view_,
		.comboIndex = static_cast<size_t>(select.selectedComboIndex),
	};

	// コンボから総時間を取得
	context.totalTime = model.CalculateTotalTime(context.comboIndex);
	// 最低時間保証
	if (context.totalTime < 0.1f) {
		context.totalTime = 0.1f;
	}

	//=========================================================================================================================
	//	コンボ再生
	//=========================================================================================================================

	if (runtime_.isPlaying) {

		// 時間を進める
		float deltaTime = SakuEngine::GameTimer::GetDeltaTime();
		runtime_.currentTime += deltaTime * runtime_.playSpeed;

		// 総時間を超えたら停止
		if (context.totalTime < runtime_.currentTime) {

			runtime_.currentTime = context.totalTime;
			runtime_.isPlaying = false;
		}
	}

	//=========================================================================================================================
	// ランタイム表示
	//=========================================================================================================================

	// ランタイム表示
	DrawRuntime(context);

	ImGui::Separator();

	//=========================================================================================================================
	// タイムライン本体描画
	//=========================================================================================================================

	float totalTrackHeight = context.view.rulerHeight + context.view.trackHeight * static_cast<float>(tracks_.size());

	ImGuiTableFlags tableFlags =
		ImGuiTableFlags_Resizable |
		ImGuiTableFlags_BordersInnerV |
		ImGuiTableFlags_BordersOuterV |
		ImGuiTableFlags_SizingStretchSame;

	if (ImGui::BeginTable("##timeline_layout", 2, tableFlags)) {

		ImGui::TableSetupColumn("##track_names", ImGuiTableColumnFlags_WidthFixed, view_.trackNameWidth);
		ImGui::TableSetupColumn("##timeline_content", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableNextRow();

		float syncedScrollY = 0.0f;

		//============================================================================================
		// 右：タイムライン本体
		//============================================================================================
		ImGui::TableSetColumnIndex(1);
		ImGui::BeginChild("##timeline_content", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);

		// contentChildのスクロール
		context.scroll.x = ImGui::GetScrollX();
		context.scroll.y = ImGui::GetScrollY();
		syncedScrollY = context.scroll.y;

		// 可視時間範囲
		{
			float px0 = context.scroll.x;
			float px1 = context.scroll.x + ImGui::GetContentRegionAvail().x;

			context.visibleTimeStart = PlayerComboTimelineHelper::LocalXToTime(context.view, (std::max)(0.0f, px0));
			context.visibleTimeEnd = PlayerComboTimelineHelper::LocalXToTime(context.view, (std::max)(0.0f, px1));
		}

		// content描画領域
		context.contentScreenPos = ImGui::GetCursorScreenPos();
		context.contentScreenSize = ImGui::GetContentRegionAvail();
		context.drawList = ImGui::GetWindowDrawList();

		// スクロール領域確保、総時間に応じて幅を作る
		float contentWidth = context.view.contentPadding.x * 2.0f + PlayerComboTimelineHelper::TimeToLocalX(context.view, context.totalTime);
		ImGui::Dummy(ImVec2(contentWidth, totalTrackHeight));

		// ルーラー＆スクラブ
		DrawRuler(context);

		// トラック描画
		DrawTracks(context, tracks_);

		// キー操作
		HandleKeyStep(context);

		ImGui::EndChild();

		//============================================================================================
		// 左：トラック名
		//============================================================================================

		ImGui::TableSetColumnIndex(0);
		ImGuiWindowFlags nameFlags =
			ImGuiWindowFlags_NoScrollbar |
			ImGuiWindowFlags_NoScrollWithMouse;

		ImGui::BeginChild("##timeline_track_names", ImVec2(0, 0), true, nameFlags);
		ImGui::SetScrollY(syncedScrollY);

		// ヘッダー
		{
			ImVec2 p = ImGui::GetCursorScreenPos();
			ImDrawList* dl = ImGui::GetWindowDrawList();
			ImVec2 q = ImVec2(p.x + ImGui::GetContentRegionAvail().x, p.y + context.view.rulerHeight);
			dl->AddRectFilled(p, q, IM_COL32(18, 18, 18, 255));
			dl->AddText(ImVec2(p.x + 8.0f, p.y + 8.0f), IM_COL32(230, 230, 230, 255), "Tracks");
			ImGui::Dummy(ImVec2(0.0f, context.view.rulerHeight));
		}

		// トラック名行
		for (size_t ti = 0; ti < tracks_.size(); ++ti) {

			ImGui::PushID(static_cast<int32_t>(ti));
			ImGui::Selectable(tracks_[ti]->TrackName(), false, 0, ImVec2(0.0f, context.view.trackHeight));
			ImGui::PopID();
		}

		ImGui::EndChild();
		ImGui::EndTable();
	}
}

void PlayerComboActionTimelinePanel::DrawRuntime(PlayerComboTimelineDrawContext& context) {

	ImGui::Text("TotalTime : %.2f", context.totalTime);
	ImGui::Text("Current   : %.2f", context.runtime.currentTime);

	// 再生、停止ボタン
	if (!context.runtime.isPlaying) {
		if (ImGui::Button("Play")) {

			context.runtime.isPlaying = true;
		}
	} else {
		if (ImGui::Button("Pause")) {

			context.runtime.isPlaying = false;
		}
	}

	ImGui::SameLine();

	// 再生速度
	ImGui::SetNextItemWidth(140.0f);
	ImGui::DragFloat("Speed", &context.runtime.playSpeed, 0.01f, 0.01f, 3.0f, "%.2f");

	ImGui::SameLine();

	// ステップ時間
	ImGui::SetNextItemWidth(140.0f);
	ImGui::DragFloat("Step", &context.runtime.stepTime, 0.01f, 0.01f, 1.0f, "%.2f");

	// 現在の時間をスライダーで変更
	float currentTime = context.runtime.currentTime;
	ImGui::SetNextItemWidth(-1.0f);
	if (ImGui::SliderFloat("##TimeSlider", &currentTime, 0.0f, context.totalTime, "Time %.2f")) {

		context.runtime.currentTime = currentTime;
		context.runtime.isPlaying = false;
	}
}

void PlayerComboActionTimelinePanel::DrawRuler(PlayerComboTimelineDrawContext& context) {

	// ルーラー領域
	float y0 = context.contentScreenPos.y;
	float y1 = y0 + context.view.rulerHeight;

	// 背景
	context.drawList->AddRectFilled(ImVec2(context.contentScreenPos.x, y0),
		ImVec2(context.contentScreenPos.x + context.contentScreenSize.x, y1), IM_COL32(18, 18, 18, 255));

	// グリッド線
	float grid = context.view.gridStep;
	// 0.0f防止
	if (grid <= 0.0f) {
		grid = 0.1f;
	}

	// 開始、終了時間
	float start = std::floor(context.visibleTimeStart / grid) * grid;
	float end = std::ceil(context.visibleTimeEnd / grid) * grid;
	for (float t = start; t <= end + 0.0001f; t += grid) {

		float localX = context.view.contentPadding.x + PlayerComboTimelineHelper::TimeToLocalX(context.view, t) - context.scroll.x;
		float x = context.contentScreenPos.x + localX;

		bool isMajor = (std::fmod(std::fabs(t), context.view.majorGridStep) < 0.0001f);

		ImU32 color = isMajor ? IM_COL32(90, 90, 90, 255) : IM_COL32(45, 45, 45, 255);
		float lineTop = y0;
		float lineBot = y0 + context.view.rulerHeight;
		context.drawList->AddLine(ImVec2(x, lineTop), ImVec2(x, lineBot), color);
		if (isMajor) {

			char buf[32];
			std::snprintf(buf, sizeof(buf), "%.1f", t);
			context.drawList->AddText(ImVec2(x + 2.0f, y0 + 2.0f), IM_COL32(220, 220, 220, 255), buf);
		}
	}

	// スクラブ、現在時間線
	{
		float localX = context.view.contentPadding.x + PlayerComboTimelineHelper::TimeToLocalX(
			context.view, context.runtime.currentTime) - context.scroll.x;
		float x = context.contentScreenPos.x + localX;

		context.drawList->AddLine(ImVec2(x, y0),
			ImVec2(x, context.contentScreenPos.y + context.view.rulerHeight + context.view.trackHeight *
				static_cast<float>(tracks_.size())), IM_COL32(80, 170, 255, 255), 2.0f);

		// ルーラー部ドラッグで時間変更
		ImGui::SetCursorScreenPos(ImVec2(context.contentScreenPos.x, y0));
		ImGui::InvisibleButton("##RulerDrag", ImVec2(context.contentScreenSize.x, context.view.rulerHeight));

		if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {

			float local = (ImGui::GetIO().MousePos.x - context.contentScreenPos.x) + context.scroll.x - context.view.contentPadding.x;
			float newTime = (std::max)(0.0f, PlayerComboTimelineHelper::LocalXToTime(context.view, local));
			newTime = (std::min)(context.totalTime, PlayerComboTimelineHelper::SnapTime(newTime));
			context.runtime.currentTime = newTime;
			context.runtime.isPlaying = false;
		}
	}
}

void PlayerComboActionTimelinePanel::DrawTrackNames(const std::vector<std::unique_ptr<IPlayerComboTimelineTrack>>& tracks,
	float totalTrackHeight, float syncedScrollY) {

	// 次の段階で「左だけ別child」＋「ScrollY同期」にする場合に使う
	(void)tracks;
	(void)totalTrackHeight;
	(void)syncedScrollY;
}

void PlayerComboActionTimelinePanel::DrawTracks(PlayerComboTimelineDrawContext& context,
	const std::vector<std::unique_ptr<IPlayerComboTimelineTrack>>& tracks) {

	// ルーラーの下から開始
	float baseY = context.contentScreenPos.y + context.view.rulerHeight;

	// トラック全体グリッド縦線をトラック領域にも描く
	{
		float grid = context.view.gridStep;
		// 0.0f防止
		if (grid <= 0.0f) {
			grid = 0.1f;
		}
		// 開始、終了時間
		float start = std::floor(context.visibleTimeStart / grid) * grid;
		float end = std::ceil(context.visibleTimeEnd / grid) * grid;

		// トラック領域
		float trackTop = baseY;
		float trackBottom = baseY + context.view.trackHeight * static_cast<float>(tracks.size());
		for (float t = start; t <= end + 0.0001f; t += grid) {

			float localX = context.view.contentPadding.x + PlayerComboTimelineHelper::TimeToLocalX(context.view, t) - context.scroll.x;
			float x = context.contentScreenPos.x + localX;

			bool isMajor = (std::fmod(std::fabs(t), context.view.majorGridStep) < 0.0001f);
			ImU32 color = isMajor ? IM_COL32(65, 65, 65, 255) : IM_COL32(35, 35, 35, 255);

			context.drawList->AddLine(ImVec2(x, trackTop), ImVec2(x, trackBottom), color);
		}
	}

	// 各トラック描画
	for (size_t ti = 0; ti < tracks.size(); ++ti) {

		float y = baseY + context.view.trackHeight * static_cast<float>(ti);
		tracks[ti]->DrawTrack(context, y);

		// トラック区切り線
		context.drawList->AddLine(ImVec2(context.contentScreenPos.x, y + context.view.trackHeight),
			ImVec2(context.contentScreenPos.x + context.contentScreenSize.x, y + context.view.trackHeight),
			IM_COL32(10, 10, 10, 255));
	}
}

void PlayerComboActionTimelinePanel::HandleKeyStep(PlayerComboTimelineDrawContext& context) {

	// contentChild がフォーカスされている時だけ
	if (!ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) {
		return;
	}

	// 左右矢印でコマ送り
	// コマ戻し
	if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) {

		float t = context.runtime.currentTime - context.runtime.stepTime;
		t = (std::max)(0.0f, PlayerComboTimelineHelper::SnapTime(t));
		context.runtime.currentTime = t;
		context.runtime.isPlaying = false;
	}
	// コマ送り
	if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) {

		float t = context.runtime.currentTime + context.runtime.stepTime;
		t = (std::min)(context.totalTime, PlayerComboTimelineHelper::SnapTime(t));
		context.runtime.currentTime = t;
		context.runtime.isPlaying = false;
	}
}