#include "Smoke.h"

using namespace KamataEngine;

void Smoke::Initialize(KamataEngine::Model* model,
    const KamataEngine::Vector3& position,
    const KamataEngine::Vector3& velocity,
    float lifeTime,
    float startScale,
    float endScale
) {
    // パラメータ受け取り
    model_ = model;
    velocity_ = velocity;
    life_ = 0.0f;
    maxLife_ = lifeTime;
    startScale_ = startScale;
    endScale_ = endScale;
    isFinished_ = false;

    // ワールド変換の初期化
    worldTransform_.Initialize();
    worldTransform_.translation_ = position;
    worldTransform_.rotation_ = { 0.0f, 0.0f, 0.0f };
    worldTransform_.scale_ = { startScale_, startScale_, startScale_ };
    worldTransform_.UpdateMatrix();
}

void Smoke::Update(float dt) {
    if (isFinished_) { return; }

    // 経過時間
    life_ += dt;
    if (life_ >= maxLife_) {
        isFinished_ = true;
        return;
    }

    // 位置更新
    worldTransform_.translation_.x += velocity_.x;
    worldTransform_.translation_.y += velocity_.y;
    worldTransform_.translation_.z += velocity_.z;

    // スケール補間（線形）
    float t = life_ / maxLife_;
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    float s = startScale_ + (endScale_ - startScale_) * t;
    if (s < 0.0f) s = 0.0f;

    worldTransform_.scale_ = { s, s, s };

    // 行列更新
    worldTransform_.UpdateMatrix();
}

void Smoke::Draw(const Camera* camera) {
    if (isFinished_) { return; }
    model_->Draw(worldTransform_, *camera);
}