#pragma once

#include <KamataEngine.h>
#include <vector>
#include <random>

#include "Application/Skydome/Skydome.h"
#include "Application/Asteroid/Asteroid.h"
#include "Application/Player/Player.h"
#include "Application/RailCamera/RailCamera.h"
#include "Application/CountDown/CountDown.h"

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

    // ========== 基本 ==========
    KamataEngine::DirectXCommon* dxCommon_ = nullptr;
    KamataEngine::Input* input_ = nullptr;
    KamataEngine::Audio* audio_ = nullptr;

    KamataEngine::WorldTransform* worldTransform_;
    KamataEngine::Camera camera_;
    KamataEngine::Model* model_ = nullptr;

    // ========== ３カウントUI ==========
    CountDown countDown_;

    // ========== 各オブジェクト ==========
    // レールカメラ
    bool isRailCameraActive_ = true;
    RailCamera* railCamera_ = nullptr;

    // 天球
    Skydome* skydome_ = nullptr;

    // プレイヤー
    Player* player_ = nullptr;

    // ========== シーン制御 ==========
    bool isEnd_ = false;
};
