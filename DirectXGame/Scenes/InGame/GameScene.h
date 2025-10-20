#pragma once

#include <KamataEngine.h>
#include <vector>
#include <random>

#include "Skydome.h"
#include "Asteroid.h"
#include "Player.h"
#include "RailCamera.h"

#include "IScene.h"
class FinishScene;

class GameScene : public IScene {
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
    bool IsEnd() const override { return isEnd_; }
    IScene* NextScene() const override;

    SceneName GetSceneName() const override { return SceneName::InGame; }  // シーン名

private:

    // ========== 基本 ==========
    KamataEngine::DirectXCommon* dxCommon_ = nullptr;
    KamataEngine::Input* input_ = nullptr;
    KamataEngine::Audio* audio_ = nullptr;

    KamataEngine::WorldTransform* worldTransform_;
    KamataEngine::Camera camera_;
    KamataEngine::Model* model_ = nullptr;

    // ========== ３カウント用UI ==========
    uint32_t count1TextureHandle_ = 0;
    KamataEngine::Sprite* count1Sprite_ = nullptr;
    uint32_t count2TextureHandle_ = 0;
    KamataEngine::Sprite* count2Sprite_ = nullptr;
    uint32_t count3TextureHandle_ = 0;
    KamataEngine::Sprite* count3Sprite_ = nullptr;
    uint32_t startTextureHandle_ = 0;
    KamataEngine::Sprite* startSprite_ = nullptr;

    enum class StartPhase { ReadyDelay, Count3, Count2, Count1, Go, Done };
    StartPhase startPhase_ = StartPhase::ReadyDelay;

    // タイミング（秒）
    float readyDelay_ = 0.10f;  // 着地後の“間”
    float countUnit_ = 0.50f;   // 3,2,1 の各表示時間
    float goDuration_ = 0.40f;  // GO!! の表示時間

    float phaseTimer_ = 0.0f;   // 現在フェーズの経過時間（0→指定秒）
    bool  inputLocked_ = true;  // カウント中は入力無効

    // ビジュアルパラメータ
    float countScaleStart_ = 1.20f;  // 数字のスケール開始値
    float countScaleEnd_ = 1.00f;    // 数字のスケール終了値
    float backS_ = 1.70f;            // easeOutBack のオーバー量
    float goFlashAlpha_ = 0.25f;     // GO!! で乗せる小フラッシュ（描画側で使うなら）

    // サウンド
    uint32_t seBeepHandle_ = 0;  // 3,2,1 の短いビープ
    uint32_t seGoHandle_ = 0;    // GO!! の明るいSE
    bool     goPlayed_ = false;

    // ========== 各オブジェクト ==========
    // レールカメラ
    bool isRailCameraActive_ = true;
    RailCamera* railCamera_ = nullptr;

    // 天球
    Skydome* skydome_ = nullptr;

    // プレイヤー
    Player* player_ = nullptr;

    // ========== シーン制御 ==========
    bool isEnd_ = false;

private:

    //========= 3カウント制御 =========
    // t∈[0,1] → easeOutBack
    float EaseOutBack(float t, float s) const;

    // 今のフェーズの総時間を返す
    float CurrentPhaseDuration() const;

    // 今のフェーズに対応するスプライトを返す（nullptrなら何も描画しない）
    KamataEngine::Sprite* CurrentPhaseSprite() const;

    // 今のフェーズの進行（0→1）に応じたアルファ（0→1→0 の三角波）
    float CurrentPhaseAlpha(float t01) const;

    // 今のフェーズの進行（0→1）に応じたスケール（easeOutBack）
    float CurrentPhaseScale(float t01) const;

    // フェーズ遷移
    void AdvancePhase();
};
