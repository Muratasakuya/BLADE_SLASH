#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Input/InputStructures.h>
#include <Engine/MathLib/Vector2.h>

namespace SakuEngine {

	// front
	class UIApplication;
	class UIUserWidget;

	//============================================================================
	//	UISlateWidget structures
	//============================================================================

	// UI矩形
	struct UIRect {

		// 位置、サイズ
		Vector2 pos = Vector2::AnyInit(0.0f);
		Vector2 size = Vector2::AnyInit(0.0f);

		// 矩形の最小、最大座標取得
		Vector2 Min() const { return pos; }
		Vector2 Max() const { return Vector2(pos.x + size.x, pos.y + size.y); }
		// 矩形内判定
		bool Contains(const Vector2& checkPos) const;
	};

	// アンカー
	struct UIAnchors {

		Vector2 min = Vector2::AnyInit(0.0f);
		Vector2 max = Vector2::AnyInit(0.0f);
	};

	// マージン
	struct UIMargin {

		float left = 0.0f;
		float top = 0.0f;
		float right = 0.0f;
		float bottom = 0.0f;
	};

	// アンカーデータ
	struct UIAnchorData {

		UIAnchors anchors{};
		UIMargin offsets{};
		Vector2 alignment = Vector2::AnyInit(0.0f);
	};

	// UI表示設定
	enum class UIVisibility :
		uint8_t {

		Visible,           // 通常表示
		Hidden,            // 画面に出さないがレイアウトには出す
		Collapsed,         // レイアウトにも出さない
		HitTestInvisible,  // 描画はするが入力を受けない
	};

	// ポインタボタン
	enum class UIPointerButton :
		uint8_t {

		Left,
		Right,
		Middle
	};

	// UIイベントタイプ
	enum class UIEventType :
		uint8_t {

		PointerMove,  // ポインタ移動
		PointerDown,  // ポインタ押下
		PointerUp,    // ポインタ離上
		PointerWheel, // ポインタホイール
	};

	// 入力デバイスタイプ
	struct UIPointerEvent {

		UIEventType type{};
		Vector2 screenPos{};
		Vector2 delta{};
		float wheelDelta = 0.0f;

		// ボタン情報
		UIPointerButton button = UIPointerButton::Left;
		InputType inputDevice = InputType::Keyboard;
	};

	// UIイベント返信
	struct UIReply {

		bool handled = false;
		bool captureMouse = false;
		bool releaseMouse = false;
		bool setKeyboardFocus = false;

		// 入力イベントが処理されたか
		static UIReply Handled() { return UIReply{ true,false,false,false }; }
		static UIReply Unhandled() { return UIReply{ false,false,false,false }; }
		// 各デバイス設定
		UIReply& CaptureMouse() { captureMouse = true; return *this; }
		UIReply& ReleaseMouse() { releaseMouse = true; return *this; }
		UIReply& SetKeyboardFocus() { setKeyboardFocus = true; return *this; }
	};

	// 配置されたウィジェット情報
	struct UIArrangedWidget {

		// ウィジェット情報
		class UISlateWidget* widget = nullptr;
		// 配置矩形
		UIRect rect{};
		// 描画順序
		int32_t zOrder = 0;
	};

	//============================================================================
	//	UISlateWidget class
	//============================================================================
	class UISlateWidget {
	public:
		//========================================================================
		//	public Methods
		//========================================================================

		UISlateWidget() = default;
		virtual ~UISlateWidget() = default;

		// コピー禁止
		UISlateWidget(const UISlateWidget&) = delete;
		UISlateWidget& operator=(const UISlateWidget&) = delete;

		// フレーム更新
		virtual void Tick([[maybe_unused]] UIUserWidget& owner, [[maybe_unused]] float deltaTime) {}

		// 子供を持つか
		virtual bool IsPanel() const { return false; }
		// 子の列挙
		virtual void GetChildren([[maybe_unused]] std::vector<UISlateWidget*>& outChildren) {}

		// ツリーに入った後に呼ばれる
		virtual void OnAddedToTree([[maybe_unused]] UIUserWidget& owner) {}
		// プロパティ同期
		virtual void SynchProperties([[maybe_unused]] UIUserWidget& owner) {}
		// 子の配置
		virtual void ArrangeChildren([[maybe_unused]] const UIRect& allotted, [[maybe_unused]] std::vector<UIArrangedWidget>& outArranged) {}

		// 入力対応しているか
		virtual bool SupportsKeyboardFocus() const { return false; }
		// ポインターイベント処理の呼び出し
		virtual UIReply OnPointerEvent([[maybe_unused]] UIUserWidget& owner, [[maybe_unused]] const UIPointerEvent& event) { return UIReply::Unhandled(); }

		// 描画パラメータを更新する
		virtual void PaintDrawData([[maybe_unused]] UIUserWidget& owner) {}

		// タイプ名の取得
		virtual std::string GetTypeName() const = 0;

		// json
		virtual void FromJson(const Json& data);
		virtual void ToJson(Json& data);

		//--------- accessor -----------------------------------------------------

		//固有設定
		void SetId(uint64_t id) { id_ = id; }
		void SetGeneration(uint32_t generation) { generation_ = generation; }
		void SetName(std::string name) { name_ = name; }

		// ウィジェットの可視設定
		void SetVisibility(UIVisibility visibility) { visibility_ = visibility; }
		// 有効無効設定
		void SetEnabled(bool enabled) { enabled_ = enabled; }
		// レイアウト情報設定
		void SetLayout(const UIAnchorData& layout) { layout_ = layout; }
		// 矩形キャッシュ設定
		void SetCachedRect(const UIRect& rect) { cachedRect_ = rect; }

		// 固有情報の取得
		uint64_t GetId() const { return id_; }
		uint32_t GetGeneration() const { return generation_; }
		const std::string& GetName() const { return name_; }

		// ウィジェットの可視設定取得
		UIVisibility GetVisibility() const { return visibility_; }
		// 有効無効取得
		bool IsEnabled() const { return enabled_; }
		// レイアウト情報取得
		UIAnchorData& GetLayout() { return layout_; }
		// 矩形キャッシュ取得
		const UIRect& GetCachedRect() const { return cachedRect_; }
	protected:
		//========================================================================
		//	protected Methods
		//========================================================================

		//--------- functions ----------------------------------------------------

		// アンカーから矩形計算
		static UIRect ComputeRectFromAnchorData(const UIRect& parent, const UIAnchorData& data);
	private:
		//========================================================================
		//	private Methods
		//========================================================================

		//--------- variables ----------------------------------------------------

		// 固有ID
		uint64_t id_ = 0;
		// 世代番号
		uint32_t generation_ = 1;
		// ウィジェット名
		std::string name_ = "Widget";

		// ウィジェットの可視設定
		UIVisibility visibility_ = UIVisibility::Visible;
		bool enabled_ = true;

		// レイアウト情報
		UIAnchorData layout_{};
		UIRect cachedRect_{};
	};
} // SakuEngine