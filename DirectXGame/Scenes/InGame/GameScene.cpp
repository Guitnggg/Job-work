#include "GameScene.h"

#include "Scenes/Finish/FinishScene.h"
#include "Scenes/Clear/ClearScene.h"

using namespace KamataEngine;

GameScene::GameScene() {}

GameScene::~GameScene() {
    delete railCamera_;
    delete skydome_;
    delete player_;

    delete worldTransform_;
    delete model_;

    delete smokeModel_;
}

void GameScene::Initialize() {
    // 各初期化処理
    dxCommon_ = DirectXCommon::GetInstance();
    input_ = Input::GetInstance();
    audio_ = Audio::GetInstance();

    worldTransform_ = new WorldTransform();
    worldTransform_->Initialize();
    camera_.Initialize();

    model_ = Model::Create();

    // レールカメラ
    railCamera_ = new RailCamera();
    railCamera_->Initialize();

    // 天球
    skydome_ = new Skydome();
    skydome_->Initialize(&camera_);

    // プレイヤー
    player_ = new Player();
    player_->Initialize(&camera_);
    player_->SetParent(&railCamera_->GetWorldTransform());

    // スピードライン初期化
    speedLine_.Initialize(&camera_, 10);

    // ダメージ演出
    damageParticleModel_ = Model::Create();
    damageParticles_.clear();

    // エンジンスモーク初期化
    smokeModel_ = Model::Create();
    engineSmokes_.clear();
    smokeEmitTimer_ = 0.0f;

    // 通常時スモーク
    normalSmokeParams_ = {
        0.08f,  // emitInterval
        0.15f,  // lifeTime
        0.14f,  // startScale
        1,      // burstCount
        -0.8f   // baseZSpeed
    };

    // クリア演出（ブースト）
    boostSmokeParams_ = {
        0.01f,  // emitInterval
        0.30f,  // lifeTime
        0.20f,  // startScale
        3,      // burstCount
        -1.6f   // baseZSpeed
    };

    // カウントダウン
    countDown_.InitializeFromPaths(
        "./Resources/InGame/3.png",
        "./Resources/InGame/2.png",
        "./Resources/InGame/1.png",
        "./Resources/InGame/GO.png"
    );
    countDown_.SetTimings(0.1f, 0.5f, 0.4f);
    countDown_.SetScaleRange(1.2f, 1.0f);
    countDown_.SetBackOvershoot(1.7f);

    countDown_.SetAudio(
        audio_->LoadWave("./Resources/SE/CountBeep.wav"),
        audio_->LoadWave("./Resources/SE/Start.wav")
    );

    // UI（HPバー／スコア）
    uiManager_.Initialize(player_);

    // 弾・敵
    bulletManager_.Initialize();
    enemyManager_.Initialize();
    enemyManager_.LoadEnemyScv("Resources/levels/stage1.json");

    // 開始
    countDown_.Start();

    isEnd_ = false;
    result_ = GameResult::None;
    clearScore_ = 0;

    isClearAnimating_ = false;
    clearAnimTimer_ = 0.0f;
}

void GameScene::Update() {
    const float dt = kFixedDeltaTime_;

    CountDownUpdate(dt);
    BackgroundUpdate();
    PlayerUpdate();
    SpawnDamageParticles();

    BattleUpdate(dt);

    UIUpdate();
    DamageParticleUpdate(dt);
    EngineSmokesUpdate(dt);

    CameraUpdate();
    SpeedLineUpdate(dt);

    JudgeResultAndStartClear();
    ClearAnimationUpdate(dt);
}

void GameScene::Draw() {
    ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

#pragma region 背景スプライト描画
    Sprite::PreDraw(commandList);

    // 背景スプライトの描画があればここに

    Sprite::PostDraw();
    dxCommon_->ClearDepthBuffer();
#pragma endregion 

#pragma region 3Dオブジェクト描画
    Model::PreDraw();

    // 天球
    skydome_->Draw();

    // スピードライン
    if (!countDown_.IsInputLocked() && result_ == GameResult::None) {
        speedLine_.Draw();
    }

    // エンジンスモーク
    if (smokeModel_) {
        for (auto& s : engineSmokes_) {
            s->Draw(&camera_);
        }
    }

    // ダメージパーティクル
    if (damageParticleModel_) {
        for (auto& p : damageParticles_) {
            p->Draw(&camera_);
        }
    }

    // ゲーム中
    if (result_ == GameResult::None) {
        enemyManager_.Draw(&camera_);   // 敵
        bulletManager_.Draw(&camera_);  // 弾
    }

    // プレイヤー
    if (!countDown_.IsInputLocked()) {
        player_->Draw(&camera_);
    }

    Model::PostDraw();
#pragma endregion

#pragma region 前景スプライト描画
    Sprite::PreDraw(commandList);

    countDown_.Draw();
    uiManager_.Draw();

    Sprite::PostDraw();
#pragma endregion
}

void GameScene::CountDownUpdate(float dt) { countDown_.Update(dt); }

void GameScene::BackgroundUpdate() { skydome_->Update(); }

void GameScene::PlayerUpdate() {
    // クリア演出中は更新しない
    if (!countDown_.IsInputLocked()) {
        if (!(result_ == GameResult::Clear && isClearAnimating_)) {
            player_->Update();
        }
    }
}

void GameScene::SpawnDamageParticles() {
    // 
    if (!player_->ConsumeTookDamageEvent()) {
        return;
    }

    Vector3 pos = player_->GetWorldTranslation();
    static std::mt19937 rng{ (std::random_device{}()) };
    std::uniform_real_distribution<float> dist(-1, 1);

    // 
    for (int i = 0; i < kDamageParticleCount_; i++) {
        Vector3 vel = {
            dist(rng) * kDamageParticleSpeedXY_,
            dist(rng) * kDamageParticleSpeedXY_,
            dist(rng) * kDamageParticleSpeedZ_
        };
        auto p = std::make_unique<DamageParticle>();

        p->Initialize(
            damageParticleModel_,
            pos,
            vel,
            kDamageParticleLife_,
            kDamageParticleStartScale_,
            kDamageParticleEndScale_
        );
        damageParticles_.push_back(std::move(p));
    }
}

void GameScene::BattleUpdate(float dt) {
    // 弾（戦闘中のみ）
    if (result_ == GameResult::None) {
        bulletManager_.Update(input_, player_, countDown_);
    }

    // 敵・衝突・スコア（入力ロック解除＆戦闘中のみ）
    if (!countDown_.IsInputLocked() && result_ == GameResult::None) {
        const Vector3 playerPos = player_->GetWorldTranslation();
        enemyManager_.Update(dt, playerPos);

        // 衝突判定
        CollisionManager::ResolvePlayerEnemyCollisions(player_, enemyManager_.GetEnemies(), countDown_);
        CollisionManager::ResolveBulletEnemyCollisions(bulletManager_.GetBullets(), enemyManager_.GetEnemies(), countDown_);

        // スコア加算（倒れた数 × 100）
        int deadCount = 0;
        for (auto& e : enemyManager_.GetEnemies()) {
            if (auto* s = dynamic_cast<SeekerEnemy*>(e.get())) {
                if (s->IsDead()) deadCount++;
            }
        }
        if (deadCount > 0) {
            uiManager_.GetScore()->Add(deadCount * kScorePerEnemy_);
        }

        // 死亡した敵を削除
        enemyManager_.RemoveDeadEnemies();
    }
}

void GameScene::UIUpdate()
{
    uiManager_.Update();
}

void GameScene::DamageParticleUpdate(float dt)
{
    // 
    for (auto& p : damageParticles_) {
        p->Update(dt);
    }

    // 
    damageParticles_.erase(
        std::remove_if(damageParticles_.begin(), damageParticles_.end(),
            [](const std::unique_ptr<DamageParticle>& p) { return p->IsFinished(); }),
        damageParticles_.end()
    );
}

void GameScene::EngineSmokesUpdate(float dt)
{
    if (!countDown_.IsInputLocked());

    const bool canEmit = (result_ == GameResult::None) ||
        (result_ == GameResult::Clear && isClearAnimating_);

    if (!canEmit) { return; }

    smokeEmitTimer_ += dt;

    // 使用するパラメータを選択
    const SmokeParams& params =
        (result_ == GameResult::Clear && isClearAnimating_) ? boostSmokeParams_ : normalSmokeParams_;

    static std::mt19937 rng{ std::random_device{}() };
    std::uniform_real_distribution<float> xyDist(-kSmokeRandXY_, kSmokeRandXY_);
    std::uniform_real_distribution<float> zDist(-kSmokeRandZ_, kSmokeRandZ_);

    // 
    while (smokeEmitTimer_ >= params.emitInterval) {
        smokeEmitTimer_ -= params.emitInterval;

        const Vector3 playerPos = player_->GetWorldTranslation();
        const Vector3 spawnPos = {
            playerPos.x,
            playerPos.y + kSmokeOffsetY_,
            playerPos.z + kSmokeOffsetZ_
        };

        // 
        for (int i = 0; i < params.burstCount; ++i) {
            Vector3 vel{
                xyDist(rng),
                xyDist(rng),
                params.baseZSpeed + zDist(rng)
            };

            auto smoke = std::make_unique<Smoke>();
            smoke->Initialize(
                smokeModel_,
                spawnPos,
                vel,
                params.lifeTime,
                params.startScale,
                0.0f
            );
            engineSmokes_.push_back(std::move(smoke));
        }
    }

    for (auto& s : engineSmokes_) {
        s->Update(dt);
    }

    engineSmokes_.erase(
        std::remove_if(
            engineSmokes_.begin(), engineSmokes_.end(),
            [](const std::unique_ptr<Smoke>& p) {
                return p->IsFinished();
            }),
        engineSmokes_.end()
    );
}

void GameScene::CameraUpdate()
{
    // 
    if (isRailCameraActive_) {
        railCamera_->Update();
        camera_.matView = railCamera_->GetCamera()->matView;
        camera_.matProjection = railCamera_->GetCamera()->matProjection;
        camera_.TransferMatrix();
    }
    // 
    else {
        camera_.UpdateMatrix();
    }
}

void GameScene::SpeedLineUpdate(float dt)
{
    // 
    if (isRailCameraActive_) {
        // 
        speedLine_.Update(dt, railCamera_->GetWorldTransform().translation_);
    }
}

void GameScene::JudgeResultAndStartClear()
{
    if (result_ == GameResult::None) {
        if (player_->IsExplosionFinished()) {
            result_ = GameResult::Fail;
            isEnd_ = true;
        }
        else if (uiManager_.GetScore()->GetScore() >= kClearScore_) {
            result_ = GameResult::Clear;
            clearScore_ = uiManager_.GetScore()->GetScore();
            isClearAnimating_ = true;
            clearAnimTimer_ = 0.0f;
        }
    }
}

void GameScene::ClearAnimationUpdate(float dt)
{
    // 
    if (result_ == GameResult::Clear && isClearAnimating_) {
        clearAnimTimer_ += dt;
        auto& wt = player_->GetWorldTransform();

        // 
        wt.translation_.z += kClearBoostSpeedZ_ * dt;
        wt.translation_.y += kClearBoostSpeedY_ * dt;
        wt.rotation_.x -= kClearRotateSpeedX_ * dt;

        // 
        if (clearAnimTimer_ > kClearShrinkStart_) {
            float shrink = clearAnimTimer_ - kClearShrinkStart_;
            float scale = (std::max)(0.0f, 1.0f - shrink * kClearShrinkSpeed_);
            wt.scale_ = { scale, scale, scale };
        }

        // 
        wt.UpdateMatrix();

        // 
        if (clearAnimTimer_ >= kClearAnimEndTime_) {
            isEnd_ = true;
            isClearAnimating_ = false;
        }
    }
}

IScene* GameScene::NextScene() const {
    // 結果に応じて遷移先を切り替える
    if (result_ == GameResult::Clear) {
        return new ClearScene(clearScore_);  // クリア時
    }
    else {
        return new FinishScene();  // 失敗時（Fail or None）
    }
}
