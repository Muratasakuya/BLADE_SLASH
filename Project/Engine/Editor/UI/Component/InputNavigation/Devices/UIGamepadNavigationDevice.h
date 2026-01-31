#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Component/InputNavigation/UIInputNavigationComponent.h>
#include <Engine/Input/Base/IInputDevice.h>
#include <Engine/Editor/UI/Component/InputNavigation/Devices/UINavigationAction.h>

namespace SakuEngine {

	//============================================================================
	//	UIGamepadNavigationDevice class
	//	UIのゲームパッド入力ナビゲーション
	//============================================================================
	class UIGamepadNavigationDevice :
		public IInputDevice<UINavigationAction> {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		UIGamepadNavigationDevice(UIInputNavigationComponent* component);
		~UIGamepadNavigationDevice() = default;

		float GetVector(UINavigationAction axis) const override;

		bool IsPressed(UINavigationAction button) const override;
		bool IsTriggered(UINavigationAction button) const override;

		//--------- accessor -----------------------------------------------------

		InputType GetInputType() const override { return InputType::GamePad; }
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		UIInputNavigationComponent* component_ = nullptr;
	};
} // SakuEngine