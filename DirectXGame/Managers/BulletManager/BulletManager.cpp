#include "BulletManager.h"

using namespace KamataEngine;

void BulletManager::Initialize()
{
    bullets_.clear();
    fireCooldownFrames_ = 0;
}

void BulletManager::HandleShooting(Input* input, Player* player, const CountDown& countDown)
{
    if (countDown.IsInputLocked()) {
        return;
    }

    if (fireCooldownFrames_ > 0) {
        --fireCooldownFrames_;
    }

    if (input->PushKey(DIK_SPACE) && fireCooldownFrames_ == 0) {
        fireCooldownFrames_ = kFireCooldownMax_;

        auto b = std::make_unique<Bullet>();
        b->Initialize();

        const Vector3 muzzle = player->GetWorldTranslation();
        b->FireFrom(muzzle, { 0.0f, 0.0f, 1.0f }); // +Z方向へ

        bullets_.push_back(std::move(b));
    }
}

void BulletManager::UpdateBullets()
{
    for (auto& b : bullets_) {
        if (b) {
            b->Update();
        }
    }

    RemoveDeadBullets();
}

void BulletManager::RemoveDeadBullets()
{
    bullets_.erase(
        std::remove_if(bullets_.begin(), bullets_.end(),
            [](const std::unique_ptr<Bullet>& b) {
                return !b || b->IsDead();
            }),
        bullets_.end());
}

void BulletManager::Update(Input* input, Player* player, const CountDown& countDown)
{
    HandleShooting(input, player, countDown);
    UpdateBullets();
}

void BulletManager::Draw(Camera* camera)
{
    for (auto& b : bullets_) {
        b->Draw(camera);
    }
}
