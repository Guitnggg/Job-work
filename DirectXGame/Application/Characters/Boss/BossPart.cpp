#include "BossPart.h"

#include "Application/Characters/Boss/Boss.h"

using namespace KamataEngine;

void BossPart::Initialize(Boss* owner, Type type, const Vector3& localOffset, int32_t hp, float radius, const Vector4& color) {
	owner_ = owner;
	type_ = type;
	localOffset_ = localOffset;

	CharacterBase::Initialize();
	maxHp_ = hp;
	hp_ = hp;

	model_.reset(Model::CreateSphere());
	objectColor_.Initialize();
	objectColor_.SetColor(color);
	worldTransform_.scale_ = {radius, radius, radius};

	if (collider_) {
		collider_->SetRadius(radius);
	}
}

void BossPart::Update() {
	if (!owner_) {
		CharacterBase::Update();
		return;
	}

	const Vector3 ownerPos = owner_->GetWorldTranslation();
	worldTransform_.translation_ = {ownerPos.x + localOffset_.x, ownerPos.y + localOffset_.y, ownerPos.z + localOffset_.z};
	CharacterBase::Update();
}

void BossPart::Draw(const Camera* camera) {
	if (!camera || !model_ || !isVisible_ || IsDead()) {
		return;
	}

	model_->Draw(worldTransform_, *camera, textureHandle_, &objectColor_);
}

void BossPart::OnCollision(CharacterBase* /*other*/) {
	if (!owner_ || !IsCollidable()) {
		return;
	}
	owner_->ApplyPartDamage(*this, GetLastDamageSource());
}

bool BossPart::IsCollidable() const {
	if (IsDead()) {
		return false;
	}
	if (type_ == Type::Body && owner_) {
		return owner_->CanDamageBody();
	}
	return true;
}
