#pragma once

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include <KamataEngine.h>

#include "Application/Characters/Enemy/SeekerEnemy.h"
#include "Application/Characters/Enemy/TurretEnemy.h"

/// <summary>
/// JSON の出現データを読み込み、敵の生成、更新、描画、破棄を管理する。
/// </summary>
class EnemyManager {
public:
    /// <summary>
    /// 謨ｵ蜃ｺ迴ｾ繝・・繧ｿ・・SON 1莉ｶ蛻・ｼ・
    /// </summary>
    struct EnemySpawnData {
        float time = 0.0f;
        KamataEngine::Vector3 pos{ 0.0f, 0.0f, 0.0f };

        // 敵のタイプ（seeker" / "turret"）
        std::string type = "seeker";

        // ===== Enemy共通 =====
        int32_t hp = kDefaultHp;
        float radius = kDefaultColliderRadius;
        float lifeTime = kDefaultLifeTime;

        // ===== Seeker用 =====
        float speed = kDefaultSeekerSpeed;
        float turnRate = kDefaultSeekerTurnRate;

        // ===== Turret用 =====
        int32_t shootIntervalFrames = kDefaultShootIntervalFrames;
        float bulletSpeed = kDefaultBulletSpeed;
        float bulletLifeTime = kDefaultBulletLifeTime;
    };

public:
    /// <summary>
    /// 初期化処理
    /// </summary>
    void Initialize();

    /// <summary>
    /// JSON形式の敵出現データを読み込む
    /// </summary>
    void LoadEnemyCsv(const std::string& path);

    /// <summary>
    /// 更新処理
    /// </summary>
    /// <param name="dt">デルタタイム</param>
    /// <param name="playerPos">プレイヤーの位置</param>
    void Update(float dt, const KamataEngine::Vector3& playerPos);

    /// <summary>
    /// 描画処理
    /// </summary>
    void Draw(const KamataEngine::Camera* camera);

    /// <summary>
    /// 衝突判定用の敵リストを取得する
    /// </summary>
    std::vector<std::unique_ptr<CharacterBase>>& GetEnemies() { return enemies_; }

    /// <summary>
    /// 指定位置から近い順に生存中の敵を取得する。CollisionManager などで使用する
    /// </summary>
    std::vector<CharacterBase*> GetNearestEnemies(const KamataEngine::Vector3& from, int32_t maxCount) const;

    /// <summary>
    /// 死亡した敵をリストから削除する
    /// </summary>
    void RemoveDeadEnemies();

    /// <summary>
    /// 出現待ちを含めて敵が残っていないかを返す
    /// </summary>
    bool IsAllEnemyDestroyed() const { return enemies_.empty() && enemySpawnList_.empty(); }

    /// <summary>
    /// Boss戦への切り替え時に、出現中・出現待ちの通常敵をすべて破棄する
    /// </summary>
    void ClearAllEnemies();
    void ClearPendingSpawns();

private:
    /// <summary>
    /// CSVの出現データに基づいて敵を出現させる
    /// </summary>
    void SpawnEnemiesByCsv(const KamataEngine::Vector3& playerPos);

    /// <summary>
    /// 出現データに基づいて敵を生成する
    /// </summary>
    std::unique_ptr<CharacterBase> CreateEnemy_(const EnemySpawnData& spawnData, const KamataEngine::Vector3& spawnPos) const;

private:
    // ===== 定数（デフォルト値） ====
    static constexpr float kDefaultSeekerSpeed = 0.2f;
    static constexpr float kDefaultSeekerTurnRate = 0.15f;
    static constexpr int32_t kDefaultHp = 1;
    static constexpr float kDefaultColliderRadius = 1.0f;
    static constexpr float kDefaultLifeTime = 30.0f;

    static constexpr int32_t kDefaultShootIntervalFrames = 60;
    static constexpr float kDefaultBulletSpeed = 2.8f;
    static constexpr float kDefaultBulletLifeTime = 3.0f;

private:
    // 生存中の敵リスト
    std::vector<std::unique_ptr<CharacterBase>> enemies_;

    // 出現町の敵データ
    std::vector<EnemySpawnData> enemySpawnList_;

    // 経過時間
    float enemySpawnTimer_ = 0.0f;
};
