#include "PlayerOperateHUD.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Json/JsonAdapter.h>
#include <Game/UI/HUD/Player/Share/PlayerShareHUD.h>

//============================================================================
//	PlayerOperateHUD classMethods
//============================================================================

namespace {

	// 各操作アイコンのインデックス
	constexpr const uint32_t kAttackIconIndex = 1;
	constexpr const uint32_t kDashIconIndex = 4;
	constexpr const uint32_t kSkilIconIndex = 7;
	constexpr const uint32_t kParryIconIndex = 10;
}

void PlayerOperateHUD::Init() {

	// 入力アイコン初期化
	operateIcons_ = std::make_unique<SakuEngine::GameObject2DArray>();
	operateIcons_->Init();
	// アイコンを追加
	// 攻撃
	operateIcons_->Add("playerIconBase", "playerIconBase", "PlayerHUD");
	operateIcons_->Add("playerAttackIcon", "playerAttackIcon", "PlayerHUD");
	operateIcons_->Add("XButton", "XButton", "PlayerHUD");
	// ダッシュ
	operateIcons_->Add("playerIconBase", "playerIconBase", "PlayerHUD");
	operateIcons_->Add("playerDashIcon", "playerDashIcon", "PlayerHUD");
	operateIcons_->Add("AButton", "AButton", "PlayerHUD");
	// スキル
	operateIcons_->Add("playerIconBase", "playerIconBase", "PlayerHUD");
	operateIcons_->Add("playerSkillIcon", "playerSkillIcon", "PlayerHUD");
	operateIcons_->Add("YButton", "YButton", "PlayerHUD");
	// パリィ
	operateIcons_->Add("playerIconBase", "playerIconBase", "PlayerHUD");
	operateIcons_->Add("playerParryIcon", "playerParryIcon", "PlayerHUD");
	operateIcons_->Add("RBButton", "RBButton", "PlayerHUD");

	// 入力示唆スプライト初期化
	for (auto& inputSuggest : inputSuggests_) {

		// 作成
		inputSuggest.sprite = std::make_unique<SakuEngine::GameObject2D>();
		inputSuggest.sprite->Init("inputSuggestRing", "inputSuggest", "PlayerHUD");

		// 最初は非表示
		inputSuggest.sprite->SetAlpha(0.0f);
	}

	// json適用
	ApplyJson();
}

void PlayerOperateHUD::StartInputSuggest() {

	// 開始
	isInputSuggestActive_ = true;
	endDelayInputSuggest_ = false;
	inputSuggestDelay_.Reset();

	// アニメーションをリセット
	for (auto& inputSuggest : inputSuggests_) {

		inputSuggest.sizeAnim.Reset();
		inputSuggest.colorAnim.Reset();
		inputSuggest.emissiveAnim.Reset();
	}
	// index0番目を発生させる
	inputSuggests_.front().sizeAnim.Start();
	inputSuggests_.front().colorAnim.Start();
	inputSuggests_.front().emissiveAnim.Start();
}

void PlayerOperateHUD::EndInputSuggest() {

	// 終了
	isInputSuggestActive_ = false;
	// アニメーションをリセット
	for (auto& inputSuggest : inputSuggests_) {

		inputSuggest.sizeAnim.Reset();
		inputSuggest.colorAnim.Reset();
		inputSuggest.emissiveAnim.Reset();

		// 非表示にする
		inputSuggest.sprite->SetAlpha(0.0f);
	}
}

void PlayerOperateHUD::SetIsDisplay(bool isDisplay) {

	// フラグに応じて設定するα値を設定する
	float alpha = isDisplay ? 1.0f : 0.0f;

	// α値を設定
	operateIcons_->SetAlpha(alpha);

	// 非表示にする場合は入力示唆も終了させる
	if (!isDisplay) {

		EndInputSuggest();
	}
}

void PlayerOperateHUD::Update() {

	// 操作アイコンの更新
	operateIcons_->Update();

	// 入力示唆の更新
	UpdateInputSuggest();
	// スキルアイコンの色更新
	UpdateSkillIconColor();
}

void PlayerOperateHUD::UpdateInputSuggest() {

	// 入力示唆が有効でない場合は処理しない
	if (!isInputSuggestActive_) {
		return;
	}

	// 遅延時間更新
	inputSuggestDelay_.Update();
	// 時間経過後にindex1番目を発生させる
	if (!endDelayInputSuggest_ && inputSuggestDelay_.IsReached()) {

		inputSuggests_.back().sizeAnim.Start();
		inputSuggests_.back().colorAnim.Start();
		inputSuggests_.back().emissiveAnim.Start();
		endDelayInputSuggest_ = true;
	}

	// アニメーション更新
	for (auto& inputSuggest : inputSuggests_) {

		// 座標設定
		inputSuggest.sprite->SetTranslation(operateIcons_->GetObjectPtr(kParryIconIndex)->GetTransform().GetWorldPos());

		// サイズ
		SakuEngine::Vector2 size = inputSuggest.sprite->GetSize();
		inputSuggest.sizeAnim.LerpValue(size);
		inputSuggest.sprite->SetSize(size);

		// α値
		SakuEngine::Color color = inputSuggest.sprite->GetColor();
		inputSuggest.colorAnim.LerpValue(color);
		inputSuggest.sprite->SetColor(color);
		inputSuggest.sprite->SetEmissionColor(inputSuggestEmissionColor_);

		// エミッシブ強度
		float emissive = inputSuggest.sprite->GetEmissiveIntensity();
		inputSuggest.emissiveAnim.LerpValue(emissive);
		inputSuggest.sprite->SetEmissiveIntensity(emissive);

		// 補間が終了次第リセットして再スタート
		if (inputSuggest.sizeAnim.IsFinished() &&
			inputSuggest.colorAnim.IsFinished() &&
			inputSuggest.emissiveAnim.IsFinished()) {

			inputSuggest.sizeAnim.Start();
			inputSuggest.colorAnim.Start();
			inputSuggest.emissiveAnim.Start();
		}
	}
}

void PlayerOperateHUD::UpdateSkillIconColor() {

	// スキルアイコンの色を更新
	// スキル値の色を取得して設定
	operateIcons_->SetColorRGB(shareHud_->GetCurrentSkillColor(), kSkilIconIndex);
	operateIcons_->GetObjectPtr(kSkilIconIndex)->SetUseVertexColor(shareHud_->IsActiveSkillColor());
	for (uint32_t i = 0; i < SakuEngine::kSpriteVertexPosNum; ++i) {

		// 頂点カラーを設定
		SakuEngine::SpriteVertexPos pos = static_cast<SakuEngine::SpriteVertexPos>(i);
		operateIcons_->GetObjectPtr(kSkilIconIndex)->SetVertexColor(pos, shareHud_->GetCurrentSkillVertexColor(pos));
	}
}

void PlayerOperateHUD::ImGui() {

	if (ImGui::BeginTabBar("PlayerOperateHUDTabBar")) {
		if (ImGui::BeginTabItem("Config")) {
			if (ImGui::Button("SaveJson")) {

				SaveJson();
			}
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("OperateIcon")) {

			operateIcons_->ImGui();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("InputSuggest")) {

			ImGui::Text(std::format("endDelayInputSuggest: {}", endDelayInputSuggest_).c_str());
			if (ImGui::Button("Start")) {

				StartInputSuggest();
			}
			ImGui::SameLine();
			if (ImGui::Button("End")) {

				EndInputSuggest();
			}
			ImGui::Separator();

			ImGui::ColorEdit3("inputSuggestEmissionColor", &inputSuggestEmissionColor_.x);
			inputSuggestDelay_.ImGui("InputSuggestDelay");

			if (ImGui::CollapsingHeader("Front")) {

				inputSuggests_.front().sizeAnim.ImGui("FrontSizeAnim", false);
				inputSuggests_.front().colorAnim.ImGui("FrontcolorAnim", false);
				inputSuggests_.front().emissiveAnim.ImGui("FrontEmissiveAnim", false);
			}
			if (ImGui::CollapsingHeader("Back")) {

				inputSuggests_.back().sizeAnim.ImGui("BackSizeAnim", false);
				inputSuggests_.back().colorAnim.ImGui("BackcolorAnim", false);
				inputSuggests_.back().emissiveAnim.ImGui("BackEmissiveAnim", false);
			}
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
}

void PlayerOperateHUD::ApplyJson() {

	Json data;
	if (!SakuEngine::JsonAdapter::LoadCheck("Player/operateHUDParameters.json", data)) {
		return;
	}

	inputSuggests_.front().sizeAnim.FromJson(data.value("inputSuggestFrontSizeAnim", Json()));
	inputSuggests_.front().colorAnim.FromJson(data.value("inputSuggestFrontcolorAnim", Json()));
	inputSuggests_.front().emissiveAnim.FromJson(data.value("inputSuggestFrontEmissiveAnim", Json()));
	inputSuggests_.back().sizeAnim.FromJson(data.value("inputSuggestBackSizeAnim", Json()));
	inputSuggests_.back().colorAnim.FromJson(data.value("inputSuggestBackcolorAnim", Json()));
	inputSuggests_.back().emissiveAnim.FromJson(data.value("inputSuggestBackEmissiveAnim", Json()));
	inputSuggestDelay_.FromJson(data.value("inputSuggestDelay", Json()));
	inputSuggestEmissionColor_ = SakuEngine::Vector3::FromJson(data.value("inputSuggestEmissionColor", Json()));

	operateIcons_->FromJson(data.value("operateIcons", Json()));
}

void PlayerOperateHUD::SaveJson() {

	Json data;

	inputSuggests_.front().sizeAnim.ToJson(data["inputSuggestFrontSizeAnim"]);
	inputSuggests_.front().colorAnim.ToJson(data["inputSuggestFrontcolorAnim"]);
	inputSuggests_.front().emissiveAnim.ToJson(data["inputSuggestFrontEmissiveAnim"]);
	inputSuggests_.back().sizeAnim.ToJson(data["inputSuggestBackSizeAnim"]);
	inputSuggests_.back().colorAnim.ToJson(data["inputSuggestBackcolorAnim"]);
	inputSuggests_.back().emissiveAnim.ToJson(data["inputSuggestBackEmissiveAnim"]);
	inputSuggestDelay_.ToJson(data["inputSuggestDelay"]);
	data["inputSuggestEmissionColor"] = inputSuggestEmissionColor_.ToJson();

	operateIcons_->ToJson(data["operateIcons"]);

	SakuEngine::JsonAdapter::Save("Player/operateHUDParameters.json", data);
}