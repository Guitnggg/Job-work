#pragma once

#include <KamataEngine.h>
#include <vector>
#include <random>
#include <memory>
#include <algorithm>
#include <cmath>

#include "Application/Objects/Asteroid/Asteroid.h"
#include "Application/Background/Skydome/Skydome.h"
#include "UI/Score/Score.h"

#include "Scenes/IScene.h"
class TitleScene;

enum class ClearPhase {
    CameraMove,
    TitlePop,
    ResultCount,
    WaitInput
};

/// <summary>
/// ゲームクリア時に遷移するシーン
/// 背景演出→GameClearテキスト表示→スコアカウントアップ→入力待ち
/// →タイトルへ遷移
/// </summary>
class ClearScene :public IScene {
public:

    /// <summary>
    /// コンストラクタ
    /// </summary>
    /// <param name="finalScore">ゲーム中で獲得した最終スコア</param>
    ClearScene(int finalScore);

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~ClearScene();

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
    /// フェードアウト/入力完了でシーン変遷
    /// </summary>
    bool IsEnd() const override { return isEnd_; }  // シーン終了フラグ

    /// <summary>
    /// シーン終了後に遷移するシーンを返す
    /// </summary>
    IScene* NextScene() const override;

    /// <summary>
    /// デバック用：シーン名取得
    /// </summary>
    SceneName GetSceneName() const override { return SceneName::Clear; }

private:

    // DirectX / 入力 / カメラ / 行列
    KamataEngine::DirectXCommon* dxCommon_ = nullptr;
    KamataEngine::Input* input_ = nullptr;
    KamataEngine::WorldTransform* worldTransform_ = nullptr;
    KamataEngine::Camera camera_;

    // 効果音
    uint32_t changeSEHandle_ = 0;
    uint32_t pointSEHandle_ = 0;

    // 背景（天球 / 小惑星）
    Skydome* skydome_ = nullptr;
    KamataEngine::Model* asteroidModel_ = nullptr;
    std::vector<Asteroid*> asteroids_;
    int   asteroidCount_ = 10;
    float spawnZMin_ = 0.0f;
    float spawnZMax_ = 140.0f;
    float recycleZ_ = -50.0f;
    float spawnInterval_ = 1.0f;
    float spawnTimer_ = 0.0f;
    std::mt19937 mt_{ std::random_device{}() };

    // 演出フェーズ
    ClearPhase phase_ = ClearPhase::CameraMove;
    float phaseTimer_ = 0.0f;

    // スコアUI
    int finalScore_ = 0;
    int displayedScore_ = 0;
    Score scoreUI_;

    // GAME CLEAR テキスト
    KamataEngine::Sprite* clearTextSprite_ = nullptr;
    KamataEngine::Vector2 clearTextBaseSize_ = { 512.0f * 2, 128.0f * 2 };

    // RETURN テキスト（スペースでタイトルに戻る）
    KamataEngine::Sprite* returnTextSprite_ = nullptr;
    KamataEngine::Vector2 returnTextBaseSize_ = { 1024.0f, 128.8f };

    // 終了フラグ
    bool isEnd_ = false;

private:

    /// <summary>
    /// 小惑星を生成して配列へ登録
    /// </summary>
    Asteroid* SpawnAsteroid();

    /// <summary>
    /// 指定範囲の乱数を返す
    /// </summary>
    float Rand(float min, float max);
};