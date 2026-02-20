#pragma once

//============================================================================
//	include
//============================================================================

// c++
#include <string>
#include <optional>
// imgui
#include <imgui.h>

//============================================================================
//	IGameEditor class
//	エディターの基底クラス
//============================================================================
namespace SakuEngine {

	class IGameEditor {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		IGameEditor(const std::string& name, const std::string& groupName = " Independent");
		virtual ~IGameEditor();

		virtual void ImGui() = 0;

		//--------- accessor -----------------------------------------------------

		void SetSelectObjectID(uint32_t id) { selectObjectID_ = id; }

		const std::string& GetName() const { return name_; }
		const std::string& GetGroupName() const { return groupName_; }
	protected:
		//========================================================================
		//	protected Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		// 選択されているオブジェクトのID
		uint32_t selectObjectID_;

		// imgui
		static constexpr const float itemWidth_ = 224.0f;
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		// エディターの名前
		std::string name_;
		// エディターのグループ名
		std::string groupName_;
	};
}; // SakuEngine