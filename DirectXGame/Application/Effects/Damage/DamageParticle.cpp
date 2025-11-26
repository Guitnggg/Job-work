#include "DamageParticle.h"

using namespace KamataEngine;

void DamageParticle::Initialize(KamataEngine::Model* model,
    const KamataEngine::Vector3& pos,
    const KamataEngine::Vector3& vel,
    float lifeTime,
    float startScale,
    float endScale)
{
    model_ = model;
    worldTransform_.Initialize();
    worldTransform_.translation_ = pos;
    velocity_ = vel;
    lifeTime_ = lifeTime;
    startScale_ = startScale;
    endScale_ = endScale;
    currentTime_ = 0.0f;
}

void DamageParticle::Update(float dt) {
    currentTime_ += dt;

    worldTransform_.translation_.x += velocity_.x * dt;
    worldTransform_.translation_.y += velocity_.y * dt;
    worldTransform_.translation_.z += velocity_.z * dt;

    float t = (std::min)(currentTime_ / lifeTime_, 1.0f);
    float scale = startScale_ + (endScale_ - startScale_) * t;
    worldTransform_.scale_ = { scale, scale, scale };

    worldTransform_.UpdateMatrix();
}

void DamageParticle::Draw(KamataEngine::Camera* camera) {
    if (model_ && !IsFinished()) {
        model_->Draw(worldTransform_, *camera);
    }
}