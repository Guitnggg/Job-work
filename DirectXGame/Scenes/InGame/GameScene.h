#pragma once

#include <KamataEngine.h>
#include <algorithm>
#include <array>
#include <memory>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

#include "Application/Background/Skydome/Skydome.h"
#include "Application/Cameras/RailCamera/RailCamera.h"
#include "Application/Characters/Player/Player.h"
#include "Application/Effects/Smoke/GpuSmokeEmitter.h"
#include "Application/Effects/SpeedLine/SpeedLine.h"
#include "Application/Objects/Asteroid/Asteroid.h"

#include "Application/UI/CountDown/CountDown.h"
#include "Application/UI/HPBar/Graph.h"
#include "Application/UI/PauseMenu/PauseMenu.h"
#include "Application/UI/Score/Score.h"

#include "Managers/BulletManager/BulletManager.h"
#include "Managers/BossManager/BossManager.h"
#include "Managers/CollisionManager/CollisionManager.h"
#include "Managers/EnemyManager/EnemyManager.h"
#include "Managers/UIManager/UIManager.h"

#include "Scenes/IScene.h"
class FinishScene;
class ClearScene;
class TitleScene;
class GameSceneUpdateExecutor;

enum class GameState {
	CountDown, // 開始前カウントダウン
	Playing,   // ゲーム本編
};

enum class GameResult {
	Clear,
	Fail,
	None,
};

enum class SceneTransitionPhase {
	None,
	IntroCinematic,
	ClearCinematic,
	FailCinematic,
	BossExitCinematic,
	BossStartCinematic,
	BossIntroCinematic,
	BossClearCinematic,
};

/// <summary>
/// ゲーム本編シーン。
/// プレイヤー、敵、弾、UI、カメラ、演出、結果判定、次シーン遷移をまとめて管理する。
/// </summary>
class GameScene : public IScene {
public:
	explicit GameScene(std::string levelJsonPath = "./Resources/Levels/Tutorial.json");
	~GameScene();

	/// <summary>
	/// ゲーム本編で使うオブジェクトと状態を初期化する。
	/// </summary>
	void Initialize() override;

	/// <summary>
	/// 1フレーム分の更新処理を行う。
	/// </summary>
	void Update() override;

	/// <summary>
	/// 背景、3Dモデル、前面UIの順で描画する。
	/// </summary>
	void Draw() override;

private:
	friend class GameSceneUpdateExecutor;

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

	// ========== ゲーム状態 ==========
	GameState state_ = GameState::CountDown;

	// ========== カウントダウンUI ==========
	CountDown countDown_;

	// ========== レールカメラ ==========
	bool isRailCameraActive_ = true;
	std::unique_ptr<RailCamera> railCamera_;

	// ========== 背景 ==========
	std::unique_ptr<Skydome> skydome_;

	// ========== プレイヤー ==========
	std::unique_ptr<Player> player_;
	KamataEngine::Vector3 previousPlayerPos_{};

	// ========== WASDキーUI ==========
	uint32_t wasdTextureHandle_ = 0;
	std::unique_ptr<KamataEngine::Sprite> wasdWSprite_;
	std::unique_ptr<KamataEngine::Sprite> wasdASprite_;
	std::unique_ptr<KamataEngine::Sprite> wasdSSprite_;
	std::unique_ptr<KamataEngine::Sprite> wasdDSprite_;
	static constexpr float kWasdBaseX = 36.0f;
	static constexpr float kWasdBaseY = 566.0f;
	static constexpr float kWasdKeySize = 64.0f;
	static constexpr float kWasdSpacing = 6.0f;

	// ========== マウス操作UI ==========
	uint32_t mouseTextureHandle_ = 0;
	std::unique_ptr<KamataEngine::Sprite> mouseBaseSprite_;
	std::unique_ptr<KamataEngine::Sprite> mouseLeftSprite_;
	std::unique_ptr<KamataEngine::Sprite> mouseRightSprite_;
	static constexpr float kMouseBaseX = 286.0f;
	static constexpr float kMouseBaseY = 558.0f;
	static constexpr float kMouseSize = 104.0f;

	// ========== 照準 ==========
	uint32_t reticleTexHandle_ = 0;
	std::unique_ptr<KamataEngine::Sprite> reticleSprite_;
	KamataEngine::Vector2 reticlePos_{640.0f, 360.0f};
	KamataEngine::Vector3 shootDirection_{0.0f, 0.0f, 1.0f};

	// ========== ロックオンマーカー ==========
	struct LockOnMarker {
		std::unique_ptr<KamataEngine::Sprite> sprite;
		CharacterBase* target = nullptr;
		float pulseT = 0.0f;
	};

	uint32_t lockOnTexHandle_ = 0;
	std::vector<LockOnMarker> lockOnMarkers_;

	// ========== レベルデータ ==========
	std::string levelJsonPath_;

	// ========== 敵・弾・ボス管理 ==========
	EnemyManager enemyManager_;
	BulletManager bulletManager_;
	BossManager bossManager_;
	bool isBossStageEnabled_ = false;
	bool hasBossBattleStarted_ = false;

	// ========== スピードライン ==========
	SpeedLine speedLine_;

	// ========== GPUパーティクル ==========
	std::unique_ptr<GpuSmokeEmitter> engineSmokeEmitter_;
	std::unique_ptr<GpuSmokeEmitter> damageSmokeEmitter_;
	std::unique_ptr<GpuSmokeEmitter> missileAfterburnerEmitter_;
	float smokeEmitTimer_ = 0.0f;

	// ========== 煙パラメータ ==========
	struct SmokeParams {
		float emitInterval = 0.1f;
		float lifeTime = 0.2f;
		float startScale = 0.15f;
		int burstCount = 1;
		float baseZSpeed = -0.6f;
	};
	SmokeParams normalSmokeParams_; // 通常煙パラメータ
	SmokeParams boostSmokeParams_;  // ブースト煙パラメータ

	static constexpr float kSmokeOffsetY = -0.3f;
	static constexpr float kSmokeOffsetZ = -1.5f;
	static constexpr float kSmokeRandXY = 0.5f;
	static constexpr float kSmokeRandZ = 0.10f;

	// ========== GPUダメージエフェクト ==========
	std::unordered_map<CharacterBase*, int32_t> prevEnemyHpMap_;
	int32_t prevPlayerHp_ = 0;
	static constexpr float kDamageGpuLife = 0.35f;
	static constexpr float kDamageGpuStartScale = 0.20f;
	static constexpr float kDamageGpuEndScale = 0.8f;
	static constexpr int kDamageGpuBurst = 32;
	static constexpr float kDamageGpuSpeed = 6.0f;

	// ========== 効果音 ==========
	uint32_t seEnemyHitHandle_ = 0;
	uint32_t seEnemyKillHandle_ = 0;
	int hitStopFrames_ = 0;
	int hitStopRequestFrames_ = 0;
	static constexpr int kEnemyHitStopFrames = 3;
	static constexpr int kEnemyKillBurstScale = 4;
	static constexpr int kEnemyHitBurstScale = 1;
	uint32_t scorePopupTexHandle_ = 0;
	std::unique_ptr<KamataEngine::Sprite> scorePopupDigitSprite_;

	struct ScorePopup {
		KamataEngine::Vector3 worldPos{0.0f, 0.0f, 0.0f};
		int value = 0;
		float life = 0.0f;
		float maxLife = 0.0f;
		float riseSpeed = 0.0f;
	};
	std::vector<ScorePopup> scorePopups_;

	// ========== ミサイルアフターバーナー ==========
	static constexpr float kMissileAfterburnerLife = 0.28f;
	static constexpr float kMissileAfterburnerStartScale = 0.13f;
	static constexpr float kMissileAfterburnerEndScale = 0.0f;
	static constexpr float kMissileAfterburnerSpeed = -3.8f;
	static constexpr float kMissileAfterburnerOffsetZ = -0.55f;
	static constexpr float kMissileAfterburnerRand = 0.2f;

	// ========== UI・スコア関連 ==========
	UIManager uiManager_;
	int clearScore_ = 0;

	static constexpr int kScorePerEnemy = 100;
	int requiredClearScore_ = 5000;
	bool isTutorialLevel_ = false;

	// ========== ポーズ ==========
	bool isPaused_ = false;
	bool isDebugUpdatePaused_ = false;
	std::unique_ptr<PauseMenu> pauseMenu_;
	std::unique_ptr<KamataEngine::Sprite> pauseTitleSprite_;
	uint32_t pauseTitleTexHandle_ = 0;
	static constexpr float kScreenWidth = 1280.0f;
	static constexpr float kScreenHeight = 720.0f;
	static constexpr float kPauseTitlePosX = 20.0f;
	static constexpr float kPauseTitlePosY = 16.0f;
	static constexpr float kPauseTitleScale = 0.7f;

	// ========== リトライ・タイトル遷移 ==========
	bool requestRetry_ = false;
	bool requestToTitle_ = false;

	// ========== クリアアニメーション ==========
	bool isClearAnimating_ = false;
	bool isBossTransitionAnimating_ = false;
	float clearAnimTimer_ = 0.0f;
	KamataEngine::Vector3 bossTransitionStartPosition_{};
	KamataEngine::Vector3 bossTransitionStartRotation_{};
	KamataEngine::Vector3 bossTransitionStartScale_{1.0f, 1.0f, 1.0f};

	static constexpr float kClearBoostSpeedZ = 1.0f;
	static constexpr float kClearBoostSpeedY = 5.0f;
	static constexpr float kClearRotateSpeedX = 0.5f;

	static constexpr float kClearShrinkStart = 0.5f;
	static constexpr float kClearShrinkSpeed = 1.0f;
	static constexpr float kClearAnimEndTime = 2.0f;

	static constexpr int kSpeedLineCount = 10;
	static constexpr float kCountDownStartDelay = 0.1f;
	static constexpr float kCountDownNumberDuration = 0.5f;
	static constexpr float kCountDownGoDuration = 0.4f;
	static constexpr float kCountDownScaleStart = 1.2f;
	static constexpr float kCountDownScaleEnd = 1.0f;
	static constexpr float kCountDownBackOvershoot = 1.7f;
	static constexpr float kCameraShakeMin = -1.0f;
	static constexpr float kCameraShakeMax = 1.0f;
	static constexpr float kCameraShakeDuration = 1.0f;
	static constexpr float kCameraInputScale = 40.0f;
	static constexpr float kBossStartCinematicDuration = 1.5f;

	// ========== シーン遷移 ==========
	SceneTransitionPhase transitionPhase_ = SceneTransitionPhase::None;
	float transitionTimer_ = 0.0f;
	float timeScale_ = 1.0f;

	int transitionScoreBonus_ = 0;
	bool failSecondExplosionDone_ = false;

	uint32_t seExplosionHandle_ = 0;

	// ========== ゲーム終了 ==========
	bool isEnd_ = false;
	GameResult result_ = GameResult::None;
};
