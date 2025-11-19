#pragma once

#include <vector>
#include <memory>

#include "Application/Charactors/Player/Player.h"
#include "Application/Charactors/Enemy/SeekerEnemy.h"
#include "Application/Charactors/Player/Bullet.h"
#include "Application/Charactors/Collider.h"   // 実際のパスに合わせてください

#include "UI/CountDown/CountDown.h"


class CollisionManager {
public:
    /// プレイヤー × 敵
    static void ResolvePlayerEnemyCollisions(
        Player* player,
        std::vector<std::unique_ptr<CharactorBase>>& enemies,
        const CountDown& countDown
    );

    /// 弾 × 敵
    static void ResolveBulletEnemyCollisions(
        std::vector<std::unique_ptr<Bullet>>& bullets,
        std::vector<std::unique_ptr<CharactorBase>>& enemies,
        const CountDown& countDown
    );
};
