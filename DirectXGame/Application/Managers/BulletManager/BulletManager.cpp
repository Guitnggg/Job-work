#include "BulletManager.h"

#include <algorithm>

#include "Application/Managers/EnemyManager/EnemyManager.h"
#include "Application/Utility/MyMath/MyMath.h"

using namespace KamataEngine;

namespace {
    constexpr Vector3 kForward{ 0.0f, 0.0f, 1.0f };
    constexpr float kDirectionLengthEpsilon = 0.000001f;
} // namespace

void BulletManager::Initialize() {
    audio_ = Audio::GetInstance();
    shotSeHandle_ = audio_ ? audio_->LoadWave("./Resources/SE/shot.wav") : 0;
    missileSeHandle_ = audio_ ? audio_->LoadWave("./Resources/SE/missile.wav") : 0;

    bullets_.clear();
    homingMissiles_.clear();
    lockedTargets_.clear();

    // Object Pool Pattern:
    // 通常弾は連射で頻繁に生成・破棄されるため、最初に一定数を生成してプールとして保持する。
    // 発射時はこの中から未使用(isDead_)の弾を取り出して再利用し、毎回の動的確保・解放を避ける。
    bullets_.reserve(kInitialBulletPoolSize);
    for (int32_t i = 0; i < kInitialBulletPoolSize; ++i) {
        auto bullet = std::make_unique<Bullet>();
        bullet->Initialize();
        bullet->Deactivate();
        bullets_.push_back(std::move(bullet));
    }

    fireCooldownFrames_ = 0;
    burstShotsRemaining_ = 0;
    burstDirection_ = kForward;
    isHomingLocking_ = false;
    homingLockFrame_ = 0;
    homingCooldownFrames_ = 0;
    wasHomingPressing_ = false;
}

float BulletManager::GetHomingCooldownRate() const {
    const float rate = 1.0f - static_cast<float>(homingCooldownFrames_) / static_cast<float>(kHomingCooldownMaxFrame);
    return std::clamp(rate, 0.0f, 1.0f);
}

void BulletManager::HandleShooting_(KamataEngine::Input* input, Player* player, const CountDown& countDown, const KamataEngine::Vector3& shootDir, EnemyManager* enemyManager) {
    if (!input || !player || countDown.IsInputLocked()) {
        return;
    }

    // クールダウン経過
    if (fireCooldownFrames_ > 0) {
        --fireCooldownFrames_;
    }

    if (input->IsTriggerMouse(0) && burstShotsRemaining_ <= 0 && fireCooldownFrames_ <= 0) {
        burstShotsRemaining_ = kBurstShotCount;

        // Snapshot the aim only once. Every shot in this burst must keep flying
        // toward the point that was selected when the mouse was clicked.
        burstDirection_ = shootDir;
        const float burstDirLenSq = burstDirection_.x * burstDirection_.x + burstDirection_.y * burstDirection_.y + burstDirection_.z * burstDirection_.z;
        if (burstDirLenSq <= kDirectionLengthEpsilon) {
            burstDirection_ = kForward;
        } else {
            burstDirection_ = MyMath::Normalize(burstDirection_);
        }
    }

    if (burstShotsRemaining_ <= 0 || fireCooldownFrames_ > 0) {
        HandleHomingMissile_(input, player, countDown, enemyManager);
        return;
    }

    Vector3 dir = burstDirection_;
    const float lenSq = dir.x * dir.x + dir.y * dir.y + dir.z * dir.z;
    if (lenSq <= kDirectionLengthEpsilon) {
        dir = kForward;
    }

    // Object Pool Pattern:
    // 新しい弾を直接生成せず、プールから再利用できる弾を取得して発射する。
    Bullet* bullet = AcquireBullet_();
    if (bullet) {
        bullet->FireFrom(player->GetWorldTranslation(), dir);
    }

    --burstShotsRemaining_;
    fireCooldownFrames_ = (burstShotsRemaining_ > 0) ? kBurstIntervalFrames : kBurstCooldownFrames;

    if (audio_ && shotSeHandle_ != 0) {
        audio_->PlayWave(shotSeHandle_);
    }

    HandleHomingMissile_(input, player, countDown, enemyManager);
}

void BulletManager::HandleHomingMissile_(KamataEngine::Input* input, Player* player, const CountDown& countDown, EnemyManager* enemyManager) {
    if (!input || !player || !enemyManager || countDown.IsInputLocked()) {
        isHomingLocking_ = false;
        lockedTargets_.clear();
        homingLockFrame_ = 0;
        wasHomingPressing_ = false;
        return;
    }

    if (homingCooldownFrames_ > 0) {
        --homingCooldownFrames_;
    }

    // 右クリック押下でロック開始
    if (input->IsTriggerMouse(1) && homingCooldownFrames_ <= 0) {
        isHomingLocking_ = true;
        homingLockFrame_ = 0;
        lockedTargets_.clear();
    }

    if (!isHomingLocking_) {
        return;
    }

    ++homingLockFrame_;
    if (homingLockFrame_ >= kHomingLockStartFrame) {
        lockedTargets_ = enemyManager->GetNearestEnemies(player->GetWorldTranslation(), kHomingMaxLockCount);
    }

    const bool isPressing = input->IsPressMouse(1);
    const bool released = wasHomingPressing_ && !isPressing;
    wasHomingPressing_ = isPressing;
    const bool lockCompleted = homingLockFrame_ >= kHomingLockMaxFrame;
    if (!released && !lockCompleted) {
        return;
    }

    bool firedMissile = false;
    for (CharacterBase* target : lockedTargets_) {
        if (!target || target->IsDead()) {
            continue;
        }

        auto missile = std::make_unique<HomingMissile>();
        missile->Initialize();
        missile->FireFrom(player->GetWorldTranslation(), target);
        homingMissiles_.push_back(std::move(missile));
        firedMissile = true;
    }

    if (firedMissile && audio_ && missileSeHandle_ != 0) {
        audio_->PlayWave(missileSeHandle_);
    }

    homingCooldownFrames_ = kHomingCooldownMaxFrame;
    isHomingLocking_ = false;
    homingLockFrame_ = 0;
    lockedTargets_.clear();
}

void BulletManager::ValidateHomingTargets_(EnemyManager* enemyManager) {
    if (!enemyManager) {
        for (auto& m : homingMissiles_) {
            if (m) {
                m->ClearTarget();
            }
        }
        return;
    }

    auto& enemies = enemyManager->GetEnemies();
    for (auto& m : homingMissiles_) {
        if (!m || m->IsDead()) {
            continue;
        }

        CharacterBase* target = m->GetTarget();
        if (!target) {
            continue;
        }

        const bool exists = std::any_of(enemies.begin(), enemies.end(), [target](const std::unique_ptr<CharacterBase>& e) { return e && e.get() == target && !e->IsDead(); });

        if (!exists) {
            m->ClearTarget();
        }
    }
}

void BulletManager::UpdateBullets_(EnemyManager* enemyManager) {
    // 通常弾更新
    for (auto& b : bullets_) {
        if (b) {
            b->Update();
        }
    }

    // ホーミング対象の安全性を事前確認
    ValidateHomingTargets_(enemyManager);

    // ホーミングミサイル更新
    for (auto& m : homingMissiles_) {
        if (m) {
            m->Update();
        }
    }

    // 死亡した弾を削除
    RemoveDeadBullets_();
}

void BulletManager::RemoveDeadBullets_() {
    // Object Pool Pattern:
    // 通常弾は死んでも vector から削除しない。isDead_ == true の弾として残し、
    // 次の発射時に AcquireBullet_() で再利用する。

    homingMissiles_.erase(std::remove_if(homingMissiles_.begin(), homingMissiles_.end(), [](const std::unique_ptr<HomingMissile>& m) { return !m || m->IsDead(); }), homingMissiles_.end());
}

Bullet* BulletManager::AcquireBullet_() {
    // Object Pool Pattern:
    // まずプール内の未使用弾を探す。見つかればそのインスタンスを再利用する。
    for (auto& bullet : bullets_) {
        if (bullet && bullet->IsDead()) {
            return bullet.get();
        }
    }

    // プールが足りない場合だけ追加生成する。通常時のメモリ確保回数を抑えつつ、
    // 弾数が一時的に増えた場合にも対応できるようにしている。
    auto bullet = std::make_unique<Bullet>();
    bullet->Initialize();
    bullet->Deactivate();
    Bullet* result = bullet.get();
    bullets_.push_back(std::move(bullet));
    return result;
}

void BulletManager::Update(KamataEngine::Input* input, Player* player, const CountDown& countDown, const KamataEngine::Vector3& shootDir, EnemyManager* enemyManager) {
    HandleShooting_(input, player, countDown, shootDir, enemyManager);
    UpdateBullets_(enemyManager);
}

void BulletManager::Draw(const Camera* camera) {
    // 通常弾
    for (auto& b : bullets_) {
        b->Draw(camera);
    }

    // ホーミングミサイル
    for (auto& m : homingMissiles_) {
        m->Draw(camera);
    }
}
