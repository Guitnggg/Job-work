#include "Player.h"
#include <algorithm>

using namespace KamataEngine;

void Player::Initialize(Camera* camera) {
    // 
    CharactorBase::Initialize();

    camera_ = camera;
    model_ = Model::CreateFromOBJ("Player", true);

    audio_ = Audio::GetInstance();
    seExplosion_ = audio_->LoadWave("./Resources/SE/Explosion.wav");

    worldTransform_.translation_ = { 0.0f,-2.0f,20.0f };
    worldTransform_.scale_ = { 1.0f,1.0f,1.0f };
    worldTransform_.rotation_ = { 0.0f,0.0f,0.0f };

    input_ = Input::GetInstance();

    // HP
    SetHP(100);
    isDead_ = false;
    isExploding_ = false;
    isExplosionFinished_ = false;
    explosionFrame_ = 0;

    // コライダー初期設定（必要に応じて半径調整）
    if (collider_) {
        collider_->SetRadius(1.0f);
        collider_->SetTranslate(GetWorldTranslation());
    }
}

void Player::Update() {
    // 爆発中は演出優先
    if (isExploding_) {
        UpdateExplosion_();
        worldTransform_.UpdateMatrix();
        // Collider追従
        if (collider_) { 
            collider_->SetTranslate(GetWorldTranslation()); 
        }
        return;
    }
    if (isDead_) {
        // 消滅後：何もしない
        return;
    }

    // ロールアニメ中
    if (isRolling_) {
        rollFrame_ += 1.0f;
        if (rollFrame_ > rollDurationFrames_) {
            rollFrame_ = rollDurationFrames_;
        }

        // イージング
        float t = rollFrame_ / rollDurationFrames_;
        float et = EaseOutCubic(t);

        worldTransform_.translation_.x = rollStartPos_.x + (rollEndPos_.x - rollStartPos_.x) * et;
        worldTransform_.translation_.y = rollStartPos_.y;
        worldTransform_.translation_.z = rollStartPos_.z;

        // Z回転は±2πの一回転
        const float twoPi = 6.28318530717958647692f;
        worldTransform_.rotation_.z = rollStartRotZ_ - rollDir_ * twoPi * t;

        if (rollFrame_ >= rollDurationFrames_) {
            // 終了で位相を元に戻す
            worldTransform_.rotation_.z = rollStartRotZ_;
            isRolling_ = false;
        }

        worldTransform_.UpdateMatrix();
        if (collider_) {
            collider_->SetTranslate(GetWorldTranslation());
        }
        return;
    }

    // 入力（今回はA/Dでロール）
    if (input_ && input_->TriggerKey(DIK_D)) { StartRoll(1.0f); }
    if (input_ && input_->TriggerKey(DIK_A)) { StartRoll(-1.0f); }

    // 画面左右クランプ
    worldTransform_.translation_.x = std::clamp(worldTransform_.translation_.x, -8.0f, 8.0f);

    // 行列更新 & コライダー追従
    worldTransform_.UpdateMatrix();
    if (collider_) { 
        collider_->SetTranslate(GetWorldTranslation());
    }
}

void Player::Draw(Camera* camera) {
    if (isExplosionFinished_) { return; }
    if (!model_ || !camera_) { return; }

    model_->Draw(worldTransform_, *camera);


}

void Player::OnCollision(CharactorBase* /*enemy*/) {
    // 例：衝突でダメージ
    Damage(10);
}

void Player::Damage(int amount) {
    if (isExploding_ || isDead_) { return;}
    int hp = GetHP();
    hp -= amount;
    SetHP(hp);
    if (hp <= 0) {
        Kill();
    }
}

void Player::Kill() {
    if (isExploding_ || isDead_) { return; }
    isExploding_ = true;
    explosionFrame_ = 0;
    isExplosionFinished_ = false;

    if (audio_ && seExplosion_ >= 0) {
        audio_->PlayWave(seExplosion_);
    }
}

void Player::StartRoll(float dir) {
    if (isRolling_) { return; }
    isRolling_ = true;
    rollFrame_ = 0.0f;
    rollDir_ = dir;
    rollStartRotZ_ = worldTransform_.rotation_.z;
    rollStartPos_ = worldTransform_.translation_;

    rollEndPos_ = rollStartPos_;
    rollEndPos_.x += rollMoveDistance_ * rollDir_;
}

float Player::EaseOutCubic(float t) const {
    float inv = 1.0f - t;
    return 1.0f - inv * inv * inv;
}

void Player::UpdateExplosion_() {
    // 0.0 → 1.0
    float t = static_cast<float>(explosionFrame_) / static_cast<float>(explosionDurationFrames_);
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;

    // 後半加速のEaseOut
    float et = EaseOutCubic(t);

    // 縮小：1.0 → 0.0
    float scaleMul = 1.0f - et;
    if (scaleMul < 0.0f) scaleMul = 0.0f;
    worldTransform_.scale_ = { initialScale_.x * scaleMul,
                               initialScale_.y * scaleMul,
                               initialScale_.z * scaleMul };

    // 演出
    worldTransform_.translation_.y -= 0.1f;
    worldTransform_.rotation_.y += 0.2f;

    explosionFrame_++;
    if (explosionFrame_ >= explosionDurationFrames_) {
        isExploding_ = false;
        isDead_ = true;
        isExplosionFinished_ = true;

        // 完全消滅
        worldTransform_.scale_ = { 0.0f, 0.0f, 0.0f };
    }
}