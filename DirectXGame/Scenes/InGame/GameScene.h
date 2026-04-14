#pragma once

#include <KamataEngine.h>
#include <vector>
#include <random>
#include <memory>
#include <algorithm>
#include <array>
#include <string>
#include <unordered_map>

#include "Application/Background/Skydome/Skydome.h"
#include "Application/Cameras/RailCamera/RailCamera.h"
#include "Application/Characters/Player/Player.h"
#include "Application/Effects/SpeedLine/SpeedLine.h"
#include "Application/Effects/Smoke/GpuSmokeEmitter.h"
#include "Application/Objects/Asteroid/Asteroid.h"

#include "Application/UI/PauseMenu/PauseMenu.h"
#include "Application/UI/CountDown/CountDown.h"
#include "Application/UI/HPBar/Graph.h"
#include "Application/UI/Score/Score.h"

#include "Managers/EnemyManager/EnemyManager.h"
#include "Managers/BulletManager/BulletManager.h"
#include "Managers/CollisionManager/CollisionManager.h"
#include "Managers/UIManager/UIManager.h"

#include "Scenes/IScene.h"
class FinishScene;
class ClearScene;
class TitleScene;
class GameSceneUpdateExecutor;

enum class GameState {
    CountDown,   // 3カウント
    Playing,     // ゲーム本編
};

enum class GameResult {
    Clear,  // クリアに成功
    Fail,   // プレイヤーが倒された
    None,   // 判定前
};

enum class SceneTransitionPhase {
    None,
    IntroCinematic, // ゲーム開始前の演出
    ClearCinematic, // クリア後の演出
    FailCinematic,  // 失敗後の演出
};

/// <summary>
/// ゲーム本編シーン。  
/// カウントダウン → 戦闘 → 判定（クリア／失敗） → 演出 → シーン遷移  
/// までの流れを一括で制御する。  
/// 弾・敵・HP・スコア・パーティクル・カメラを統合する中枢クラス。
/// </summary>
class GameScene : public IScene {
public:
    explicit GameScene(std::string levelJsonPath = "./Resources/Levels/Tutorial.json");
    ~GameScene();

    /// <summary>
    /// 初期化処理
    /// 全てのゲーム要素を生成・初期化する
    /// プレイヤー・天球・カメラ・敵・弾・UI・演出・各モデルを生成する。
    /// </summary>
    void Initialize() override;

    /// <summary>
    /// 更新処理
    /// </summary>
    void Update() override;

    /// <summary>
    /// 描画処理
    /// 背景→3Dオブジェクト→UIの順に描画
    /// </summary>
    void Draw() override;

private:
    friend class GameSceneUpdateExecutor;

    /// <summary>
    /// デバッグ調整用 ImGui
    /// </summary>
    void DrawImGui();

public:
    bool IsEnd() const override { return isEnd_; }
    std::unique_ptr<IScene> NextScene() const override;

    SceneName GetSceneName() const override { return SceneName::InGame; }

private:
    // ========== 基本 ==========
    KamataEngine::DirectXCommon* dxCommon_ = nullptr;
    KamataEngine::Input* input_ = nullptr;
    KamataEngine::Audio* audio_ = nullptr;

    KamataEngine::WorldTransform* worldTransform_ = nullptr;
    KamataEngine::Camera camera_;
    KamataEngine::Model* model_ = nullptr;

    // ========== フレーム ==========
    float kFixedDeltaTime_ = 1.0f / 60.0f;

    // ========== ステート ==========
    GameState state_ = GameState::CountDown;

    // ========== ３カウントUI ==========
    CountDown countDown_;

    // ========== レールカメラ ==========
    bool isRailCameraActive_ = true;
    RailCamera* railCamera_ = nullptr;

    // ========== 天球 ==========
    Skydome* skydome_ = nullptr;

    // ========== プレイヤー ==========
    Player* player_ = nullptr;
    KamataEngine::Vector3 previousPlayerPos_{};

    // ========== 照準 ==========
    uint32_t reticleTexHandle_ = 0;
    std::unique_ptr<KamataEngine::Sprite>reticleSprite_;
    KamataEngine::Vector2 reticlePos_{ 640.0f,360.0f };
    KamataEngine::Vector3 shootDirection_{ 0.0f,0.0f,1.0f };

    // ========== ミサイルロックオン演出 ==========
    struct LockOnMarker {
        std::unique_ptr<KamataEngine::Sprite> sprite;
        CharacterBase* target = nullptr;
        float pulseT = 0.0f;
    };

    uint32_t lockOnTexHandle_ = 0;
    std::vector<LockOnMarker> lockOnMarkers_;

    // ========== ステージデータ ==========
    std::string levelJsonPath_;

    // ========== 弾・敵管理 ==========
    EnemyManager enemyManager_;
    BulletManager bulletManager_;

    // ========== スピード演出 ==========
    SpeedLine speedLine_;

    // ========== エンジンスモーク ==========
    std::unique_ptr<GpuSmokeEmitter> engineSmokeEmitter_;
    std::unique_ptr<GpuSmokeEmitter> damageSmokeEmitter_;
    std::unique_ptr<GpuSmokeEmitter> missileAfterburnerEmitter_;
    float smokeEmitTimer_ = 0.0f;

    // スモークのパラメータ
    struct SmokeParams {
        float emitInterval = 0.1f;
        float lifeTime = 0.2f;
        float startScale = 0.15f;
        int burstCount = 1;
        float baseZSpeed = -0.6f;
    };
    SmokeParams normalSmokeParams_;  // 通常時
    SmokeParams boostSmokeParams_;   // クリア演出時

    float kSmokeOffsetY_ = -0.3f;
    float kSmokeOffsetZ_ = -1.5f;
    float kSmokeRandXY_ = 0.5f;
    float kSmokeRandZ_ = 0.10f;

    // ========== GPU被弾演出 ==========
    std::unordered_map<CharacterBase*, int32_t> prevEnemyHpMap_;
    int32_t prevPlayerHp_ = 0;
    float kDamageGpuLife_ = 0.35f;
    float kDamageGpuStartScale_ = 0.20f;
    float kDamageGpuEndScale_ = 0.8f;
    int kDamageGpuBurst_ = 32;
    float kDamageGpuSpeed_ = 6.0f;

    // ========== ミサイルアフターバーナー ==========
    float kMissileAfterburnerLife_ = 0.28f;
    float kMissileAfterburnerStartScale_ = 0.13f;
    float kMissileAfterburnerEndScale_ = 0.0f;
    float kMissileAfterburnerSpeed_ = -3.8f;
    float kMissileAfterburnerOffsetZ_ = -0.55f;
    float kMissileAfterburnerRand_ = 0.2f;

    // ========== UI（HPバー／スコアなど） ==========
    UIManager uiManager_;
    int clearScore_ = 0;

    int kScorePerEnemy_ = 100;
    int kClearScore_ = 5000;

    // ========== Pauseメニュー ==========
    bool isPaused_ = false;
    bool isDebugUpdatePaused_ = false;
    std::unique_ptr<PauseMenu> pauseMenu_;
    std::unique_ptr<KamataEngine::Sprite> pauseTitleSprite_;
    uint32_t pauseTitleTexHandle_ = 0;
    static constexpr float kScreenWidth_ = 1280.0f;
    static constexpr float kScreenHeight_ = 720.0f;
    static constexpr float kPauseTitlePosX_ = 20.0f;
    static constexpr float kPauseTitlePosY_ = 16.0f;

    // ポーズからの遷移要求
    bool requestRetry_ = false;
    bool requestToTitle_ = false;

    // ========== クリア後演出用 ==========
    bool  isClearAnimating_ = false;
    float clearAnimTimer_ = 0.0f;

    float kClearBoostSpeedZ_ = 1.0f;
    float kClearBoostSpeedY_ = 5.0f;
    float kClearRotateSpeedX_ = 0.5f;

    float kClearShrinkStart_ = 0.5f;
    float kClearShrinkSpeed_ = 1.0f;
    float kClearAnimEndTime_ = 2.0f;

    // ========== 遷移演出 ==========
    SceneTransitionPhase transitionPhase_ = SceneTransitionPhase::None;
    float transitionTimer_ = 0.0f;
    float timeScale_ = 1.0f;

    int transitionScoreBonus_ = 0;
    bool failSecondExplosionDone_ = false;

    uint32_t seExplosionHandle_ = 0;

    // ========== シーン制御 ==========
    bool isEnd_ = false;
    GameResult result_ = GameResult::None;
};
