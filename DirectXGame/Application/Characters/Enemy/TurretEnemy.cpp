#include "TurretEnemy.h"

#include "Application/Utility/GameTime.h"
#include "base/TextureManager.h"
#include <algorithm>
#include <cmath>

using namespace KamataEngine;

namespace {
constexpr float kFlashScaleAmount = 0.3f;
constexpr float kDirectionLengthEpsilon = 0.000001f;
constexpr float kShakeFrequencyX = 40.0f;
constexpr float kShakeFrequencyY = 52.0f;
constexpr float kVerticalShakeScale = 0.5f;
}

/// <summary>
/// TurretEnemy の行動を状態ごとに分離するための State Pattern 用基底クラス。
/// </summary>
class TurretEnemyStateBase {
public:
    virtual ~TurretEnemyStateBase() = default;
    virtual void Enter(TurretEnemy&) {}
    virtual void Update(TurretEnemy& turret, float dt) = 0;
    virtual void Exit(TurretEnemy&) {}
};

/// <summary>
/// 通常待機・照準・射撃クールタイム状態。
/// </summary>
class TurretEnemyActiveState : public TurretEnemyStateBase {
public:
    void Update(TurretEnemy& turret, float dt) override;
};

/// <summary>
/// 発射を1フレーム分の明確な状態として扱う。
/// </summary>
class TurretEnemyShootingState : public TurretEnemyStateBase {
public:
    void Enter(TurretEnemy& turret) override;
    void Update(TurretEnemy& turret, float dt) override;
};

void TurretEnemyActiveState::Update(TurretEnemy& turret, float) {
    turret.AimToTarget_();

    turret.shootTimerFrames_++;
    if (turret.shootTimerFrames_ >= turret.shootIntervalFrames_) {
        turret.shootTimerFrames_ = 0;
        turret.RequestStateChange_(std::make_unique<TurretEnemyShootingState>());
    }
}

void TurretEnemyShootingState::Enter(TurretEnemy& turret) {
    turret.AimToTarget_();
    turret.Fire_();
}

void TurretEnemyShootingState::Update(TurretEnemy& turret, float) {
    turret.RequestStateChange_(std::make_unique<TurretEnemyActiveState>());
}

TurretEnemy::TurretEnemy() = default;

TurretEnemy::~TurretEnemy() = default;


// 砲台のモデル・射撃設定・HP・コライダーなど初期状態を設定する
void TurretEnemy::Initialize() {
    // --- 基底クラス初期化 ---
    CharacterBase::Initialize();

    // --- モデル生成 ---
    // 現在は球体モデルを使用（専用モデルがあれば差し替え可能）
    model_.reset(Model::CreateSphere());
    objectColor_.Initialize();
    objectColor_.SetColor({ 0.95f, 0.32f, 0.18f, 1.0f });
    flashColor_.Initialize();
    flashColor_.SetColor({ 1.0f, 0.95f, 0.28f, 0.85f });
    flashTextureHandle_ = TextureManager::Load("./Resources/white1x1.png");

    // --- HP設定 ---
    maxHp_ = initialHP_;
    hp_ = maxHp_;

    // --- 見た目スケール ---
    worldTransform_.scale_ = kTurretScale;
    worldTransform_.UpdateMatrix();

    // --- コライダー設定 ---
    if (collider_) {
        collider_->SetRadius(colliderRadius_);
        collider_->SetTranslate(GetWorldTranslation());
        collider_->Update();
    }

    // --- 初期状態 ---
    bullets_.clear();
    shootTimerFrames_ = 0;
    pendingState_.reset();
    ChangeState_(std::make_unique<TurretEnemyActiveState>());
    isDead_ = false;
    flashTimer_ = 0.0f;
    shakeTimer_ = 0.0f;
    baseTranslation_ = worldTransform_.translation_;
}

// ターゲット追尾・射撃クールタイム・弾更新・死亡判定・コライダー同期を現在の State に委譲する
void TurretEnemy::Update() {
    // 死亡中も弾は更新して自然消滅させる
    if (IsDead()) {
        UpdateBullets_();
        return;
    }

    const float dt = GameTime::kDeltaTime;
    UpdateBodyFeedback_(dt);

    if (!state_) {
        ChangeState_(std::make_unique<TurretEnemyActiveState>());
    }
    state_->Update(*this, dt);
    ApplyPendingStateChange_();

    SyncCollider_();

    UpdateBullets_();
}

// 砲台本体・発射済み弾・デバック用コライダーを描画する
void TurretEnemy::Draw(const Camera* camera) {
    if (!camera) {
        return;
    }

    // 砲台本体
    if (!IsDead() && model_) {
        model_->Draw(worldTransform_, *camera, textureHandle_, &objectColor_);
        if (flashTimer_ > 0.0f && flashTextureHandle_ != 0u) {
            Vector3 backupScale = worldTransform_.scale_;
            const float t = flashTimer_ / kFlashDuration;
            worldTransform_.scale_ = {
                backupScale.x * (1.0f + t * kFlashScaleAmount), backupScale.y * (1.0f + t * kFlashScaleAmount), backupScale.z * (1.0f + t * kFlashScaleAmount) };
            worldTransform_.UpdateMatrix();
            model_->Draw(worldTransform_, *camera, flashTextureHandle_, &flashColor_);
            worldTransform_.scale_ = backupScale;
            worldTransform_.UpdateMatrix();
        }
    }

    // 弾描画
    for (auto& bullet : bullets_) {
        bullet->Draw(camera);
    }
}

// 砲台が被弾したときのダメージ処理と死亡判定を行う
void TurretEnemy::OnCollision(CharacterBase* /*other*/) {
    if (IsDead()) {
        return;
    }

    // ダメージ処理
    hp_--;
    flashTimer_ = kFlashDuration;
    shakeTimer_ = kShakeDuration;
    if (hp_ <= 0) {
        isDead_ = true;
    }
}

// ターゲット方向を向くように砲台のYaw回転を更新する
void TurretEnemy::AimToTarget_() {
    if (!hasTarget_) {
        return;
    }

    const Vector3 selfPos = GetWorldTranslation();
    Vector3 toTarget{ targetPos_.x - selfPos.x, targetPos_.y - selfPos.y, targetPos_.z - selfPos.z };

    const float lenSq = toTarget.x * toTarget.x + toTarget.y * toTarget.y + toTarget.z * toTarget.z;

    if (lenSq < kDirectionLengthEpsilon) {
        return;
    }

    // Y軸回転のみでターゲットを向く
    const float yaw = std::atan2(toTarget.x, toTarget.z);
    worldTransform_.rotation_.y = yaw;
}

// 現在位置と向きから新しい弾を生成し、弾リストへ登録する
void TurretEnemy::Fire_() {
    if (!hasTarget_) {
        return;
    }

    // --- 発射位置 ---
    Vector3 muzzlePos = GetWorldTranslation();
    const float yaw = worldTransform_.rotation_.y;
    muzzlePos.x += std::sin(yaw) * kMuzzleForward;
    muzzlePos.z += std::cos(yaw) * kMuzzleForward;

    // --- 発射方向 ---
    Vector3 dir{ targetPos_.x - muzzlePos.x, targetPos_.y - muzzlePos.y, targetPos_.z - muzzlePos.z };

    const float len = std::sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
    if (len < kDirectionLengthEpsilon) {
        return;
    }
    dir.x /= len;
    dir.y /= len;
    dir.z /= len;

    // --- Bullet生成 ---
    auto bullet = std::make_unique<Bullet>();
    bullet->Initialize();
    bullet->SetSpeed(bulletSpeed_);
    bullet->SetLifeTime(bulletLifeTimeSec_);
    bullet->FireFrom(muzzlePos, dir);

    bullets_.push_back(std::move(bullet));
}

// 砲台が発射した弾を更新し、不要になった弾を削除する
void TurretEnemy::UpdateBullets_() {
    for (auto& bullet : bullets_) {
        bullet->Update();
    }

    bullets_.erase(std::remove_if(bullets_.begin(), bullets_.end(), [](const std::unique_ptr<Bullet>& b) { return b->IsDead(); }), bullets_.end());
}

// 被弾フラッシュや揺れなど、状態に依存しない見た目のフィードバックを更新する
void TurretEnemy::UpdateBodyFeedback_(float dt) {
    if (flashTimer_ > 0.0f) {
        flashTimer_ -= dt;
    }
    if (shakeTimer_ > 0.0f) {
        shakeTimer_ -= dt;
    }

    // 砲台は移動しないため translation_ は更新しない
    worldTransform_.translation_ = baseTranslation_;
    if (shakeTimer_ > 0.0f) {
        const float t = shakeTimer_ / kShakeDuration;
        worldTransform_.translation_.x += std::sin(t * kShakeFrequencyX) * kShakePower * t;
        worldTransform_.translation_.y += std::cos(t * kShakeFrequencyY) * kShakePower * kVerticalShakeScale * t;
    }
    worldTransform_.UpdateMatrix();
}

// コライダー位置を現在のワールド座標へ同期する
void TurretEnemy::SyncCollider_() {
    if (collider_) {
        collider_->SetTranslate(GetWorldTranslation());
        collider_->Update();
    }
}

// State Pattern の状態を即時切り替えする
void TurretEnemy::ChangeState_(std::unique_ptr<TurretEnemyStateBase> nextState) {
    if (state_) {
        state_->Exit(*this);
    }
    state_ = std::move(nextState);
    if (state_) {
        state_->Enter(*this);
    }
}

// State 更新中でも安全に切り替えられるよう、次の状態を予約する
void TurretEnemy::RequestStateChange_(std::unique_ptr<TurretEnemyStateBase> nextState) {
    pendingState_ = std::move(nextState);
}

// 予約済みの State を反映する
void TurretEnemy::ApplyPendingStateChange_() {
    if (pendingState_) {
        ChangeState_(std::move(pendingState_));
    }
}

// 敵の当たり判定サイズを外部から調整する
void TurretEnemy::SetColliderRadius(float radius) {
    colliderRadius_ = radius;
    if (collider_) {
        collider_->SetRadius(radius);
    }
}
