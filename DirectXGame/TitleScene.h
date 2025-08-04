#pragma once

#include <KamataEngine.h>

#include "IScene.h"
class GameScene;

class TitleScene : public IScene {
public:

    /// <summary>
    /// コンストラクタ
    /// </summary>
    TitleScene();

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~TitleScene();

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

    SceneName GetSceneName() const override { return SceneName::Title; }  // シーン名

private:

    KamataEngine::DirectXCommon* dxCommon_ = nullptr;
    KamataEngine::Input* input_ = nullptr;
    KamataEngine::WorldTransform* worldTransform_;
    KamataEngine::Camera* camera_;


    KamataEngine::Model* model_ = nullptr;

    uint32_t textureHandle_ = 0;
    KamataEngine::Sprite* sprite_ = nullptr;

    // 終了フラグ
    bool isEnd_ = false;
};

