#pragma once

#include <KamataEngine.h>

#include "SceneName.h"
#include "Skydome.h"

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

    KamataEngine::DirectXCommon* dxCommon_ = nullptr;  // DirectX関連の管理クラス
    KamataEngine::Input* input_ = nullptr;             // 入力管理クラス
    KamataEngine::Model* model_ = nullptr;             // モデル管理クラス
    KamataEngine::Camera camera_;                      // カメラ管理クラス

    // 各種サウンド
    uint32_t changeSEHandle_ = 0;  // シーン変遷SE
    KamataEngine::Audio* changeSE_ = nullptr;

    // 天球
    Skydome* skydome_ = nullptr;

    // 次のシーン
    SceneName nextScene_ = SceneName::None;

    // シーン終了フラグ
    bool isEnd_ = false;
};

