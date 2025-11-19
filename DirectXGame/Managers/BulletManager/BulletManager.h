#pragma once

#include <KamataEngine.h>
#include <vector>
#include <memory>

#include "Application/Charactors/Player/Bullet.h"
#include "Application/Charactors/Player/Player.h"
#include "UI/CountDown/CountDown.h"

class BulletManager {
public:
    BulletManager() = default;
    ~BulletManager() = default;

    void Initialize();

    /// 入力処理 + 弾の更新 + 後始末
    void Update(KamataEngine::Input* input, Player* player, const CountDown& countDown);

    /// 描画
    void Draw(KamataEngine::Camera* camera);

    /// 弾リストへのアクセス（当たり判定用）
    std::vector<std::unique_ptr<Bullet>>& GetBullets() { return bullets_; }

private:
    void HandleShooting(KamataEngine::Input* input, Player* player, const CountDown& countDown);
    void UpdateBullets();
    void RemoveDeadBullets();

private:
    std::vector<std::unique_ptr<Bullet>> bullets_;
    int fireCooldownFrames_ = 0;
    static constexpr int kFireCooldownMax_ = 9;  // 約0.15秒@60fps
};
