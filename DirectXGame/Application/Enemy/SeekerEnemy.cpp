#include "SeekerEnemy.h"
#include <cmath>

using namespace KamataEngine;

void SeekerEnemy::Initialize() {

    // 親の初期化処理を呼び出す（worldTransform_ / collider_ の生成まで）
    EnemyBase::Initialize();

    // 見た目モデル（OBJがなければSphereにフォールバック）
    model_.reset(Model::CreateFromOBJ("Enemy", true));
    if (!model_) {
        model_.reset(Model::CreateSphere());
    }

    // 位置/姿勢/スケール初期化
    worldTransform_.translation_ = initialPosition_;
    worldTransform_.rotation_ = { 0.0f, 0.0f, 0.0f };
    worldTransform_.scale_ = { 1.0f, 1.0f, 1.0f };
    worldTransform_.UpdateMatrix();

    // HP設定
    maxHp_ = initialHP_;
    HP_ = maxHp_;

    // コライダー設定
    if (collider_) {
        collider_->SetRadius(colliderRadius_);
        collider_->SetTranslate(GetWorldTranslation());
        collider_->Update();
    }

    // タイマー/死亡フラグ初期化
    timeSec_ = 0.0f;
    isDead_ = false;

    // 進行方向の初期値（-Z直進）
    velocity_ = { 0.0f, 0.0f, -1.0f };
}

void SeekerEnemy::Update() {
    if (isDead_) { return; }

    // 固定Δt（エンジンに可変Δtがあるなら差し替えてOK）
    const float dt = 1.0f / 60.0f;
    timeSec_ += dt;

    // ---- 移動：目標へ向かう単純ステアリング ----
    Vector3 pos = worldTransform_.translation_;
    if (hasTarget_) {
        // 目標方向（正規化）
        Vector3 toTarget = { targetPos_.x - pos.x, targetPos_.y - pos.y, targetPos_.z - pos.z };
        float len = std::sqrt(toTarget.x * toTarget.x + toTarget.y * toTarget.y + toTarget.z * toTarget.z);
        if (len > 1e-5f) {
            toTarget.x /= len; toTarget.y /= len; toTarget.z /= len;
        }

        // 旋回追従（一次フィルタ）
        velocity_.x = (1.0f - turnRate_) * velocity_.x + turnRate_ * toTarget.x;
        velocity_.y = (1.0f - turnRate_) * velocity_.y + turnRate_ * toTarget.y;
        velocity_.z = (1.0f - turnRate_) * velocity_.z + turnRate_ * toTarget.z;

        // 正規化
        float vlen = std::sqrt(velocity_.x * velocity_.x + velocity_.y * velocity_.y + velocity_.z * velocity_.z);
        if (vlen > 1e-5f) {
            velocity_.x /= vlen; velocity_.y /= vlen; velocity_.z /= vlen;
        }
    }

    // 前進
    pos.x += velocity_.x * speed_;
    pos.y += velocity_.y * speed_;
    pos.z += velocity_.z * speed_;
    worldTransform_.translation_ = pos;

    // 見た目用の簡易ヨー回転
    worldTransform_.rotation_.y += 0.6f * dt;

    // 行列更新
    worldTransform_.UpdateMatrix();

    // コライダー追従
    if (collider_) {
        collider_->SetTranslate(GetWorldTranslation());
        collider_->Update();
    }

    // 画面外/寿命/HPで消す
    ClampDeathByBounds_();
    if (HP_ <= 0) {
        isDead_ = true;
    }
}

void SeekerEnemy::Draw(KamataEngine::Camera* camera)
{
    if (!camera || isDead_) { return; }
    if (model_) {
        model_->Draw(worldTransform_, *camera, textureHandle_);
    }
    // デバッグ可視化したければ
    // if (collider_) { collider_->Draw(*camera); }
}

void SeekerEnemy::OnCollision(EnemyBase* /*enemy*/){
    // １回当たったら削除
    isDead_ = true;
}

void SeekerEnemy::ClampDeathByBounds_() {
    const Vector3 p = GetWorldTranslation();
    if (p.z < -40.0f || p.z > 220.0f || std::abs(p.x) > 220.0f || std::abs(p.y) > 220.0f) {
        isDead_ = true;
    }
    if (timeSec_ >= lifeTimeSec_) {
        isDead_ = true;
    }
}
