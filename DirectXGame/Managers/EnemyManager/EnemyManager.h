#pragma once

#include <KamataEngine.h>
#include <vector>
#include <memory>
#include <string>
#include <algorithm>

#include "Application/Charactors/Enemy/SeekerEnemy.h"
#include "Application/Effects/Damage/DamageParticle.h"

class EnemyManager {
public:
    /// 敵スポーン用データ
    struct EnemySpawnData {
        float time = 0.0f;
        KamataEngine::Vector3 pos{ 0,0,0 };
        float speed = 0.2f;
        float turnRate = 0.15f;
        int   hp = 1;
        float radius = 1.0f;
        float lifeTime = 30.0f;
    };

public:
    EnemyManager() = default;
    ~EnemyManager() = default;

    /// 初期化
    void Initialize();

    /// JSON(SCV) 読み込み
    void LoadEnemySCV(const std::string& path);

    /// 更新（スポーン & 敵更新）
    /// @param dt         経過時間(秒)
    /// @param playerPos  プレイヤーのワールド位置
    void Update(float dt, const KamataEngine::Vector3& playerPos);

    /// 描画
    void Draw(KamataEngine::Camera* camera);

    /// 敵リストへのアクセス（当たり判定など用）
    std::vector<std::unique_ptr<CharactorBase>>& GetEnemies() { return enemies_; }

    /// 死亡した敵の削除
    void RemoveDeadEnemies();

private:
    /// JSONに従って敵をスポーン
    void SpawnEnemiesBySCV(const KamataEngine::Vector3& playerPos);

    // スポーン情報とタイマー
    void SpawnExplosionAt(const KamataEngine::Vector3& pos);

private:
    // 敵本体
    std::vector<std::unique_ptr<CharactorBase>> enemies_;  

    // スポーン情報とタイマー
    std::vector<EnemySpawnData> enemySpawnList_;
    float enemySpawnTimer_ = 0.0f;

    // 敵爆発用パーティクル
    KamataEngine::Model* explosionModel_ = nullptr;
    std::vector<std::unique_ptr<DamageParticle>> explosionParticles_;
};
