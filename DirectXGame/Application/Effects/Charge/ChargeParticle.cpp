#include "ChargeParticle.h"
#include <algorithm>
#include <cmath>

using namespace KamataEngine;

void ChargeParticle::Initialize(Model* model, const Vector3& pos, const Vector3& vel, float lifeTime, float startScale, float endScale, float attractStrength) {
	model_ = model;
	velocity_ = vel;
	lifeTime_ = lifeTime;
	startScale_ = startScale;
	endScale_ = endScale;
	attractStrength_ = attractStrength;

	worldTransform_.Initialize();
	worldTransform_.translation_ = pos;

	currentTime_ = 0.0f;
}

void ChargeParticle::Update(float dt, const Vector3& targetPos) {
	currentTime_ += dt;

	// ターゲット方向へ吸引（加速度）
	if (attractStrength_ > 0.0f) {
		Vector3 toTarget = {targetPos.x - worldTransform_.translation_x, targetPos.y - worldTransform_.translation_.y, targetPos.z - worldTransform_.translation_.z};

		const float distSq = toTarget.x * toTarget.x + toTarget.y * toTarget.y + toTarget.z * toTarget.z;
		const float dist = std::sqrt((std::max)(distSq, 0.00001f));

		// 近づいたら消す（集まった感）
		if (dist < killDistance_) {
			currentTime_ = lifeTime_;
			return;
		}

		// 正規化して加速
		toTarget.x /= dist;
		toTarget.y /= dist;
		toTarget.z /= dist;
		velocity_.x += toTarget.x * attractStrength_ * dt;
		velocity_.y += toTarget.y * attractStrength_ * dt;
		velocity_.z += toTarget.z * attractStrength_ * dt;
	}

	// 移動
	worldTransform_.translation_.x += velocity_.x * dt;
	worldTransform_.translation_.y += velocity_.y * dt;
	worldTransform_.translation_.z += velocity_.z * dt;

	// スケール補間（DamageParticleと同じ）
	const float t = (std::min)(currentTime_ / (std::max)(lifeTime_, 0.0001f), 1.0f);
	const float scale = startScale_ + (endScale_ - startScale_) * t;
	worldTransform_.scale_ = {scale, scale, scale};

	worldTransform_.UpdateMatrix();
}

void ChargeParticle::Draw(Camera* camera) {
	if (model_ && camera && !IsFinished()) {
		model_->Draw(worldTransform_, *camera);
	}
}