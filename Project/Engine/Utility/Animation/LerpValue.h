#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Timer/StateTimer.h>
#include <Engine/MathLib/MathUtils.h>

namespace SakuEngine {

	//============================================================================
	//	LerpValue structure
	//	補間ヘルパー構造体
	//============================================================================
	template <typename T>
	struct LerpValue {

		// 補間タイマー
		StateTimer timer;

		T start;  // 開始値
		T target; // 目標値

		// エディター
		void ImGui(const std::string& label, bool isEditLerpValue = false);
	};

	//============================================================================
	//	LerpValue template Methods
	//============================================================================

	template<typename T>
	inline void LerpValue<T>::ImGui(const std::string& label, bool isEditLerpValue) {

		timer.ImGui(label.c_str(), true);

		ImGui::SeparatorText("Lerp Values");

		const char* startLabel = "Start";
		const char* targetLabel = "Target";
		if (!isEditLerpValue) {
			// 値の表示
			if constexpr (std::is_same_v<T, float>) {

				ImGui::Text("Start:  %.3f", label.c_str(), start);
				ImGui::Text("Target: %.3f", label.c_str(), target);
			} else if constexpr (std::is_same_v<T, Vector2>) {

				ImGui::Text("Start:  (%.3f, %.3f)", start.x, start.y);
				ImGui::Text("Target: (%.3f, %.3f)", target.x, target.y);
			} else if constexpr (std::is_same_v<T, Vector3>) {

				ImGui::Text("Start:  (%.3f, %.3f, %.3f)", start.x, start.y, start.z);
				ImGui::Text("Target: (%.3f, %.3f, %.3f)", target.x, target.y, target.z);
			} else if constexpr (std::is_same_v<T, Quaternion>) {

				ImGui::Text("Start:  (%.3f, %.3f, %.3f, %.3f)", start.x, start.y, start.z, start.w);
				ImGui::Text("Target: (%.3f, %.3f, %.3f, %.3f)", target.x, target.y, target.z, target.w);
			} else if constexpr (std::is_same_v<T, Color>) {

				ImGui::Text("Start:  (%.3f, %.3f, %.3f, %.3f)", start.r, start.g, start.b, start.a);
				ImGui::Text("Target: (%.3f, %.3f, %.3f, %.3f)", target.r, target.g, target.b, target.a);
			}
		} else {
			// 値の編集
			if constexpr (std::is_same_v<T, float>) {

				ImGui::DragFloat(startLabel, &start);
				ImGui::DragFloat(targetLabel, &target);
			} else if constexpr (std::is_same_v<T, Vector2>) {

				ImGui::DragFloat2(startLabel, &start.x);
				ImGui::DragFloat2(targetLabel, &target.x);
			} else if constexpr (std::is_same_v<T, Vector3>) {

				ImGui::DragFloat3(startLabel, &start.x);
				ImGui::DragFloat3(targetLabel, &target.x);
			} else if constexpr (std::is_same_v<T, Color>) {

				ImGui::ColorEdit4(startLabel, &start.r);
				ImGui::ColorEdit4(targetLabel, &target.r);
			}
		}
	}
}