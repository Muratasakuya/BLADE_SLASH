#include "CameraEditor.h"

using namespace SakuEngine;

//============================================================================
//	include
//============================================================================
#include <Engine/Editor/GameObject/ImGuiObjectEditor.h>
#include <Engine/Core/Debug/SpdLogger.h>
#include <Engine/Scene/SceneView.h>
#include <Engine/Config.h>
#include <Engine/Utility/Enum/EnumAdapter.h>
#include <Engine/Utility/Algorithm/Algorithm.h>

// キーイベント
#include <Engine/Editor/Camera/Module/Updaters/CameraKeyEventShakeUpdater.h>
#include <Engine/Editor/Camera/Module/Updaters/CameraKeyEventWaitUpdater.h>

// imgui
#include <imgui_internal.h>

//============================================================================
//	CameraEditor classMethods
//============================================================================

CameraEditor* CameraEditor::instance_ = nullptr;

CameraEditor* CameraEditor::GetInstance() {

	if (instance_ == nullptr) {
		instance_ = new CameraEditor();
	}
	return instance_;
}

void CameraEditor::Finalize() {

	if (instance_ != nullptr) {

		delete instance_;
		instance_ = nullptr;
	}
}

void CameraEditor::Init(SceneView* sceneView) {

	sceneView_ = nullptr;
	sceneView_ = sceneView;

	// 初期化
	isPreviewInverseKeyframe_ = false;

	// キーイベント更新登録
	UpdateRegistry::GetInstance().Register<CameraKeyEventWaitUpdater>();
	UpdateRegistry::GetInstance().Register<CameraKeyEventShakeUpdater>();
}

void CameraEditor::LoadJson(const std::string& fileName, bool isInEditor) {

	LOG_SCOPE_MS_LABEL("CameraEditorLoadJson");

	// エディタからの呼び出しならfileNameに"CameraEditor/xxx.json"が入っているのでそのまま、
	// ゲーム側からなら sonBasePath_ 前にくっつける
	std::string jsonFileName = fileName;
	if (!isInEditor) {
		jsonFileName = jsonBasePath_ + fileName;
	}

	// Json読み込み
	Json data;
	if (!SakuEngine::JsonAdapter::LoadCheck(jsonFileName, data)) {
		// 読み込めなければエラーにする
		ASSERT(FALSE, "failed to open file:" + jsonFileName);
		return;
	}

	// keyName、CameraKeyObjectを探索して見つからなければ早期リターン
	if (!data.contains("keyName") || !data.contains("CameraKeyObject")) {
		LOG_INFO("this file is not CameraKeyDataFile: [{}]", jsonFileName);
		return;
	}

	// キーの名前
	std::string keyName = data["keyName"];
	// 同じキーの場合
	if (SakuEngine::Algorithm::Find(keyMaps_, keyName)) {

		// エディター読み込みの場合は名前に連番をつける
		if (isInEditor) {

			keyName = CheckName(keyName);
		}
		// ゲーム読み込みの場合は読み込まない
		else {
			return;
		}
	}

	// キーオブジェクト
	std::unique_ptr<KeyframeObject3D> keyObject = std::make_unique<KeyframeObject3D>();

	// 初期化
	keyObject->Init(keyObjectName_, keyModelName_);
	// 追加するキー情報
	keyObject->AddKeyValue(AnyMold::Float, addKeyValueFov_);
	// Jsonから復元
	keyObject->FromJson(data["CameraKeyObject"]);

	// キーマップ構築
	KeyMap keyMap{};
	keyMap.keyObject = std::move(keyObject);

	// キーイベントの復元
	if (data.contains("keyEvents") && data["keyEvents"].is_array()) {
		for (const auto& eventJson : data["keyEvents"]) {

			KeyEvent event{};
			event.eventId = keyEventUidCounter_++;
			event.startTime = eventJson.value("startTime", 0.0f);
			std::string idStr = eventJson.value("updaterID", std::string{ EnumAdapter<CameraKeyEventUpdaterID>::ToString(CameraKeyEventUpdaterID::Stop) });
			auto idOptional = EnumAdapter<CameraKeyEventUpdaterID>::FromString(idStr);
			if (!idOptional.has_value()) {
				continue;
			}
			auto updater = UpdateRegistry::GetInstance().Create(idOptional.value());
			if (!updater) {
				continue;
			}
			updater->Init();
			if (eventJson.contains("updater")) {
				updater->FromJson(eventJson["updater"]);
			}
			event.updater = std::move(updater);
			keyMap.keyEvents.emplace_back(std::move(event));
		}
	}

	// 名前で追加
	keyMaps_.emplace(keyName, std::move(keyMap));

	// エディター
	previewMode_ = SakuEngine::EnumAdapter<PreviewMode>::FromString(data["previewMode_"]).value();
	previewLoopSpacing_ = data.value("previewLoopSpacing_", 1.0f);

	LOG_INFO("loaded CameraKeyData: fileName: [{}]", jsonFileName);
}

void CameraEditor::SetParentTransform(const std::string& keyName, const SakuEngine::Transform3D& parent) {

	// 無ければ処理できない
	auto it = keyMaps_.find(keyName);
	if (it != keyMaps_.end()) {

		it->second.keyObject->SetParent(keyName, parent);
	}
}

const std::optional<std::string> SakuEngine::CameraEditor::GetActiveAnimationKeyName() const {

	if (activeKeyMap_) {
		for (const auto& [key, value] : keyMaps_) {
			if (&value == activeKeyMap_) {
				return key;
			}
		}
	}
	return std::nullopt;
}

std::vector<std::string> SakuEngine::CameraEditor::GetKeyObjectNames() const {

	std::vector<std::string> names;
	names.reserve(keyMaps_.size());
	for (const auto& [name, _] : keyMaps_) {

		names.emplace_back(name);
	}
	std::sort(names.begin(), names.end());
	return names;
}

void CameraEditor::StartAnim(const std::string& keyName, bool isAddFirstKey,
	bool isUpdateKey, const std::optional<KeyframeInverseSetting>& inverseSetting) {

	// 無ければ処理できない
	auto it = keyMaps_.find(keyName);
	if (it == keyMaps_.end()) {
		return;
	}

	// 再生中のカメラアニメーションがあれば終了させる
	for (auto& key : std::views::values(keyMaps_)) {
		if (key.keyObject->IsUpdating()) {

			key.keyObject->Reset();
		}
	}

	// アクティブなキーオブジェクトに設定
	activeKeyMap_ = &it->second;
	lastActiveKeyName_ = keyName;

	// ランタイム情報のリセット
	ResetKeyEventRuntimeState(it->second);

	// 最初のキーを追加するかどうか
	// 追加する場合
	if (isAddFirstKey) {

		// シーンから現在のカメラ情報を取得
		BaseCamera* camera = sceneView_->GetCamera();

		// トランスフォームとfovY
		const SakuEngine::Transform3D& cameraTransform = camera->GetTransform();
		float fovY = camera->GetFovY();
		std::vector<KeyframeObject3D::AnyValue> anyValues;
		anyValues.emplace_back(fovY);

		// キー情報の更新
		if (isUpdateKey) {

			activeKeyMap_->keyObject->UpdateKey(true);
		}

		// 補間開始
		activeKeyMap_->keyObject->StartLerp(cameraTransform, anyValues, inverseSetting);
	}
	// 追加しない場合
	else {

		// キー情報の更新
		if (isUpdateKey) {

			activeKeyMap_->keyObject->UpdateKey(true);
		}

		// 補間開始
		activeKeyMap_->keyObject->StartLerp(std::nullopt, std::nullopt, inverseSetting);
	}
}

void CameraEditor::EndAnim() {

	// 無ければ処理できない
	if (!activeKeyMap_) {
		return;
	}
	// リセットして非アクティブ状態にする
	activeKeyMap_->keyObject->Reset();
	activeKeyMap_ = nullptr;

	// 更新を戻す
	BaseCamera* camera = sceneView_->GetCamera();
	camera->SetIsUpdateEditor(false);
}

bool CameraEditor::IsAnimFinished() const {

	// 無ければ終了しているとみなす
	if (!activeKeyMap_) {
		return true;
	}

	//  キーイベントはPlay時のみ評価対象にする
	if (previewMode_ != PreviewMode::Play) {
		return !activeKeyMap_->keyObject->IsUpdating();
	}

	// キーイベントも含めて終了しているか
	auto it = keyMaps_.find(lastActiveKeyName_);
	if (it == keyMaps_.end()) {
		return !activeKeyMap_->keyObject->IsUpdating();
	}
	return !activeKeyMap_->keyObject->IsUpdating() && AreAllKeyEventsFinished(it->second);
}

void CameraEditor::ResetAllKeyData() {

	keyMaps_.clear();
	nameCounts_.clear();
	activeKeyMap_ = nullptr;
	selectedKeyEventUid_ = 0;
	keyEventUidCounter_ = 1;
}

void CameraEditor::Update() {

	// キーオブジェクトの更新
	UpdateKeyObjects();

	// エディターの更新
	UpdateEditor();
}

void CameraEditor::UpdateKeyObjects() {

	// キーオブジェクトの更新
	for (auto& key : std::views::values(keyMaps_)) {

		//常に行う
		key.keyObject->UpdateKey();
	}

	// アクティブなキーオブジェクトの更新
	if (!activeKeyMap_) {
		return;
	}

	// 現在のゲームカメラ
	// カメラをエディターで更新中にする
	BaseCamera* camera = sceneView_->GetCamera();
	camera->SetIsUpdateEditor(true);

	// デルタタイム取得
	float deltaTime = SakuEngine::GameTimer::GetScaledDeltaTime();

	// 現在処理しているイベントが補間を停止させるか
	auto HasBlockingActive = [&]() {
		for (const auto& event : activeKeyMap_->keyEvents) {
			if (event.isStarted && !event.isFinished && IsKeyEventBlocking(event)) {
				return true;
			}
		}
		return false;
		};

	// イベントの開始呼び出し
	auto StartDueEvents = [&](float currentTime) {
		for (auto& event : activeKeyMap_->keyEvents) {

			// すでに開始済み、または終了済みならスキップ
			if (event.isStarted || event.isFinished) {
				continue;
			}
			// 開始時間に達していれば開始
			if (event.startTime <= currentTime + Config::kEpsilon) {

				event.updater->SetCamera(camera);
				event.updater->Start();
				event.isStarted = true;
				event.isFinished = false;
			}
		}
		};

	// 現在のキー補間の処理時間
	float currentTime = activeKeyMap_->keyObject->GetCurrentTimer();
	StartDueEvents(currentTime);

	if (!HasBlockingActive() && activeKeyMap_->keyObject->IsUpdating()) {

		// このフレームで到達するイベントがあれば、その時間で補間を停止する
		float stopAt = std::numeric_limits<float>::infinity();
		for (const auto& event : activeKeyMap_->keyEvents) {
			// すでに開始済み、または終了済みならスキップ
			if (event.isStarted || event.isFinished) {
				continue;
			}
			if (!IsKeyEventBlocking(event)) {
				continue;
			}
			float absTime = event.startTime;
			// このフレームで到達する場合
			if (currentTime < absTime && absTime <= currentTime + deltaTime + Config::kEpsilon) {
				stopAt = (std::min)(stopAt, absTime);
			}
		}

		// 停止時間が有限値ならその時間まで進める
		if (std::isfinite(stopAt)) {

			// ぴったり開始時間まで進める
			const float advance = (std::max)(0.0f, stopAt - currentTime);
			activeKeyMap_->keyObject->AdvanceTime(advance);
			currentTime = activeKeyMap_->keyObject->GetCurrentTimer();
			StartDueEvents(currentTime);
		} else {

			// 通常更新
			activeKeyMap_->keyObject->AdvanceTime(deltaTime);
			currentTime = activeKeyMap_->keyObject->GetCurrentTimer();
			StartDueEvents(currentTime);
		}
	}

	// 補間結果を反映
	ApplyToCamera(*sceneView_->GetCamera(), *activeKeyMap_->keyObject, std::nullopt);

	// キーイベント更新
	bool anyEventUpdated = false;
	for (auto& event : activeKeyMap_->keyEvents) {

		// 開始していない、または終了しているならスキップ
		if (!event.isStarted || event.isFinished) {
			continue;
		}

		// イベント更新
		event.updater->Update();
		anyEventUpdated = true;
		// 終了判定
		if (event.updater->IsEnd()) {

			event.isFinished = true;
			event.updater->End();
		}
	}
	if (anyEventUpdated) {
		// カメラの更新
		camera->UpdateView(BaseCamera::UpdateMode::Quaternion);
	}

	// 補間が終了し、キーイベントもすべて終了したらアクティブ状態を解除
	if (!activeKeyMap_->keyObject->IsUpdating() && AreAllKeyEventsFinished(*activeKeyMap_)) {

		camera->BindEndEditCameraPose();
		EndAnim();
	}
}

void CameraEditor::UpdateEditor() {
#if defined(_DEBUG) || defined(_DEVELOPBUILD)

	// ゲーム側のカメラ操作がアクティブ状態の時はここを通らない
	if (activeKeyMap_) {
		return;
	}

	// 現在のゲームカメラ
	BaseCamera* camera = sceneView_->GetCamera();
	// 更新設定
	camera->SetIsUpdateEditor(isPreViewGameCamera_);

	// プレビュー表示していないときはそもそも更新しない
	if (!isPreViewGameCamera_) {
		return;
	}
	// 存在しないキーでは処理させない
	if (!SakuEngine::Algorithm::Find(keyMaps_, selectedKeyObjectName_)) {
		return;
	}

	// 参照を取得
	auto* keyObject = keyMaps_[selectedKeyObjectName_].keyObject.get();

	// プレビュー反転設定
	std::optional<KeyframeInverseSetting> previewInverseSetting = std::nullopt;
	if (isPreviewInverseKeyframe_) {

		previewInverseSetting = keyObject->GetEditInverseSetting();
	}

	// モード別の更新
	switch (previewMode_) {
	case CameraEditor::PreviewMode::Keyframe: {

		// IDの同期
		SynchSelectedKeyIndex();

		// 0以上の場合のみ
		if (previewKeyIndex_ < 0 || keyObject->GetKeyObjectIDs().empty()) {
			break;
		}

		// 現在のキー位置のカメラ情報を取得して反映させる
		SakuEngine::Transform3D transform;
		if (previewInverseSetting.has_value()) {

			transform = keyObject->GetIndexKeyTransformInversed(
				static_cast<uint32_t>(previewKeyIndex_), previewInverseSetting.value());
		} else {

			transform = keyObject->GetIndexKeyTransform(static_cast<uint32_t>(previewKeyIndex_));
		}
		float fovY = 0.0f;
		KeyframeObject3D::AnyValue fovValue =
			keyObject->GetIndexAnyValue(static_cast<uint32_t>(previewKeyIndex_), addKeyValueFov_);
		if (const auto& keyFovY = std::get_if<float>(&fovValue)) {

			fovY = *keyFovY;
		}

		// 現在のキー位置のカメラ情報を渡す
		camera->SetTranslation(transform.translation);
		camera->SetRotation(transform.rotation);
		camera->SetFovY(fovY);

		// カメラの更新
		camera->UpdateView(BaseCamera::UpdateMode::Quaternion);
		break;
	}
	case CameraEditor::PreviewMode::Manual: {

		// 時間を渡して更新
		keyObject->ExternalInputTUpdate(previewTimer_);

		// カメラへ適応
		ApplyToCamera(*camera, *keyObject, previewInverseSetting);
		break;
	}
	case CameraEditor::PreviewMode::Play: {

		// 対象のKeyMapを取得
		KeyMap& keyMap = keyMaps_[selectedKeyObjectName_];

		// 再生開始を検知したらKeyEventのランタイム状態をリセット
		static std::string prevKeyName;
		static bool prevUpdating = false;

		// キーが変わったらリセットする
		if (prevKeyName != selectedKeyObjectName_) {
			prevKeyName = selectedKeyObjectName_;
			prevUpdating = false;
		}

		// 再生開始を検知
		bool nowUpdating = keyObject->IsUpdating();
		if (nowUpdating && !prevUpdating) {

			// ランタイム状態リセット
			ResetKeyEventRuntimeState(keyMap);
		}
		prevUpdating = nowUpdating;

		// デルタタイム取得
		float deltaTime = SakuEngine::GameTimer::GetScaledDeltaTime();

		// キーイベントが補間を停止させるか
		auto HasBlockingActive = [&]() {
			for (const auto& event : keyMap.keyEvents) {
				if (event.isStarted && !event.isFinished && event.updater && IsKeyEventBlocking(event)) {
					return true;
				}
			}
			return false;
			};

		auto StartDueEvents = [&](float currentTime) {
			for (auto& event : keyMap.keyEvents) {

				if (!event.updater || event.isStarted || event.isFinished) {
					continue;
				}

				// 現在の時間が開始時間を超えていればイベントを開始する
				if (event.startTime <= currentTime) {

					event.updater->SetCamera(camera);
					event.updater->Start();
					event.isStarted = true;
					event.isFinished = false;
				}
			}
			};

		// 現在の補間時間
		float currentTime = keyObject->GetCurrentTimer();

		// 到達済みイベントを開始
		StartDueEvents(currentTime);

		// ブロッキングイベント中なら補間を進めない
		if (!HasBlockingActive() && keyObject->IsUpdating()) {

			// このフレームで到達するブロッキングイベントがあれば、その時刻で止める
			float stopAt = std::numeric_limits<float>::infinity();
			for (const auto& event : keyMap.keyEvents) {

				if (!event.updater || event.isStarted || event.isFinished) {
					continue;
				}
				if (!IsKeyEventBlocking(event)) {
					continue;
				}

				float absTime = event.startTime;
				if (currentTime < absTime && absTime <= currentTime + deltaTime + Config::kEpsilon) {
					stopAt = (std::min)(stopAt, absTime);
				}
			}

			if (std::isfinite(stopAt)) {

				const float advance = (std::max)(0.0f, stopAt - currentTime);
				keyObject->AdvanceTime(advance);
				currentTime = keyObject->GetCurrentTimer();
				StartDueEvents(currentTime);

			} else {

				keyObject->AdvanceTime(deltaTime);
				currentTime = keyObject->GetCurrentTimer();
				StartDueEvents(currentTime);
			}
		}

		// まず補間結果を反映
		ApplyToCamera(*camera, *keyObject, previewInverseSetting);

		// KeyEvent 更新
		bool anyEventUpdated = false;
		for (auto& event : keyMap.keyEvents) {

			if (!event.updater || !event.isStarted || event.isFinished) {
				continue;
			}

			event.updater->Update();
			anyEventUpdated = true;

			if (event.updater->IsEnd()) {
				event.isFinished = true;
				event.updater->End();
			}
		}

		// カメラ更新
		if (anyEventUpdated) {

			camera->UpdateView(BaseCamera::UpdateMode::Quaternion);
		}

		// 再生中はループ待ちに入らない
		if (keyObject->IsUpdating()) {
			break;
		}

		// キー補間が終わったので、ループ間隔タイマーは進める
		previewLoopTimer_ += SakuEngine::GameTimer::GetDeltaTime();

		// 間隔に達していなければ何もしない
		if (previewLoopTimer_ < previewLoopSpacing_) {
			break;
		}

		// 間隔に達したが、開始済みイベントが終わっていなければ待つ
		if (!AreAllKeyEventsFinished(keyMap)) {
			break;
		}

		// 次ループ開始前にイベント状態をリセット
		ResetKeyEventRuntimeState(keyMap);

		keyObject->StartLerp();
		previewLoopTimer_ = 0.0f;
		break;
	}
	}
#endif
}

void CameraEditor::ResetKeyEventRuntimeState(KeyMap& keyMap) {

	// キーイベントの状態リセット
	for (auto& event : keyMap.keyEvents) {

		event.isStarted = false;
		event.isFinished = false;
		if (event.updater) {

			event.updater->End();
			event.updater->SetCamera(nullptr);
		}
	}

	// 開始時間でソート、同じならイベントIDでソートする
	std::stable_sort(keyMap.keyEvents.begin(), keyMap.keyEvents.end(),
		[](const KeyEvent& eventA, const KeyEvent& eventB) {
			if (eventA.startTime == eventB.startTime) {
				return eventA.eventId < eventB.eventId;
			}
			return eventA.startTime < eventB.startTime;
		});
}

bool CameraEditor::IsKeyEventBlocking(const KeyEvent& event) const {

	return event.updater && (event.updater->IsAdvanceKey() == false);
}

bool CameraEditor::AreAllKeyEventsFinished(const KeyMap& keyMap) const {

	if (previewMode_ != PreviewMode::Play) {
		return true;
	}
	for (const auto& event : keyMap.keyEvents) {
		if (event.updater && event.isStarted && !event.isFinished) {
			return false;
		}
	}
	return true;
}

void CameraEditor::ApplyToCamera(BaseCamera& camera, const KeyframeObject3D& keyObject,
	const std::optional<KeyframeInverseSetting>& inverseSetting) {

	// 現在のキー位置のカメラ情報
	SakuEngine::Transform3D transform;
	// 反転設定がある場合と無い場合で反転
	if (inverseSetting.has_value()) {

		transform = keyObject.GetCurrentTransformInversed(inverseSetting.value());
	} else {

		transform = keyObject.GetCurrentTransformForPlayback();
	}

	float fovY = 0.0f;
	KeyframeObject3D::AnyValue fovValue = keyObject.GetCurrentAnyValue(addKeyValueFov_);
	if (const auto& keyFovY = std::get_if<float>(&fovValue)) {

		fovY = *keyFovY;
	}

	// 現在のキー位置のカメラ情報を渡す
	camera.SetTranslation(transform.translation);
	camera.SetRotation(transform.rotation);
	camera.SetFovY(fovY);

	// カメラの更新
	camera.UpdateView(BaseCamera::UpdateMode::Quaternion);
}

void CameraEditor::SynchSelectedKeyIndex() {

	// 選択されているオブジェクトに合わせる
	const auto& selected = ImGuiObjectEditor::GetInstance()->GetSelected3D();
	// 未選択
	if (!selected.has_value()) {
		previewKeyIndex_ = -1;
		return;
	}

	// 同じIDを検索
	for (const auto& id : keyMaps_[selectedKeyObjectName_].keyObject->GetKeyObjectIDs()) {
		if (id == selected.value()) {

			//　選択されているオブジェクトが何番目のキーインデックスか取得して設定
			previewKeyIndex_ = keyMaps_[selectedKeyObjectName_].keyObject->GetKeyIndexFromObjectID(id);
		}
	}
}

std::string CameraEditor::CheckName(const std::string& name) {

	int trailingNumber = 0;
	// ベースネームと末尾の数字に分離
	std::string base = SplitBaseNameAndNumber(name, trailingNumber);

	int& count = nameCounts_[base];

	if (trailingNumber > count) {
		count = trailingNumber;
	}

	std::string uniqueName;
	if (count == 0) {

		uniqueName = base;
	} else {

		uniqueName = base + std::to_string(count);
	}
	// 次回用に名前カウントを増やす
	count++;

	return uniqueName;
}

std::string CameraEditor::SplitBaseNameAndNumber(const std::string& name, int& number) {

	int idx = static_cast<int>(name.size()) - 1;
	while (idx >= 0 && std::isdigit(name[idx])) {
		idx--;
	}

	int startOfDigits = idx + 1;
	if (startOfDigits < static_cast<int>(name.size())) {

		// 末尾に数字がある場合
		number = std::stoi(name.substr(startOfDigits));
	} else {

		// 末尾に数字が無い場合
		number = 0;
	}

	return name.substr(0, startOfDigits);
}

void CameraEditor::ImGui() {

	// キーオブジェクトの追加、選択
	AddAndSelectKeyObjectMap();
	// キーオブジェクトの編集
	EditSelectedKeyObject();
}

void CameraEditor::AddAndSelectKeyObjectMap() {

	ImVec2 areaSize = SakuEngine::ImGuiHelper::GetWindowAreaSizeRatio(0.5f, 0.5f);
	const float areaHeight = 128.0f;

	//================================================================================================================
	//	キーオブジェクトの追加
	//================================================================================================================

	// 左側の枠
	if (SakuEngine::ImGuiHelper::BeginFramedChild("##Add", nullptr, ImVec2(areaSize.x, areaHeight))) {

		ImGui::TextUnformatted("Add Key");

		// 入力欄
		static std::string inputName;
		char buf[128] = {};
		strncpy_s(buf, sizeof(buf), inputName.c_str(), _TRUNCATE);
		if (ImGui::InputText("Name", buf, IM_ARRAYSIZE(buf))) {

			inputName = buf;
		}

		if (ImGui::Button("Add Key")) {

			// 既に存在しているキーの名前で追加できない
			if (!inputName.empty() && !SakuEngine::Algorithm::Find(keyMaps_, inputName)) {

				// キーオブジェクトを生成
				std::unique_ptr<KeyframeObject3D> object = std::make_unique<KeyframeObject3D>();
				// fovYを任意の値として追加
				object->AddKeyValue(AnyMold::Float, addKeyValueFov_);
				object->Init(keyObjectName_, keyModelName_);

				// 追加
				keyMaps_.emplace(inputName, KeyMap{ .keyObject = std::move(object) });

				// 選択を更新
				selectedKeyObjectName_ = inputName;
				inputName.clear();
			}
		}

		ImGui::SameLine();

		// 読み込み処理
		if (ImGui::Button("Load CameraKey")) {
			std::string outRelPath;
			if (SakuEngine::ImGuiHelper::OpenJsonDialog(outRelPath)) {

				LoadJson(outRelPath, true);
			}
		}
	}
	SakuEngine::ImGuiHelper::EndFramedChild();

	// 同じライン
	ImGui::SameLine();

	//================================================================================================================
	//	キーオブジェクトの選択
	//================================================================================================================

	// 右側の枠
	if (SakuEngine::ImGuiHelper::BeginFramedChild("##Select", nullptr, ImVec2(areaSize.y, areaHeight))) {

		// キーオブジェクトの選択
		// stringを配列にまとめる
		std::vector<std::string> keyNames;
		for (const auto& key : keyMaps_) {

			keyNames.emplace_back(key.first);
		}

		// 現在のindexをkeysから逆引き
		int32_t currentIndex = -1;
		if (!selectedKeyObjectName_.empty()) {
			for (int i = 0; i < static_cast<int32_t>(keyMaps_.size()); ++i) {
				// 同じ名前のインデックスを探す
				if (keyNames[i] == selectedKeyObjectName_) {

					// インデックスを更新
					currentIndex = i;
					break;
				}
			}
		}

		// 何もない場合は文字を表示する
		if (keyNames.empty()) {

			ImGui::TextDisabled("Key is Empty");
		}
		// リスト選択
		else {
			if (SakuEngine::ImGuiHelper::SelectableListFromStrings("CameraKey List", &currentIndex, keyNames, 32)) {
				// 選択されたキーオブジェクト名を更新
				if (currentIndex >= 0 && currentIndex < static_cast<int32_t>(keyNames.size())) {

					selectedKeyObjectName_ = keyNames[currentIndex];
				}
			}
		}
	}
	SakuEngine::ImGuiHelper::EndFramedChild();
}

void CameraEditor::EditSelectedKeyObject() {

	// 未選択ならなにもしない
	if (selectedKeyObjectName_.empty()) {
		return;
	}
	// 存在しないキーなら処理しないし選択も解除
	auto it = keyMaps_.find(selectedKeyObjectName_);
	if (it == keyMaps_.end()) {
		selectedKeyObjectName_.clear();
		return;
	}

	ImGui::PushItemWidth(200.0f);

	//================================================================================================================
	//	保存
	//================================================================================================================

	// 保存ボタン
	if (ImGui::Button("Save CameraKey")) {

		jsonSaveState_.showPopup = true;
	}
	// 保存処理
	{
		std::string outRelPath;
		if (SakuEngine::ImGuiHelper::SaveJsonModal("Save CameraKey", jsonBasePath_.c_str(),
			jsonBasePath_.c_str(), jsonSaveState_, outRelPath)) {

			SaveJson(outRelPath);
		}
	}
	ImGui::Separator();

	//================================================================================================================
	//	キーオブジェクトの編集
	//================================================================================================================

	if (ImGui::BeginTabBar("CameraEditorTabBar")) {

		//================================================================================================================
		//	ゲームカメラとの連携
		//================================================================================================================
		if (ImGui::BeginTabItem("GameCamera")) {

			// モード選択
			ImGui::Checkbox("isPreViewGameCamera", &isPreViewGameCamera_);
			ImGui::Checkbox("isPreviewInverseKeyframe", &isPreviewInverseKeyframe_);
			SakuEngine::EnumAdapter<PreviewMode>::Combo("PreviewMode", &previewMode_);

			ImGui::SeparatorText("Option");

			// モード別オプション
			switch (previewMode_) {
			case CameraEditor::PreviewMode::Keyframe: {

				// 選択中のキーのインデックスを取得してその位置のキー表示する
				if (previewKeyIndex_ < 0) {

					ImGui::TextDisabled("No Key Selected");
				} else {

					ImGui::Text("currentKeyIndex: %d", previewKeyIndex_);
				}
				break;
			}
			case CameraEditor::PreviewMode::Manual: {

				ImGui::DragFloat("previewTimer", &previewTimer_, 0.001f, 0.0f, 1.0f);
				break;
			}
			case CameraEditor::PreviewMode::Play: {

				ImGui::DragFloat("previewLoopSpacing", &previewLoopSpacing_, 0.01f);
				ImGui::Text("current: %.2f / %.2f", previewLoopTimer_, previewLoopSpacing_);
				break;
			}
			}
			ImGui::EndTabItem();
		}
		//================================================================================================================
		//	各キーの調整、it->second->ImGui()内では親子付けまでできる
		//================================================================================================================		
		if (ImGui::BeginTabItem("KeyObject")) {

			// keyframeObjectのImGui関数を呼びだす
			it->second.keyObject->ImGui();
			ImGui::EndTabItem();
		}
		//================================================================================================================
		// KeyEvent 編集
		//================================================================================================================
		if (ImGui::BeginTabItem("KeyEvent")) {

			EditSelectedKeyEvents(it->second);
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
	ImGui::PopItemWidth();
}

void CameraEditor::EditSelectedKeyEvents(KeyMap& keyMap) {
#if defined(_DEBUG) || defined(_DEVELOPBUILD)

	// uid 未設定(過去データ等)に対応
	for (auto& ev : keyMap.keyEvents) {
		if (ev.eventId == 0) {
			ev.eventId = keyEventUidCounter_++;
		}
	}

	// キーが無いと時間が確定できない
	const float totalKeyTime = (std::max)(keyMap.keyObject->GetTotalTime(), 0.0f);
	if (totalKeyTime <= 0.0f) {
		ImGui::TextDisabled("No keyframes. Add keys first.");
		return;
	}

	ImGui::SeparatorText("KeyEvent Timeline");
	ImGui::TextDisabled("Left click on timeline: add event / Drag circle: move event");

	// KeyframeObject3D のタイムラインをプレビュー表示(ドラッグ編集不可)
	keyMap.keyObject->DrawKeyTimelinePreview();

	// タイムライン矩形(直前の Dummy の矩形)
	const bool hoveredTimeline = ImGui::IsItemHovered();
	const ImVec2 p0 = ImGui::GetItemRectMin();
	const ImVec2 p1 = ImGui::GetItemRectMax();

	// KeyframeObject3D::DrawKeyTimelineInternal と同じ幾何計算を再現して、イベント描画用の内側バーを取る
	constexpr float kBaseBarWidth = 520.0f;
	constexpr float kBaseBarHeight = 12.0f;
	const float barWidth = (p1.x - p0.x);
	const float widthScale = std::clamp(barWidth / kBaseBarWidth, 0.55f, 1.0f);
	const float barHeight = (std::max)(kBaseBarHeight * widthScale, 6.0f);
	const float radius = (std::max)(barHeight * 0.7f, 3.0f);
	const float padX = radius + 1.0f;
	const float yCenter = (p0.y + p1.y) * 0.5f;
	ImVec2 b0 = p0;
	ImVec2 b1 = p1;
	b0.x += padX;
	b1.x -= padX;
	if (b1.x <= b0.x) {
		b0 = p0;
		b1 = p1;
	}

	ImDrawList* dl = ImGui::GetWindowDrawList();
	const ImVec2 mouse = ImGui::GetIO().MousePos;

	auto GetEventColor = [&](CameraKeyEventUpdaterID id) {
		switch (id) {
		case CameraKeyEventUpdaterID::ScreenShake:
			return IM_COL32(255, 120, 50, 255);
		case CameraKeyEventUpdaterID::Stop:
		default:
			return IM_COL32(80, 180, 255, 255);
		}
		};

	//------------------------------
	// イベント丸の描画・ドラッグ
	//------------------------------
	static uint64_t s_dragUid = 0;
	static bool s_dragging = false;
	bool anyEventHovered = false;

	for (auto& ev : keyMap.keyEvents) {
		if (!ev.updater) {
			continue;
		}
		const float t = std::clamp(ev.startTime / (std::max)(totalKeyTime, 1e-6f), 0.0f, 1.0f);
		const float x = std::lerp(b0.x, b1.x, t);
		// キー丸と重ならないように少し上側に表示
		const ImVec2 center(x, yCenter - radius * 1.8f);

		const bool hovered = ImLengthSqr(mouse - center) <= (radius * radius);
		if (hovered) {
			anyEventHovered = true;
			ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
			dl->AddCircleFilled(center, radius, IM_COL32(255, 255, 255, 255));
		}

		const bool selected = (selectedKeyEventUid_ != 0 && ev.eventId == selectedKeyEventUid_);
		const ImU32 col = GetEventColor(ev.updater->GetID());
		dl->AddCircleFilled(center, radius * 0.8f, col);
		if (selected) {
			dl->AddCircle(center, radius * 1.1f, IM_COL32(255, 255, 0, 255), 0, 2.0f);
		}

		// クリック選択 + ドラッグ開始
		if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
			selectedKeyEventUid_ = ev.eventId;
			s_dragUid = ev.eventId;
			s_dragging = true;
		}

		// ドラッグ中更新
		if (s_dragging && s_dragUid == ev.eventId) {
			if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
				float u = (b1.x > b0.x) ? (mouse.x - b0.x) / (b1.x - b0.x) : 0.0f;
				u = std::clamp(u, 0.0f, 1.0f);
				ev.startTime = u * totalKeyTime;
			} else {
				s_dragging = false;
				s_dragUid = 0;
			}
		}
	}

	//------------------------------
	// 追加ポップアップ
	//------------------------------
	static bool s_openAddPopup = false;
	static float s_addStartTime = 0.0f;
	static CameraKeyEventUpdaterID s_addUpdaterID = CameraKeyEventUpdaterID::Stop;

	if (hoveredTimeline && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !anyEventHovered) {
		float u = (b1.x > b0.x) ? (mouse.x - b0.x) / (b1.x - b0.x) : 0.0f;
		u = std::clamp(u, 0.0f, 1.0f);
		s_addStartTime = u * totalKeyTime;
		s_openAddPopup = true;
	}
	if (s_openAddPopup) {
		ImGui::OpenPopup("AddKeyEvent");
		s_openAddPopup = false;
	}

	if (ImGui::BeginPopup("AddKeyEvent")) {

		ImGui::Text("Add KeyEvent");
		ImGui::DragFloat("StartTime", &s_addStartTime, 0.01f, 0.0f, totalKeyTime);
		SakuEngine::EnumAdapter<CameraKeyEventUpdaterID>::Combo("UpdaterID", &s_addUpdaterID);

		if (ImGui::Button("Add")) {
			auto updater = UpdateRegistry::GetInstance().Create(s_addUpdaterID);
			if (updater) {
				updater->Init();
				KeyEvent ev{};
				ev.eventId = keyEventUidCounter_++;
				ev.startTime = std::clamp(s_addStartTime, 0.0f, totalKeyTime);
				ev.updater = std::move(updater);
				keyMap.keyEvents.emplace_back(std::move(ev));
				selectedKeyEventUid_ = keyMap.keyEvents.back().eventId;
			}
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel")) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	ImGui::SeparatorText("KeyEvent List");

	// 選択イベントの参照を取得
	KeyEvent* selectedEvent = nullptr;
	for (auto& ev : keyMap.keyEvents) {
		if (ev.eventId != 0 && ev.eventId == selectedKeyEventUid_) {
			selectedEvent = &ev;
			break;
		}
	}

	// リスト
	if (ImGui::BeginChild("KeyEventList", ImVec2(0, 160.0f), true)) {
		for (auto& ev : keyMap.keyEvents) {
			ImGui::PushID(static_cast<int>(ev.eventId));
			std::string label;
			if (ev.updater) {
				label = std::format("[{:.3f}s] {}", ev.startTime, EnumAdapter<CameraKeyEventUpdaterID>::ToString(ev.updater->GetID()));
			} else {
				label = std::format("[{:.3f}s] (null)", ev.startTime);
			}
			if (ImGui::Selectable(label.c_str(), selectedKeyEventUid_ == ev.eventId)) {
				selectedKeyEventUid_ = ev.eventId;
			}
			ImGui::SameLine();
			if (ImGui::Button("Delete")) {

				// 削除
				const uint64_t delUid = ev.eventId;
				ImGui::PopID();
				keyMap.keyEvents.erase(std::remove_if(keyMap.keyEvents.begin(), keyMap.keyEvents.end(),
					[&](const KeyEvent& e) { return e.eventId == delUid; }),
					keyMap.keyEvents.end());
				if (selectedKeyEventUid_ == delUid) {
					selectedKeyEventUid_ = 0;
				}
				break;
			}
			ImGui::PopID();
		}
	}
	ImGui::EndChild();

	// パラメータ編集
	if (selectedEvent && selectedEvent->updater) {
		ImGui::SeparatorText("Selected KeyEvent");
		ImGui::PushID(static_cast<int>(selectedEvent->eventId));
		ImGui::DragFloat("StartTime##Selected", &selectedEvent->startTime, 0.01f, 0.0f, totalKeyTime);
		ImGui::Text("UpdaterID: %s", EnumAdapter<CameraKeyEventUpdaterID>::ToString(selectedEvent->updater->GetID()));
		ImGui::Text(std::format("IsStarted: {}", selectedEvent->isStarted).c_str());
		ImGui::Text(std::format("IsFinished: {}", selectedEvent->isFinished).c_str());
		ImGui::Text("CurrentTime: %.3f", keyMap.keyObject->GetCurrentTimer());
		ImGui::Text("StartDuration: %.3f", keyMap.keyObject->GetStartDuration());
		ImGui::TextDisabled("IsAdvanceKey: %s", selectedEvent->updater->IsAdvanceKey() ? "true" : "false");
		ImGui::Separator();
		selectedEvent->updater->ImGui();
		ImGui::PopID();
	}

#endif
}

void CameraEditor::SaveJson(const std::string& fileName) {

	LOG_SCOPE_MS_LABEL("CameraEditorSaveJson");

	Json data;

	// キーオブジェクト
	keyMaps_[selectedKeyObjectName_].keyObject->ToJson(data["CameraKeyObject"]);
	// 名前
	data["keyName"] = selectedKeyObjectName_;

	// エディター
	data["previewMode_"] = SakuEngine::EnumAdapter<PreviewMode>::ToString(previewMode_);
	data["previewLoopSpacing_"] = previewLoopSpacing_;

	// KeyEvent
	Json keyEvents = Json::array();
	for (const auto& ev : keyMaps_[selectedKeyObjectName_].keyEvents) {

		Json evJson;
		evJson["startTime"] = ev.startTime;
		if (ev.updater) {
			evJson["updaterID"] = SakuEngine::EnumAdapter<CameraKeyEventUpdaterID>::ToString(ev.updater->GetID());
			ev.updater->ToJson(evJson["updater"]);
		}
		keyEvents.emplace_back(std::move(evJson));
	}
	data["keyEvents"] = std::move(keyEvents);

	SakuEngine::JsonAdapter::Save(fileName, data);

	LOG_INFO("saved CameraKeyData: fileName: [{}]", fileName);
}
