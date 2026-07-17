#include "GameScene.h"

#include <cmath>
#include <fstream>
#include <utility>

#include <json.hpp>

#ifdef USE_IMGUI
#include <imgui.h>
#endif

#include "GameSceneUpdateExecutor.h"
#include "Scenes/Clear/ClearScene.h"
#include "Scenes/Finish/FinishScene.h"
#include "Scenes/SceneHelper.h"
#include "Scenes/Title/TitleScene.h"

using namespace KamataEngine;
using json = nlohmann::json;

namespace {
    constexpr bool kEnableBossStage = false;

    struct LevelDefaults {
        const char* fileName;
        int clearScore;
        bool isTutorial;
        bool enablesBossStage;
    };

    constexpr LevelDefaults kLevelDefaults[] = {
        {"Tutorial.json", 1000, true, false},
        {"Easy.json", 5000, false, false},
        {"Normal.json", 10000, false, true},
        {"Hard.json", 15000, false, true},
    };

    const LevelDefaults& FindLevelDefaults(const std::string& path) {
        for (const LevelDefaults& defaults : kLevelDefaults) {
            if (path.find(defaults.fileName) != std::string::npos) {
                return defaults;
            }
        }
        return kLevelDefaults[2];
    }

    int LoadClearScoreFromLevelJson(const std::string& path, int fallbackScore) {
        std::ifstream ifs(path);
        if (!ifs) {
            return fallbackScore;
        }

        json root;
        ifs >> root;
        return root.value("clearScore", fallbackScore);
    }

    void ApplyPressedSpriteState(Sprite* sprite, const Vector2& basePosition, const Vector2& baseSize, bool isPressed) {
        if (!sprite) {
            return;
        }

        const Vector4 normalColor = { 1.0f, 1.0f, 1.0f, 0.88f };
        const Vector4 pressedColor = { 0.35f, 0.95f, 1.0f, 1.0f };
        const float scale = isPressed ? 0.92f : 1.0f;
        const float pressOffsetY = isPressed ? 4.0f : 0.0f;
        const Vector2 size = { baseSize.x * scale, baseSize.y * scale };
        const Vector2 position = { basePosition.x + (baseSize.x - size.x) * 0.5f, basePosition.y + (baseSize.y - size.y) * 0.5f + pressOffsetY };

        sprite->SetPosition(position);
        sprite->SetSize(size);
        sprite->SetColor(isPressed ? pressedColor : normalColor);
    }

    void DrawPopupNumber(Sprite* digitSprite, int value, const Vector2& center, float scale, float alpha) {
        if (!digitSprite) {
            return;
        }

        const int absValue = (std::max)(0, value);
        const std::string text = std::to_string(absValue);
        const Vector2 digitSize = { 24.0f * scale, 40.0f * scale };
        const float totalWidth = digitSize.x * static_cast<float>(text.size());
        float x = center.x - totalWidth * 0.5f;

        for (char c : text) {
            const int digit = static_cast<int>(c - '0');
            digitSprite->SetPosition({ x, center.y });
            digitSprite->SetSize(digitSize);
            digitSprite->SetTextureRect({ 32.0f * static_cast<float>(digit), 0.0f }, { 32.0f, 64.0f });
            digitSprite->SetColor({ 1.0f, 0.95f, 0.35f, alpha });
            digitSprite->Draw();
            x += digitSize.x;
        }
    }
} // namespace

GameScene::GameScene(std::string levelJsonPath) : levelJsonPath_(std::move(levelJsonPath)) {}

GameScene::~GameScene() = default;

void GameScene::Initialize() {
    // ===== 基本システム =====
    dxCommon_ = DirectXCommon::GetInstance();
    input_ = Input::GetInstance();
    audio_ = Audio::GetInstance();

    worldTransform_ = std::make_unique<WorldTransform>();
    worldTransform_->Initialize();
    camera_.Initialize();

    model_.reset(Model::Create());

    // =====レールカメラ =====
    railCamera_ = std::make_unique<RailCamera>();
    railCamera_->Initialize();

    // ===== 天球 =====
    skydome_ = std::make_unique<Skydome>();
    skydome_->Initialize(&camera_);

    // ===== プレイヤー =====
    player_ = std::make_unique<Player>();
    player_->Initialize(&camera_);
    player_->SetParent(&railCamera_->GetWorldTransform());

    previousPlayerPos_ = player_->GetWorldTranslation(); // プレイヤー位置情報

    // ===== 操作UI =====
    wasdTextureHandle_ = TextureManager::Load("./Resources/InGame/WASD.png");
    const Vector2 keySize = { kWasdKeySize, kWasdKeySize };
    const Vector2 keyUvSize = { 96.0f, 96.0f };
    wasdWSprite_ = SceneHelper::CreateSprite(wasdTextureHandle_, { kWasdBaseX + (keySize.x + kWasdSpacing), kWasdBaseY });
    wasdASprite_ = SceneHelper::CreateSprite(wasdTextureHandle_, { kWasdBaseX, kWasdBaseY + (keySize.y + kWasdSpacing) });
    wasdSSprite_ = SceneHelper::CreateSprite(wasdTextureHandle_, { kWasdBaseX + (keySize.x + kWasdSpacing), kWasdBaseY + (keySize.y + kWasdSpacing) });
    wasdDSprite_ = SceneHelper::CreateSprite(wasdTextureHandle_, { kWasdBaseX + (keySize.x + kWasdSpacing) * 2.0f, kWasdBaseY + (keySize.y + kWasdSpacing) });
    if (wasdWSprite_ && wasdASprite_ && wasdSSprite_ && wasdDSprite_) {
        wasdWSprite_->SetAnchorPoint({ 0.0f, 0.0f });
        wasdASprite_->SetAnchorPoint({ 0.0f, 0.0f });
        wasdSSprite_->SetAnchorPoint({ 0.0f, 0.0f });
        wasdDSprite_->SetAnchorPoint({ 0.0f, 0.0f });
        wasdWSprite_->SetSize(keySize);
        wasdASprite_->SetSize(keySize);
        wasdSSprite_->SetSize(keySize);
        wasdDSprite_->SetSize(keySize);
        wasdWSprite_->SetTextureRect({ 96.0f, 0.0f }, keyUvSize);
        wasdASprite_->SetTextureRect({ 0.0f, 96.0f }, keyUvSize);
        wasdSSprite_->SetTextureRect({ 96.0f, 96.0f }, keyUvSize);
        wasdDSprite_->SetTextureRect({ 192.0f, 96.0f }, keyUvSize);
    }

    mouseTextureHandle_ = TextureManager::Load("./Resources/InGame/Mouse.png");
    const Vector2 mouseSize = { kMouseSize, kMouseSize };
    const Vector2 mouseButtonUvSize = { 320.0f, 270.0f };
    const Vector2 mouseButtonSize = { kMouseSize * 0.5f, kMouseSize * 270.0f / 640.0f };
    mouseBaseSprite_ = SceneHelper::CreateSprite(mouseTextureHandle_, { kMouseBaseX, kMouseBaseY }, { 1.0f, 1.0f, 1.0f, 0.55f });
    mouseLeftSprite_ = SceneHelper::CreateSprite(mouseTextureHandle_, { kMouseBaseX, kMouseBaseY });
    mouseRightSprite_ = SceneHelper::CreateSprite(mouseTextureHandle_, { kMouseBaseX + mouseButtonSize.x, kMouseBaseY });
    if (mouseBaseSprite_ && mouseLeftSprite_ && mouseRightSprite_) {
        mouseBaseSprite_->SetAnchorPoint({ 0.0f, 0.0f });
        mouseLeftSprite_->SetAnchorPoint({ 0.0f, 0.0f });
        mouseRightSprite_->SetAnchorPoint({ 0.0f, 0.0f });
        mouseBaseSprite_->SetSize(mouseSize);
        mouseLeftSprite_->SetSize(mouseButtonSize);
        mouseRightSprite_->SetSize(mouseButtonSize);
        mouseBaseSprite_->SetTextureRect({ 0.0f, 0.0f }, { 640.0f, 640.0f });
        mouseLeftSprite_->SetTextureRect({ 0.0f, 0.0f }, mouseButtonUvSize);
        mouseRightSprite_->SetTextureRect({ 320.0f, 0.0f }, mouseButtonUvSize);
    }

    // ===== スピードライン初期化 =====
    speedLine_.Initialize(&camera_, kSpeedLineCount);

    // ===== エンジンスモーク初期化 =====
    engineSmokeEmitter_ = std::make_unique<GpuSmokeEmitter>();
    engineSmokeEmitter_->Initialize(256);
    damageSmokeEmitter_ = std::make_unique<GpuSmokeEmitter>();
    damageSmokeEmitter_->Initialize(768);
    missileAfterburnerEmitter_ = std::make_unique<GpuSmokeEmitter>();
    missileAfterburnerEmitter_->Initialize(512);
    smokeEmitTimer_ = 0.0f;
    prevEnemyHpMap_.clear();
    prevPlayerHp_ = player_->GetHP();

    // ===== 通常時スモーク =====
    normalSmokeParams_ = {
        0.08f, // emitInterval
        1.0f,  // lifeTime
        0.3f,  // startScale
        10,    // burstCount
        -3.0f  // baseZSpeed
    };

    // ===== クリア演出（ブースト） =====
    boostSmokeParams_ = {
        0.01f, // emitInterval
        0.30f, // lifeTime
        0.20f, // startScale
        3,     // burstCount
        -1.6f  // baseZSpeed
    };

    // ===== カウントダウン =====
    countDown_.InitializeFromPaths("./Resources/InGame/3.png", "./Resources/InGame/2.png", "./Resources/InGame/1.png", "./Resources/InGame/GO.png");
    countDown_.SetTimings(kCountDownStartDelay, kCountDownNumberDuration, kCountDownGoDuration);
    countDown_.SetScaleRange(kCountDownScaleStart, kCountDownScaleEnd);
    countDown_.SetBackOvershoot(kCountDownBackOvershoot);

    countDown_.SetAudio(audio_->LoadWave("./Resources/SE/CountBeep.wav"), audio_->LoadWave("./Resources/SE/Start.wav"));

    // ===== UI（HPバー／スコア） =====
    uiManager_.Initialize(player_.get());

    // ===== 弾・敵 =====
    bulletManager_.Initialize();
    enemyManager_.Initialize();
    bossManager_.Initialize();
    enemyManager_.LoadEnemyCsv(levelJsonPath_);
    const LevelDefaults& levelDefaults = FindLevelDefaults(levelJsonPath_);
    isTutorialLevel_ = levelDefaults.isTutorial;
    isBossStageEnabled_ = kEnableBossStage && levelDefaults.enablesBossStage;
    hasBossBattleStarted_ = false;

    // ===== 開始 =====
    requiredClearScore_ = LoadClearScoreFromLevelJson(levelJsonPath_, levelDefaults.clearScore);
    countDown_.Start();

    // 開始　３カウントから
    state_ = GameState::CountDown;

    // ===== 照準 =====
    reticleTexHandle_ = TextureManager::Load("./Resources/InGame/Reticle.png");
    lockOnTexHandle_ = TextureManager::Load("./Resources/InGame/Lockon.png");
    reticleSprite_ = SceneHelper::CreateSprite(reticleTexHandle_, reticlePos_);
    if (reticleSprite_) {
        reticleSprite_->SetAnchorPoint({ 0.5f, 0.5f });
        reticleSprite_->SetSize({ 32.0f, 32.0f });
    }

    // ===== Pause =====
    pauseMenu_ = std::make_unique<PauseMenu>();
    pauseMenu_->Initialize();
    pauseTitleTexHandle_ = TextureManager::Load("./Resources/InGame/Pause.png");
    pauseTitleSprite_ = SceneHelper::CreateSprite(pauseTitleTexHandle_, { kPauseTitlePosX, kPauseTitlePosY });
    pauseTitleSprite_->SetAnchorPoint({ 0.0f, 0.0f });
    const Vector2 pauseTitleSize = pauseTitleSprite_->GetSize();
    pauseTitleSprite_->SetSize({ pauseTitleSize.x * 0.7f, pauseTitleSize.y * 0.7f });
    seExplosionHandle_ = audio_->LoadWave("./Resources/SE/Explosion.wav");
    seEnemyHitHandle_ = audio_->LoadWave("./Resources/SE/shot.wav");
    seEnemyKillHandle_ = audio_->LoadWave("./Resources/SE/Explosion.wav");
    scorePopupTexHandle_ = TextureManager::Load("./Resources/InGame/number.png");
    scorePopupDigitSprite_ = SceneHelper::CreateSprite(scorePopupTexHandle_, { 0.0f, 0.0f });

    // ===== その他 =====
    isEnd_ = false;
    result_ = GameResult::None;
    clearScore_ = 0;

    isClearAnimating_ = false;
    isBossTransitionAnimating_ = false;
    clearAnimTimer_ = 0.0f;

    transitionPhase_ = SceneTransitionPhase::None;
    transitionTimer_ = 0.0f;
    timeScale_ = 1.0f;
    transitionScoreBonus_ = 0;
    failSecondExplosionDone_ = false;
    hitStopFrames_ = 0;
    hitStopRequestFrames_ = 0;
    scorePopups_.clear();
}

void GameScene::Update() {
    GameSceneUpdateExecutor::Update(*this);
}

void GameScene::Draw() {
    ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();
    const Camera* cam = railCamera_->GetCamera();

    SceneHelper::Begin3DDraw(dxCommon_, commandList);

#pragma region 3Dオブジェクト描画
    SceneHelper::DrawModelLayer([this, cam]() {

        // 空/スカイドームは常に描く（雰囲気）
        skydome_->Draw();

        // （任意）CountDown中もプレイヤーだけ見せたいなら描く
        if (state_ == GameState::Playing) {
            player_->Draw(cam);
        }

        // ゲーム中だけ描きたいものは Playing に寄せる
        if (state_ == GameState::Playing && (result_ == GameResult::None || transitionPhase_ == SceneTransitionPhase::FailCinematic)) {

            // スピードライン
            speedLine_.Draw();

            // 敵・弾・ボス
            enemyManager_.Draw(cam);
            bossManager_.Draw(cam);
            bulletManager_.Draw(cam);
        }

        // エンジンスモークは通常時とクリア演出中に描画
        const bool canDrawSmoke = (state_ == GameState::Playing) && ((result_ == GameResult::None) || (result_ == GameResult::Clear && isClearAnimating_));
        if (canDrawSmoke && engineSmokeEmitter_) {
            engineSmokeEmitter_->Draw(cam);
        }

        // GPU煙エフェクトは最後に描画（Model描画状態を壊さないため）
        if (state_ == GameState::Playing && result_ == GameResult::None) {
            if (damageSmokeEmitter_) {
                damageSmokeEmitter_->Draw(cam);
            }
            if (missileAfterburnerEmitter_) {
                missileAfterburnerEmitter_->Draw(cam);
            }
        }

        });
#pragma endregion

#pragma region 前景スプライト
    SceneHelper::DrawSpriteLayer(commandList, [this]() {

        // 3カウントは CountDown 中だけ
        if (state_ == GameState::CountDown) {
            countDown_.Draw();
        }

        // UI（スコア/HP等）は Playing 中だけ（結果画面で出したいなら条件追加）
        if (state_ == GameState::Playing) {
            uiManager_.Draw();
            bossManager_.DrawUI();

            if (reticleSprite_ && result_ == GameResult::None) {
                reticleSprite_->Draw();
            }

            // ロックオン演出
            for (auto& marker : lockOnMarkers_) {
                if (marker.sprite) {
                    marker.sprite->Draw();
                }
            }

            const Camera* drawCam = railCamera_ ? railCamera_->GetCamera() : &camera_;
            if (drawCam) {
                const Matrix4x4 viewProj = MyMath::Multiply(drawCam->matView, drawCam->matProjection);
                for (const auto& popup : scorePopups_) {
                    const Vector3 clip = MyMath::Transform(popup.worldPos, viewProj);
                    if (clip.z < 0.0f || clip.z > 1.0f) {
                        continue;
                    }
                    const Vector2 screenPos = { (clip.x * 0.5f + 0.5f) * kScreenWidth, (-clip.y * 0.5f + 0.5f) * kScreenHeight };
                    const float t = popup.maxLife > 0.0f ? (1.0f - popup.life / popup.maxLife) : 1.0f;
                    const float scale = 0.85f + 0.35f * (1.0f - t);
                    const float alpha = (std::max)(0.0f, 1.0f - t);
                    DrawPopupNumber(scorePopupDigitSprite_.get(), popup.value, screenPos, scale, alpha);
                }
            }
        }

        // Pauseメニュー
        if (isPaused_) {
            pauseMenu_->Draw();
        }

        // 操作UI
        if (!isPaused_) {
            if (wasdWSprite_ && wasdASprite_ && wasdSSprite_ && wasdDSprite_) {
                const bool isWPressed = input_->PushKey(DIK_W);
                const bool isAPressed = input_->PushKey(DIK_A);
                const bool isSPressed = input_->PushKey(DIK_S);
                const bool isDPressed = input_->PushKey(DIK_D);
                const Vector2 keySize = { kWasdKeySize, kWasdKeySize };

                ApplyPressedSpriteState(wasdWSprite_.get(), { kWasdBaseX + (keySize.x + kWasdSpacing), kWasdBaseY }, keySize, isWPressed);
                ApplyPressedSpriteState(wasdASprite_.get(), { kWasdBaseX, kWasdBaseY + (keySize.y + kWasdSpacing) }, keySize, isAPressed);
                ApplyPressedSpriteState(wasdSSprite_.get(), { kWasdBaseX + (keySize.x + kWasdSpacing), kWasdBaseY + (keySize.y + kWasdSpacing) }, keySize, isSPressed);
                ApplyPressedSpriteState(wasdDSprite_.get(), { kWasdBaseX + (keySize.x + kWasdSpacing) * 2.0f, kWasdBaseY + (keySize.y + kWasdSpacing) }, keySize, isDPressed);

                wasdWSprite_->Draw();
                wasdASprite_->Draw();
                wasdSSprite_->Draw();
                wasdDSprite_->Draw();
            }

            if (mouseBaseSprite_ && mouseLeftSprite_ && mouseRightSprite_) {
                const bool isLeftPressed = input_->IsPressMouse(0);
                const bool isRightPressed = input_->IsPressMouse(1);
                const Vector2 mouseButtonSize = { kMouseSize * 0.5f, kMouseSize * 270.0f / 640.0f };

                ApplyPressedSpriteState(mouseLeftSprite_.get(), { kMouseBaseX, kMouseBaseY }, mouseButtonSize, isLeftPressed);
                ApplyPressedSpriteState(mouseRightSprite_.get(), { kMouseBaseX + mouseButtonSize.x, kMouseBaseY }, mouseButtonSize, isRightPressed);

                mouseBaseSprite_->Draw();
                mouseLeftSprite_->Draw();
                mouseRightSprite_->Draw();
            }
        }

        // Pauseガイド
        if (!isPaused_ && state_ == GameState::Playing) {
            if (pauseTitleSprite_) {
                pauseTitleSprite_->Draw();
            }
        }

        });
#pragma endregion
}

std::unique_ptr<IScene> GameScene::NextScene() const {
    // Pause 由来の遷移を最優先
    if (requestRetry_) {
        return std::make_unique<GameScene>(levelJsonPath_);
    }

    if (requestToTitle_) {
        return std::make_unique<TitleScene>();
    }

    // 結果に応じて遷移先を切り替える
    if (result_ == GameResult::Clear) {
        return std::make_unique<ClearScene>(clearScore_); // クリア時
    }
    else {
        return std::make_unique<FinishScene>(); // 失敗時（Fail or None）
    }
}
