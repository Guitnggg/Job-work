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
#include "Application/Score/Score.h"

#include "EnemyManager.h"
#include "BulletManager.h"
#include "CollisionManager.h"
#include "UIManager.h"

#include "Scenes/IScene.h"
class FinishScene;
class ClearScene;

enum class GameResult {
    Clear,
    Fail,
    None
};

class GameScene : public IScene {
public:
    GameScene();
    ~GameScene();

    void Initialize() override;
    void Update() override;
    void Draw() override;

public:
    bool IsEnd() const override { return isEnd_; }
    IScene* NextScene() const override;

    SceneName GetSceneName() const override { return SceneName::InGame; }

private:
    // ========== 基本 ==========
    KamataEngine::DirectXCommon* dxCommon_ = nullptr;
    KamataEngine::Input* input_ = nullptr;
    KamataEngine::Audio* audio_ = nullptr;

    KamataEngine::WorldTransform* worldTransform_ = nullptr;
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

    // ========== 弾・敵管理 ==========
    EnemyManager enemyManager_;
    BulletManager bulletManager_;

    // ========== UI（HPバー／スコアなど） ==========
    UIManager uiManager_;
    int clearScore_ = 0;

    // ========== クリア後演出用 ==========
    bool  isClearAnimating_ = false;
    float clearAnimTimer_ = 0.0f;

    // ========== シーン制御 ==========
    bool isEnd_ = false;
    GameResult result_ = GameResult::None;
};
