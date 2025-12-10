#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/MathLib/Vector3.h>
#include <Engine/MathLib/Vector4.h>

// c++
#include <numbers>

//============================================================================
//	PunctualLight
//============================================================================

// 平行光源
struct DirectionalLight {

	Color color;
	Vector3 direction;
	float intensity;

	void Init();

	void ImGui(float itemWidth);
};

// 点光源
struct PointLight {

	Color color;
	Vector3 pos;
	float intensity;
	float radius;
	float decay;
	float padding[2];

	void Init();

	void ImGui(float itemWidth);
};

// スポットライト
struct SpotLight {

	Color color;
	Vector3 pos;
	float intensity;
	Vector3 direction;
	float distance;
	float decay;
	float cosAngle;
	float cosFalloffStart;
	float padding[2];

	void Init();

	void ImGui(float itemWidth);
};

// すべてのライト情報
struct PunctualLight {

	DirectionalLight directional;
	PointLight point;
	SpotLight spot;
};

//============================================================================
//	BasePunctualLight class
//	ライトの初期化から更新を行うクラス
//============================================================================
class BasePunctualLight {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	BasePunctualLight() = default;
	virtual ~BasePunctualLight() = default;

	//--------- functions ----------------------------------------------------

	void Init();
	virtual void DerivedInit() {}

	virtual void Update() {}

	void ImGui();
	virtual void DerivedImGui() {}

	//--------- accessor -----------------------------------------------------

	const PunctualLight& GetPunctualLight() const { return light_; }
protected:
	//========================================================================
	//	protected Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// ライト情報
	PunctualLight light_;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	const float itemWidth_ = 224.0f;
};