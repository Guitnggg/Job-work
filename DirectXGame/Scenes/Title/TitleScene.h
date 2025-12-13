#pragma once

#include <KamataEngine.h>
#include <vector>
#include <random>

#include "Application/Background/Skydome/Skydome.h"
#include "Application/Objects/Asteroid/Asteroid.h"

#include "Scenes/IScene.h"
class IntroductionScene;

/// <summary>
/// タイトルシーン
/// 背景演出（宇宙空間＋小惑星流れ）とタイトルロゴの落下演出を行い
/// ロゴ落下完了後に「Press SPACE」が点滅して表示される
/// SPACE入力で IntroductionScene に遷移する
/// </summary>
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
    /// シーン終了フラグの取得
    /// </summary>
    bool IsEnd()const override { return isEnd_; }

    /// <summary>
    /// 遷移先のシーン取得
    /// </summary>
    IScene* NextScene()const override;

    /// <summary>
    /// デバック識別用のシーン名取得
    /// </summary>
    SceneName GetSceneName() const override { return SceneName::Title; }

private:

    KamataEngine::DirectXCommon* dxCommon_ = nullptr;  // DirectX関連の管理クラス
    KamataEngine::Input* input_ = nullptr;             // 入力管理クラス
    KamataEngine::WorldTransform* worldTransform_;     // ワールド変換管理クラス
    KamataEngine::Camera camera_;                      // カメラ管理クラス

    // 各種テクスチャ
    uint32_t titleTextureHandle_ = 0;
    KamataEngine::Sprite* titleSprite_ = nullptr;
    uint32_t startTextureHandle_ = 0;
    KamataEngine::Sprite* startSprite_ = nullptr;

    // 各種サウンド
    uint32_t changeSEHandle_ = 0;  // シーン変遷SE
    KamataEngine::Audio* changeSE_ = nullptr;

    // スタート点滅用
    float blinkTimer_ = 0.0f;     // 点滅タイマー
    float blinkInterval_ = 1.0f;  // 点滅間隔

    // タイトル移動用
    KamataEngine::Vector2 titlePosition_ = { 110.0f,-500.0f };       // 初期位置
    KamataEngine::Vector2 titleTargetPosition_ = { 110.0f,150.0f };  // ゴール位置
    float titleFallSpeed_ = 3.0f;       // 移動速度
    bool isTitleFallFinished_ = false;  // 移動終了判定

    // 天球
    Skydome* skydome_ = nullptr;

    // 小惑星
    KamataEngine::Model* asteroidModel_ = nullptr;
    std::vector<Asteroid*> asteroids_;
    static constexpr int kAsteroidCount = 10;    // 背景に流す数
    float spawnZMin_ = 0.0f;                     // 出現Z（奥）
    float spawnZMax_ = 140.0f;
    float recycleZ_ = -50.0f;                    // カメラを超えたら再出現
    float spawnInterval_ = 1.0f;                 // 出現間隔
    float spawnTimer_ = 0.0f;                    // 出現タイマー

    // ランダム生成器
    std::mt19937 mt_{ std::random_device{}() };

    // 終了フラグ
    bool isEnd_ = false;

private:

    // 生成
    Asteroid* SpawnAsteroid();

    // 乱数
    float Rand(float min, float max);
};

