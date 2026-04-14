#include "GameScene.h"

#include <cmath>
#include <utility>

#ifdef USE_IMGUI
#include <imgui.h>
#endif

#include "Scenes/Clear/ClearScene.h"
#include "Scenes/Finish/FinishScene.h"
#include "Scenes/Title/TitleScene.h"
#include "GameSceneUpdateExecutor.h"

using namespace KamataEngine;

GameScene::GameScene(std::string levelJsonPath) : levelJsonPath_(std::move(levelJsonPath)) {}

GameScene::~GameScene() {
    delete railCamera_;
    delete skydome_;
    delete player_;

    delete worldTransform_;
    delete model_;
}

void GameScene::Initialize() {
    // ===== 基本 =====
    dxCommon_ = DirectXCommon::GetInstance();
    input_ = Input::GetInstance();
    audio_ = Audio::GetInstance();

    worldTransform_ = new WorldTransform();
    worldTransform_->Initialize();
    camera_.Initialize();

    model_ = Model::Create();

    // =====レールカメラ =====
    railCamera_ = new RailCamera();
    railCamera_->Initialize();

    // ===== 天球 =====
    skydome_ = new Skydome();
    skydome_->Initialize(&camera_);

    // ===== プレイヤー =====
    player_ = new Player();
    player_->Initialize(&camera_);
    player_->SetParent(&railCamera_->GetWorldTransform());

    previousPlayerPos_ = player_->GetWorldTranslation(); // プレイヤー位置情報

    // ===== スピードライン初期化 =====
    speedLine_.Initialize(&camera_, 10);

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
    countDown_.SetTimings(0.1f, 0.5f, 0.4f);
    countDown_.SetScaleRange(1.2f, 1.0f);
    countDown_.SetBackOvershoot(1.7f);

    countDown_.SetAudio(audio_->LoadWave("./Resources/SE/CountBeep.wav"), audio_->LoadWave("./Resources/SE/Start.wav"));

    // ===== UI（HPバー／スコア） =====
    uiManager_.Initialize(player_);

    // ===== 弾・敵 =====
    bulletManager_.Initialize();
    enemyManager_.Initialize();
    enemyManager_.LoadEnemyCsv(levelJsonPath_);
    if (levelJsonPath_.find("Tutorial.json") != std::string::npos) {
        kClearScore_ = 1200;
    }
    else if (levelJsonPath_.find("Easy.json") != std::string::npos) {
        kClearScore_ = 1200;
    }
    else if (levelJsonPath_.find("Hard.json") != std::string::npos) {
        kClearScore_ = 2200;
    }
    else {
        kClearScore_ = 5000; // Normalの既定値
    }

    // ===== 開始 =====
    countDown_.Start();

    // 開始　３カウントから
    state_ = GameState::CountDown;

    // ===== 照準 =====
    reticleTexHandle_ = TextureManager::Load("./Resources/InGame/Reticle.png");
    lockOnTexHandle_ = TextureManager::Load("./Resources/InGame/Lockon.png"); // ロックオン演出
    reticleSprite_.reset(Sprite::Create(reticleTexHandle_, reticlePos_));
    if (reticleSprite_) {
        reticleSprite_->SetAnchorPoint({ 0.5f, 0.5f });
        reticleSprite_->SetSize({ 32.0f, 32.0f });
    }

    // ===== Pause =====
    pauseMenu_ = std::make_unique<PauseMenu>();
    pauseMenu_->Initialize();
    pauseTitleTexHandle_ = TextureManager::Load("./Resources/InGame/Pause.png");
    pauseTitleSprite_.reset(Sprite::Create(pauseTitleTexHandle_, { kPauseTitlePosX_, kPauseTitlePosY_ }));
    pauseTitleSprite_->SetAnchorPoint({ 0.0f, 0.0f });
    const Vector2 pauseTitleSize = pauseTitleSprite_->GetSize();
    pauseTitleSprite_->SetSize({ pauseTitleSize.x * 0.7f, pauseTitleSize.y * 0.7f });
    seExplosionHandle_ = audio_->LoadWave("./Resources/SE/Explosion.wav");

    // ===== その他 =====
    isEnd_ = false;
    result_ = GameResult::None;
    clearScore_ = 0;

    isClearAnimating_ = false;
    clearAnimTimer_ = 0.0f;

    transitionPhase_ = SceneTransitionPhase::None;
    transitionTimer_ = 0.0f;
    timeScale_ = 1.0f;
    transitionScoreBonus_ = 0;
    failSecondExplosionDone_ = false;
}

void GameScene::Update() {
    // 
    GameSceneUpdateExecutor::Update(*this);
}

void GameScene::Draw() {
    ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();
    const Camera* cam = railCamera_->GetCamera();

#pragma region 背景スプライト
    Sprite::PreDraw(commandList);
    // 背景スプライトの描画があればここ

    Sprite::PostDraw();
    dxCommon_->ClearDepthBuffer();
#pragma endregion

#pragma region 3Dオブジェクト描画
    Model::PreDraw();

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

        // 敵・弾
        enemyManager_.Draw(cam);
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

    Model::PostDraw();
#pragma endregion

#pragma region 前景スプライト
    Sprite::PreDraw(commandList);

    // 3カウントは CountDown 中だけ
    if (state_ == GameState::CountDown) {
        countDown_.Draw();
    }

    // UI（スコア/HP等）は Playing 中だけ（結果画面で出したいなら条件追加）
    if (state_ == GameState::Playing) {
        uiManager_.Draw();

        if (reticleSprite_ && result_ == GameResult::None) {
            reticleSprite_->Draw();
        }

        // ロックオン演出
        for (auto& marker : lockOnMarkers_) {
            if (marker.sprite) {
                marker.sprite->Draw();
            }
        }
    }

    // Pauseメニュー
    if (isPaused_) {
        pauseMenu_->Draw();
    }

    // Pauseガイド
    if (!isPaused_ && state_ == GameState::Playing) {
        if (pauseTitleSprite_) {
            pauseTitleSprite_->Draw();
        }
    }

    Sprite::PostDraw();
#pragma endregion
}

void GameScene::DrawImGui() {
#ifdef USE_IMGUI
    ImGui::Begin("GameScene InGame");

    ImGui::Checkbox("Pause Game Progress", &isDebugUpdatePaused_);
    ImGui::Separator();

    if (ImGui::BeginTabBar("GameSceneTabs")) {
        if (ImGui::BeginTabItem("State")) {
            ImGui::Text("GameState: %s", state_ == GameState::Playing ? "Playing" : "CountDown");
            ImGui::Text("Result: %s", result_ == GameResult::Clear ? "Clear" : result_ == GameResult::Fail ? "Fail" : "None");
            ImGui::Text("PauseMenu: %s", isPaused_ ? "Open" : "Closed");
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Score")) {
            ImGui::DragInt("Score Per Enemy", &kScorePerEnemy_, 1.0f, 0, 10000);
            ImGui::DragInt("Clear Score", &kClearScore_, 10.0f, 0, 999999);
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Damage")) {
            ImGui::DragInt("GPU Burst", &kDamageGpuBurst_, 1.0f, 1, 200);
            ImGui::DragFloat("GPU Speed", &kDamageGpuSpeed_, 0.01f, 0.0f, 40.0f);
            ImGui::DragFloat("GPU Life", &kDamageGpuLife_, 0.01f, 0.01f, 10.0f);
            ImGui::DragFloat("GPU Start Scale", &kDamageGpuStartScale_, 0.01f, 0.0f, 5.0f);
            ImGui::DragFloat("GPU End Scale", &kDamageGpuEndScale_, 0.01f, 0.0f, 5.0f);
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Smoke")) {
            ImGui::Text("Normal");
            ImGui::DragFloat("Normal Emit Interval", &normalSmokeParams_.emitInterval, 0.001f, 0.001f, 1.0f);
            ImGui::DragFloat("Normal Life", &normalSmokeParams_.lifeTime, 0.01f, 0.01f, 10.0f);
            ImGui::DragFloat("Normal Start Scale", &normalSmokeParams_.startScale, 0.01f, 0.0f, 5.0f);
            ImGui::DragInt("Normal Burst", &normalSmokeParams_.burstCount, 1.0f, 1, 100);
            ImGui::DragFloat("Normal Base Z Speed", &normalSmokeParams_.baseZSpeed, 0.01f, -20.0f, 20.0f);

            ImGui::Separator();
            ImGui::Text("Boost");
            ImGui::DragFloat("Boost Emit Interval", &boostSmokeParams_.emitInterval, 0.001f, 0.001f, 1.0f);
            ImGui::DragFloat("Boost Life", &boostSmokeParams_.lifeTime, 0.01f, 0.01f, 10.0f);
            ImGui::DragFloat("Boost Start Scale", &boostSmokeParams_.startScale, 0.01f, 0.0f, 5.0f);
            ImGui::DragInt("Boost Burst", &boostSmokeParams_.burstCount, 1.0f, 1, 100);
            ImGui::DragFloat("Boost Base Z Speed", &boostSmokeParams_.baseZSpeed, 0.01f, -20.0f, 20.0f);
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Clear")) {
            ImGui::DragFloat("Clear Boost Z", &kClearBoostSpeedZ_, 0.01f, -20.0f, 20.0f);
            ImGui::DragFloat("Clear Boost Y", &kClearBoostSpeedY_, 0.01f, -20.0f, 20.0f);
            ImGui::DragFloat("Clear Rotate Speed X", &kClearRotateSpeedX_, 0.01f, -20.0f, 20.0f);
            ImGui::DragFloat("Clear Shrink Start", &kClearShrinkStart_, 0.01f, 0.0f, 10.0f);
            ImGui::DragFloat("Clear Shrink Speed", &kClearShrinkSpeed_, 0.01f, 0.0f, 20.0f);
            ImGui::DragFloat("Clear End Time", &kClearAnimEndTime_, 0.01f, 0.1f, 20.0f);
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
#endif
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
