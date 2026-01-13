#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Renderer/Line/Base/BaseLineRenderer.h>

namespace SakuEngine {

	//============================================================================
	//	LineRenderer2D class
	//	2Dライン描画を担当するクラス
	//============================================================================
	class LineRenderer2D :
		public BaseLineRenderer {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		LineRenderer2D() = default;
		~LineRenderer2D() = default;

		// エディター
		void ImGui() override;
		// エディターデバッグ描画
		void DrawDebug() override;

		//========================================================================
		//	描画関数群
		//========================================================================

		// フルスクリーングリッド
		void DrawFullScreenGrid(const Vector2& gridSize, const Color& color, LineType type = LineType::None);

		//--------- プリミティブ形状 ------------------------------------------------

		// 矩形
		void DrawRect(const Vector2& center, const Vector2& size, const Vector2& anchor,
			const Color& color, LineType type = LineType::None);

		//--------- accessor -----------------------------------------------------

		// 描画次元を取得
		LineDimension GetLineDimension() const override { return LineDimension::Line2D; }

	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		// デバッグ表示設定
		bool isDrawGrid_ = false;
		Vector2 gridSize_ = Vector2::AnyInit(64.0f);
		Color gridColor_ = Color::White();
	};
}