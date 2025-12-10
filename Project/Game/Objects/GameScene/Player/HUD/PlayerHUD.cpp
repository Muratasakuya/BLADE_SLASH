#include "PlayerHUD.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Input/Input.h>
#include <Game/Camera/Follow/FollowCamera.h>
#include <Game/Objects/GameScene/Player/Entity/Player.h>
#include <Game/Objects/GameScene/Enemy/Boss/Entity/BossEnemy.h>
#include <Engine/Utility/Json/JsonAdapter.h>

//============================================================================
//	PlayerHUD classMethods
//============================================================================

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

	// キーボード操作とパッド操作のtextureの名前を格納する
	std::unordered_map<InputType, std::string> dynamicTextures{};

	// 攻撃
	dynamicTextures[InputType::Keyboard] = "leftMouseClick";
	dynamicTextures[InputType::GamePad] = "XButton";
	attack_.Init(0, "attackIcon", dynamicTextures);

	// ダッシュ
	dynamicTextures[InputType::Keyboard] = "rightMouseClick";
	dynamicTextures[InputType::GamePad] = "AButton";
	dash_.Init(1, "dashIcon", dynamicTextures);

	// スキル
	dynamicTextures[InputType::Keyboard] = "EButton";
	dynamicTextures[InputType::GamePad] = "YButton";
	skil_.Init(2, "skilIcon", dynamicTextures);

	// パリィ
	dynamicTextures[InputType::Keyboard] = "spaceButton";
	dynamicTextures[InputType::GamePad] = "LBAndRBButton";
	parry_.Init(3, "parryIcon", dynamicTextures);

	// ダメージ表示
	damageDisplay_ = std::make_unique<GameDisplayDamage>();
	damageDisplay_->Init("playerDamageNumber", "BossEnemyHUD", 4, 3);

	// input状態を取得
	inputType_ = SakuEngine::Input::GetInstance()->GetType();
	preInputType_ = inputType_;

	// 最初の表示状態を設定
	attack_.ChangeDynamicSprite(inputType_);
	dash_.ChangeDynamicSprite(inputType_);
	skil_.ChangeDynamicSprite(inputType_);
	parry_.ChangeDynamicSprite(inputType_);

	// 入力示唆スプライト初期化
	for (auto& inputSuggest : inputSuggests_) {

		// 作成
		inputSuggest.sprite = std::make_unique<SakuEngine::GameObject2D>();
		inputSuggest.sprite->Init("inputSuggestRing", "inputSuggest", "PlayerHUD");

		// 最初は非表示
		inputSuggest.sprite->SetAlpha(0.0f);
	}
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
	attack_.SetAlpha(inputType_, 0.0f);
	dash_.SetAlpha(inputType_, 0.0f);
	skil_.SetAlpha(inputType_, 0.0f);
	parry_.SetAlpha(inputType_, 0.0f);
	damageDisplay_->SetAlpha(0.0f);
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

	// HP残量を更新
	hpBar_->Update(stats_.currentHP, stats_.maxHP, true);
	// スキル値を更新
	skilBar_->Update(stats_.currentSkilPoint, stats_.maxSkilPoint, true);

	// ダメージ表記の更新
	damageDisplay_->Update(player, *followCamera_);

	// 入力示唆の更新
	UpdateInputSuggest();
	// 入力リアクションアニメーションの更新
	UpdateInputReactAnim();

	// 入力状態に応じて表示を切り替える
	ChangeAllOperateSprite();
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
	attack_.SetAlpha(inputType_, alpha);
	dash_.SetAlpha(inputType_, alpha);
	skil_.SetAlpha(inputType_, alpha);
	parry_.SetAlpha(inputType_, alpha);
	damageDisplay_->SetAlpha(alpha);

	if (returnAlphaTime_ < returnAlphaTimer_) {

		// 念のため1.0fに固定
		hpBackground_->SetAlpha(1.0f);
		hpBar_->SetAlpha(1.0f);
		skilBar_->SetAlpha(1.0f);
		nameText_->SetAlpha(1.0f);
		attack_.SetAlpha(inputType_, 1.0f);
		dash_.SetAlpha(inputType_, 1.0f);
		skil_.SetAlpha(inputType_, 1.0f);
		parry_.SetAlpha(inputType_, 1.0f);
		damageDisplay_->SetAlpha(1.0f);

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

void PlayerHUD::StartInputReactAnim(PlayerState state) {

	// 状態に応じて処理
	inputReactState_ = state;
	// 無効状態なら処理しない
	if (inputReactState_ == PlayerState::SkilAttack) {

		// スキルUIをアクティブにしてアニメーションさせる
		skil_.isActiveInput = true;
		// パリィ入力のUIサイズを元に戻す
		// 入力画像は2倍する
		parry_.SetSize(staticSpriteSize_, SakuEngine::Vector2(dynamicSpriteSize_.x * 2.0f, dynamicSpriteSize_.y));
		// 攻撃入力のUIサイズを元に戻す
		attack_.SetSize(staticSpriteSize_, dynamicSpriteSize_);
	} else if (inputReactState_ == PlayerState::Parry) {

		// パリィUIをアクティブにしてアニメーションさせる
		parry_.isActiveInput = true;
		// スキル入力のUIサイズを元に戻す
		skil_.SetSize(staticSpriteSize_, dynamicSpriteSize_);
		// 攻撃入力のUIサイズを元に戻す
		attack_.SetSize(staticSpriteSize_, dynamicSpriteSize_);
	} else if (
		inputReactState_ == PlayerState::Attack_1st ||
		inputReactState_ == PlayerState::Attack_2nd ||
		inputReactState_ == PlayerState::Attack_3rd ||
		inputReactState_ == PlayerState::Attack_4th) {

		// パリィUIをアクティブにしてアニメーションさせる
		attack_.isActiveInput = true;
		// スキル入力のUIサイズを元に戻す
		skil_.SetSize(staticSpriteSize_, dynamicSpriteSize_);
		// パリィ入力のUIサイズを元に戻す
		// 入力画像は2倍する
		parry_.SetSize(staticSpriteSize_, SakuEngine::Vector2(dynamicSpriteSize_.x * 2.0f, dynamicSpriteSize_.y));
	} else {
		// それ以外の状態なら処理しない
		return;
	}

	// 入力リアクションアニメーション開始
	inputReactSizeAnim_.Start();
	inputReactColorAnim_.Start();
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

void PlayerHUD::UpdateInputReactAnim() {

	// 無効状態なら処理しない
	if (!parry_.isActiveInput && !skil_.isActiveInput && !attack_.isActiveInput) {
		return;
	}

	SakuEngine::Vector2 size{};
	SakuEngine::Color color{};
	switch (inputReactState_) {
	case PlayerState::SkilAttack: {

		// スキルUIをアニメーションさせる
		// サイズ
		size = skil_.staticSprite->GetSize();
		inputReactSizeAnim_.LerpValue(size);
		skil_.staticSprite->SetSize(size);
		// 色
		color = skil_.staticSprite->GetColor();
		inputReactColorAnim_.LerpValue(color);
		skil_.staticSprite->SetColor(color);
		break;
	}
	case PlayerState::Parry: {

		// パリィUIをアニメーションさせる
		// サイズ
		size = parry_.staticSprite->GetSize();
		inputReactSizeAnim_.LerpValue(size);
		parry_.staticSprite->SetSize(size);
		// 色
		color = parry_.staticSprite->GetColor();
		inputReactColorAnim_.LerpValue(color);
		parry_.staticSprite->SetColor(color);
		break;
	}
	case PlayerState::Attack_1st:
	case PlayerState::Attack_2nd:
	case PlayerState::Attack_3rd:
	case PlayerState::Attack_4th:

		// パリィUIをアニメーションさせる
		// サイズ
		size = attack_.staticSprite->GetSize();
		inputReactSizeAnim_.LerpValue(size);
		attack_.staticSprite->SetSize(size);
		// 色
		color = attack_.staticSprite->GetColor();
		inputReactColorAnim_.LerpValue(color);
		attack_.staticSprite->SetColor(color);
		break;
	}

	// アニメーションが終了次第リセット
	if (inputReactSizeAnim_.IsFinished() &&
		inputReactColorAnim_.IsFinished()) {

		// サイズと色を元に戻す
		switch (inputReactState_) {
		case PlayerState::SkilAttack: {

			skil_.SetSize(staticSpriteSize_, dynamicSpriteSize_);
			skil_.staticSprite->SetColor(SakuEngine::Color::White());
			break;
		}
		case PlayerState::Parry: {

			// 入力画像は2倍する
			parry_.SetSize(staticSpriteSize_, SakuEngine::Vector2(dynamicSpriteSize_.x * 2.0f, dynamicSpriteSize_.y));
			parry_.staticSprite->SetColor(SakuEngine::Color::White());
			break;
		}
		case PlayerState::Attack_1st:
		case PlayerState::Attack_2nd:
		case PlayerState::Attack_3rd:
		case PlayerState::Attack_4th:

			attack_.SetSize(staticSpriteSize_, dynamicSpriteSize_);
			attack_.staticSprite->SetColor(SakuEngine::Color::White());
			break;
		}
		inputReactSizeAnim_.Reset();
		inputReactColorAnim_.Reset();
		// 無効状態にする
		skil_.isActiveInput = false;
		parry_.isActiveInput = false;
		attack_.isActiveInput = false;
	} 
}

void PlayerHUD::ChangeAllOperateSprite() {

	if (preInputType_ == inputType_) {
		return;
	}

	inputType_ = InputType::GamePad;
	preInputType_ = InputType::GamePad;
	attack_.ChangeDynamicSprite(inputType_);
	dash_.ChangeDynamicSprite(inputType_);
	skil_.ChangeDynamicSprite(inputType_);
	parry_.ChangeDynamicSprite(inputType_);

	preInputType_ = inputType_;
}

void PlayerHUD::SetAllOperateTranslation() {

	attack_.SetTranslation(leftSpriteTranslation_,
		dynamicSpriteOffsetY_, operateSpriteSpancingX_);

	dash_.SetTranslation(leftSpriteTranslation_,
		dynamicSpriteOffsetY_, operateSpriteSpancingX_);

	skil_.SetTranslation(leftSpriteTranslation_,
		dynamicSpriteOffsetY_, operateSpriteSpancingX_);

	parry_.SetTranslation(leftSpriteTranslation_,
		dynamicSpriteOffsetY_, operateSpriteSpancingX_);

	// パリィアイコンの位置に入力示唆を合わせる
	for (auto& inputSuggest : inputSuggests_) {

		inputSuggest.sprite->SetTranslation(parry_.staticSprite->GetTranslation());
	}
}

void PlayerHUD::SetAllOperateSize() {

	attack_.SetSize(staticSpriteSize_, dynamicSpriteSize_);
	dash_.SetSize(staticSpriteSize_, dynamicSpriteSize_);
	skil_.SetSize(staticSpriteSize_, dynamicSpriteSize_);


	// 入力画像は2倍する
	parry_.SetSize(staticSpriteSize_, SakuEngine::Vector2(dynamicSpriteSize_.x * 2.0f, dynamicSpriteSize_.y));
}

void PlayerHUD::ImGui() {

	if (ImGui::Button("SaveJson...hudParameter.json")) {

		SaveJson();
	}

	if (ImGui::BeginTabBar("PlayerHUDTabBar")) {
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
		if (ImGui::BeginTabItem("InputReactAnim")) {

			if (ImGui::Button("Start Skil")) {

				StartInputReactAnim(PlayerState::SkilAttack);
			}
			ImGui::SameLine();
			if (ImGui::Button("Start Parry")) {

				StartInputReactAnim(PlayerState::Parry);
			}
			ImGui::SameLine();
			if (ImGui::Button("Start Attack")) {

				StartInputReactAnim(PlayerState::Attack_2nd);
			}
			ImGui::Separator();

			if (ImGui::CollapsingHeader("Size")) {

				inputReactSizeAnim_.ImGui("InputReactSizeAnim", true);
			}
			if (ImGui::CollapsingHeader("Color")) {

				inputReactColorAnim_.ImGui("InputReactColorAnim", true);
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

			bool edit = false;

			edit |= ImGui::DragFloat2("leftSpriteTranslation", &leftSpriteTranslation_.x, 1.0f);
			edit |= ImGui::DragFloat("dynamicSpriteOffsetY", &dynamicSpriteOffsetY_, 1.0f);
			edit |= ImGui::DragFloat("operateSpriteSpancingX", &operateSpriteSpancingX_, 1.0f);
			ImGui::DragFloat("returnAlphaTime", &returnAlphaTime_, 0.01f);
			Easing::SelectEasingType(returnAlphaEasingType_);

			if (edit) {

				SetAllOperateTranslation();
			}

			edit |= ImGui::DragFloat2("staticSpriteSize", &staticSpriteSize_.x, 0.1f);
			edit |= ImGui::DragFloat2("dynamicSpriteSize", &dynamicSpriteSize_.x, 0.1f);

			if (edit) {

				SetAllOperateSize();
			}
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

	skilBarParameter_.ApplyJson(data["skilBar"]);
	GameCommon::SetInitParameter(*skilBar_, skilBarParameter_);

	nameTextParameter_.ApplyJson(data["nameText"]);
	GameCommon::SetInitParameter(*nameText_, nameTextParameter_);

	damageDisplay_->ApplyJson(data);

	leftSpriteTranslation_ = leftSpriteTranslation_.FromJson(data["leftSpriteTranslation"]);
	staticSpriteSize_ = leftSpriteTranslation_.FromJson(data["staticSpriteSize"]);
	dynamicSpriteSize_ = leftSpriteTranslation_.FromJson(data["dynamicSpriteSize"]);
	dynamicSpriteOffsetY_ = SakuEngine::JsonAdapter::GetValue<float>(data, "dynamicSpriteOffsetY");
	operateSpriteSpancingX_ = SakuEngine::JsonAdapter::GetValue<float>(data, "operateSpriteSpancingX");
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

	inputReactSizeAnim_.FromJson(data.value("inputReactSizeAnim", Json()));
	inputReactColorAnim_.FromJson(data.value("inputReactColorAnim", Json()));

	SetAllOperateTranslation();
	SetAllOperateSize();
}

void PlayerHUD::SaveJson() {

	Json data;

	hpBackgroundParameter_.SaveJson(data["hpBackground"]);
	hpBarParameter_.SaveJson(data["hpBar"]);
	skilBarParameter_.SaveJson(data["skilBar"]);
	nameTextParameter_.SaveJson(data["nameText"]);

	damageDisplay_->SaveJson(data);

	data["leftSpriteTranslation"] = leftSpriteTranslation_.ToJson();
	data["staticSpriteSize"] = staticSpriteSize_.ToJson();
	data["dynamicSpriteSize"] = dynamicSpriteSize_.ToJson();
	data["dynamicSpriteOffsetY"] = dynamicSpriteOffsetY_;
	data["operateSpriteSpancingX"] = operateSpriteSpancingX_;
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

	inputReactSizeAnim_.ToJson(data["inputReactSizeAnim"]);
	inputReactColorAnim_.ToJson(data["inputReactColorAnim"]);

	SakuEngine::JsonAdapter::Save("Player/hudParameter.json", data);
}

void PlayerHUD::InputStateSprite::Init(uint32_t spriteIndex, const std::string& staticSpriteTextureName,
	const std::unordered_map<InputType, std::string>& dynamicSpritesTextureName) {

	index = spriteIndex;

	// 変化しないspriteの初期化
	staticSprite = std::make_unique<SakuEngine::GameObject2D>();
	staticSprite->Init(staticSpriteTextureName, staticSpriteTextureName, groupName);

	// 変化するspriteをタイプごとに初期化
	for (auto& [type, texture] : dynamicSpritesTextureName) {

		dynamicSprites[type] = std::make_unique<SakuEngine::GameObject2D>();
		dynamicSprites[type]->Init(texture, texture, groupName);
	}
}

void PlayerHUD::InputStateSprite::ChangeDynamicSprite(InputType type) {

	// 表示の切り替え
	for (auto& [key, sprite] : dynamicSprites) {
		if (key == type) {

			sprite->SetAlpha(1.0f);
		} else {

			sprite->SetAlpha(0.0f);
		}
	}
}

void PlayerHUD::InputStateSprite::SetTranslation(const SakuEngine::Vector2& leftSpriteTranslation,
	float dynamicSpriteOffsetY, float operateSpriteSpancingX) {

	// X座標
	float translationX = leftSpriteTranslation.x + index * operateSpriteSpancingX;

	// 座標を設定
	staticSprite->SetTranslation(SakuEngine::Vector2(translationX, leftSpriteTranslation.y));

	dynamicSprites[InputType::Keyboard]->SetTranslation(
		SakuEngine::Vector2(translationX, leftSpriteTranslation.y + dynamicSpriteOffsetY));
	dynamicSprites[InputType::GamePad]->SetTranslation(
		SakuEngine::Vector2(translationX, leftSpriteTranslation.y + dynamicSpriteOffsetY));
}

void PlayerHUD::InputStateSprite::SetSize(const SakuEngine::Vector2& staticSpriteSize,
	const SakuEngine::Vector2& dynamicSpriteSize_) {

	// サイズ設定
	staticSprite->SetSize(staticSpriteSize);
	dynamicSprites[InputType::Keyboard]->SetSize(dynamicSpriteSize_);
	dynamicSprites[InputType::GamePad]->SetSize(dynamicSpriteSize_);
}

void PlayerHUD::InputStateSprite::SetAlpha(InputType type, float alpha) {

	// サイズ設定
	staticSprite->SetAlpha(alpha);
	dynamicSprites[type]->SetAlpha(alpha);
}