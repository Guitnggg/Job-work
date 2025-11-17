#pragma once

#include <vector>
#include <memory>

#include "Application/Charactor/Player/Player.h"
#include "Application/Charactor/Enemy/SeekerEnemy.h"
#include "Application/Charactor/Player/Bullet.h"
#include "Application/CountDown/CountDown.h"
#include "Application/Charactor/Collider.h"   // 実際のパスに合わせてください

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
