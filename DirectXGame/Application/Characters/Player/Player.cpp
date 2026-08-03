#include "Player.h"

#include "Application/Utility/GameTime.h"

#include <algorithm>
#include <cmath>

using namespace KamataEngine;

namespace {
constexpr int32_t kInvincibilityBlinkIntervalFrames = 2;
constexpr int32_t kInvincibilityBlinkCycle = 2;
constexpr float kDirectionLengthEpsilon = 0.000001f;
}

/// <summary>
/// Player の振る舞いを状態ごとに分離するための State Pattern 用基底クラス。
/// </summary>
class PlayerStateBase {
public:
    virtual ~PlayerStateBase() = default;
    virtual void Enter(Player&) {}
    virtual void Update(Player& player) = 0;
    virtual void Exit(Player&) {}
};

/// <summary>
/// 通常操作状態。移動、被弾演出、ロール開始入力を担当する。
/// </summary>
class PlayerNormalState : public PlayerStateBase {
public:
    void Enter(Player& player) override {
        player.isRolling_ = false;
    }

    void Update(Player& player) override {
        if (player.hitFlashFrames_ > 0) {
            --player.hitFlashFrames_;
        }
        if (player.invincibleFrames_ > 0) {
            --player.invincibleFrames_;
        }

        player.worldTransform_.rotation_.z -= player.lastHitRollOffset_;
        player.lastHitRollOffset_ = 0.0f;

        player.UpdateMoveAndBank_(GameTime::kDeltaTime);

        float rollOffset = 0.0f;
        if (player.hitFlashFrames_ > 0) {
            float t = 1.0f - static_cast<float>(player.hitFlashFrames_) / Player::kHitFlashDuration;
            rollOffset = std::sin(t * Player::kHitRollFreq) * Player::kHitRollAmp;
            float pulse = 1.0f + std::sin(t * Player::kHitPulseFreq) * Player::kHitPulseAmp;
            player.worldTransform_.scale_ = { player.initialScale_.x * pulse, player.initialScale_.y * pulse, player.initialScale_.z * pulse };
        }
        else {
            player.worldTransform_.scale_ = player.initialScale_;
        }

        player.worldTransform_.rotation_.z += rollOffset;
        player.lastHitRollOffset_ = rollOffset;

        if (player.input_ && player.inputEnabled_) {
            if (player.input_->TriggerKey(DIK_D)) {
                if (player.doubleTapFrameD_ > 0 && player.doubleTapFrameD_ < Player::kDoubleTapThreshold) {
                    player.StartRoll_(1.0f);
                    player.doubleTapFrameD_ = 0;
                }
                else {
                    player.doubleTapFrameD_ = 1;
                }
            }
            if (player.input_->TriggerKey(DIK_A)) {
                if (player.doubleTapFrameA_ > 0 && player.doubleTapFrameA_ < Player::kDoubleTapThreshold) {
                    player.StartRoll_(-1.0f);
                    player.doubleTapFrameA_ = 0;
                }
                else {
                    player.doubleTapFrameA_ = 1;
                }
            }

            if (player.doubleTapFrameA_ > 0 && ++player.doubleTapFrameA_ > Player::kDoubleTapThreshold) {
                player.doubleTapFrameA_ = 0;
            }
            if (player.doubleTapFrameD_ > 0 && ++player.doubleTapFrameD_ > Player::kDoubleTapThreshold) {
                player.doubleTapFrameD_ = 0;
            }
        }

        player.worldTransform_.UpdateMatrix();
        player.SyncCollider_();
    }
};

/// <summary>
/// ロール回避状態。一定時間の移動・回転と無敵扱いを担当する。
/// </summary>
class PlayerRollState : public PlayerStateBase {
public:
    void Enter(Player& player) override {
        player.worldTransform_.rotation_.z -= player.lastHitRollOffset_;
        player.lastHitRollOffset_ = 0.0f;
    }

    void Update(Player& player) override {
        if (player.hitFlashFrames_ > 0) {
            --player.hitFlashFrames_;
        }
        if (player.invincibleFrames_ > 0) {
            --player.invincibleFrames_;
        }

        player.UpdateRoll_();
        player.worldTransform_.UpdateMatrix();
        player.SyncCollider_();

        if (!player.isRolling_) {
            player.RequestStateChange_(std::make_unique<PlayerNormalState>());
        }
    }
};

/// <summary>
/// 死亡完了状態。更新処理を行わず、終了済みであることを表す。
/// </summary>
class PlayerDeadState : public PlayerStateBase {
public:
    void Enter(Player& player) override {
        player.isRolling_ = false;
        player.isExploding_ = false;
        player.isDead_ = true;
        player.isExplosionFinished_ = true;
    }

    void Update(Player&) override {}
};

/// <summary>
/// 爆発演出状態。死亡時の演出更新だけを担当する。
/// </summary>
class PlayerExplosionState : public PlayerStateBase {
public:
    void Enter(Player& player) override {
        player.inputEnabled_ = false;
        player.isRolling_ = false;
        player.worldTransform_.rotation_.z -= player.lastHitRollOffset_;
        player.lastHitRollOffset_ = 0.0f;
    }

    void Update(Player& player) override {
        player.UpdateExplosion_();
        player.worldTransform_.UpdateMatrix();
        player.SyncCollider_();

        if (player.isExplosionFinished_) {
            player.RequestStateChange_(std::make_unique<PlayerDeadState>());
        }
    }
};

Player::Player() = default;

Player::~Player() = default;

// プレイヤー本体・武器・当たり判定・演出用パラメータの初期状態を設定する
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
    inputEnabled_ = true;
    tookDamageEvent_ = false;
    hitFlashFrames_ = 0;
    invincibleFrames_ = 0;
    lastHitRollOffset_ = 0.0f;
    doubleTapFrameA_ = 0;
    doubleTapFrameD_ = 0;
    pendingState_.reset();

    ChangeState_(std::make_unique<PlayerNormalState>());
}

// 現在の State オブジェクトに更新処理を委譲する
void Player::Update() {
    if (!state_) {
        ChangeState_(std::make_unique<PlayerNormalState>());
    }
    state_->Update(*this);
    ApplyPendingStateChange_();
}

// ロール回避中の時間と回転を更新し、回避中かどうかを返す
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

// 通常移動入力を位置と傾きに反映し、画面内に収まるように補正する
void Player::UpdateMoveAndBank_(float dt) {
    if (!inputEnabled_ || !input_)
        return;

    int ix = 0, iy = 0;
    if (input_->PushKey(DIK_D) || input_->PushKey(DIK_RIGHT)) {
        ix++;
    }
    if (input_->PushKey(DIK_A) || input_->PushKey(DIK_LEFT)) {
        ix--;
    }
    if (input_->PushKey(DIK_W) || input_->PushKey(DIK_UP)) {
        iy++;
    }
    if (input_->PushKey(DIK_S) || input_->PushKey(DIK_DOWN)) {
        iy--;
    }

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

// プレイヤー本体・武器・爆発演出・デバック用コライダーを描画する
void Player::Draw(const Camera* camera) {
    if (!camera || !model_ || isExplosionFinished_)
        return;
    if (invincibleFrames_ > 0 && ((invincibleFrames_ / kInvincibilityBlinkIntervalFrames) % kInvincibilityBlinkCycle == 0))
        return;
    model_->Draw(worldTransform_, *camera);
}

// 衝突したときにプレイヤーへダメージを与える
void Player::OnCollision(CharacterBase*) {
    if (IsInvincible())
        return;
    Damage(kCollisionDamage);
}

// 外部から渡された照準方向を正規化して攻撃方法として保持する
void Player::SetAimDirection(const KamataEngine::Vector3& dir) {
    const float lenSq = dir.x * dir.x + dir.y * dir.y + dir.z * dir.z;
    if (lenSq <= kDirectionLengthEpsilon) {
        return;
    }
    aimYaw_ = std::atan2(dir.x, dir.z);
}

// 無敵時間を考慮しながらHPを減らし、必要なら撃墜処理を開始する
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

// プレイヤーを死亡状態にし、爆発演出状態へ切り替える
void Player::Kill() {
    if (isDead_ || isExploding_)
        return;

    isExploding_ = true;
    explosionFrame_ = 0;

    if (audio_ && seExplosion_ >= 0) {
        audio_->PlayWave(seExplosion_);
    }

    ChangeState_(std::make_unique<PlayerExplosionState>());
}

// 死亡後の爆発タイマーと拡大・フェード演出を更新する
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

// 爆発などの補完に使用するイージング値を計算する
float Player::EaseOutCubic(float t) const {
    float inv = 1.0f - t;
    return 1.0f - inv * inv * inv;
}

// 指定方向へロール回避を開始し、ロール用タイマーと向きを設定する
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
    rollEndPos_.x = std::clamp(rollEndPos_.x, kClampXMin, kClampXMax);

    RequestStateChange_(std::make_unique<PlayerRollState>());
}

// State Pattern の状態を即時切り替えする
void Player::ChangeState_(std::unique_ptr<PlayerStateBase> nextState) {
    if (state_) {
        state_->Exit(*this);
    }
    state_ = std::move(nextState);
    if (state_) {
        state_->Enter(*this);
    }
}

// State 更新中に安全に切り替えるため、次フレーム用状態を予約する
void Player::RequestStateChange_(std::unique_ptr<PlayerStateBase> nextState) {
    pendingState_ = std::move(nextState);
}

// 予約済みの State を反映する
void Player::ApplyPendingStateChange_() {
    if (pendingState_) {
        ChangeState_(std::move(pendingState_));
    }
}

// コライダー位置を現在のワールド座標へ同期する
void Player::SyncCollider_() {
    if (collider_) {
        collider_->SetTranslate(GetWorldTranslation());
    }
}
