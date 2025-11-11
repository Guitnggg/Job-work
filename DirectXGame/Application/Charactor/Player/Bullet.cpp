#include "Bullet.h"
#include <cmath>

using namespace KamataEngine;

void Bullet::Initialize() {
    CharactorBase::Initialize();

    // 見た目（Bullet.obj が無ければ球）
    model_.reset(Model::CreateSphere());
    if (!model_) { model_.reset(Model::CreateSphere()); }

    // 小さめスケール
    worldTransform_.scale_ = { 0.4f, 0.4f, 0.7f };
    worldTransform_.UpdateMatrix();

    // コライダー（小さめ）
    if (collider_) {
        collider_->SetRadius(0.4f);
        collider_->SetTranslate(GetWorldTranslation());
        collider_->Update();
    }
}

void Bullet::FireFrom(const Vector3& worldPos, const Vector3& dir) {
    worldTransform_.translation_ = worldPos;
    dir_ = dir;
    // Z方向へ向ける簡易回転（必要なら厳密に）
    worldTransform_.rotation_ = { 0.0f, 0.0f, 0.0f };
    worldTransform_.UpdateMatrix();
}

void Bullet::Update() {
    if (isDead_) { return; }

    const float dt = 1.0f / 60.0f;
    t_ += dt;

    // 前進
    worldTransform_.translation_.x += dir_.x * speed_;
    worldTransform_.translation_.y += dir_.y * speed_;
    worldTransform_.translation_.z += dir_.z * speed_;

    worldTransform_.UpdateMatrix();

    // コライダー追従
    if (collider_) {
        collider_->SetTranslate(GetWorldTranslation());
        collider_->Update();
    }

    // 画面外や寿命で消す（ざっくり）
    const Vector3 p = GetWorldTranslation();
    if (t_ >= lifeTimeSec_ || p.z > 220.0f || p.z < -40.0f || std::abs(p.x) > 220.0f || std::abs(p.y) > 220.0f) {
        isDead_ = true;
    }
}

void Bullet::Draw(Camera* camera) {
    if (!camera || isDead_) { return; }
    if (model_) { model_->Draw(worldTransform_, *camera, textureHandle_); }

    // デバッグ：コライダー可視化したい場合は下記
    // if (collider_) { collider_->Draw(*camera); }
}

void Bullet::OnCollision(CharactorBase* /*other*/) {
    // 当たったら弾は消える
    isDead_ = true;
}