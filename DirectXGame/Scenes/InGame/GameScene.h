#pragma once

#include <KamataEngine.h>
#include <vector>
#include <random>
#include <memory>
#include <algorithm>
#include <array>

#include "Application/Background/Skydome/Skydome.h"
#include "Application/Cameras/RailCamera/RailCamera.h"
#include "Application/Characters/Player/Player.h"
#include "Application/Effects/SpeedLine/SpeedLine.h"
#include "Application/Effects/Smoke/GpuSmokeEmitter.h"
#include "Application/Effects/Damage/DamageParticle.h"
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

enum class GameState {
	Instruction, // 操作説明（複数ページ）
	CountDown,   // 3カウント
	Playing,     // ゲーム本編
};

enum class InstructionPage {
	Move,    // 移動
	Roll,    // ロール
	Attack,  // 攻撃
	Rules,   // ルール説明
};

enum class GameResult {
    Clear,  // クリアに成功
    Fail,   // プレイヤーが倒された
    None,   // 判定前
};

/// <summary>
/// ゲーム本編シーン。  
/// カウントダウン → 戦闘 → 判定（クリア／失敗） → 演出 → シーン遷移  
/// までの流れを一括で制御する。  
/// 弾・敵・HP・スコア・パーティクル・カメラを統合する中枢クラス。
/// </summary>
class GameScene : public IScene {
public:
    GameScene();
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
    /// <summary>
    /// 3カウント更新
    /// </summary>
    /// <param name="dt"></param>
    void CountDownUpdate(float dt);

    /// <summary>
    /// 背景（天球）更新
    /// </summary>
    void BackgroundUpdate();

    /// <summary>
    /// プレイヤー更新
    /// </summary>
    void PlayerUpdate();

    /// <summary>
    /// ダメージパーティクル生成
    /// </summary>
    void SpawnDamageParticles();

    /// <summary>
    /// 戦闘処理
    /// </summary>
    /// <param name="dt"></param>
    void BattleUpdate(float dt);

    /// <summary>
    /// UI更新
    /// </summary>
    void UIUpdate();

    /// <summary>
    /// ダメージパーティクル更新
    /// </summary>
    /// <param name="dt"></param>
    void DamageParticleUpdate(float dt);

    /// <summary>
    /// エンジンスモーク更新
    /// </summary>
    /// <param name="dt"></param>
    void EngineSmokesUpdate(float dt);

    /// <summary>
    /// カメラ更新
    /// </summary>
    void CameraUpdate();

    /// <summary>
    /// スピードライン更新
    /// </summary>
    /// <param name="dt"></param>
    void SpeedLineUpdate(float dt);

    /// <summary>
    /// クリア/失敗判定
    /// </summary>
    void JudgeResultAndStartClear();

    /// <summary>
    /// クリア後演出更新
    /// </summary>
    /// <param name="dt"></param>
    void ClearAnimationUpdate(float dt);

private:

    void InstructionUpdate();

    void NextInstructionPage();

    void PrevInstructionPage();

    void DrawInstruction();

public:
    bool IsEnd() const override { return isEnd_; }
	std::unique_ptr<IScene> NextScene() const override;

    SceneName GetSceneName() const override { return SceneName::InGame; }

private:
    // ========== 基本 ==========
    KamataEngine::DirectXCommon* dxCommon_ = nullptr;
    KamataEngine::Input* input_ = nullptr;
    KamataEngine::Audio* audio_ = nullptr;

    std::unique_ptr<KamataEngine::WorldTransform> worldTransform_;
    std::unique_ptr<KamataEngine::Model> model_;
    KamataEngine::Camera camera_;

    // ========== フレーム ==========
    static constexpr float kFixedDeltaTime_ = 1.0f / 60.0f;

    // ========== ステート ==========
	GameState state_ = GameState::Instruction;
	InstructionPage instructionPage_ = InstructionPage::Move;

	static constexpr size_t kInstructionPageCount_ = 4;
	std::array<uint32_t, kInstructionPageCount_> instructionTexHandles_{};
	std::array<KamataEngine::Sprite*, kInstructionPageCount_> instructionSprites_{};
	KamataEngine::Vector2 instructionPos_{0.0f, 0.0f};

    // ========== ３カウントUI ==========
    CountDown countDown_;

    // ========== レールカメラ ==========
    bool isRailCameraActive_ = true;
    std::unique_ptr<RailCamera> railCamera_;

    // ========== 天球 ==========
    std::unique_ptr<Skydome> skydome_;

    // ========== プレイヤー ==========
    std::unique_ptr<Player> player_;
	KamataEngine::Vector3 previousPlayerPos_{};

    // ========== 弾・敵管理 ==========
    EnemyManager enemyManager_;
    BulletManager bulletManager_;

    // ========== スピード演出 ==========
    SpeedLine speedLine_;

    // ========== ダメージ演出 ==========
    std::vector<std::unique_ptr<DamageParticle>> damageParticles_;
    std::unique_ptr<KamataEngine::Model> damageParticleModel_;

    static constexpr int kDamageParticleCount_ = 10;
    static constexpr float kDamageParticleSpeedXY_ = 2.5f;
    static constexpr float kDamageParticleSpeedZ_ = -2.0f;
    static constexpr float kDamageParticleLife_ = 0.7f;
    static constexpr float kDamageParticleStartScale_ = 0.22f;
    static constexpr float kDamageParticleEndScale_ = 0.0f;

    // ========== エンジンスモーク ==========
    std::unique_ptr<GpuSmokeEmitter> engineSmokeEmitter_;
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

    static constexpr float kSmokeOffsetY_ = -0.3f;
    static constexpr float kSmokeOffsetZ_ = -1.5f;
    static constexpr float kSmokeRandXY_ = 0.5f;
    static constexpr float kSmokeRandZ_ = 0.10f;

    // ========== UI（HPバー／スコアなど） ==========
    UIManager uiManager_;
    int clearScore_ = 0;

    static constexpr int kScorePerEnemy_ = 100;
    static constexpr int kClearScore_ = 1000;

    // ========== Pauseメニュー ==========
    bool isPaused_ = false;
    std::unique_ptr<PauseMenu> pauseMenu_;
    std::unique_ptr<KamataEngine::Sprite> pauseTitleSprite_;
    uint32_t pauseTitleTexHandle_ = 0;
    static constexpr float kPauseTitlePosX_ = 20.0f;
    static constexpr float kPauseTitlePosY_ = 16.0f;
    static constexpr float kPauseTitleScale_ = 0.7f;

    // ポーズからの遷移要求
	bool requestRetry_ = false;
	bool requestToTitle_ = false;

    // ========== クリア後演出用 ==========
    bool  isClearAnimating_ = false;
    float clearAnimTimer_ = 0.0f;

    static constexpr float kClearBoostSpeedZ_ = 1.0f;
    static constexpr float kClearBoostSpeedY_ = 5.0f;
    static constexpr float kClearRotateSpeedX_ = 0.5f;

    static constexpr float kClearShrinkStart_ = 0.5f;
    static constexpr float kClearShrinkSpeed_ = 1.0f;
    static constexpr float kClearAnimEndTime_ = 2.0f;

    static constexpr int kSpeedLineCount_ = 10;
    static constexpr float kCountDownStartDelay_ = 0.1f;
    static constexpr float kCountDownNumberDuration_ = 0.5f;
    static constexpr float kCountDownGoDuration_ = 0.4f;
    static constexpr float kCountDownScaleStart_ = 1.2f;
    static constexpr float kCountDownScaleEnd_ = 1.0f;
    static constexpr float kCountDownBackOvershoot_ = 1.7f;
    static constexpr float kCameraShakeMin_ = -1.0f;
    static constexpr float kCameraShakeMax_ = 1.0f;
    static constexpr float kCameraShakeDuration_ = 1.0f;
    static constexpr float kCameraInputScale_ = 40.0f;

    // ========== シーン制御 ==========
    bool isEnd_ = false;
    GameResult result_ = GameResult::None;
};
