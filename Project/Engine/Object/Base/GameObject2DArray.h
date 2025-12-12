#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Base/GameObject2D.h>

//============================================================================
//	GameObject2DArray class
//============================================================================
namespace SakuEngine {

	class GameObject2DArray {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		GameObject2DArray() = default;
		~GameObject2DArray() = default;

		// 初期化
		void Init();

		// スプライトの追加と初期化
		void Add(const std::string& textureName, const std::string& name, const std::string& groupName);

		// 更新
		void Update();

		// エディター
		void ImGui();

		// json
		void FromJson(const Json& data);
		void ToJson(Json& data);

		//--------- accessor -----------------------------------------------------

		// transform
		// 座標
		void SetTranslation(const SakuEngine::Vector2& translation) { transform_.translation = translation; }
		// サイズ
		void SetSize(const SakuEngine::Vector2& size) { transform_.size = size; }
		void SetTextureSize(const SakuEngine::Vector2& size) { transform_.textureSize = size; }
		void SetTextureLeftTop(const SakuEngine::Vector2& leftTop) { transform_.textureLeftTop = leftTop; }
		// スケール
		void SetSizeScale(const SakuEngine::Vector2& sizeScale) { transform_.sizeScale = sizeScale; }
		// 回転
		void SetRotation(float rotation) { transform_.rotation = rotation; }

		// material
		// 色
		void SetColor(const SakuEngine::Color& color, int32_t index = -1);
		void SetAlpha(float alpha, int32_t index = -1);

		// transform
		const Transform2D& GetTransform() { return transform_; }

		// material
		Color GetColor(int32_t index = -1) const;
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		// 2Dオブジェクト配列
		std::vector<std::unique_ptr<GameObject2D>> objects_;

		// トランスフォーム(親)
		Transform2D transform_;
	};

}; // SakuEngine
