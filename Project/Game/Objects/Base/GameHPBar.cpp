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