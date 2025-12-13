#pragma once

#include <KamataEngine.h>
#include <vector>
#include <random>

#include "Scenes/SceneName.h"
#include "Application/Background/Skydome/Skydome.h"
#include "Application/Objects/Asteroid/Asteroid.h"

#include "Scenes/IScene.h"
class GameScene;

/// <summary>
/// 操作説明・ゲーム導入を目的としたイントロシーン
/// 背景アニメーション（小惑星＋天球）＋説明画像の表示を行い
/// ESCでタイトルへ戻る・SPACEでゲーム本編へ進むシーン遷移を担当する
/// </summary>
class IntroductionScene :public IScene {
public:
    /// <summary>
    /// コンストラクタ
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
    SceneName GetSceneName() const override { return SceneName::Introduction; }

    /// <summary>
    /// シーン繊維の資格演出ヒント
    /// </summary>
    TransitionHint GetTransitionHint(SceneName to)const override;

private:

    KamataEngine::DirectXCommon* dxCommon_ = nullptr;  // DirectX関連の管理クラス
    KamataEngine::Input* input_ = nullptr;             // 入力管理クラス
    KamataEngine::Model* model_ = nullptr;             // モデル管理クラス
    KamataEngine::Camera camera_;                      // カメラ管理クラス

    // 各種テクスチャ
    uint32_t returnTitleTextureHandle_ = 0;
    KamataEngine::Sprite* returnTitleSprite_ = nullptr;

    uint32_t introTextureHandle_ = 0;
    KamataEngine::Sprite* introSprite_ = nullptr;

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

    // 次のシーン
    SceneName nextScene_ = SceneName::None;

    // シーン終了フラグ
    bool isEnd_ = false;

private:

    /// <summary>
    /// 小惑星を生成し背景として流す
    /// </summary>
    Asteroid* SpawnAsteroid();

    /// <summary>
    /// 乱数[min,max]を返す
    /// </summary>
    float Rand(float min, float max);
};