#include "PlayerHUD.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Input/Input.h>
#include <Game/Gameplay/Camera/FollowCamera/FollowCamera.h>
#include <Game/Gameplay/Actors/Player/Entity/Player.h>
#include <Game/Gameplay/Actors/Enemies/Boss/Entity/BossEnemy.h>
#include <Engine/Utility/Json/JsonAdapter.h>

//============================================================================
//	PlayerHUD classMethods
//============================================================================

namespace {

	// 各操作アイコンのインデックス
	constexpr const uint32_t kAttackIconIndex = 1;
	constexpr const uint32_t kDashIconIndex = 4;
	constexpr const uint32_t kSkilIconIndex = 7;
	constexpr const uint32_t kParryIconIndex = 10;
}

void PlayerHUD::InitSprite() {

	// HP背景
	hpBackground_ = std::make_unique<SakuEngine::GameObject2D>();
	hpBackground_->Init("playerHPBackground", "hpBackground", "PlayerHUD");

	// HP残量
	hpBar_ = std::make_unique<GameHPBar>();
	hpBar_->Init("playerHPBar", "whiteAlphaGradation_1", "hpBar", "PlayerHUD");

	// スキル値
	skilBar_ = std::make_unique<GameHPBar>();
	skilBar_->Init("playerSkilBar", "whiteAlphaGradation_1", "destroyBar", "PlayerHUD");

	// 名前文字表示
	nameText_ = std::make_unique<SakuEngine::GameObject2D>();
	nameText_->Init("playerName", "playerName", "PlayerHUD");

	// ダメージ表示
	damageDisplay_ = std::make_unique<GameDisplayDamage>();
	damageDisplay_->Init("playerDamageNumber", "BossEnemyHUD", 4, 3);

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

	// スキルP閾値表示
	skillThreshold_ = std::make_unique<SakuEngine::GameObject2DArray>();
	skillThreshold_->Init();
	// スキルP閾値のスプライトを追加
	skillThreshold_->Add("barThresholdFrame", "skillThreshold", "PlayerHUD"); // 枠
	skillThreshold_->Add("barThreshold", "skillThreshold", "PlayerHUD");      // 中の色

	// input状態を取得
	inputType_ = SakuEngine::Input::GetInstance()->GetType();
	preInputType_ = inputType_;
}

void PlayerHUD::Init() {

	// sprite初期化
	InitSprite();

	// json適応
	ApplyJson();
}

void PlayerHUD::SetDamage(int damage) {

	// ダメージを設定
	damageDisplay_->SetDamage(damage);
}

void PlayerHUD::SetDisable() {

	isDisable_ = true;

	// 全てのα値を0.0fにし、表示を消す
	hpBackground_->SetAlpha(0.0f);
	hpBar_->SetAlpha(0.0f);
	skilBar_->SetAlpha(0.0f);
	nameText_->SetAlpha(0.0f);
	damageDisplay_->SetAlpha(0.0f);
	skillThreshold_->SetAlpha(0.0f);
}

void PlayerHUD::SetValid() {

	// 再度表示
	returnVaild_ = true;
}

void PlayerHUD::Update(const Player& player) {

	// input状態を取得
	inputType_ = SakuEngine::Input::GetInstance()->GetType();

	// sprite更新
	UpdateSprite(player);

	// alpha値を表示切替で更新
	UpdateAlpha();
}

void PlayerHUD::UpdateSprite(const Player& player) {

	// バーの更新
	UpdateBar();

	// ダメージ表記の更新
	damageDisplay_->Update(player, *followCamera_);

	// 入力示唆の更新
	UpdateInputSuggest();
	// スキル閾値表示の更新
	UpdateSkillThreshold();
	skillThreshold_->Update();

	// 操作アイコンの更新
	operateIcons_->Update();
}

void PlayerHUD::UpdateAlpha() {

	// 無効状態でない(表示中)、元に戻す必要がないときは処理しない
	if (!isDisable_ || !returnVaild_) {
		return;
	}

	// 時間を進める
	returnAlphaTimer_ += SakuEngine::GameTimer::GetDeltaTime();
	float alpha = returnAlphaTimer_ / returnAlphaTime_;
	alpha = EasedValue(returnAlphaEasingType_, alpha);
	alpha = std::clamp(alpha, 0.0f, 1.0f);

	// alpha値を補間して設定
	hpBackground_->SetAlpha(alpha);
	hpBar_->SetAlpha(alpha);
	skilBar_->SetAlpha(alpha);
	nameText_->SetAlpha(alpha);
	damageDisplay_->SetAlpha(alpha);
	skillThreshold_->SetAlpha(alpha);

	if (returnAlphaTime_ < returnAlphaTimer_) {

		// 念のため1.0fに固定
		hpBackground_->SetAlpha(1.0f);
		hpBar_->SetAlpha(1.0f);
		skilBar_->SetAlpha(1.0f);
		nameText_->SetAlpha(1.0f);
		damageDisplay_->SetAlpha(1.0f);
		skillThreshold_->SetAlpha(1.0f);

		// 元に戻ったので処理終了
		returnAlphaTimer_ = 0.0f;

		isDisable_ = false;
		returnVaild_ = false;
	}
}

void PlayerHUD::StartInputSuggest() {

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

void PlayerHUD::EndInputSuggest() {

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

void PlayerHUD::UpdateBar() {

	// 頂点カラーアニメーション
	hpBarColorAnim_.Update(*hpBar_.get());
	skilBarColorAnim_.Update(*skilBar_.get());

	// スキルアイコンの色を合わせる
	using namespace SakuEngine;
	for (const auto& vertexChar : EnumAdapter<SpriteVertexPos>::GetEnumArray()) {

		SpriteVertexPos vertex = EnumAdapter<SpriteVertexPos>::FromString(vertexChar).value();
		operateIcons_->GetObjectPtr(kSkilIconIndex)->SetVertexColor(vertex, skilBar_->GetVertexColor(vertex));
	}

	// HP残量を更新
	hpBar_->Update(stats_.currentHP, stats_.maxHP, true);
	// スキル値を更新
	skilBar_->Update(stats_.currentSkilPoint, stats_.maxSkilPoint, true);
}

void PlayerHUD::UpdateInputSuggest() {

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

void PlayerHUD::UpdateSkillThreshold() {

	// スキルP閾値表示の更新
	skillThreshold_->SetTranslation(skilBar_->GetThresholdPos(stats_.skilCost));

	// 無効時の色設定
	// スキルPが足りない場合は灰色にする
	// alphaを記録
	float alpha = skillThreshold_->GetColor().a;
	disableSkillThresholdColor_.a = alpha;
	enableSkillThresholdColor_.a = alpha;
	if (stats_.currentSkilPoint < stats_.skilCost) {

		skillThreshold_->SetColor(disableSkillThresholdColor_);
		skilBar_->SetColor(disableSkillThresholdColor_);
		operateIcons_->GetObjectPtr(kSkilIconIndex)->SetColor(disableSkillThresholdColor_);
		operateIcons_->GetObjectPtr(kSkilIconIndex)->SetUseVertexColor(false);

		// 頂点カラーアニメーション停止
		skilBarColorAnim_.Reset();
	} else {

		skillThreshold_->SetColor(enableSkillThresholdColor_);
		skilBar_->SetColor(SakuEngine::Color::White(skilBar_->GetColor().a));
		operateIcons_->GetObjectPtr(kSkilIconIndex)->SetColor(SakuEngine::Color::White(alpha));
		operateIcons_->GetObjectPtr(kSkilIconIndex)->SetUseVertexColor(true);

		// 頂点カラーアニメーション開始
		skilBarColorAnim_.Start();
	}
}

void PlayerHUD::ChangeAllOperateSprite() {

	// 変更がない場合は処理しない
	if (preInputType_ == inputType_) {
		return;
	}

	inputType_ = InputType::GamePad;
	preInputType_ = InputType::GamePad;

	// 前回フレームの入力状態を保存
	preInputType_ = inputType_;
}

void PlayerHUD::ImGui() {

	if (ImGui::Button("SaveJson...hudParameter.json")) {

		SaveJson();
	}

	if (ImGui::BeginTabBar("PlayerHUDTabBar")) {
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

		if (ImGui::BeginTabItem("Setting")) {

			if (hpBackgroundParameter_.ImGui("HPBackground")) {

				hpBackground_->SetTranslation(hpBackgroundParameter_.translation);
			}

			if (hpBarParameter_.ImGui("HPBar")) {

				hpBar_->SetTranslation(hpBarParameter_.translation);
			}

			if (skilBarParameter_.ImGui("SkilBar")) {

				skilBar_->SetTranslation(skilBarParameter_.translation);
			}

			if (nameTextParameter_.ImGui("NameText")) {

				nameText_->SetTranslation(nameTextParameter_.translation);
			}

			damageDisplay_->ImGui();

			ImGui::Separator();

			if (isDisable_) {
				if (ImGui::Button("Vaild")) {

					SetValid();
				}
			} else {
				if (ImGui::Button("Disable")) {

					SetDisable();
				}
			}

			ImGui::Text("returnAlphaTimer / returnAlphaTime: %f", returnAlphaTimer_ / returnAlphaTime_);

			ImGui::DragFloat("returnAlphaTime", &returnAlphaTime_, 0.01f);
			Easing::SelectEasingType(returnAlphaEasingType_);
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("ColorAnim")) {

			if (ImGui::CollapsingHeader("HPBarColorAnim")) {

				hpBarColorAnim_.ImGui("HPBarColorAnim");
			}
			if (ImGui::CollapsingHeader("SkilBarColorAnim")) {

				skilBarColorAnim_.ImGui("SkilBarColorAnim");
			}
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("SkillThreshold")) {

			ImGui::ColorEdit4("enableSkillThresholdColor", &enableSkillThresholdColor_.r);
			ImGui::ColorEdit4("disableSkillThresholdColor", &disableSkillThresholdColor_.r);
			ImGui::Separator();

			skillThreshold_->ImGui();
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
}

void PlayerHUD::ApplyJson() {

	Json data;
	if (!SakuEngine::JsonAdapter::LoadCheck("Player/hudParameter.json", data)) {
		return;
	}

	hpBackgroundParameter_.ApplyJson(data["hpBackground"]);
	GameCommon::SetInitParameter(*hpBackground_, hpBackgroundParameter_);

	hpBarParameter_.ApplyJson(data["hpBar"]);
	GameCommon::SetInitParameter(*hpBar_, hpBarParameter_);
	hpBarColorAnim_.FromJson(data.value("hpBarColorAnim", Json()));
	hpBarColorAnim_.Start();

	skilBarParameter_.ApplyJson(data["skilBar"]);
	GameCommon::SetInitParameter(*skilBar_, skilBarParameter_);
	skilBarColorAnim_.FromJson(data.value("skilBarColorAnim", Json()));
	skilBarColorAnim_.Start();

	nameTextParameter_.ApplyJson(data["nameText"]);
	GameCommon::SetInitParameter(*nameText_, nameTextParameter_);

	damageDisplay_->ApplyJson(data);

	returnAlphaTime_ = SakuEngine::JsonAdapter::GetValue<float>(data, "returnAlphaTime_");
	returnAlphaEasingType_ = static_cast<EasingType>(
		SakuEngine::JsonAdapter::GetValue<int>(data, "returnAlphaEasingType_"));

	inputSuggests_.front().sizeAnim.FromJson(data.value("inputSuggestFrontSizeAnim", Json()));
	inputSuggests_.front().colorAnim.FromJson(data.value("inputSuggestFrontcolorAnim", Json()));
	inputSuggests_.front().emissiveAnim.FromJson(data.value("inputSuggestFrontEmissiveAnim", Json()));
	inputSuggests_.back().sizeAnim.FromJson(data.value("inputSuggestBackSizeAnim", Json()));
	inputSuggests_.back().colorAnim.FromJson(data.value("inputSuggestBackcolorAnim", Json()));
	inputSuggests_.back().emissiveAnim.FromJson(data.value("inputSuggestBackEmissiveAnim", Json()));
	inputSuggestDelay_.FromJson(data.value("inputSuggestDelay", Json()));
	inputSuggestEmissionColor_ = SakuEngine::Vector3::FromJson(data.value("inputSuggestEmissionColor", Json()));

	disableSkillThresholdColor_ = SakuEngine::Color::FromJson(data.value("disableSkillThresholdColor", Json()));
	skillThreshold_->FromJson(data.value("skillThreshold", Json()));
	// 色を記録しておく
	enableSkillThresholdColor_ = skillThreshold_->GetColor();

	operateIcons_->FromJson(data.value("operateIcons", Json()));
}

void PlayerHUD::SaveJson() {

	Json data;

	hpBackgroundParameter_.SaveJson(data["hpBackground"]);
	hpBarParameter_.SaveJson(data["hpBar"]);
	hpBarColorAnim_.ToJson(data["hpBarColorAnim"]);
	skilBarParameter_.SaveJson(data["skilBar"]);
	skilBarColorAnim_.ToJson(data["skilBarColorAnim"]);
	nameTextParameter_.SaveJson(data["nameText"]);

	damageDisplay_->SaveJson(data);

	data["returnAlphaTime_"] = returnAlphaTime_;
	data["returnAlphaEasingType_"] = static_cast<int>(returnAlphaEasingType_);

	inputSuggests_.front().sizeAnim.ToJson(data["inputSuggestFrontSizeAnim"]);
	inputSuggests_.front().colorAnim.ToJson(data["inputSuggestFrontcolorAnim"]);
	inputSuggests_.front().emissiveAnim.ToJson(data["inputSuggestFrontEmissiveAnim"]);
	inputSuggests_.back().sizeAnim.ToJson(data["inputSuggestBackSizeAnim"]);
	inputSuggests_.back().colorAnim.ToJson(data["inputSuggestBackcolorAnim"]);
	inputSuggests_.back().emissiveAnim.ToJson(data["inputSuggestBackEmissiveAnim"]);
	inputSuggestDelay_.ToJson(data["inputSuggestDelay"]);
	data["inputSuggestEmissionColor"] = inputSuggestEmissionColor_.ToJson();

	data["disableSkillThresholdColor"] = disableSkillThresholdColor_.ToJson();
	skillThreshold_->ToJson(data["skillThreshold"]);

	operateIcons_->ToJson(data["operateIcons"]);

	SakuEngine::JsonAdapter::Save("Player/hudParameter.json", data);
}