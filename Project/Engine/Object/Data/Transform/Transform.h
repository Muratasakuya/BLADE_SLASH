#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/GPUObject/CBufferStructures.h>
#include <Engine/Core/Graphics/GPUObject/DxConstBuffer.h>
#include <Engine/Core/Graphics/GPUObject/DxStructuredBuffer.h>
#include <Engine/MathLib/Vector2.h>
#include <Engine/MathLib/Vector3.h>
#include <Engine/MathLib/Quaternion.h>

// c++
#include <format>

namespace SakuEngine {

	//============================================================================
	//	BaseTransform3D class
	//	3DTransformの基底クラス
	//============================================================================
	class BaseTransform3D {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		BaseTransform3D() = default;
		virtual ~BaseTransform3D() = default;

		// 初期化
		void Init();

		// 行列更新
		void UpdateMatrix();

		// エディター
		bool ImGui(float itemSize);

		// json
		void ToJson(Json& data);
		void FromJson(const Json& data);

		//--------- accessor -----------------------------------------------------

		// matrixからワールド座標、向きを取得
		Vector3 GetWorldScale() const;       // ワールドスケール
		Quaternion GetWorldRotation() const; // ワールド回転
		Vector3 GetWorldPos() const;         // ワールド座標

		Vector3 GetForward() const;

		Vector3 GetBack() const;

		Vector3 GetRight() const;

		Vector3 GetLeft() const;

		Vector3 GetUp() const;

		Vector3 GetDown() const;

		// 変更があったかどうか
		bool IsDirty() const { return isDirty_; }
		void SetIsDirty(bool isDirty);

		//--------- variables ----------------------------------------------------

		// 拡縮
		Vector3 scale;

		// 回転
		Quaternion rotation;
		Vector3 eulerRotate;

		// 座標
		Vector3 translation;
		Vector3 offsetTranslation;

		TransformationMatrix matrix;
		const BaseTransform3D* parent = nullptr;

		// 行列強制更新フラグ
		bool isCompulsion_ = false;
		// 親のスケールの影響を受けないかどうか
		bool isIgnoreParentScale = false;
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		Vector3 prevScale;
		Quaternion prevRotation;
		Vector3 prevTranslation;
		Vector3 prevOffsetTranslation;

		// 変更があったかどうかのフラグ
		bool isDirty_;
	};

	//============================================================================
	//	Transform3D class
	//	3DTransformクラス
	//============================================================================
	class Transform3D :
		public BaseTransform3D {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		Transform3D() = default;
		~Transform3D() = default;

		//--------- accessor -----------------------------------------------------

		void SetInstancingName(const std::string& name) { meshInstancingName_ = name; }
		const std::string& GetInstancingName() const { return meshInstancingName_; }
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		// meshInstancing用の名前
		std::string meshInstancingName_;
	};

	//============================================================================
	//	EffectTransform class
	//	エフェクト用Transform
	//============================================================================
	struct EffectTransform {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		void Init();

		void ImGui(float itemSize);

		//--------- variables ----------------------------------------------------

		// 回転
		Quaternion rotation;
		Vector3 eulerRotate;

		// 座標
		Vector3 translation;
	};

	//============================================================================
	//	BaseTransform2D class
	//	2DTransformの基底クラス
	//============================================================================
	class BaseTransform2D {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		BaseTransform2D() = default;
		virtual ~BaseTransform2D() = default;

		// 初期化
		void Init(ID3D12Device* device);
		// 行列更新
		void UpdateMatrix();

		// エディター
		void ImGuiCommon(float itemSize);

		// json
		void FromJsonCommon(const Json& data);
		void ToJsonCommon(Json& data);

		//--------- accessor -----------------------------------------------------

		// ワールド座標
		Vector2 GetWorldPos() const;

		// 画面の中心に設定
		void SetCenterPos();

		// 定数バッファ取得
		const DxConstBuffer<Matrix4x4>& GetBuffer() const { return buffer_; }

		//--------- variables ----------------------------------------------------

		Vector2 translation; // 座標
		float rotation;      // 回転
		Vector2 sizeScale;   // スケール
		Vector2 anchorPoint; // アンカーポイント

		// 行列
		Matrix4x4 matrix;

		// 親
		const BaseTransform2D* parent = nullptr;
		// 親がいてもその場で回転するかどうか
		bool rotateAroundSelfWhenParented = true;
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		// buffer
		DxConstBuffer<Matrix4x4> buffer_;
	};

	//============================================================================
	//	Transform2D class
	//	2DTransformクラス
	//============================================================================
	class Transform2D :
		public BaseTransform2D {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		Transform2D() = default;
		~Transform2D() = default;

		void Init(ID3D12Device* device);

		void ImGui(float itemSize);

		// json
		void ToJson(Json& data);
		void FromJson(const Json& data);

		//--------- variables ----------------------------------------------------

		Vector2 size;           // 表示サイズ
		Vector2 textureLeftTop; // テクスチャ左上座標
		Vector2 textureSize;    // テクスチャ切り出しサイズ

		// 0: 左下
		// 1: 左上
		// 2: 右下
		// 3: 右上
		std::array<Vector2, 4> vertexOffset; // 頂点オフセット
	};

	//============================================================================
	//	TextTransform2D class
	//	2DTransformクラス
	//============================================================================
	class TextTransform2D :
		public BaseTransform2D {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		TextTransform2D() = default;
		~TextTransform2D() = default;

		// 初期化、最大数分確保する
		void Secure(ID3D12Device* device, uint32_t maxGlyphs);
		// 行列更新
		void UpdateAllMatrix(const class MSDFText& text);

		// エディター
		void ImGui(float itemSize);

		//--------- variables ----------------------------------------------------

		// 文字数分のトランスフォーム
		std::vector<BaseTransform2D> charTransforms;
		// 行列
		std::vector<Matrix4x4> matrices;

		//--------- accessor -----------------------------------------------------

		// 文字ごとの行列バッファ取得
		const DxStructuredBuffer<Matrix4x4>& GetCharMatrixBuffer() const { return charMatrixBuffer_; }
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		// 最大文字数
		uint32_t maxGlyphs_;
		// 現在の文字列
		const std::string* currentText_ = nullptr;
		std::vector<char32_t> currentCodepoints_;

		// 文字ごとの行列バッファ
		DxStructuredBuffer<Matrix4x4> charMatrixBuffer_;
	};
}; // SakuEngine