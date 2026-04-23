#include "Bullet.h"

#include <cmath>

using namespace KamataEngine;

void Bullet::Initialize() {
    // 基底クラス初期化（WorldTransform / Collider）
    CharacterBase::Initialize();

    // モデル生成（簡易的に球）
    model_.reset(Model::CreateSphere());

    // 見た目スケール
    worldTransform_.scale_ = kBulletScale;
    worldTransform_.UpdateMatrix();

    // コライダー設定
    if (collider_) {
        collider_->SetRadius(kColliderRadius);
        collider_->SetTranslate(GetWorldTranslation());
        collider_->Update();
    }

    // 状態初期化
    isDead_ = false;
    elapsedTimeSec_ = 0.0f;
}

void Bullet::FireFrom(const Vector3& worldPos, const Vector3& dir) {
    // 発射位置と進行方向を設定
    worldTransform_.translation_ = worldPos;
    dir_ = dir;

    // 距離計測の基準点
    startPos_ = worldPos;

    // 姿勢初期化
    worldTransform_.rotation_ = kZeroRotation;
    worldTransform_.UpdateMatrix();
}

void Bullet::Update() {
    if (isDead_) {
        return;
    }

    // 固定Δt
    elapsedTimeSec_ += kFixedDeltaTime;

    // 前進（フレーム依存）
    worldTransform_.translation_.x += dir_.x * speed_;
    worldTransform_.translation_.y += dir_.y * speed_;
    worldTransform_.translation_.z += dir_.z * speed_;

    worldTransform_.UpdateMatrix();

    // コライダー追従
    if (collider_) {
        collider_->SetTranslate(GetWorldTranslation());
        collider_->Update();
    }

    // 距離判定
    const Vector3 pos = GetWorldTranslation();
    const Vector3 diff{ pos.x - startPos_.x, pos.y - startPos_.y, pos.z - startPos_.z };

    const float distanceSq = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;

    if (distanceSq > kMaxDistance * kMaxDistance) {
        isDead_ = true;
    }

    // 寿命判定（保険）
    if (elapsedTimeSec_ >= lifeTimeSec_) {
        isDead_ = true;
    }
}

void Bullet::Draw(const Camera* camera) {
    if (!camera || isDead_) {
        return;
    }

    if (model_) {
        model_->Draw(worldTransform_, *camera, textureHandle_);
    }
}

void Bullet::OnCollision(CharacterBase* /*other*/) {
    // 衝突したら即消滅
    isDead_ = true;
}
