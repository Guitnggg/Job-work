#include "CollisionManager.h"

using namespace KamataEngine;

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

    const auto p = playerCollider->GetTranslate();
    const float pr = playerCollider->GetRadius();

    // 各敵との衝突判定ループ
    for (auto& e : enemies) {
        if (!e) {
            continue;
        }
        Collider* enemyCollider = e->GetCollider();
        if (!enemyCollider) {
            continue;
        }

        const auto q = enemyCollider->GetTranslate();
        const float er = enemyCollider->GetRadius();

        // 距離チェック
        const float dx = p.x - q.x;
        const float dy = p.y - q.y;
        const float dz = p.z - q.z;
        const float dist2 = dx * dx + dy * dy + dz * dz;
        const float r = pr + er;

        // 半径の合計より近ければ衝突
        if (dist2 <= r * r) {
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

        const auto bp = bc->GetTranslate();
        const float br = bc->GetRadius();

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

            const auto ep = enemyCollider->GetTranslate();
            const float er = enemyCollider->GetRadius();

            // 球判定
            const float dx = bp.x - ep.x;
            const float dy = bp.y - ep.y;
            const float dz = bp.z - ep.z;
            const float dist2 = dx * dx + dy * dy + dz * dz;
            const float rr = br + er;

            if (dist2 <= rr * rr) {
                // 相互通知：敵は死に、弾は消える
                e->OnCollision(b.get());
                b->OnCollision(e.get());
                // 1発で1体想定
                break;
            }
        }
    }
}

void CollisionManager::ResolveLaserEnemyCollisions(std::vector<std::unique_ptr<Laser>>& lasers, std::vector<std::unique_ptr<CharacterBase>>& enemies, const CountDown& countDown) {
    // カウントダウン中は衝突無効
    if (countDown.IsInputLocked()) {
        return;
    }

    // レーザー×敵
    for (auto& r : lasers) {
        if (!r || r->IsDead()) {
            continue;
        }

        Collider* laserCollider = r->GetCollider();
        if (!laserCollider) {
            continue;
        }

        const auto rp = laserCollider->GetTranslate();
        const float rr = laserCollider->GetRadius();

        for (auto& e : enemies) {
            if (!e || e->IsDead()) {
                continue;
            }

            Collider* enemyCollider = e->GetCollider();
            if (!enemyCollider) {
                continue;
            }

            const auto ep = enemyCollider->GetTranslate();
            const float er = enemyCollider->GetRadius();

            const float dx = rp.x - ep.x;
            const float dy = rp.y - ep.y;
            const float dz = rp.z - ep.z;
            const float dist2 = dx * dx + dy * dy + dz * dz;
            const float rsum = rr + er;

            if (dist2 <= rsum * rsum) {
                // 相互通知：敵側がダメージ処理、レーザー側は「消える/貫通」どちらでもOK
                e->OnCollision(r.get());
                r->OnCollision(e.get());

                // 1発で1体想定（貫通レーザーにしたいなら break を外す）
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

        const auto mp = missileCollider->GetTranslate();
        const float mr = missileCollider->GetRadius();

        for (auto& e : enemies) {
            if (!e || e->IsDead()) {
                continue;
            }

            Collider* enemyCollider = e->GetCollider();
            if (!enemyCollider) {
                continue;
            }

            const auto ep = enemyCollider->GetTranslate();
            const float er = enemyCollider->GetRadius();

            const float dx = mp.x - ep.x;
            const float dy = mp.y - ep.y;
            const float dz = mp.z - ep.z;
            const float dist2 = dx * dx + dy * dy + dz * dz;
            const float rr = mr + er;

            if (dist2 <= rr * rr) {
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

    const auto pp = playerCollider->GetTranslate();
    const float pr = playerCollider->GetRadius();

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

            const auto bp = bulletCollider->GetTranslate();
            const float br = bulletCollider->GetRadius();

            const float dx = pp.x - bp.x;
            const float dy = pp.y - bp.y;
            const float dz = pp.z - bp.z;
            const float dist2 = dx * dx + dy * dy + dz * dz;
            const float rr = pr + br;

            if (dist2 <= rr * rr) {
                player->OnCollision(tb.get());
                tb->OnCollision(player);
            }
        }
    }
}

void CollisionManager::ResolvePlayerBulletTurretBulletCollisions(std::vector<std::unique_ptr<Bullet>>& playerBullets, std::vector<std::unique_ptr<CharacterBase>>& enemies, const CountDown& countDown) {
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

        const auto pp = playerBulletCollider->GetTranslate();
        const float pr = playerBulletCollider->GetRadius();

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

                const auto tp = turretBulletCollider->GetTranslate();
                const float tr = turretBulletCollider->GetRadius();

                const float dx = pp.x - tp.x;
                const float dy = pp.y - tp.y;
                const float dz = pp.z - tp.z;
                const float dist2 = dx * dx + dy * dy + dz * dz;
                const float rr = pr + tr;

                if (dist2 <= rr * rr) {
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
