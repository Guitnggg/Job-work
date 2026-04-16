#include "Player.h"

#include <algorithm>
#include <cmath>

using namespace KamataEngine;

void Player::Initialize(Camera* camera) {
    CharacterBase::Initialize();

    camera_ = camera;
    model_ = Model::CreateFromOBJ("Player", true);

    input_ = Input::GetInstance();
    audio_ = Audio::GetInstance();
    seExplosion_ = audio_->LoadWave("./Resources/SE/Explosion.wav");

    worldTransform_.translation_ = kInitialPos;
    worldTransform_.rotation_ = kInitialRot;
    worldTransform_.scale_ = kInitialScale;
    initialScale_ = kInitialScale;

    SetHP(kInitialHp);

    if (collider_) {
        collider_->SetRadius(kColliderRadius);
        collider_->SetTranslate(GetWorldTranslation());
    }

    isDead_ = false;
    isExploding_ = false;
    isExplosionFinished_ = false;
    explosionFrame_ = 0;
    aimYaw_ = 0.0f;
}

void Player::Update() {
    if (isExploding_) {
        UpdateExplosion_();
        worldTransform_.UpdateMatrix();
        if (collider_)
            collider_->SetTranslate(GetWorldTranslation());
        return;
    }
    if (isDead_) { return; }

    if (hitFlashFrames_ > 0)
        --hitFlashFrames_;
    if (invincibleFrames_ > 0)
        --invincibleFrames_;

    worldTransform_.rotation_.z -= lastHitRollOffset_;
    lastHitRollOffset_ = 0.0f;

    if (UpdateRoll_()) {
        worldTransform_.UpdateMatrix();
        if (collider_)
            collider_->SetTranslate(GetWorldTranslation());
        return;
    }

    UpdateMoveAndBank_(kFixedDeltaTime);

    float rollOffset = 0.0f;
    if (hitFlashFrames_ > 0) {
        float t = 1.0f - static_cast<float>(hitFlashFrames_) / kHitFlashDuration;
        rollOffset = std::sin(t * kHitRollFreq) * kHitRollAmp;
        float pulse = 1.0f + std::sin(t * kHitPulseFreq) * kHitPulseAmp;
        worldTransform_.scale_ = { initialScale_.x * pulse, initialScale_.y * pulse, initialScale_.z * pulse };
    }
    else {
        worldTransform_.scale_ = initialScale_;
    }

    worldTransform_.rotation_.z += rollOffset;
    lastHitRollOffset_ = rollOffset;

    if (input_) {
        if (input_->TriggerKey(DIK_D)) {
            if (doubleTapFrameD_ > 0 && doubleTapFrameD_ < kDoubleTapThreshold) {
                StartRoll_(1.0f);
                doubleTapFrameD_ = 0;
            }
            else {
                doubleTapFrameD_ = 1;
            }
        }
        if (input_->TriggerKey(DIK_A)) {
            if (doubleTapFrameA_ > 0 && doubleTapFrameA_ < kDoubleTapThreshold) {
                StartRoll_(-1.0f);
                doubleTapFrameA_ = 0;
            }
            else {
                doubleTapFrameA_ = 1;
            }
        }

        if (doubleTapFrameA_ > 0 && ++doubleTapFrameA_ > kDoubleTapThreshold)
            doubleTapFrameA_ = 0;
        if (doubleTapFrameD_ > 0 && ++doubleTapFrameD_ > kDoubleTapThreshold)
            doubleTapFrameD_ = 0;
    }

    worldTransform_.UpdateMatrix();
    if (collider_)
        collider_->SetTranslate(GetWorldTranslation());
}

bool Player::UpdateRoll_() {
    if (!isRolling_)
        return false;

    rollFrame_ += 1.0f;
    if (rollFrame_ > kRollDurationFrames)
        rollFrame_ = kRollDurationFrames;

    float t = rollFrame_ / kRollDurationFrames;
    float et = EaseOutCubic(t);

    worldTransform_.translation_.x = rollStartPos_.x + (rollEndPos_.x - rollStartPos_.x) * et;
    worldTransform_.rotation_.z = rollStartRotZ_ - rollDir_ * kTwoPi * t;

    if (rollFrame_ >= kRollDurationFrames) {
        worldTransform_.rotation_.z = rollStartRotZ_;
        isRolling_ = false;
    }
    return true;
}

void Player::UpdateMoveAndBank_(float dt) {
    if (!inputEnabled_ || !input_)
        return;

    int ix = 0, iy = 0;
    if (input_->PushKey(DIK_D) || input_->PushKey(DIK_RIGHT)) { ix++; }
    if (input_->PushKey(DIK_A) || input_->PushKey(DIK_LEFT)) { ix--; }
    if (input_->PushKey(DIK_W) || input_->PushKey(DIK_UP)) { iy++; }
    if (input_->PushKey(DIK_S) || input_->PushKey(DIK_DOWN)) { iy--; }

    worldTransform_.translation_.x += ix * kMoveSpeedXY * dt;
    worldTransform_.translation_.y += iy * kMoveSpeedXY * dt;

    worldTransform_.translation_.x = std::clamp(worldTransform_.translation_.x, kClampXMin, kClampXMax);
    worldTransform_.translation_.y = std::clamp(worldTransform_.translation_.y, kClampYMin, kClampYMax);

    targetTiltZ_ = -ix * kBankMaxRadZ;
    targetTiltX_ = iy * kPitchMaxRadX;

    currentTiltZ_ += (targetTiltZ_ - currentTiltZ_) * kTiltLerp;
    currentTiltX_ += (targetTiltX_ - currentTiltX_) * kTiltLerp;

    worldTransform_.rotation_.x = currentTiltX_;
    worldTransform_.rotation_.y = aimYaw_;
    worldTransform_.rotation_.z = currentTiltZ_;
}

void Player::Draw(const Camera* camera) {
    if (!camera || !model_ || isExplosionFinished_)
        return;
    if (invincibleFrames_ > 0 && ((invincibleFrames_ / 2) % 2 == 0))
        return;
    model_->Draw(worldTransform_, *camera);
}

void Player::OnCollision(CharacterBase*) {
    if (IsInvincible())
        return;
    Damage(kCollisionDamage);
}

void Player::SetAimDirection(const KamataEngine::Vector3& dir) {
    const float lenSq = dir.x * dir.x + dir.y * dir.y + dir.z * dir.z;
    if (lenSq <= 0.000001f) {
        return;
    }
    aimYaw_ = std::atan2(dir.x, dir.z);
}

void Player::Damage(int32_t amount) {
    if (isDead_ || isExploding_)
        return;

    tookDamageEvent_ = true;
    hitFlashFrames_ = kHitFlashDuration;
    invincibleFrames_ = kInvincibleDuration;

    SetHP(GetHP() - amount);
    if (GetHP() <= 0)
        Kill();
}

void Player::Kill() {
    if (isDead_ || isExploding_)
        return;

    isExploding_ = true;
    explosionFrame_ = 0;

    if (audio_ && seExplosion_ >= 0) {
        audio_->PlayWave(seExplosion_);
    }
}

void Player::UpdateExplosion_() {
    float t = static_cast<float>(explosionFrame_) / kExplosionDurationFrames;
    t = std::clamp(t, 0.0f, 1.0f);

    float et = EaseOutCubic(t);
    worldTransform_.scale_ = { initialScale_.x * (1.0f - et), initialScale_.y * (1.0f - et), initialScale_.z * (1.0f - et) };
    worldTransform_.translation_.y -= kExplodeFallSpeedY;
    worldTransform_.rotation_.y += kExplodeSpinSpeedY;

    explosionFrame_++;
    if (explosionFrame_ >= kExplosionDurationFrames) {
        isExploding_ = false;
        isDead_ = true;
        isExplosionFinished_ = true;
        worldTransform_.scale_ = { 0.0f, 0.0f, 0.0f };
    }
}

float Player::EaseOutCubic(float t) const {
    float inv = 1.0f - t;
    return 1.0f - inv * inv * inv;
}

void Player::StartRoll_(float dir) {
    if (isRolling_)
        return;

    isRolling_ = true;
    rollFrame_ = 0.0f;
    rollDir_ = dir;
    rollStartRotZ_ = worldTransform_.rotation_.z;
    rollStartPos_ = worldTransform_.translation_;
    rollEndPos_ = rollStartPos_;
    rollEndPos_.x += kRollMoveDistance * rollDir_;
}
