#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/UI/Component/Interface/IUIComponent.h>
#include <Engine/Object/Data/Transform/Transform.h>

namespace SakuEngine {

	//============================================================================
	//	UISpriteTransformComponent class
	//	UIのTransformコンポーネント
	//============================================================================
	class UISpriteTransformComponent :
		public IUIComponent {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		UISpriteTransformComponent() = default;
		~UISpriteTransformComponent() = default;

		// エディター
		void ImGui(const ImVec2& itemSize) override;

		//--------- variables ----------------------------------------------------

		Transform2D* transform = nullptr;

		//--------- accessor -----------------------------------------------------

		UIComponentType GetType() const override { return UIComponentType::SpriteTransform; }
	};
} // SakuEngine