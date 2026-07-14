#include "CollisionManager.h"

using namespace KamataEngine;

namespace {

bool IsOverlapping(const Collider* a, const Collider* b) {
    if (!a || !b) {
        return false;
    }

    const auto aPos = a->GetTranslate();
    const auto bPos = b->GetTranslate();
    const float radiusSum = a->GetRadius() + b->GetRadius();

    const float dx = aPos.x - bPos.x;
    const float dy = aPos.y - bPos.y;
    const float dz = aPos.z - bPos.z;
    const float distanceSq = dx * dx + dy * dy + dz * dz;

    return distanceSq <= radiusSum * radiusSum;
}

} // namespace

void CollisionManager::ResolvePlayerEnemyCollisions(Player* player, std::vector<std::unique_ptr<CharacterBase>>& enemies, const CountDown& countDown) {
    // 無効条件（入力ロック中、プレイヤー不在、プレイヤー爆散演出終了まちetc...）
    if (!player || countDown.IsInputLocked() || player->IsExplosionFinished()) {
        return;
    }

    // プレイヤー側コライダー取得
    Collider* playerCollider = player->GetCollider();
    if (!playerCollider) {
        return;
    }

    // 各敵との衝突判定ループ
    for (auto& e : enemies) {
        if (!e) {
            continue;
        }
        Collider* enemyCollider = e->GetCollider();
        if (!enemyCollider) {
            continue;
        }

        // 半径の合計より近ければ衝突
        if (IsOverlapping(playerCollider, enemyCollider)) {
            player->SetLastDamageSource(CharacterBase::DamageSource::Collision);
            e->SetLastDamageSource(CharacterBase::DamageSource::Collision);
            player->OnCollision(e.get());
            e->OnCollision(player);
        }
    }
}

void CollisionManager::ResolveBulletEnemyCollisions(std::vector<std::unique_ptr<Bullet>>& bullets, std::vector<std::unique_ptr<CharacterBase>>& enemies, const CountDown& countDown) {
    // カウントダウン中は無効
    if (countDown.IsInputLocked()) {
        return;
    }

    // 全弾ループ
    for (auto& b : bullets) {
        if (!b || b->IsDead()) {
            continue;
        }

        Collider* bc = b->GetCollider();
        if (!bc) {
            continue;
        }

        // 各敵と衝突チェック
        for (auto& e : enemies) {
            if (!e) {
                continue;
            }

            // 既に死んだ敵は飛ばす
            if (e->IsDead()) {
                continue;
            }

            Collider* enemyCollider = e->GetCollider();
            if (!enemyCollider) {
                continue;
            }

            if (IsOverlapping(bc, enemyCollider)) {
                // 相互通知：敵は死に、弾は消える
                e->SetLastDamageSource(CharacterBase::DamageSource::Bullet);
                e->OnCollision(b.get());
                b->OnCollision(e.get());
                // 1発で1体想定
                break;
            }
        }
    }
}

void CollisionManager::ResolveHomingMissileEnemyCollisions(std::vector<std::unique_ptr<HomingMissile>>& missiles, std::vector<std::unique_ptr<CharacterBase>>& enemies, const CountDown& countDown) {
    if (countDown.IsInputLocked()) {
        return;
    }

    for (auto& m : missiles) {
        if (!m || m->IsDead()) {
            continue;
        }

        Collider* missileCollider = m->GetCollider();
        if (!missileCollider) {
            continue;
        }

        for (auto& e : enemies) {
            if (!e || e->IsDead()) {
                continue;
            }

            Collider* enemyCollider = e->GetCollider();
            if (!enemyCollider) {
                continue;
            }

            if (IsOverlapping(missileCollider, enemyCollider)) {
                e->SetLastDamageSource(CharacterBase::DamageSource::HomingMissile);
                e->OnCollision(m.get());
                m->OnCollision(e.get());
                break;
            }
        }
    }
}

void CollisionManager::ResolvePlayerTurretBulletCollisions(Player* player, std::vector<std::unique_ptr<CharacterBase>>& enemies, const CountDown& countDown) {
    if (!player || countDown.IsInputLocked() || player->IsExplosionFinished()) {
        return;
    }

    Collider* playerCollider = player->GetCollider();
    if (!playerCollider) {
        return;
    }

    for (auto& e : enemies) {
        auto* turret = dynamic_cast<TurretEnemy*>(e.get());
        if (!turret || turret->IsDead()) {
            continue;
        }

        for (auto& tb : turret->GetBullets()) {
            if (!tb || tb->IsDead()) {
                continue;
            }

            Collider* bulletCollider = tb->GetCollider();
            if (!bulletCollider) {
                continue;
            }

            if (IsOverlapping(playerCollider, bulletCollider)) {
                player->SetLastDamageSource(CharacterBase::DamageSource::Bullet);
                player->OnCollision(tb.get());
                tb->OnCollision(player);
            }
        }
    }
}

void CollisionManager::ResolvePlayerBulletTurretBulletCollisions(
    std::vector<std::unique_ptr<Bullet>>& playerBullets, std::vector<std::unique_ptr<CharacterBase>>& enemies, const CountDown& countDown) {
    if (countDown.IsInputLocked()) {
        return;
    }

    for (auto& pb : playerBullets) {
        if (!pb || pb->IsDead()) {
            continue;
        }

        Collider* playerBulletCollider = pb->GetCollider();
        if (!playerBulletCollider) {
            continue;
        }

        bool hit = false;
        for (auto& e : enemies) {
            auto* turret = dynamic_cast<TurretEnemy*>(e.get());
            if (!turret || turret->IsDead()) {
                continue;
            }

            for (auto& tb : turret->GetBullets()) {
                if (!tb || tb->IsDead()) {
                    continue;
                }

                Collider* turretBulletCollider = tb->GetCollider();
                if (!turretBulletCollider) {
                    continue;
                }

                if (IsOverlapping(playerBulletCollider, turretBulletCollider)) {
                    pb->OnCollision(tb.get());
                    tb->OnCollision(pb.get());
                    hit = true;
                    break;
                }
            }

            if (hit) {
                break;
            }
        }
    }
}
