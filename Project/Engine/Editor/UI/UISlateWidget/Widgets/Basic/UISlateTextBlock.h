#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/UISlateWidget/Widgets/UISlateLeafWidget.h>
#include <Engine/Editor/UI/UISlateWidget/Mehtods/UIWidgetFactory.h>
#include <Engine/MathLib/Vector4.h>

namespace SakuEngine {

	//============================================================================
	//	UISlateTextBlock class
	//	
	//============================================================================
	class UISlateTextBlock :
		public UISlateLeafWidget {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		UISlateTextBlock() = default;
		~UISlateTextBlock() = default;

		// ツリーに入った後に呼ばれる
		void OnAddedToTree(UIUserWidget& owner) override;
		// プロパティ同期
		void SynchProperties(UIUserWidget& owner) override;

		// 描画パラメータを更新する
		void PaintDrawData(UIUserWidget& owner) override;

		// json
		void FromJson(const Json& data) override;
		void ToJson(Json& data) override;

		//--------- accessor -----------------------------------------------------

		// テクスチャ名設定
		void SetAtlasTextureName(const std::string& textureName) { atlasTextureName_ = textureName; }
		// フォント情報.jsonパス設定
		void SetFontJsonPath(const std::string& fontJsonPath) { fontJsonPath_ = fontJsonPath; }
		// 文字列設定
		void SetText(const std::string& text) { text_ = text; textDirty_ = true; }
		// 色設定
		void SetColor(const Color& color) { color_ = color; }
		// バインディングキー設定
		void SetTextBindingKey(const std::string& bindingKey) { textBindingKey_ = bindingKey; }

		// テクスチャ名取得
		const std::string& GetAtlasTextureName() const { return atlasTextureName_; }
		// フォント情報.jsonパス取得
		const std::string& GetFontJsonPath() const { return fontJsonPath_; }
		// 文字列取得
		const std::string& GetText() const { return text_; }
		// 色取得
		const Color& GetColor() const { return color_; }
		// バインディングキー取得
		const std::string& GetTextBindingKey() const { return textBindingKey_; }

		std::string GetTypeName() const override { return "TextBlock"; }
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		// テクスチャ名
		std::string atlasTextureName_{};
		// フォント情報を格納した.jsonパス
		std::string fontJsonPath_{};

		// 文字列
		std::string text_{};
		Color color_ = Color::White();

		// バインディングキー
		std::string textBindingKey_{};

		// 作成済みか
		bool isCreated_ = false;
		// 文字列変更フラグ
		bool textDirty_ = true;
		// テキストオブジェクトID
		uint32_t textObjectId_ = 0;
	};

	// 自動登録
	static inline UIWidgetAutoRegister<UISlateTextBlock> AutoRegistryText("TextBlock");
} // SakuEngine