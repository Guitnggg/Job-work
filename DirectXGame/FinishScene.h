#pragma once

#include <KamataEngine.h>

#include "IScene.h"
class TitleScene;

class FinishScene :public IScene {
public:

    /// <summary>
    /// インストラクタ
    /// </summary>
    FinishScene();

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~FinishScene();

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
    bool IsEnd() const override { return isEnd_; }  // シーン終了フラグ
    IScene* NextScene() const override;             // 次のシーンを返す

    SceneName GetSceneName() const override { return SceneName::Finish; }  // シーン名

private:

    KamataEngine::DirectXCommon* dxCommon_ = nullptr;  // DirectX関連
    KamataEngine::Input* input_ = nullptr;             // 入力関連
    KamataEngine::WorldTransform* worldTransform_;     // ワールド変換管理クラス
    KamataEngine::Camera* camera_;                     // カメラ管理クラス

    // 各テクスチャ
    uint32_t BackgroundTextureHandle_ = 0;
    KamataEngine::Sprite* BackgroundSprite_ = nullptr;
    uint32_t FinishTextureHandle_ = 0;
    KamataEngine::Sprite* FinishSprite_ = nullptr;
    uint32_t ReturnTextureHandle_ = 0;
    KamataEngine::Sprite* ReturnSprite_ = nullptr;

    // 終了フラグ
    bool isEnd_ = false;

    // 点滅用
    float blinkTimer_ = 0.0f;     // 点滅タイマー
    float blinkInterval_ = 1.0f;  // 点滅間隔
};
