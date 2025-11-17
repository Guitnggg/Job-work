#include "EnemyManager.h"

#include <fstream>
#include <random>

#include <json.hpp>
using json = nlohmann::json;

using namespace KamataEngine;

void EnemyManager::Initialize() {
    enemies_.clear();
    enemySpawnList_.clear();
    enemySpawnTimer_ = 0.0f;
}

void EnemyManager::LoadEnemySCV(const std::string& path)
{
    enemySpawnList_.clear();

    std::ifstream ifs(path);
    if (!ifs) { return; }

    json root;
    ifs >> root;

    if (root.contains("randomAreas")) {
        // 固定シードにしておくと毎回同じ配置になる
        static std::mt19937_64 rng{ 123456789 };

        for (auto& r : root["randomAreas"]) {
            int   count = r.value("count", 0);
            float timeMin = r.value("timeMin", 0.0f);
            float timeMax = r.value("timeMax", timeMin);

            auto  centerNode = r["posCenter"];
            Vector3 center{
                centerNode[0].get<float>(),
                centerNode[1].get<float>(),
                centerNode[2].get<float>()
            };

            auto rangeNode = r["posRange"];
            Vector3 range{
                rangeNode[0].get<float>(),
                rangeNode[1].get<float>(),
                rangeNode[2].get<float>()
            };

            float baseSpeed = r.value("speed", 0.2f);
            float speedRange = r.value("speedRange", 0.0f);
            float turnRate = r.value("turnRate", 0.15f);
            int   hp = r.value("hp", 1);
            float radius = r.value("radius", 1.0f);
            float lifeTime = r.value("lifeTime", 30.0f);

            std::uniform_real_distribution<float> timeDist(timeMin, timeMax);
            std::uniform_real_distribution<float> dx(-range.x, range.x);
            std::uniform_real_distribution<float> dy(-range.y, range.y);
            std::uniform_real_distribution<float> dz(-range.z, range.z);
            std::uniform_real_distribution<float> speedDist(
                baseSpeed - speedRange,
                baseSpeed + speedRange
            );

            for (int i = 0; i < count; ++i) {
                EnemySpawnData d;

                d.time = timeDist(rng);
                d.pos = {
                    center.x + dx(rng),
                    center.y + dy(rng),
                    center.z + dz(rng)
                };
                d.speed = speedDist(rng);
                d.turnRate = turnRate;
                d.hp = hp;
                d.radius = radius;
                d.lifeTime = lifeTime;

                enemySpawnList_.push_back(d);
            }
        }
    }

    // 出現時間でソート
    std::sort(enemySpawnList_.begin(), enemySpawnList_.end(),
        [](const EnemySpawnData& a, const EnemySpawnData& b) {
            return a.time < b.time;
        });
}

void EnemyManager::SpawnEnemiesBySCV(const Vector3& playerPos)
{
    const float t = enemySpawnTimer_;

    while (!enemySpawnList_.empty()) {
        const auto& d = enemySpawnList_.front();

        if (t < d.time) break;

        // JSONのposを「プレイヤーからのオフセット」として使う
        Vector3 spawnPos{
            playerPos.x + d.pos.x,
            playerPos.y + d.pos.y,
            playerPos.z + d.pos.z
        };

        // 敵生成
        auto s = std::make_unique<SeekerEnemy>();
        s->SetInitialPosition(spawnPos);
        s->SetSpeed(d.speed);
        s->SetTurnRate(d.turnRate);
        s->SetInitialHP(d.hp);
        s->SetColliderRadius(d.radius);
        s->SetLifeTime(d.lifeTime);
        s->Initialize();

        enemies_.push_back(std::move(s));
        enemySpawnList_.erase(enemySpawnList_.begin());
    }
}

void EnemyManager::Update(float dt, const Vector3& playerPos)
{
    // タイマー更新 & スポーン
    enemySpawnTimer_ += dt;
    SpawnEnemiesBySCV(playerPos);

    // 敵更新（追尾対象をプレイヤーに）
    for (auto& e : enemies_) {
        if (auto* s = dynamic_cast<SeekerEnemy*>(e.get())) {
            s->SetTarget(playerPos);
        }
        e->Update();
    }
}

void EnemyManager::Draw(Camera* camera)
{
    for (auto& e : enemies_) {
        e->Draw(camera);
    }
}

void EnemyManager::RemoveDeadEnemies()
{
    enemies_.erase(
        std::remove_if(enemies_.begin(), enemies_.end(),
            [](const std::unique_ptr<CharactorBase>& e) {
                if (auto* s = dynamic_cast<SeekerEnemy*>(e.get())) {
                    return s->IsDead();
                }
                return false;
            }),
        enemies_.end());
}
