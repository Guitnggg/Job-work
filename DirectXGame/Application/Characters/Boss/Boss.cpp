#include "Boss.h"

#include <algorithm>
#include <cmath>

using namespace KamataEngine;

void Boss::Initialize(const Vector3& position) {
	CharacterBase::Initialize();
	basePosition_ = position;
	worldTransform_.translation_ = position;
	worldTransform_.scale_ = {4.8f, 2.1f, 2.8f};
	worldTransform_.rotation_ = {0.0f, 3.14159f, 0.0f};

	model_.reset(Model::CreateSphere());
	objectColor_.Initialize();
	objectColor_.SetColor({0.28f, 0.30f, 0.36f, 1.0f});

	coreParts_[0] = std::make_unique<BossPart>();
	coreParts_[0]->Initialize(this, BossPart::Type::LeftCore, {-3.4f, 0.0f, -0.4f}, kCoreHp, 0.85f, {1.0f, 0.28f, 0.08f, 1.0f});
	coreParts_[1] = std::make_unique<BossPart>();
	coreParts_[1]->Initialize(this, BossPart::Type::RightCore, {3.4f, 0.0f, -0.4f}, kCoreHp, 0.85f, {0.35f, 1.0f, 0.22f, 1.0f});
	bodyPart_ = std::make_unique<BossPart>();
	bodyPart_->Initialize(this, BossPart::Type::Body, {0.0f, 0.0f, 0.0f}, kBodyHp, 2.4f, {1.0f, 0.12f, 0.08f, 0.95f});
	bodyPart_->SetVisible(false);

	hp_ = kBodyHp;
	maxHp_ = kBodyHp;
	Update();
}

void Boss::Update() {
	hoverTimer_ += 1.0f / 60.0f;
	worldTransform_.translation_ = basePosition_;
	worldTransform_.translation_.y += std::sin(hoverTimer_ * 1.8f) * 0.35f;
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

	int32_t damage = 5;
	if (source == DamageSource::Laser) {
		damage = 20;
	} else if (source == DamageSource::HomingMissile) {
		damage = 15;
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
