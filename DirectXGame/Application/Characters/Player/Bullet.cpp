#include "Bullet.h"

#include "Application/Utility/GameTime.h"

#include <cmath>

using namespace KamataEngine;

// 通常弾のモデル・変換情報・コライダーなどの初期状態を準備する
void Bullet::Initialize() {
    // 基底クラス初期化（WorldTransform / Collider）
    CharacterBase::Initialize();

    // モデル生成（簡易的に球）
    model_.reset(Model::CreateSphere());
    objectColor_.Initialize();
    objectColor_.SetColor({ 0.35f, 0.95f, 1.0f, 1.0f });

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
    // Object Pool Pattern:
    // Initialize() 直後はまだ発射されていないため、未使用状態として扱う。
    // BulletManager::AcquireBullet_() から FireFrom() が呼ばれた時に active になる。
    isDead_ = true;
    elapsedTimeSec_ = 0.0f;
}

// 指定位置と方向から弾を発射状態にし、向き・速度・寿命を設定する
void Bullet::FireFrom(const Vector3& worldPos, const Vector3& dir) {
    // Object Pool Pattern:
    // 再利用された弾でも正しく撃ち直せるように、発射時に寿命と死亡フラグをリセットする。
    isDead_ = false;
    elapsedTimeSec_ = 0.0f;

    // 発射位置と進行方向を設定
    worldTransform_.translation_ = worldPos;
    dir_ = dir;

    // 距離計測の基準点
    startPos_ = worldPos;

    // 姿勢初期化
    worldTransform_.rotation_ = kZeroRotation;
    worldTransform_.UpdateMatrix();
}

// 発射中の弾を移動させ、寿命や移動距離に応じて消滅判定を行う
void Bullet::Update() {
    if (isDead_) {
        return;
    }

    // 固定Δt
    elapsedTimeSec_ += GameTime::kDeltaTime;

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

// 発射中の通常弾とデバック用コライダーを描画する
void Bullet::Draw(const Camera* camera) {
    if (!camera || isDead_) {
        return;
    }

    if (model_) {
        model_->Draw(worldTransform_, *camera, textureHandle_, &objectColor_);
    }
}

// 弾が何かにあたって時に死亡フラグを立て、次の管理更新で消せるようにする
void Bullet::OnCollision(CharacterBase* /*other*/) {
    // 衝突したら即消滅
    isDead_ = true;
}
