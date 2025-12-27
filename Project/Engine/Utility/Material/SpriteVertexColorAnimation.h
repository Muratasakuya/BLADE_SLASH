#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Base/GameObject2D.h>
#include <Engine/Utility/Animation/SimpleAnimation.h>

//============================================================================
//	SpriteVertexColorAnimation class
//	スプライト頂点カラーアニメーション
//============================================================================
namespace SakuEngine {

	class SpriteVertexColorAnimation {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		SpriteVertexColorAnimation();
		~SpriteVertexColorAnimation() = default;

		// 更新
		void Update(GameObject2D& object);
		void Update();

		// エディター
		void ImGui(const std::string& label);

		// json
		void ToJson(Json& data);
		void FromJson(const Json& data);

		// 開始
		void Start();
		// リセット(停止)
		void Reset();

		//--------- accessor -----------------------------------------------------

		// 現在の頂点カラー取得
		const std::array<Color, kSpriteVertexPosNum>& GetCurrentColors() const { return currentColors_; }
		const Color& GetCurrentColor(SpriteVertexPos pos) const { return currentColors_[static_cast<uint32_t>(pos)]; }
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		// アニメーション有効フラグ
		bool isEnable_ = false;

		// 各頂点の現在の色
		std::array<Color, kSpriteVertexPosNum> currentColors_;

		// 頂点カラーアニメーション
		std::array<SimpleAnimation<Color>, kSpriteVertexPosNum> colorAnims_;
	};

}; // SakuEngine