#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Base/Interface/IGameObject.h>

// data
#include <Engine/Object/Data/Material/Material.h>
#include <Engine/Object/Data/Text/MSDFText.h>

namespace SakuEngine {

	//============================================================================
	//	GameTextObject class
	//	MSDFテキスト用 2Dゲームオブジェクト
	//============================================================================
	class GameTextObject :
		public IGameObject {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		GameTextObject() = default;
		virtual ~GameTextObject() = default;

		// 初期化
		void Init(const std::string& atlasTextureName, const std::string& fontJsonPath,
			const std::string& name, const std::string& groupName);
		void DerivedInit() override {}

		// エディター
		void ImGui() override;
		void DerivedImGui() override {}

		//--------- accessor -----------------------------------------------------

		// テキスト設定
		void SetText(const std::string& utf8) { text_->SetText(utf8); }
		void SetText(int32_t value) { text_->SetText(value); }
		void SetText(float value, int32_t precision) { text_->SetText(value, precision); }
		void SetFontSizePx(float pixel) { text_->SetFontSizePx(pixel); }

		// トランスフォーム設定
		void SetTranslation(const Vector2& translation) { transform_->translation = translation; }
		void SetRotation(float rotation) { transform_->rotation = rotation; }
		void SetAnchor(const Vector2& anchor) { transform_->anchorPoint = anchor; }
		void SetSizeScale(const Vector2& sizeScale) { transform_->sizeScale = sizeScale; }

		// マテリアルデータ設定
		void SetColor(const Color& color) { material_->material.color = color; }
		void SetOutlineColor(const Color& color) { material_->material.outlineColor = color; }
		void SetOutlineWidth(float widthPx) { material_->material.outlineWidth = widthPx; }
		void SetSoftness(float softnessPx) { material_->material.softness = softnessPx; }
		void SetBoldness(float boldnessPx) { material_->material.boldness = boldnessPx; }
		void SetEnableOutline(bool enable) { material_->material.enableOutline = static_cast<uint32_t>(enable); }
		void SetPostProcessMask(uint32_t mask) { material_->material.postProcessMask = mask; }

		const TextTransform2D& GetTransform() const { return *transform_; }
		const MSDFText& GetText() const { return *text_; }
		const MSDFTextMaterial& GetMaterial() const { return *material_; }
	protected:
		//========================================================================
		//	protected Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		TextTransform2D* transform_ = nullptr;
		MSDFTextMaterial* material_ = nullptr;
		MSDFText* text_ = nullptr;
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		std::string uniqueName_;
	};
} // SakuEngine