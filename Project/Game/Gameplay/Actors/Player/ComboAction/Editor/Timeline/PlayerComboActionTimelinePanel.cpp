#include "PlayerComboActionTimelinePanel.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Timer/GameTimer.h>
#include <Engine/Input/Input.h> 
#include <Engine/Config.h>

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

	// 初期設定
	isEditParam_ = false;
}

void PlayerComboActionTimelinePanel::EditParam() {

	if (!isEditParam_) {
		return;
	}

	ImGui::Begin("Timeline Editor Param");

	ImGui::DragFloat("pixelsPerSecond", &view_.pixelsPerSecond, 0.1f);
	ImGui::DragFloat("dragsSensitivity", &view_.dragsSensitivity, 0.1f);
	ImGui::DragFloat("rulerFontScale", &view_.rulerFontScale, 0.01f);
	ImGui::DragFloat("gridStep", &view_.gridStep, 0.01f);
	ImGui::DragFloat("majorGridStep", &view_.majorGridStep, 0.1f);
	ImGui::DragFloat("trackNameWidth", &view_.trackNameWidth, 0.1f);
	ImGui::DragFloat("trackHeight", &view_.trackHeight, 0.1f);
	ImGui::DragFloat("rulerHeight", &view_.rulerHeight, 0.1f);
	ImGui::DragFloat2("contentPadding", &view_.contentPadding.x, 0.1f);

	ImGui::End();
}

void PlayerComboActionTimelinePanel::Draw(PlayerComboActionModel& model,
	PlayerComboActionEditorSelection& select) {

	EditParam();

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

	ImGuiTableFlags tableFlags =
		ImGuiTableFlags_BordersInnerV |
		ImGuiTableFlags_BordersOuterV |
		ImGuiTableFlags_SizingFixedFit;

	if (ImGui::BeginTable("##timeline_table", 2, tableFlags)) {

		ImGui::TableSetupColumn("##track_name_col", ImGuiTableColumnFlags_WidthFixed, view_.trackNameWidth);
		ImGui::TableSetupColumn("##timeline_col", ImGuiTableColumnFlags_WidthStretch);

		ImGui::TableNextRow();

		//============================================================================================
		// タイムライン
		//============================================================================================
		ImGui::TableSetColumnIndex(1);

		ImGui::BeginChild("##timeline_content", ImVec2(0.0f, 0.0f), true, ImGuiWindowFlags_HorizontalScrollbar);

		// スクロール
		context.scroll.x = ImGui::GetScrollX();
		context.scroll.y = ImGui::GetScrollY();
		float syncedScrollY = context.scroll.y;

		// content描画領域
		context.contentScreenPos = ImGui::GetCursorScreenPos();
		context.contentScreenSize = ImGui::GetContentRegionAvail();
		context.drawList = ImGui::GetWindowDrawList();

		// ルーラー上でのみズーム
		const bool onRuler = ImGui::GetIO().MousePos.y >= context.contentScreenPos.y &&
			ImGui::GetIO().MousePos.y <= context.contentScreenPos.y + context.view.rulerHeight;

		float wheel = SakuEngine::Input::GetInstance()->GetMouseWheel();
		if (onRuler && wheel != 0.0f) {

			const float oldPPS = context.view.pixelsPerSecond;
			const float zoomBase = 1.1f; // 1ノッチあたりの倍率
			float newPPS = oldPPS * std::pow(zoomBase, wheel);

			// クランプ（お好みで）
			newPPS = std::clamp(newPPS, 20.0f, 1200.0f);

			if (std::fabs(newPPS - oldPPS) > 0.001f) {

				// マウス位置を基準にその時間がズーム後も同じ位置に来るようにscrollを補正
				float mouseXIn = ImGui::GetIO().MousePos.x - context.contentScreenPos.x;
				mouseXIn = std::clamp(mouseXIn, 0.0f, context.contentScreenSize.x);

				// ズーム前のマウス下の時間
				float anchorLocal = mouseXIn + context.scroll.x - context.view.contentPadding.x;
				float anchorTime = PlayerComboTimelineHelper::LocalXToTime(context.view, anchorLocal);
				anchorTime = std::clamp(anchorTime, 0.0f, context.totalTime);

				// ズーム適用
				context.view.pixelsPerSecond = newPPS;

				// 新しいscroll
				float newScrollX = context.view.contentPadding.x +
					PlayerComboTimelineHelper::TimeToLocalX(context.view, anchorTime) - mouseXIn;

				// スクロール範囲にクランプ
				float contentWidth = context.view.contentPadding.x * 2.0f +
					PlayerComboTimelineHelper::TimeToLocalX(context.view, context.totalTime);

				// 最大スクロール位置
				float maxScrollX = (std::max)(0.0f, contentWidth - context.contentScreenSize.x);
				newScrollX = std::clamp(newScrollX, 0.0f, maxScrollX);

				// スクロール適用
				ImGui::SetScrollX(newScrollX);
				context.scroll.x = newScrollX;
			}
		}

		// 表示範囲(時間)を更新
		{
			float leftLocal = context.scroll.x - context.view.contentPadding.x;
			float rightLocal = context.scroll.x + context.contentScreenSize.x - context.view.contentPadding.x;
			// 0未満にならないように制限
			context.visibleTimeStart = (std::max)(0.0f,
				PlayerComboTimelineHelper::LocalXToTime(context.view, leftLocal));
			// totalTimeを超えないように
			context.visibleTimeEnd = (std::min)(context.totalTime,
				PlayerComboTimelineHelper::LocalXToTime(context.view, rightLocal));
		}

		// 全体の領域確保
		float totalTrackHeight = context.view.rulerHeight + context.view.trackHeight * static_cast<float>(tracks_.size());
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
		// 左、トラック名
		//============================================================================================
		ImGui::TableSetColumnIndex(0);

		ImGuiWindowFlags nameFlags =
			ImGuiWindowFlags_NoScrollbar |
			ImGuiWindowFlags_NoScrollWithMouse;

		ImGui::BeginChild("##timeline_track_names", ImVec2(0.0f, 0.0f), true, nameFlags);
		ImGui::SetScrollY(syncedScrollY);

		DrawTrackNames(tracks_, syncedScrollY);

		ImGui::EndChild();

		ImGui::EndTable();
	}
}

void PlayerComboActionTimelinePanel::DrawRuntime(PlayerComboTimelineDrawContext& context) {

	ImGui::Checkbox("Edit Parameters", &isEditParam_);
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

	// グリッド
	float grid = context.view.gridStep;
	if (grid <= 0.0f) {
		grid = 0.1f;
	}

	float major = context.view.majorGridStep;
	if (major <= 0.0f) {
		major = 1.0f;
	}

	// ルーラー文字サイズ
	ImFont* font = ImGui::GetFont();
	float fontSize = ImGui::GetFontSize() * (std::max)(0.5f, context.view.rulerFontScale);

	// 表示範囲
	int32_t iStart = static_cast<int32_t>(std::floor(context.visibleTimeStart / grid));
	int32_t iEnd = static_cast<int32_t>(std::ceil(context.visibleTimeEnd / grid));

	// 左右の描画境界
	float boundL = context.contentScreenPos.x + 2.0f;
	float boundR = context.contentScreenPos.x + context.contentScreenSize.x - 2.0f;

	// ラベル重なり防止
	float lastLabelRight = -FLT_MAX;
	const float labelPadding = 6.0f;

	for (int32_t i = iStart; i <= iEnd; ++i) {

		float t = static_cast<float>(i) * grid;

		float localX = context.view.contentPadding.x + PlayerComboTimelineHelper::TimeToLocalX(context.view, t) - context.scroll.x;
		float x = context.contentScreenPos.x + localX;

		// 線
		{
			// major判定roundを使って誤差に強くする
			float majorIndexF = std::round(t / major);
			float majorTime = majorIndexF * major;
			bool isMajor = (std::fabs(t - majorTime) <= (grid * 0.25f + Config::kEpsilon));

			ImU32 color = isMajor ? IM_COL32(90, 90, 90, 255) : IM_COL32(45, 45, 45, 255);
			context.drawList->AddLine(ImVec2(x, y0), ImVec2(x, y1), color);

			// majorラベル
			if (isMajor) {

				char buf[32];
				std::snprintf(buf, sizeof(buf), "%.1f", t);

				ImVec2 textSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, buf);

				// 線の中心に文字が来るように配置
				float tx = x - (textSize.x * 0.5f);
				float ty = y0 + (context.view.rulerHeight - textSize.y) * 0.5f;

				// 画面外にはみ出すものは描かない
				if (tx < boundL || (tx + textSize.x) > boundR) {
					continue;
				}

				// ラベルが重なるなら描かない
				if (tx <= lastLabelRight + labelPadding) {
					continue;
				}

				context.drawList->AddText(font, fontSize, ImVec2(tx, ty), IM_COL32(220, 220, 220, 255), buf);
				lastLabelRight = tx + textSize.x;
			}
		}
	}

	// スクラブ、現在時間線
	{
		float localX = context.view.contentPadding.x + PlayerComboTimelineHelper::TimeToLocalX(
			context.view, context.runtime.currentTime) - context.scroll.x;
		float x = context.contentScreenPos.x + localX;

		float totalH = context.view.rulerHeight + context.view.trackHeight * static_cast<float>(tracks_.size());
		context.drawList->AddLine(ImVec2(x, y0),
			ImVec2(x, context.contentScreenPos.y + totalH), IM_COL32(80, 170, 255, 255), 2.0f);

		// ルーラー部ドラッグで時間変更
		ImGui::SetCursorScreenPos(ImVec2(context.contentScreenPos.x, y0));
		ImGui::InvisibleButton("##RulerDrag",
			ImVec2((std::max)(1.0f, context.contentScreenSize.x), (std::max)(1.0f, context.view.rulerHeight)));

		if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {

			float local = (ImGui::GetIO().MousePos.x - context.contentScreenPos.x) + context.scroll.x - context.view.contentPadding.x;
			float newTime = (std::max)(0.0f, PlayerComboTimelineHelper::LocalXToTime(context.view, local));
			newTime = (std::min)(context.totalTime, PlayerComboTimelineHelper::SnapTime(newTime));
			context.runtime.currentTime = newTime;
			context.runtime.isPlaying = false;
		}
	}
}

void PlayerComboActionTimelinePanel::DrawTrackNames(
	const std::vector<std::unique_ptr<IPlayerComboTimelineTrack>>& tracks, float syncedScrollY) {

	(void)syncedScrollY;

	ImDrawList* drawList = ImGui::GetWindowDrawList();

	// この子ウィンドウの描画開始位置
	ImVec2 base = ImGui::GetCursorScreenPos();
	float width = ImGui::GetContentRegionAvail().x;

	// タイムラインと同じ総高さを確保
	float totalHeight = view_.rulerHeight + view_.trackHeight * static_cast<float>(tracks.size());
	ImGui::Dummy(ImVec2(width, totalHeight));

	//============================================================================================================
	// Tracks ヘッダー
	//============================================================================================================

	{
		ImVec2 p0 = base;
		ImVec2 p1 = ImVec2(base.x + width, base.y + view_.rulerHeight);

		drawList->AddRectFilled(p0, p1, IM_COL32(18, 18, 18, 255));
		drawList->AddRect(p0, p1, IM_COL32(10, 10, 10, 255));

		const char* text = "Tracks";
		ImVec2 ts = ImGui::CalcTextSize(text);
		ImVec2 tp = ImVec2(p0.x + (width - ts.x) * 0.5f, p0.y + (view_.rulerHeight - ts.y) * 0.5f);
		drawList->AddText(tp, IM_COL32(230, 230, 230, 255), text);
	}

	//============================================================================================================
	// 各トラック名
	//============================================================================================================

	for (size_t ti = 0; ti < tracks.size(); ++ti) {

		float y = base.y + view_.rulerHeight + view_.trackHeight * static_cast<float>(ti);

		ImVec2 p0 = ImVec2(base.x, y);
		ImVec2 p1 = ImVec2(base.x + width, y + view_.trackHeight);

		ImU32 bg = (ti % 2 == 0) ? IM_COL32(25, 25, 25, 255) : IM_COL32(22, 22, 22, 255);
		drawList->AddRectFilled(p0, p1, bg);
		drawList->AddRect(p0, p1, IM_COL32(10, 10, 10, 255));

		const char* name = tracks[ti]->TrackName();
		ImVec2 ts = ImGui::CalcTextSize(name);
		ImVec2 tp = ImVec2(p0.x + (width - ts.x) * 0.5f, p0.y + (view_.trackHeight - ts.y) * 0.5f);
		drawList->AddText(tp, IM_COL32(230, 230, 230, 255), name);
	}
}

void PlayerComboActionTimelinePanel::DrawTracks(PlayerComboTimelineDrawContext& context,
	const std::vector<std::unique_ptr<IPlayerComboTimelineTrack>>& tracks) {

	// ルーラーの下から開始
	float baseY = context.contentScreenPos.y + context.view.rulerHeight;

	// 各トラック描画
	for (size_t ti = 0; ti < tracks.size(); ++ti) {

		float y = baseY + context.view.trackHeight * static_cast<float>(ti);

		// 背景 → グリッド → 要素の順に描画
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
	if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) {

		float t = context.runtime.currentTime - context.runtime.stepTime;
		t = (std::max)(0.0f, PlayerComboTimelineHelper::SnapTime(t));
		context.runtime.currentTime = t;
		context.runtime.isPlaying = false;
	}

	if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) {

		float t = context.runtime.currentTime + context.runtime.stepTime;
		t = (std::min)(context.totalTime, PlayerComboTimelineHelper::SnapTime(t));
		context.runtime.currentTime = t;
		context.runtime.isPlaying = false;
	}
}