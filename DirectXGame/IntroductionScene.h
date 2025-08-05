#pragma once

#include <KamataEngine.h>

#include "IScene.h"
class GameScene;

class IntroductionScene :public IScene {
public:

    /// <summary>
    /// インストラクタ
    /// </summary>
    IntroductionScene();

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~IntroductionScene();

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
    /// GameSceneへのシーン変遷
    /// </summary>
    bool IsEnd()const override { return isEnd_; }
    IScene* NextScene()const override;

    SceneName GetSceneName() const override { return SceneName::InGame; }

private:

    KamataEngine::DirectXCommon* dxCommon_ = nullptr;  // 
    KamataEngine::Input* input_ = nullptr;             // 

    // シーン終了フラグ
    bool isEnd_ = false;
};

