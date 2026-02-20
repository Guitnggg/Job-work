#include "BulletManager.h"
#include <algorithm>

using namespace KamataEngine;

namespace {
    constexpr Vector3 kForward{ 0.0f,0.0f,1.0f };
} // namespace

void BulletManager::Initialize() {
    bullets_.clear();
    lasers_.clear();

    fireCooldownFrames_ = 0;
}

void BulletManager::HandleShooting_(KamataEngine::Input* input, Player* player, const CountDown& countDown, const KamataEngine::Vector3& shootDir) {
    if (!input || !player || countDown.IsInputLocked()) { return; }

    // クールダウン経過
    if (fireCooldownFrames_ > 0) {
        --fireCooldownFrames_;
    }

    // 
    if (!input->IsTriggerMouse(0)) {
        return;
    }


    Vector3 dir = shootDir;
    const float lenSq = dir.x * dir.x + dir.y * dir.y + dir.z * dir.z;
    if (lenSq <= 0.000001f) {
        dir = kForward;
    }

    auto b = std::make_unique<Bullet>();
    b->Initialize();
    b->FireFrom(player->GetWorldTranslation(), dir);
    bullets_.push_back(std::move(b));

    fireCooldownFrames_ = kFireCooldownMax;
}

void BulletManager::UpdateBullets_() {
    // 通常弾更新
    for (auto& b : bullets_) {
        if (b) {
            b->Update();
        }
    }

    // レーザー更新
    for (auto& r : lasers_) {
        if (r) {
            r->Update();
        }
    }

    // 死亡した弾を削除
    RemoveDeadBullets_();
}

void BulletManager::RemoveDeadBullets_() {
    bullets_.erase(std::remove_if(bullets_.begin(), bullets_.end(), [](const std::unique_ptr<Bullet>& b) { return !b || b->IsDead(); }), bullets_.end());

    lasers_.erase(std::remove_if(lasers_.begin(), lasers_.end(), [](const std::unique_ptr<Laser>& r) { return !r || r->IsDead(); }), lasers_.end());
}

void BulletManager::Update(KamataEngine::Input* input, Player* player, const CountDown& countDown, const KamataEngine::Vector3& shootDir){
    HandleShooting_(input, player, countDown, shootDir);
    UpdateBullets_();
}

void BulletManager::Draw(const Camera* camera) {
    // 通常弾
    for (auto& b : bullets_) {
        b->Draw(camera);
    }
    // レーザー
    for (auto& r : lasers_) {
        r->Draw(camera);
    }
}
