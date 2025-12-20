#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Base/GameObject2D.h>

//============================================================================
//	GameDigitDisplay class
//	2Dスプライトで連番数字の表示を行う
//============================================================================
class GameDigitDisplay {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	GameDigitDisplay() = default;
	~GameDigitDisplay() = default;

	// 初期化
	void Init(uint32_t maxDigit, const std::string& textureName,
		const std::string& name, const std::string& groupName);

	// 最大桁数分の数字を表示、更新
	void Update(uint32_t maxDigit, int num);

	//--------- accessor -----------------------------------------------------

	void SetSpriteLayer(SakuEngine::SpriteLayer layer);

	void SetTranslation(const SakuEngine::Vector2& translation, const SakuEngine::Vector2& offset);
	SakuEngine::Vector2 ProjectToScreen(const SakuEngine::Vector3& translation, const SakuEngine::BaseCamera& camera);

	void SetSize(const SakuEngine::Vector2& size);
	void SetDigitSize(uint32_t digitIndex, const SakuEngine::Vector2& size);

	void SetAlpha(float alpha);
	void SetEmissive(uint32_t digitIndex, float emissive);
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// 各桁のsprite
	std::vector<std::unique_ptr<SakuEngine::GameObject2D>> digitSprites_;

	// 各桁のサイズ
	SakuEngine::Vector2 digitSize_;
};