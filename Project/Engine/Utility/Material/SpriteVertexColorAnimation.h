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

		SpriteVertexColorAnimation() = default;
		~SpriteVertexColorAnimation() = default;

		// 更新
		void Update(GameObject2D& object);

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

	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		// アニメーション有効フラグ
		bool isEnable_ = false;

		// 頂点カラーアニメーション
		SakuEngine::SimpleAnimation<SakuEngine::Color> leftBottomColorAnim_;
		SakuEngine::SimpleAnimation<SakuEngine::Color> leftTopColorAnim_;
		SakuEngine::SimpleAnimation<SakuEngine::Color> rightBottomColorAnim_;
		SakuEngine::SimpleAnimation<SakuEngine::Color> rightTopColorAnim_;

		//--------- functions ----------------------------------------------------

	};

}; // SakuEngine