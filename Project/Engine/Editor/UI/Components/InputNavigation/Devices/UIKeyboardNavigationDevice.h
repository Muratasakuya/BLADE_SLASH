#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Components/InputNavigation/UIInputNavigationComponent.h>
#include <Engine/Input/Base/IInputDevice.h>
#include <Engine/Editor/UI/Components/InputNavigation/Devices/UINavigationAction.h>

namespace SakuEngine {

	//============================================================================
	//	UIKeyboardNavigationDevice class
	//	UIのキーボード入力ナビゲーション
	//============================================================================
	class UIKeyboardNavigationDevice :
		public IInputDevice<UINavigationAction> {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		UIKeyboardNavigationDevice(UIInputNavigationComponent* component);
		~UIKeyboardNavigationDevice() = default;

		float GetVector(UINavigationAction axis) const override;

		bool IsPressed(UINavigationAction button) const override;
		bool IsTriggered(UINavigationAction button) const override;

		//--------- accessor -----------------------------------------------------

		InputType GetInputType() const override { return InputType::Keyboard; }
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		UIInputNavigationComponent* component_ = nullptr;
	};
} // SakuEngine