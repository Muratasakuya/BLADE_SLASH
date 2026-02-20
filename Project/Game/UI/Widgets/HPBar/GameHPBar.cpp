#include "GameHPBar.h"

//============================================================================
//	GameHPBar classMethods
//============================================================================

void GameHPBar::Init(const std::string& textureName, const std::string& alphaTextureName,
	const std::string& name, const std::string& groupName) {

	// object初期化
	GameObject2D::Init(textureName, name, groupName);

	// alphaTexture設定
	sprite_->SetAlphaTextureName(alphaTextureName);
	material_->material.useAlphaColor = true;
}

void GameHPBar::Update(int current, int max, bool isReverse) {

	// 0除算回避
	if (max <= 0) {
		return;
	}

	// HPの割合を計算
	float ratio = 0.0f;
	if (isReverse) {

		ratio = 1.0f - static_cast<float>(current) / static_cast<float>(max);
	} else {

		ratio = static_cast<float>(current) / static_cast<float>(max);
	}

	material_->material.alphaReference = std::clamp(ratio, 0.0f, 1.0f);
}

SakuEngine::Vector2 GameHPBar::GetThresholdPos(int32_t threshold) const {

	// 閾値の割合を計算
	float thresholdRatio = std::clamp(static_cast<float>(threshold) / 100.0f, 0.0f, 1.0f);

	// HPバーのTransform取得
	const SakuEngine::Transform2D& transform = GetTransform();

	// 実際の描画幅
	float barWidth = transform.size.x * transform.sizeScale.x;

	// 左端、右端のX座標
	float barLeft = transform.translation.x - barWidth * transform.anchorPoint.x;
	float barRight = barLeft + barWidth;
	float thresholdX = barRight - barWidth * thresholdRatio;

	// Yは現在の値を維持してXだけ更新
	SakuEngine::Vector2 pos = transform.translation;
	pos.x = thresholdX;

	return pos;
}