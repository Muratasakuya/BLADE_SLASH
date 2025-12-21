#pragma once

//============================================================================
//	include
//============================================================================

// c++
#include <cstddef>
#include <vector>
#include <memory>
#include <functional>
#include <typeindex>
#include <unordered_map>
#include <utility>

//============================================================================
//	GameEventBus class
//	クラスの説明がここに入る
//============================================================================
class GameEventBus {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	GameEventBus() = default;
	~GameEventBus() = default;
	// コピー、ムーブ禁止
	GameEventBus(const GameEventBus&) = delete;
	GameEventBus& operator=(const GameEventBus&) = delete;

	//============================================================================
	//	Subscription class
	//	クラスの説明がここに入る
	//============================================================================
	template <typename Event>
	class Subscription {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		Subscription() = default;
		~Subscription() { Reset(); }
		// コピー、ムーブ禁止
		Subscription(const Subscription&) = delete;
		Subscription& operator=(const Subscription&) = delete;
		// ムーブコンストラクタ
		Subscription(Subscription&& other) noexcept
			: bus_(other.bus_), id_(other.id_) {
			other.bus_ = nullptr;
			other.id_ = 0;
		}
		Subscription& operator=(Subscription&& other) noexcept;

		// リセット
		void Reset();

		//--------- accessor -----------------------------------------------------

		// 有効かどうか
		bool Valid() const { return bus_ != nullptr; }
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		friend class GameEventBus;
		Subscription(GameEventBus* bus, size_t id) : bus_(bus), id_(id) {}

		GameEventBus* bus_ = nullptr;
		// 関数ハンドラID
		size_t id_ = 0;
	};

	// ここに関数の説明が入る
	template <typename Event>
	Subscription<Event> Subscribe(std::function<void(const Event&)> handler);

	// ここに関数の説明が入る
	template <typename Event>
	void Publish(const Event& event) const;
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- structure ----------------------------------------------------

	// チャンネルインターフェース
	struct IChannel {

		virtual ~IChannel() = default;
	};
	// ここに構造体の説明が入る
	template <typename Event>
	struct Channel :
		IChannel {

		// ここに変数の説明が入る
		size_t nextId = 1;
		// ここに変数の説明が入る
		std::unordered_map<size_t, std::function<void(const Event&)>> handlers;
	};

	//--------- variables ----------------------------------------------------

	// チャンネル群
	mutable std::unordered_map<std::type_index, std::unique_ptr<IChannel>> channels_;

	//--------- functions ----------------------------------------------------

	// 指定のイベントチャンネルを取得または作成
	template <typename Event>
	Channel<Event>& GetOrCreateChannel() const;

	// 指定Idのハンドラを削除
	template <typename Event>
	void Unsubscribe(size_t id);
};

//============================================================================
//	GameEventBus templateMethods
//============================================================================

template<typename Event>
inline GameEventBus::Subscription<Event> GameEventBus::Subscribe(std::function<void(const Event&)> handler) {

	// チャンネルを取得または作成
	auto& channel = GetOrCreateChannel<Event>();
	// ハンドラを登録してIdを取得、進める
	const size_t id = channel.nextId++;
	channel.handlers.emplace(id, std::move(handler));
	return Subscription<Event>(this, id);
}

template<typename Event>
inline void GameEventBus::Publish(const Event& event) const {

	const auto key = std::type_index(typeid(Event));
	auto it = channels_.find(key);
	// チャンネルが存在しない場合は何もしない
	if (it == channels_.end()) {
		return;
	}
	auto* channel = static_cast<Channel<Event>*>(it->second.get());

	// dispatch中にUnsubscribeされても安全にするためコピーして呼ぶ
	std::vector<std::function<void(const Event&)>> calls;
	calls.reserve(channel->handlers.size());
	for (const auto& handler : channel->handlers) {

		calls.push_back(handler.second);
	}
	// コピーした関数を呼び出して処理
	for (auto& fn : calls) {
		if (fn) {

			fn(event);
		}
	}
}

template<typename Event>
inline GameEventBus::Channel<Event>& GameEventBus::GetOrCreateChannel() const {

	const auto key = std::type_index(typeid(Event));
	auto it = channels_.find(key);
	// キーチャンネルが存在しない場合は作成して返す
	if (it == channels_.end()) {
		auto channel = std::make_unique<Channel<Event>>();
		auto* raw = channel.get();
		channels_.emplace(key, std::move(channel));
		return *raw;
	}
	// 存在する場合はキャストして返す
	return *static_cast<Channel<Event>*>(it->second.get());
}

template<class Event>
inline void GameEventBus::Unsubscribe(size_t id) {

	const auto key = std::type_index(typeid(Event));
	auto it = channels_.find(key);
	// チャンネルが存在しない場合は何もしない
	if (it == channels_.end()) {
		return;
	}
	auto* channel = static_cast<Channel<Event>*>(it->second.get());
	channel->handlers.erase(id);
}

template<typename Event>
inline GameEventBus::Subscription<Event>& GameEventBus::Subscription<Event>::operator=(Subscription&& other) noexcept {

	// 自分自身への代入チェック
	if (this != &other) {

		// 既存の登録を解除
		Reset();
		bus_ = other.bus_;
		id_ = other.id_;
		other.bus_ = nullptr;
		other.id_ = 0;
	}
	return *this;
}

template<typename Event>
inline void GameEventBus::Subscription<Event>::Reset() {

	if (bus_) {

		// 指定Idのハンドラを削除
		bus_->Unsubscribe<Event>(id_);
		bus_ = nullptr;
		id_ = 0;
	}
}