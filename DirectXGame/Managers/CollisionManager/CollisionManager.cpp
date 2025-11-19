#include "CollisionManager.h"

using namespace KamataEngine;

void CollisionManager::ResolvePlayerEnemyCollisions(
    Player* player,
    std::vector<std::unique_ptr<CharactorBase>>& enemies,
    const CountDown& countDown)
{
    if (!player || countDown.IsInputLocked() || player->IsExplosionFinished()) {
        return;
    }

    Collider* pc = player->GetCollider().get();
    if (!pc) { return; }

    const auto p = pc->GetTranslate();
    const float pr = pc->GetRadius();

    for (auto& e : enemies) {
        if (!e) { continue; }
        Collider* ec = e->GetCollider().get();
        if (!ec) { continue; }

        const auto q = ec->GetTranslate();
        const float er = ec->GetRadius();

        const float dx = p.x - q.x;
        const float dy = p.y - q.y;
        const float dz = p.z - q.z;
        const float dist2 = dx * dx + dy * dy + dz * dz;
        const float r = pr + er;

        if (dist2 <= r * r) {
            player->OnCollision(e.get());
            e->OnCollision(player);
        }
    }
}

void CollisionManager::ResolveBulletEnemyCollisions(
    std::vector<std::unique_ptr<Bullet>>& bullets,
    std::vector<std::unique_ptr<CharactorBase>>& enemies,
    const CountDown& countDown)
{
    if (countDown.IsInputLocked()) { return; }

    for (auto& b : bullets) {
        if (!b || b->IsDead()) { continue; }
        Collider* bc = b->GetCollider().get();
        if (!bc) { continue; }

        const auto bp = bc->GetTranslate();
        const float br = bc->GetRadius();

        for (auto& e : enemies) {
            if (!e) { continue; }

            // 既に死んだ敵は飛ばす
            if (auto* s = dynamic_cast<SeekerEnemy*>(e.get())) {
                if (s->IsDead()) { continue; }
            }

            Collider* ec = e->GetCollider().get();
            if (!ec) { continue; }

            const auto ep = ec->GetTranslate();
            const float er = ec->GetRadius();

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
