#pragma once

#include <KamataEngine.h>
#include <vector>
#include <memory>
#include <string>
#include <algorithm>

#include "Application/Characters/Enemy/SeekerEnemy.h"
#include "Application/Effects/Damage/DamageParticle.h"

/// <summary>
/// 敵の生成・更新・削除を一括管理するクラス。  
/// JSON(SCV)で定義されたスポーン情報に基づき時間差で出現させ、  
/// 生存中はUpdateで動作／死亡後は爆発演出を発生させ削除する。
/// 「敵のライフサイクル管理」を担当し、ゲーム進行テンポを制御する中心的クラス。
/// </summary>
class EnemyManager {
public:    
    /// <summary>
    /// SCV(JSON)に記述されている1体分のスポーン設定データ
    /// </summary>
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

    /// <summary>
    /// 初期化処理
    /// </summary>
    void Initialize();

    /// <summary>
    /// 敵スポーンSCV(JSON)の読み込み。  
    /// 読み込まれたデータは time 昇順にソートされ、SpawnEnemiesBySCVで逐次出現させる。
    /// </summary>
    /// <param name="path">読み込むSCVファイルパス</param>
    void LoadEnemyScv(const std::string& path);

    /// <summary>
    /// 敵管理のメイン処理。  
    /// ・スポーンタイマー更新  
    /// ・SCVに従って新規敵生成  
    /// ・既存敵のUpdate実行  
    /// ・爆発パーティクルの更新／削除  
    /// </summary>
    /// <param name="dt">経過時間（秒）</param>
    /// <param name="playerPos">プレイヤーのワールド位置（追従・スポーン位置計算用）</param>
    void Update(float dt, const KamataEngine::Vector3& playerPos);

    /// <summary>
    /// 描画処理
    /// </summary>
    /// <param name="camera">描画に使用するカメラ</param>
    void Draw(KamataEngine::Camera* camera);

    /// <summary>
    /// 敵リストへの参照を返す（外部で当たり判定などに使用）
    /// </summary>
    std::vector<std::unique_ptr<CharacterBase>>& GetEnemies() { return enemies_; }

     /// <summary>
     /// 死亡フラグの立った敵を削除する
     /// </summary>
     void RemoveDeadEnemies();

private:
    /// <summary>
    /// SCVスポーンリストに従い、出現時間を迎えた敵を生成する。
    /// スポーン座標は「playerPos + 相対値」で決定する。
    /// </summary>
    void SpawnEnemiesByScv(const KamataEngine::Vector3& playerPos);

    /// <summary>
    /// 指定座標に爆発パーティクルを生成
    /// </summary>
    void SpawnExplosionAt(const KamataEngine::Vector3& pos);

private:
    // 敵本体
    std::vector<std::unique_ptr<CharacterBase>> enemies_;  

    // スポーン情報とタイマー
    std::vector<EnemySpawnData> enemySpawnList_;
    float enemySpawnTimer_ = 0.0f;

    // 敵爆発用パーティクル
    KamataEngine::Model* explosionModel_ = nullptr;
    std::vector<std::unique_ptr<DamageParticle>> explosionParticles_;
};
