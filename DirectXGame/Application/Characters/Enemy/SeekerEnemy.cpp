#include "SeekerEnemy.h"
#include <cmath>

using namespace KamataEngine;

// ===== SeekerEnemy.cpp 専用定数 =====
namespace {
    // 固定フレーム時間（Update 内で使用）
    constexpr float kFixedDeltaTime = 1.0f / 60.0f;

    // 見た目用の回転速度（rad/sec）
    constexpr float kYawRotateSpeed = 0.6f;

    // 衝突時ダメージ
    constexpr int kCollisionDamage = 1;

    // 強制消滅範囲
    constexpr float kKillZ = -40.0f;
    constexpr float kKillXY = 220.0f;
}

void SeekerEnemy::Initialize() {
    // 親の初期化処理を呼び出す（worldTransform_ / collider_ の生成など）
    CharacterBase::Initialize();

    // モデル読み込み（OBJがなければSphereにフォールバック）
    model_.reset(Model::CreateFromOBJ("Enemy", true));
    if (!model_) {
        model_.reset(Model::CreateSphere());
    }

    // 位置/姿勢/スケール初期化
    worldTransform_.translation_ = initialPosition_;
    worldTransform_.rotation_ = { 0.0f, 0.0f, 0.0f };
    worldTransform_.scale_ = { 1.0f, 1.0f, 1.0f };
    baseScale_ = worldTransform_.scale_;
    worldTransform_.UpdateMatrix();

    // HP設定
    maxHp_ = initialHP_;
    hp_ = maxHp_;

    // コライダー設定
    if (collider_) {
        collider_->SetRadius(colliderRadius_);
        collider_->SetTranslate(GetWorldTranslation());
        collider_->Update();
    }

    // 時間・死亡
    timeSec_ = 0.0f;
    isDead_ = false;

    // 進行方向の初期値（-Z方向に直進）
    velocity_ = { 0.0f, 0.0f, -1.0f };

    // 演出
    state_ = State::Active;
    flashTimer_ = 0.0f;
    hitStopTimer_ = 0.0f;
    pendingExplode_ = false;
}

void SeekerEnemy::Update() {
    if (isDead_) { return; }

    const float dt = kFixedDeltaTime;
    timeSec_ += dt;

    // 寿命・範囲外チェック
    ClampDeathByBounds_();
    if (isDead_) { return; }

    // ダメージフラッシュタイマー
    if (flashTimer_ > 0.0f) {
        flashTimer_ -= dt;
        if (flashTimer_ < 0.0f) { flashTimer_ = 0.0f; }
    }

    // 被弾モーションタイマー
    if (hitMotionTimer_ > 0.0f) {
        hitMotionTimer_ -= dt;
        if (hitMotionTimer_ < 0.0f) { hitMotionTimer_ = 0.0f; }
    }

    switch (state_) {
    case State::Active: {
        // ==== 通常の追尾移動 ====
        Vector3 pos = worldTransform_.translation_;

        if (hasTarget_) {
            // 目標方向
            Vector3 toTarget{
                targetPos_.x - pos.x,
                targetPos_.y - pos.y,
                targetPos_.z - pos.z
            };
            float len = std::sqrt(toTarget.x * toTarget.x + toTarget.y * toTarget.y + toTarget.z * toTarget.z);
            if (len > 1e-5f) {
                toTarget.x /= len; toTarget.y /= len; toTarget.z /= len;
            }

            // 旋回追従
            velocity_.x = (1.0f - turnRate_) * velocity_.x + turnRate_ * toTarget.x;
            velocity_.y = (1.0f - turnRate_) * velocity_.y + turnRate_ * toTarget.y;
            velocity_.z = (1.0f - turnRate_) * velocity_.z + turnRate_ * toTarget.z;

            float vlen = std::sqrt(velocity_.x * velocity_.x + velocity_.y * velocity_.y + velocity_.z * velocity_.z);
            if (vlen > 1e-5f) {
                velocity_.x /= vlen; velocity_.y /= vlen; velocity_.z /= vlen;
            }
        }

        // 前進
        pos.x += velocity_.x * speed_;
        pos.y += velocity_.y * speed_;
        pos.z += velocity_.z * speed_;

        // ここに「当たった瞬間の揺れオフセット」を足す
        if (hitMotionTimer_ > 0.0f) {
            float t = hitMotionTimer_ / hitMotionDuration_; // 1 → 0
            // 単純に t でスケールする（最初大きく、その後だんだん減る）
            float k = t;
            pos.x += hitDir_.x * hitKnockback_ * k;
            pos.y += hitDir_.y * hitKnockback_ * k;
            pos.z += hitDir_.z * hitKnockback_ * k;

            // Z回転にも少し反映（ちょっと傾く）
            worldTransform_.rotation_.z = hitBaseRollZ_ + hitRollRad_ * k;
        }

        worldTransform_.translation_ = pos;

        // 見た目用回転（ぐるぐる）
        worldTransform_.rotation_.y += kYawRotateSpeed * dt;

        worldTransform_.UpdateMatrix();

        // コライダー追従
        if (collider_) {
            collider_->SetTranslate(GetWorldTranslation());
            collider_->Update();
        }
    }
                      break;

    case State::HitStop: {
        // ヒットストップ中：移動・AIは止めるが、「被弾モーション」は行う
        hitStopTimer_ -= dt;
        float tMotion = 0.0f;

        if (hitMotionTimer_ > 0.0f) {
            float t = hitMotionTimer_ / hitMotionDuration_; // 1 → 0
            tMotion = t;
        }

        // 基準位置＋ノックバック
        Vector3 pos = hitBasePos_;
        if (tMotion > 0.0f) {
            pos.x += hitDir_.x * hitKnockback_ * tMotion;
            pos.y += hitDir_.y * hitKnockback_ * tMotion;
            pos.z += hitDir_.z * hitKnockback_ * tMotion;

            worldTransform_.rotation_.z = hitBaseRollZ_ + hitRollRad_ * tMotion;
        }
        else {
            // モーション終了後は元の姿勢に戻す
            worldTransform_.rotation_.z = hitBaseRollZ_;
        }

        worldTransform_.translation_ = pos;
        worldTransform_.UpdateMatrix();

        if (collider_) {
            collider_->SetTranslate(GetWorldTranslation());
            collider_->Update();
        }

        if (hitStopTimer_ <= 0.0f) {
            hitStopTimer_ = 0.0f;
            if (pendingExplode_) {
                // ヒットストップ終了後に消滅
                isDead_ = true;
            }
            else {
                state_ = State::Active;
            }
        }
    }
                       break;
    }
}

void SeekerEnemy::Draw(const Camera* camera) {
    if (!camera || isDead_ || !model_) { return; }

    // 一旦スケールを保存
    Vector3 originalScale = worldTransform_.scale_;

    if (flashTimer_ > 0.0f) {
        float t = flashTimer_ / flashDuration_; // 1 → 0

        float scaleMul = 1.0f + 0.5f * t; // 最大 1.5倍 まで膨らむ

        // スケールパンチ
        worldTransform_.scale_.x = baseScale_.x * scaleMul;
        worldTransform_.scale_.y = baseScale_.y * scaleMul;
        worldTransform_.scale_.z = baseScale_.z * scaleMul;
        worldTransform_.UpdateMatrix();

        // 簡易点滅（最初の方だけチカチカ）
        bool visible = true;
        if (t > 0.5f) {
            int blink = static_cast<int>(t * 10.0f);
            visible = (blink % 2) == 0;
        }

        if (visible) {
            model_->Draw(worldTransform_, *camera, textureHandle_);
        }
    }
    else {
        model_->Draw(worldTransform_, *camera, textureHandle_);
    }

    // スケールを戻す
    worldTransform_.scale_ = originalScale;
    worldTransform_.UpdateMatrix();
}

void SeekerEnemy::OnCollision(CharacterBase* /*other*/) {
    if (isDead_) { return; }

    // とりあえず 1 ダメージ固定
    hp_ -= kCollisionDamage;
    if (hp_ < 0) hp_ = 0;

    // ==== 当たった瞬間の演出セットアップ ====

    // ダメージフラッシュ
    flashTimer_ = flashDuration_;

    // ヒットストップ
    hitStopTimer_ = hitStopDuration_;
    state_ = State::HitStop;

    // 被弾モーションの基準
    hitBasePos_ = worldTransform_.translation_;
    hitBaseRollZ_ = worldTransform_.rotation_.z;
    hitMotionTimer_ = hitMotionDuration_;

    // ノックバック方向：基本は「進行方向の反対」
    hitDir_ = { -velocity_.x, -velocity_.y, -velocity_.z };
    float len = std::sqrt(hitDir_.x * hitDir_.x + hitDir_.y * hitDir_.y + hitDir_.z * hitDir_.z);
    if (len > 1e-5f) {
        hitDir_.x /= len; hitDir_.y /= len; hitDir_.z /= len;
    }
    else {
        // 万が一0ベクトルなら手前方向に押す
        hitDir_ = { 0.0f, 0.0f, -1.0f };
    }

    // HP0 ならヒットストップ後に消す
    if (hp_ <= 0) {
        pendingExplode_ = true;
    }
}

void SeekerEnemy::ClampDeathByBounds_() {
    const Vector3 p = GetWorldTranslation();

    // 画面外orZ手前などで強制消去
    if (p.z < kKillZ || std::abs(p.x) > kKillXY || std::abs(p.y) > kKillXY) {
        isDead_ = true;
    }

    // 寿命
    if (timeSec_ >= lifeTimeSec_) {
        isDead_ = true;
    }
}
