#include "Bullet.h"
#include <cmath>

using namespace KamataEngine;

// ===== Bullet.cpp 専用定数（実装詳細）=====
namespace {
    // 固定Δt
    constexpr float kFixedDeltaTime = 1.0f / 60.0f;

    // 見た目スケール（弾の形）
    constexpr Vector3 kBulletScale{ 0.4f, 0.4f, 0.7f };

    // コライダー半径
    constexpr float kColliderRadius = 0.4f;

    // 姿勢リセット
    constexpr Vector3 kZeroRot{ 0.0f, 0.0f, 0.0f };
}


void Bullet::Initialize() {
    // 親クラスの初期化
    CharacterBase::Initialize();

    // 見た目（Bullet.obj が無ければ球）
    model_.reset(Model::CreateSphere());

    // 小さめスケール
    worldTransform_.scale_ = kBulletScale;
    worldTransform_.UpdateMatrix();

    // コライダー（小さめ）
    if (collider_) {
        collider_->SetRadius(kColliderRadius);
        collider_->SetTranslate(GetWorldTranslation());
        collider_->Update();
    }
}

void Bullet::FireFrom(const Vector3& worldPos, const Vector3& dir) {
    // 座標、進行方向セット
    worldTransform_.translation_ = worldPos;
    dir_ = dir;

    // 飛行距離カウントの基準座標
    startPos_ = worldPos;

    // 姿勢リセット
    worldTransform_.rotation_ = kZeroRot;
    worldTransform_.UpdateMatrix();
}

void Bullet::Update() {
    if (isDead_) { return; }

    const float dt = kFixedDeltaTime;
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

    // 発射位置からの距離を計算して一定距離を超えたら消す
    const Vector3 p = GetWorldTranslation();
    Vector3 diff = { p.x - startPos_.x, p.y - startPos_.y, p.z - startPos_.z };
    float distanceSq = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;

    if (distanceSq > maxDistance_ * maxDistance_) {
        isDead_ = true;
    }

    // 寿命でも消す（保険）
    if (t_ >= lifeTimeSec_) {
        isDead_ = true;
    }
}

void Bullet::Draw(const Camera* camera) {
    if (!camera || isDead_) { return; }
    if (model_) { model_->Draw(worldTransform_, *camera, textureHandle_); }

    // デバッグ：コライダー可視化したい場合は下記
    // if (collider_) { collider_->Draw(*camera); }
}

void Bullet::OnCollision(CharacterBase* /*other*/) {
    // 当たったら弾は消える
    isDead_ = true;
}