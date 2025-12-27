#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Utility/Enum/EnumAdapter.h>

// c++
#include <typeinfo>
#include <memory>
#include <vector>
#include <unordered_map>

namespace SakuEngine {

	// enumのみに制約をかける
	template<typename T>
	concept EditorModuleEnum = std::is_enum_v<T>;

	//============================================================================
	//	EditorModuleRegistry class
	//	エディターモジュールの登録、生成を行うクラス
	//============================================================================
	template <typename BaseT, EditorModuleEnum EnumT>
	class EditorModuleRegistry {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		EditorModuleRegistry() = default;
		~EditorModuleRegistry() = default;

		//---------- using ------------------------------------------------------

		using CreateFn = std::unique_ptr<BaseT>(*)();

		//--------- functions ----------------------------------------------------

		// モジュールの生成
		std::unique_ptr<BaseT> Create(EnumT id) const;

		// モジュールの登録
		template <typename ModuleT>
		void Register();

		//--------- accessor -----------------------------------------------------

		// 登録されているモジュール名の取得
		std::vector<const char*> GetNames() const;

		// singleton
		static EditorModuleRegistry& GetInstance() {

			static EditorModuleRegistry registry{};
			return registry;
		};
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		std::unordered_map<EnumT, CreateFn> creators_;

		//--------- functions ----------------------------------------------------

		// 関数でモジュールを登録
		void RegisterFn(EnumT id, CreateFn fn);
	};

	//============================================================================
	//	EditorModuleRegistry templateMethods
	//============================================================================

	template<typename BaseT, EditorModuleEnum EnumT>
	inline std::unique_ptr<BaseT> EditorModuleRegistry<BaseT, EnumT>::Create(EnumT id) const {

		auto it = creators_.find(id);
		return it != creators_.end() ? it->second() : nullptr;
	}

	template<typename BaseT, EditorModuleEnum EnumT>
	inline std::vector<const char*> EditorModuleRegistry<BaseT, EnumT>::GetNames() const {

		std::vector<const char*> names{};
		for (auto [type, fn] : creators_) {

			names.emplace_back(SakuEngine::EnumAdapter<EnumT>::ToString(type));
		}
		return names;
	}

	template<typename BaseT, EditorModuleEnum EnumT>
	template<typename ModuleT>
	inline void EditorModuleRegistry<BaseT, EnumT>::Register() {

		RegisterFn(ModuleT::ID, []() -> std::unique_ptr<BaseT> {
			return std::make_unique<ModuleT>(); });
	}

	template<typename BaseT, EditorModuleEnum EnumT>
	inline void EditorModuleRegistry<BaseT, EnumT>::RegisterFn(EnumT id, CreateFn fn) {

		creators_[id] = fn;
	}
}