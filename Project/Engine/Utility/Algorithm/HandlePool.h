#pragma once

//============================================================================
//	include
//============================================================================

// c++
#include <cstdint>
#include <vector>
#include <optional>
#include <limits>
#include <utility>
#include <type_traits>

namespace SakuEngine {

	//============================================================================
	//	HandlePool class
	//	フリーリストを用いたハンドル参照管理クラス
	//============================================================================
	template <typename T>
	class HandlePool {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		// 無効なインデックス値
		static constexpr uint32_t kInvalidIndex = (std::numeric_limits<uint32_t>::max)();

		//--------- structure ----------------------------------------------------

		// ハンドル構造体
		struct Handle {

			uint32_t index = kInvalidIndex; // スロットのインデックス
			uint32_t generation = 0;        // 世代管理用カウンタ

			// 有効なハンドルかどうかを返す
			bool IsValid() const { return index != kInvalidIndex; }
			// ハンドル同士が等しいかどうかを返す
			static bool Equal(const Handle& a, const Handle& b);
		};
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		HandlePool() = default;
		~HandlePool() = default;

		// コピー禁止
		HandlePool(const HandlePool&) = delete;
		HandlePool& operator=(const HandlePool&) = delete;
		// ムーブ可能
		HandlePool(HandlePool&&) noexcept = default;
		HandlePool& operator=(HandlePool&&) noexcept = default;

		// 作成
		Handle Create();

		// 任意引き数で作成
		template <typename ...Args>
		Handle Emplace(Args&& ...args);

		// 要素が存在するか
		bool IsAlive(Handle handle) const;
		// 破棄
		void Destroy(Handle handle);
		void Clear();

		// 生存している要素に対して関数を実行
		template <typename Func>
		void ForEachAlive(Func&& func);

		//--------- accessor -----------------------------------------------------

		// 生存している要素数を返す
		size_t AliveCount() const;

		// スロットの要素の値を返す
		T* Get(Handle handle);
		const T* Get(Handle handle) const;
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- structure ----------------------------------------------------

		// スロット構造体
		struct Slot {

			std::optional<T> value;  // 存在している場合のみ値を保持
			uint32_t generation = 1; // 世代管理用カウンタ
		};

		//--------- variables ----------------------------------------------------

		// スロット配列
		std::vector<Slot> slots_;
		// フリーリスト
		std::vector<uint32_t> freeList_;
	};

	//============================================================================
	//	HandlePool templateMethods
	//============================================================================

	template<typename T>
	inline bool HandlePool<T>::Handle::Equal(const Handle& a, const Handle& b) {

		return a.index == b.index && a.generation == b.generation;
	}

	template<typename T>
	inline HandlePool<T>::Handle HandlePool<T>::Create() {

		return this->Emplace();
	}

	template<typename T>
	template<typename ...Args>
	inline HandlePool<T>::Handle HandlePool<T>::Emplace(Args&& ...args) {

		uint32_t index = 0;

		// 再利用できるスロットがあるか
		if (!freeList_.empty()) {

			// フリーリストからインデックスを取得
			index = freeList_.back();
			freeList_.pop_back();

			// スロットに値を構築
			Slot& slot = slots_[index];
			slot.value.emplace(std::forward<Args>(args)...);

			return Handle{ index, slot.generation };
		}

		// 新規追加
		index = static_cast<uint32_t>(slots_.size());

		// スロットに値を構築
		Slot slot;
		slot.value.emplace(std::forward<Args>(args)...);
		slots_.emplace_back(std::move(slot));

		return Handle{ index, slots_[index].generation };
	}

	template<typename T>
	template<typename Func>
	inline void HandlePool<T>::ForEachAlive(Func&& func) {

		for (uint32_t i = 0; i < static_cast<uint32_t>(slots_.size()); ++i) {

			auto& slot = slots_[i];
			if (!slot.value.has_value()) {
				continue;
			}

			// 生存している要素に対して関数を実行
			Handle handle{ i, slot.generation };
			func(handle, *slot.value);
		}
	}

	template<typename T>
	inline bool HandlePool<T>::IsAlive(Handle handle) const {

		//ハンドルが無効になっていないかチェック
		if (!handle.IsValid()) {
			return false;
		}
		// インデントが範囲内かチェック
		if (static_cast<uint32_t>(slots_.size()) <= handle.index) {
			return false;
		}

		const Slot& slot = slots_[handle.index];
		if (!slot.value.has_value()) {
			return false;
		}
		// 世代カウンタが一致するかチェック
		return slot.generation == handle.generation;
	}

	template<typename T>
	inline void HandlePool<T>::Destroy(Handle handle) {

		if (!IsAlive(handle)) {
			return;
		}

		// スロットを解放
		Slot& slot = slots_[handle.index];
		slot.value = std::nullopt;
		// 世代カウントを進める
		++slot.generation;
		freeList_.emplace_back(handle.index);
	}

	template<typename T>
	inline void HandlePool<T>::Clear() {

		slots_.clear();
		freeList_.clear();
	}

	template<typename T>
	inline size_t HandlePool<T>::AliveCount() const {

		size_t count = 0;
		for (const auto& slot : slots_) {
			if (slot.value.has_value()) {
				++count;
			}
		}
		return count;
	}

	template<typename T>
	inline T* HandlePool<T>::Get(Handle handle) {

		if (!IsAlive(handle)) {
			return nullptr;
		}
		return &(*slots_[handle.index].value);
	}

	template<typename T>
	inline const T* HandlePool<T>::Get(Handle handle) const {

		if (!IsAlive(handle)) {
			return nullptr;
		}
		return &(*slots_[handle.index].value);
	}
} // SakuEngine