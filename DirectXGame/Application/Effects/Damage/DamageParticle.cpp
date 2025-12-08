#include "DamageParticle.h"

using namespace KamataEngine;

void DamageParticle::Initialize(KamataEngine::Model* model,
    const KamataEngine::Vector3& pos,
    const KamataEngine::Vector3& vel,
    float lifeTime,
    float startScale,
    float endScale) {

    // パラメータ受け取り
    model_ = model;
    velocity_ = vel;
    lifeTime_ = lifeTime;
    startScale_ = startScale;
    endScale_ = endScale;

    // 変換初期化
    worldTransform_.Initialize();
    worldTransform_.translation_ = pos;

    // タイマー
    currentTime_ = 0.0f;
}

void DamageParticle::Update(float dt) {
    // 時間経過
    currentTime_ += dt;

    // 速度による移動
    worldTransform_.translation_.x += velocity_.x * dt;
    worldTransform_.translation_.y += velocity_.y * dt;
    worldTransform_.translation_.z += velocity_.z * dt;

    // スケール補間
    float t = (std::min)(currentTime_ / lifeTime_, 1.0f);
    float scale = startScale_ + (endScale_ - startScale_) * t;
    worldTransform_.scale_ = { scale, scale, scale };

    // 行列更新
    worldTransform_.UpdateMatrix();
}

void DamageParticle::Draw(KamataEngine::Camera* camera) {
    // モデルが有効の時に描画
    if (model_ && !IsFinished()) {
        model_->Draw(worldTransform_, *camera);
    }
}