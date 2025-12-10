#pragma once

//============================================================================
//	include
//============================================================================

// c++
#include <string>

//============================================================================
//	ObjectTag structure
//============================================================================
struct ObjectTag {

	uint32_t objectID;         // objectのID、重複しない
	std::string identifier;    // objectの識別子、重複しない
	std::string name;          // objectの名前、重複したら数字がつく
	std::string groupName;     // objectの所属しているgroupの名前
	bool destroyOnLoad = true; // 全破棄時に破棄しないか
};