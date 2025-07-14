#pragma once

#include <KamataEngine.h>

#include "IScene.h"

class GameScene : public IScene
{
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
    void Initialize();

    /// <summary>
    /// 更新処理
    /// </summary>
    void Update();

    /// <summary>
    /// 描画処理
    /// </summary>
    void Draw();

    /// <summary>
    /// シーン終了フラグ
    /// </summary>
    bool isEnd_ = false;

public:

    /// <summary>
    /// シーン変遷
    /// </summary>    
    bool IsEnd() const override { return isEnd_; }
    IScene* NextScene() const override { return nullptr; }  // 一旦ゲームが終了するようになっている

    SceneName GetSceneName() const override { return SceneName::InGame; }  // シーン名

private:

    KamataEngine::DirectXCommon* dxCommon_ = nullptr;
    KamataEngine::Input* input_ = nullptr;
    KamataEngine::Audio* audio_ = nullptr;

    KamataEngine::WorldTransform worldTransform_;
    KamataEngine::Camera camera_;
    KamataEngine::Model* model_ = nullptr;
};
