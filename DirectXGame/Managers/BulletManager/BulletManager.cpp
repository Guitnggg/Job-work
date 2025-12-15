#include "BulletManager.h"

using namespace KamataEngine;

namespace {
    Vector3 kBulletForward{ 0.0f,0.0f,1.0f };
}

void BulletManager::Initialize() {
    // 弾リスト初期化＆クールダウンリセット
    bullets_.clear();
    fireCooldownFrames_ = 0;
}

void BulletManager::HandleShooting(Input* input, Player* player, const CountDown& countDown) {
    // カウントダウン中は入力無効
    if (countDown.IsInputLocked()) {
        return;
    }

    // クールダウン経過
    if (fireCooldownFrames_ > 0) {
        --fireCooldownFrames_;
    }

    // 発射キー判定
    if (input->PushKey(DIK_SPACE) && fireCooldownFrames_ == 0) {
        // クールダウン再設定
        fireCooldownFrames_ = kFireCooldownMax;

        // 弾生成
        auto b = std::make_unique<Bullet>();
        b->Initialize();

        // 発射位置はプレイヤー座標
        const Vector3 muzzle = player->GetWorldTranslation();

        // 前方（+z）へ射出
        b->FireFrom(muzzle, kBulletForward);

        // リストへ追加
        bullets_.push_back(std::move(b));
    }
}

void BulletManager::UpdateBullets() {
    // 全弾を更新
    for (auto& b : bullets_) {
        if (b) {
            b->Update();
        }
    }

    // 死亡した弾を削除
    RemoveDeadBullets();
}

void BulletManager::RemoveDeadBullets() {
    // nuiiptr or IsDead() = true の弾を削除
    bullets_.erase(
        std::remove_if(bullets_.begin(), bullets_.end(),
            [](const std::unique_ptr<Bullet>& b) {
                return !b || b->IsDead();
            }),
        bullets_.end());
}

void BulletManager::Update(Input* input, Player* player, const CountDown& countDown) {
    // 弾発射処理
    HandleShooting(input, player, countDown);

    // 弾更新、死亡処理
    UpdateBullets();
}

void BulletManager::Draw(Camera* camera) {
    // 全弾描画
    for (auto& b : bullets_) {
        b->Draw(camera);
    }
}
