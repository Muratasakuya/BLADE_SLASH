#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/UISlateWidget/Widgets/UISlateLeafWidget.h>
#include <Engine/Editor/UI/UISlateWidget/Mehtods/UIWidgetFactory.h>
#include <Engine/MathLib/Vector4.h>

namespace SakuEngine {

	//============================================================================
	//	UISlateImage class
	//	
	//============================================================================
	class UISlateImage :
		public UISlateLeafWidget {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		UISlateImage() = default;
		~UISlateImage() = default;

		// ツリーに入った後に呼ばれる
		void OnAddedToTree(UIUserWidget& owner) override;

		// 描画パラメータを更新する
		void PaintDrawData(UIUserWidget& owner) override;

		// json
		void FromJson(const Json& data) override;
		void ToJson(Json& data) override;

		//--------- accessor -----------------------------------------------------

		// テクスチャ名設定
		void SetTextureName(const std::string& textureName) { textureName_ = textureName; }
		void SetAlphaTextureName(const std::optional<std::string>& textureName) { alphaTextureName_ = textureName; }
		// 色設定
		void SetColor(const Color& color) { color_ = color; }
		
		// テクスチャ名取得
		const std::string& GetTextureName() const { return textureName_; }
		const std::optional<std::string>& GetAlphaTextureName() const { return alphaTextureName_; }
		// 色取得
		const Color& GetColor() const { return color_; }

		std::string GetTypeName() const override { return "Image"; }
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		// テクスチャ名
		std::string textureName_{};
		std::optional<std::string> alphaTextureName_{};

		// 色
		Color color_ = Color::White();

		// 作成済みか
		bool isCreated_ = false;
		// スプライトオブジェクトID
		uint32_t spriteObjectId_ = 0;
	};

	// 自動登録
	static inline UIWidgetAutoRegister<UISlateImage> AutoRegistryImage("Image");
} // SakuEngine