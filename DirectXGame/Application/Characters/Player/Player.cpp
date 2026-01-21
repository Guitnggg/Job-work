#include "Player.h"

#include <algorithm>
#include <cmath>

#include "math/MathUtility.h"

using namespace KamataEngine;
using namespace KamataEngine::MathUtility;

// ===== Player.cpp専用定数 =====
namespace {
    // 固定Δt（固定フレーム前提ならここに置く）
    constexpr float kFixedDeltaTime = 1.0f / 60.0f;

    // 初期姿勢/位置
    constexpr Vector3 kInitialPos{ 0.0f, -2.0f, 20.0f };
    constexpr Vector3 kInitialRot{ 0.0f,  0.0f,  0.0f };
    constexpr Vector3 kInitialScale{ 1.0f, 1.0f, 1.0f };

    // コライダー
    constexpr float kColliderRadius = 1.0f;

    // 初期HP
    constexpr int kInitialHp = 100;

    // 衝突ダメージ（OnCollisionで固定ならここ）
    constexpr int kCollisionDamage = 20;

    // 被弾演出（波の周波数・振幅）
    constexpr float kHitRollAmp = 0.18f;     // ロール揺れ振幅
    constexpr float kHitRollFreq = 10.0f;    // ロール揺れ周波数係数
    constexpr float kHitPulseAmp = 0.06f;    // スケール脈動振幅
    constexpr float kHitPulseFreq = 18.0f;   // スケール脈動周波数係数

    // 爆発演出
    constexpr float kExplodeFallSpeedY = 0.1f;
    constexpr float kExplodeSpinSpeedY = 0.2f;

    // 2π（ロール一回転用）
    constexpr float kTwoPi = 6.28318530717958647692f;
}

void Player::Initialize(Camera* camera) {
    // 親クラス初期化
    CharacterBase::Initialize();

    // 参照登録、モデル読み込み
    camera_ = camera;
    model_ = Model::CreateFromOBJ("Player", true);

    // 音声ロード
    audio_ = Audio::GetInstance();
    seExplosion_ = audio_->LoadWave("./Resources/SE/Explosion.wav");

    // 初期位置、姿勢
    worldTransform_.translation_ = kInitialPos;
    worldTransform_.scale_ = kInitialScale;
    worldTransform_.rotation_ = kInitialRot;

    // 入力を受け付けるように
    input_ = Input::GetInstance();

    // HPや状態
    initialScale_ = kInitialScale;
    SetHP(kInitialHp);
    isDead_ = false;
    isExploding_ = false;
    isExplosionFinished_ = false;
    explosionFrame_ = 0;

    // コライダー
    if (collider_) {
        collider_->SetRadius(kColliderRadius);
        collider_->SetTranslate(GetWorldTranslation());
    }
}

void Player::Update() {
    // ===== 爆発中は演出優先 =====
    if (isExploding_) {
        UpdateExplosion();
        worldTransform_.UpdateMatrix();
        if (collider_) { collider_->SetTranslate(GetWorldTranslation()); }
        return;
    }
    if (isDead_) { return; }

    // ===== ヒット演出・無敵タイマー =====
    if (hitFlashFrames_ > 0) { --hitFlashFrames_; }
    if (invincibleFrames_ > 0) { --invincibleFrames_; }

    // 前フレームの被弾ゆれ(Zロール)を差し戻す
    worldTransform_.rotation_.z -= lastHitRollOffset_;
    lastHitRollOffset_ = 0.0f;

    // ===== ロール一回転（回避）中は専用関数で処理して早期return =====
    if (UpdateRoll_()) {
        worldTransform_.UpdateMatrix();
        if (collider_) { collider_->SetTranslate(GetWorldTranslation()); }
        return;
    }

    // ===== 通常移動＋バンク（傾き） =====
    const float dt = kFixedDeltaTime;
    UpdateMoveAndBank_(dt);

    // ===== 被弾時の“揺れ”を最後に上書きで足す =====
    float newRollOffset = 0.0f;
    if (hitFlashFrames_ > 0) {
        float ht = 1.0f - (float)hitFlashFrames_ / (float)kHitFlashDuration_;
        newRollOffset = std::sin(ht * kHitRollFreq) * kHitRollAmp;
        float pulse = 1.0f + std::sin(ht * kHitPulseFreq) * kHitPulseAmp;
        worldTransform_.scale_ = { initialScale_.x * pulse, initialScale_.y * pulse, initialScale_.z * pulse };
    }
    else {
        worldTransform_.scale_ = initialScale_;
    }
    worldTransform_.rotation_.z += newRollOffset;
    lastHitRollOffset_ = newRollOffset;

    // ===== ロール回避の入力（A/D：素早く2回押し） =====
    if (input_) {
        // 右(D)
        if (input_->TriggerKey(DIK_D)) {
            if (doubleTapFrameD_ > 0 && doubleTapFrameD_ < kDoubleTapThreshold_) {
                StartRoll(1.0f);
                doubleTapFrameD_ = 0;
            }
            else {
                doubleTapFrameD_ = 1;
            }
        }
        // 左(A)
        if (input_->TriggerKey(DIK_A)) {
            if (doubleTapFrameA_ > 0 && doubleTapFrameA_ < kDoubleTapThreshold_) {
                StartRoll(-1.0f);
                doubleTapFrameA_ = 0;
            }
            else {
                doubleTapFrameA_ = 1;
            }
        }
        // 経過
        if (doubleTapFrameA_ > 0) { ++doubleTapFrameA_; if (doubleTapFrameA_ > kDoubleTapThreshold_) doubleTapFrameA_ = 0; }
        if (doubleTapFrameD_ > 0) { ++doubleTapFrameD_; if (doubleTapFrameD_ > kDoubleTapThreshold_) doubleTapFrameD_ = 0; }
    }

    // ===== 反映 =====
    worldTransform_.UpdateMatrix();
    if (collider_) { collider_->SetTranslate(GetWorldTranslation()); }
}

// --- ロール回転の処理を関数化 ---
bool Player::UpdateRoll_() {
    if (!isRolling_) { return false; }

    rollFrame_ += 1.0f;
    if (rollFrame_ > rollDurationFrames_) { rollFrame_ = rollDurationFrames_; }

    const float t = rollFrame_ / rollDurationFrames_;
    const float et = EaseOutCubic(t);

    // 平行移動（横へスライド）
    worldTransform_.translation_.x = rollStartPos_.x + (rollEndPos_.x - rollStartPos_.x) * et;
    worldTransform_.translation_.y = rollStartPos_.y;
    worldTransform_.translation_.z = rollStartPos_.z;

    // Zは±2πの一回転
    worldTransform_.rotation_.z = rollStartRotZ_ - rollDir_ * kTwoPi * t;

    // 被弾中ならスケール脈動だけ適用
    if (hitFlashFrames_ > 0) {
        float ht = 1.0f - (float)hitFlashFrames_ / (float)kHitFlashDuration_;
        float pulse = 1.0f + std::sin(ht * 18.0f) * 0.06f;
        worldTransform_.scale_ = { initialScale_.x * pulse, initialScale_.y * pulse, initialScale_.z * pulse };
    }
    else {
        worldTransform_.scale_ = initialScale_;
    }

    if (rollFrame_ >= rollDurationFrames_) {
        // きっちり元角度へ戻して終了
        worldTransform_.rotation_.z = rollStartRotZ_;
        isRolling_ = false;
    }
    return true;
}

void Player::UpdateMoveAndBank_(float dt) {
    if (!inputEnabled_ || !input_) { return; }

    // 入力ベクトル（矢印キー＆WASD両対応）
    int ix = 0, iy = 0;
    if (input_->PushKey(DIK_RIGHT) || input_->PushKey(DIK_D)) { ix += 1; }
    if (input_->PushKey(DIK_LEFT) || input_->PushKey(DIK_A)) { ix -= 1; }
    if (input_->PushKey(DIK_UP) || input_->PushKey(DIK_W)) { iy += 1; }
    if (input_->PushKey(DIK_DOWN) || input_->PushKey(DIK_S)) { iy -= 1; }

    // 平行移動（画面内のローカルXY）
    worldTransform_.translation_.x += (float)ix * kMoveSpeedXY_ * dt;
    worldTransform_.translation_.y += (float)iy * kMoveSpeedXY_ * dt;

    // 画面内でクランプ
    worldTransform_.translation_.x = std::clamp(worldTransform_.translation_.x, clampXMin_, clampXMax_);
    worldTransform_.translation_.y = std::clamp(worldTransform_.translation_.y, clampYMin_, clampYMax_);

    // 傾きの目標値
    targetTiltZ_ = -(float)ix * kBankMaxRadZ_;  // 右入力で右にバンク（Zマイナス）
    targetTiltX_ = (float)iy * kPitchMaxRadX_; // 上入力で少し上向き（Xプラス）

    // スムーズに追従（LERP）
    currentTiltZ_ = Lerp(currentTiltZ_, targetTiltZ_, kTiltLerp_);
    currentTiltX_ = Lerp(currentTiltX_, targetTiltX_, kTiltLerp_);

    // 回転へ適用（Yは触らない）
    worldTransform_.rotation_.x = currentTiltX_;
    // Zはこのあと被弾揺れを加えるので、まずはベースを設定
    worldTransform_.rotation_.z = currentTiltZ_;
}

void Player::Draw(const Camera* camera) {
    if (isExplosionFinished_) { return; }
    if (!model_ || !camera_) { return; }

    // 無敵点滅（描画スキップ）
    if (invincibleFrames_ > 0 && ((invincibleFrames_ / 2) % 2 == 0)) { return; }

    model_->Draw(worldTransform_, *camera);
}

void Player::OnCollision(CharacterBase* /*enemy*/) {

    // ロール中はダメージを受けない
    if (isRolling_ || invincibleFrames_ > 0) {
        return;
    }

    Damage(kCollisionDamage);
}

void Player::Damage(int amount) {
    if (isExploding_ || isDead_) { return; }

    tookDamageEvent_ = true;
    hitFlashFrames_ = kHitFlashDuration_;
    invincibleFrames_ = kInvincibleDuration_;

    int hp = GetHP();
    hp -= amount;
    SetHP(hp);
    if (hp <= 0) { Kill(); }
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

void Player::UpdateExplosion() {
    float t = static_cast<float>(explosionFrame_) / static_cast<float>(explosionDurationFrames_);
    t = std::clamp(t, 0.0f, 1.0f);
    float et = EaseOutCubic(t);

    float scaleMul = (std::max)(0.0f, 1.0f - et);
    worldTransform_.scale_ = { initialScale_.x * scaleMul, initialScale_.y * scaleMul, initialScale_.z * scaleMul };

    worldTransform_.translation_.y -= kExplodeFallSpeedY;
    worldTransform_.rotation_.y += kExplodeSpinSpeedY;

    explosionFrame_++;
    if (explosionFrame_ >= explosionDurationFrames_) {
        isExploding_ = false;
        isDead_ = true;
        isExplosionFinished_ = true;
        worldTransform_.scale_ = { 0.0f, 0.0f, 0.0f };
    }
}
