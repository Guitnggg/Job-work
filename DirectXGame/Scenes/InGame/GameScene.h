#pragma once

#include <KamataEngine.h>
#include <vector>
#include <random>
#include <memory>
#include <algorithm>

#include "Application/Skydome/Skydome.h"
#include "Application/Asteroid/Asteroid.h"
#include "Application/RailCamera/RailCamera.h"
#include "Application/CountDown/CountDown.h"
#include "Application/Charactor/Player/Player.h"
#include "Application/Charactor/Player/Graph.h"
#include "Application/Charactor/Player/Bullet.h"
#include "Application/Charactor/Enemy/SeekerEnemy.h"


#include "IScene.h"
class FinishScene;

class GameScene : public IScene {
public:

    /// <summary>
    /// コンストラクタ
    /// </summary>
    GameScene();

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~GameScene();

    /// <summary>
    /// 初期化処理
    /// </summary>
    void Initialize()override;

    /// <summary>
    /// 更新処理
    /// </summary>
    void Update()override;

    /// <summary>
    /// 描画処理
    /// </summary>
    void Draw()override;

public:

    /// <summary>
    /// シーン変遷
    /// </summary>
    bool IsEnd() const override { return isEnd_; }
    IScene* NextScene() const override;

    SceneName GetSceneName() const override { return SceneName::InGame; }  // シーン名

private:

    /// <summary>
    /// 当たり判定
    /// </summary>
    void ResolvePlayerEnemyCollisions();
    void ResolveBulletEnemyCollisions();

private:

    // ========== 基本 ==========
    KamataEngine::DirectXCommon* dxCommon_ = nullptr;
    KamataEngine::Input* input_ = nullptr;
    KamataEngine::Audio* audio_ = nullptr;

    KamataEngine::WorldTransform* worldTransform_;
    KamataEngine::Camera camera_;
    KamataEngine::Model* model_ = nullptr;

    // ========== ３カウントUI ==========
    CountDown countDown_;

    // ========== レールカメラ ==========
    bool isRailCameraActive_ = true;
    RailCamera* railCamera_ = nullptr;

    // ========== 天球 ==========
    Skydome* skydome_ = nullptr;

    // ========== プレイヤー ==========
    Player* player_ = nullptr;

    // 
    std::unique_ptr<Graph>hpGraph_;

    // 
    std::vector<std::unique_ptr<Bullet>> bullets_;
    int fireCooldownFrames_ = 0;      // 発射クールダウン
    const int kFireCooldownMax_ = 9;  // 約0.15秒@60fps

    // ========== 敵 ==========
    std::vector<std::unique_ptr<CharactorBase>> enemies_;
    float enemySpawnTimer_ = 0.0f;
    const float kEnemySpawnInterval_ = 2.0f; // 敵出現間隔（秒）

    // ========== シーン制御 ==========
    bool isEnd_ = false;
};
