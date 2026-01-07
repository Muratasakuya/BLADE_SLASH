#include "BasePlayerMoveKeyAttack.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Object/Core/ObjectManager.h>
#include <Game/Gameplay/Actors/Player/Entity/Player.h>

//============================================================================
//	BasePlayerMoveKeyAttack classMethods
//============================================================================

void BasePlayerMoveKeyAttack::Init(const std::string& label) {

	// 空の親トランスフォームの生成
	CreateTransform(label, "MoveTransform", moveTransform_, moveTag_, false);
	// 攻撃対象のトランスフォーム補正用の生成
	CreateTransform(label, "FixedTargetTransform", fixedTargetTransform_, fixedTargetTag_, true);
}

void BasePlayerMoveKeyAttack::UpdateTransform() {

	// 行列更新
	moveTransform_->UpdateMatrix();
}

void BasePlayerMoveKeyAttack::CreateTransform(const std::string& label, const std::string& name,
	SakuEngine::Transform3D*& transform, SakuEngine::ObjectTag*& tag, bool isCompulsion) {

	SakuEngine::ObjectManager* objectManager = SakuEngine::ObjectManager::GetInstance();

	// タグの作成
	std::string objectName = label + name;
	uint32_t id = objectManager->BuildEmptyobject(objectName, "Player");
	tag = objectManager->GetData<SakuEngine::ObjectTag>(id);
	// トランスフォームを追加
	transform = objectManager->GetObjectPoolManager()->AddData<SakuEngine::Transform3D>(id);
	transform->Init();
	transform->SetInstancingName(tag->name);
	transform->isCompulsion_ = isCompulsion;
}

void BasePlayerMoveKeyAttack::SetTargetByRange(SakuEngine::KeyframeObject3D& keyObject) {

	// 攻撃可能範囲にいるかチェック
	isInRange_ = areaChecker_->IsInRange(AreaReactionType::LerpPos);
	if (isInRange_) {

		// 位置補正用トランスフォームを攻撃対象の位置に設定
		fixedTargetTransform_->translation = attackTarget_->GetTranslation();
		fixedTargetTransform_->translation.y = 0.0f;
		fixedTargetTransform_->rotation = attackTarget_->GetRotation();
		// 行列を更新
		fixedTargetTransform_->UpdateMatrix();

		// 範囲内なので攻撃対象を親の位置として設定する
		keyObject.SetParent(fixedTargetTag_->name, *fixedTargetTransform_);
	} else {

		// 範囲外なので空の親トランスフォームを親の位置として設定する
		keyObject.SetParent(moveTag_->name, *moveTransform_);
	}
}