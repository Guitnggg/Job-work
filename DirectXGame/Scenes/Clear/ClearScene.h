#pragma once

#include <KamataEngine.h>
#include <vector>
#include <random>

#include "Application/Asteroid/Asteroid.h"
#include "Application//Skydome/Skydome.h"

#include "Scenes/IScene.h"
class TitleScene;

class ClearScene :public IScene {
public:

    ClearScene();

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

    // 終了フラグ
    bool isEnd_ = false;

private:

    // 生成
    Asteroid* SpawnAsteroid();

    // 乱数
    float Rand(float min, float max);
};

