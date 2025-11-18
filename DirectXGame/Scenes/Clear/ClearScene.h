#pragma once

#include <KamataEngine.h>
#include <vector>
#include <random>
#include <memory>
#include <algorithm>

#include "Application/Asteroid/Asteroid.h"
#include "Application/Skydome/Skydome.h"
#include "Application/Score/Score.h"

#include "Scenes/IScene.h"
class TitleScene;

enum class ClearPhase {
    CameraMove,
    TitlePop,
    ResultCount,
    WaitInput
};

class ClearScene :public IScene {
public:

    ClearScene(int finalScore);

    ~ClearScene();

    void Initialize()override;

    void Update()override;

    void Draw()override;

public:

    bool IsEnd() const override { return isEnd_; }  // シーン終了フラグ
    IScene* NextScene() const override;             // 次のシーンを返す

    SceneName GetSceneName() const override { return SceneName::Clear; }

private:

    KamataEngine::DirectXCommon* dxCommon_ = nullptr;  // DirectX関連
    KamataEngine::Input* input_ = nullptr;             // 入力関連
    KamataEngine::WorldTransform* worldTransform_;     // ワールド変換管理クラス
    KamataEngine::Camera camera_;                      // カメラ管理クラス

    // 各種サウンド
    uint32_t changeSEHandle_ = 0;  // シーン変遷SE
    KamataEngine::Audio* changeSE_ = nullptr;

    uint32_t pointSEHandle_ = 0;
    KamataEngine::Audio* pointSE_ = nullptr;

    // 天球
    Skydome* skydome_ = nullptr;

    // 小惑星
    KamataEngine::Model* asteroidModel_ = nullptr;
    std::vector<Asteroid*> asteroids_;
    int   asteroidCount_ = 10;    // 背景に流す数
    float spawnZMin_ = 0.0f;      // 出現Z（奥）
    float spawnZMax_ = 140.0f;
    float recycleZ_ = -50.0f;     // カメラを超えたら再出現
    float spawnInterval_ = 1.0f;  // 出現間隔
    float spawnTimer_ = 0.0f;     // 出現タイマー

    // ランダム生成器
    std::mt19937 mt_{ std::random_device{}() };

    // ★ 追加：クリア演出用
    ClearPhase phase_ = ClearPhase::CameraMove;
    float phaseTimer_ = 0.0f;

    // ★ 追加：最終スコアと表示用スコア
    int finalScore_ = 0;
    int displayedScore_ = 0;
    Score scoreUI_;

    // ★ 追加：GAME CLEAR!! テキスト
    KamataEngine::Sprite* clearTextSprite_ = nullptr;
    KamataEngine::Vector2 clearTextBaseSize_ = { 512.0f*2, 128.0f*2 };

    // 終了フラグ
    bool isEnd_ = false;

private:

    // 生成
    Asteroid* SpawnAsteroid();

    // 乱数
    float Rand(float min, float max);
};