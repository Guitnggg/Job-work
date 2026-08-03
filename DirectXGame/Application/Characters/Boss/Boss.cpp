#include "Boss.h"

#include "Application/Utility/GameTime.h"

#include <algorithm>
#include <cmath>

using namespace KamataEngine;

namespace {
constexpr Vector3 kBossScale{4.8f, 2.1f, 2.8f};
constexpr Vector3 kBossRotation{0.0f, 3.14159f, 0.0f};
constexpr Vector4 kBossColor{0.28f, 0.30f, 0.36f, 1.0f};
constexpr Vector3 kLeftCoreOffset{-3.4f, 0.0f, -0.4f};
constexpr Vector3 kRightCoreOffset{3.4f, 0.0f, -0.4f};
constexpr float kCoreColliderRadius = 0.85f;
constexpr float kBodyColliderRadius = 2.4f;
constexpr Vector4 kLeftCoreColor{1.0f, 0.28f, 0.08f, 1.0f};
constexpr Vector4 kRightCoreColor{0.35f, 1.0f, 0.22f, 1.0f};
constexpr Vector4 kBodyColor{1.0f, 0.12f, 0.08f, 0.95f};
constexpr float kHoverAngularSpeed = 1.8f;
constexpr float kHoverAmplitude = 0.35f;
constexpr int32_t kDefaultAttackDamage = 5;
constexpr int32_t kHomingMissileDamage = 15;
}

void Boss::Initialize(const Vector3& position) {
	CharacterBase::Initialize();
	basePosition_ = position;
	worldTransform_.translation_ = position;
	worldTransform_.scale_ = kBossScale;
	worldTransform_.rotation_ = kBossRotation;

	model_.reset(Model::CreateSphere());
	objectColor_.Initialize();
	objectColor_.SetColor(kBossColor);

	coreParts_[0] = std::make_unique<BossPart>();
	coreParts_[0]->Initialize(this, BossPart::Type::LeftCore, kLeftCoreOffset, kCoreHp, kCoreColliderRadius, kLeftCoreColor);
	coreParts_[1] = std::make_unique<BossPart>();
	coreParts_[1]->Initialize(this, BossPart::Type::RightCore, kRightCoreOffset, kCoreHp, kCoreColliderRadius, kRightCoreColor);
	bodyPart_ = std::make_unique<BossPart>();
	bodyPart_->Initialize(this, BossPart::Type::Body, {}, kBodyHp, kBodyColliderRadius, kBodyColor);
	bodyPart_->SetVisible(false);

	hp_ = kBodyHp;
	maxHp_ = kBodyHp;
	Update();
}

void Boss::Update() {
	hoverTimer_ += GameTime::kDeltaTime;
	worldTransform_.translation_ = basePosition_;
	worldTransform_.translation_.y += std::sin(hoverTimer_ * kHoverAngularSpeed) * kHoverAmplitude;
	CharacterBase::Update();

	const bool bodyVisible = AreCoresDestroyed();
	if (bodyPart_) {
		bodyPart_->SetVisible(bodyVisible);
	}
	for (auto& core : coreParts_) {
		if (core) {
			core->Update();
		}
	}
	if (bodyPart_) {
		bodyPart_->Update();
		hp_ = bodyPart_->GetHP();
	}
}

void Boss::Draw(const Camera* camera) {
	if (!camera || IsDead()) {
		return;
	}
	if (model_) {
		model_->Draw(worldTransform_, *camera, textureHandle_, &objectColor_);
	}
	for (auto& core : coreParts_) {
		if (core) {
			core->Draw(camera);
		}
	}
	if (bodyPart_) {
		bodyPart_->Draw(camera);
	}
}

void Boss::OnCollision(CharacterBase* /*other*/) {}

bool Boss::AreCoresDestroyed() const {
	return std::all_of(coreParts_.begin(), coreParts_.end(), [](const std::unique_ptr<BossPart>& part) { return !part || part->IsDead(); });
}

void Boss::ApplyPartDamage(BossPart& part, DamageSource source) {
	if (!part.IsCollidable()) {
		return;
	}

	int32_t damage = kDefaultAttackDamage;
	if (source == DamageSource::HomingMissile) {
		damage = kHomingMissileDamage;
	}

	part.SetHP((std::max)(0, part.GetHP() - damage));
	if (&part == bodyPart_.get()) {
		hp_ = part.GetHP();
	}
}

float Boss::GetHpRate() const {
	const int32_t maxHp = GetBodyMaxHP();
	if (maxHp <= 0) {
		return 0.0f;
	}
	return std::clamp(static_cast<float>(GetBodyHP()) / static_cast<float>(maxHp), 0.0f, 1.0f);
}

std::vector<BossPart*> Boss::GetCollisionParts() {
	std::vector<BossPart*> parts;
	for (auto& core : coreParts_) {
		if (core && core->IsCollidable()) {
			parts.push_back(core.get());
		}
	}
	if (bodyPart_ && bodyPart_->IsCollidable()) {
		parts.push_back(bodyPart_.get());
	}
	return parts;
}
