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

    auto* audio = Audio::GetInstance();
    countDown_.SetAudio(
        audio->LoadWave("./Resources/SE/CountBeep.wav"),
        audio->LoadWave("./Resources/SE/Start.wav")
    );

    // UI（HPバー／スコア）
    uiManager_.Initialize(player_);

    // 弾・敵
    bulletManager_.Initialize();
    enemyManager_.Initialize();
    enemyManager_.LoadEnemySCV("Resources/levels/stage1.json");

    // 開始
    countDown_.Start();

    isEnd_ = false;
    result_ = GameResult::None;
    clearScore_ = 0;

    isClearAnimating_ = false;
    clearAnimTimer_ = 0.0f;
}

void GameScene::Update() {
    const float dt = 1.0f / 60.0f;

    // 3カウント
    countDown_.Update(dt);

    // 天球
    skydome_->Update();

    // プレイヤー
    if (!countDown_.IsInputLocked()) {
        if (!(result_ == GameResult::Clear && isClearAnimating_)) {
            player_->Update();
        }
    }

    // damage
    if (player_->ConsumeTookDamageEvent()) {
        Vector3 pos = player_->GetWorldTranslation();

        // ランダムな飛び散り方向
        static std::mt19937 rng{ std::random_device{}() };
        std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

        for (int i = 0; i < 10; i++) {
            Vector3 vel = {
                dist(rng) * 2.5f,
                dist(rng) * 2.5f,
                dist(rng) * -2.0f
            };

            auto p = std::make_unique<DamageParticle>();
            p->Initialize(
                damageParticleModel_,
                pos,
                vel,
                0.60f,   // life
                0.22f,   // start
                0.0f     // end
            );
            damageParticles_.push_back(std::move(p));
        }
    }

    // 弾（入力＋更新）
    if (result_ == GameResult::None) {
        bulletManager_.Update(input_, player_, countDown_);
    }

    // 敵スポーン & 更新
    if (!countDown_.IsInputLocked() && result_ == GameResult::None) {
        const Vector3 playerPos = player_->GetWorldTranslation();
        enemyManager_.Update(dt, playerPos);

        // 当たり判定
        CollisionManager::ResolvePlayerEnemyCollisions(
            player_, enemyManager_.GetEnemies(), countDown_);

        CollisionManager::ResolveBulletEnemyCollisions(
            bulletManager_.GetBullets(), enemyManager_.GetEnemies(), countDown_);

        // ======= 死亡した敵のスコア加算 =======
        {
            auto& enemies = enemyManager_.GetEnemies();

            int deadCount = 0;
            for (auto& e : enemies) {
                if (auto* s = dynamic_cast<SeekerEnemy*>(e.get())) {
                    if (s->IsDead()) {
                        deadCount++;
                    }
                }
            }

            // 敵1体につき 100 点（数値は好みで）
            if (deadCount > 0) {
                uiManager_.GetScore()->Add(deadCount * 100);
            }
        }

        // 死んだ敵の削除
        enemyManager_.RemoveDeadEnemies();
    }

    // UI更新（HPバー、スコア）
    uiManager_.Update();

    // ===== ダメージパーティクル更新 =====
    for (auto& p : damageParticles_) {
        p->Update(dt);
    }
    damageParticles_.erase(
        std::remove_if(
            damageParticles_.begin(), damageParticles_.end(),
            [](const std::unique_ptr<DamageParticle>& p) { return p->IsFinished(); }),
        damageParticles_.end()
    );

    // ===== プレイヤーエンジン煙パーティクル =====
    if (!countDown_.IsInputLocked() &&
        (result_ == GameResult::None || (result_ == GameResult::Clear && isClearAnimating_))) {
        smokeEmitTimer_ += dt;

        // ★ クリア演出中だけパラメータを変更して「ドバーッ」と出す
        if (result_ == GameResult::Clear && isClearAnimating_) {
            emitInterval = 0.01f;   // 間隔をギュッと詰める（1秒で100回判定）
            lifeTime = 0.30f;   // 少し長めに残す
            startScale = 0.20f;   // ちょっと大きめ
            burstCount = 3;       // 1回の判定で3個出す
            baseZSpeed = -1.6f;   // 後ろへ強く吹き出す
        }

        // 
        while (smokeEmitTimer_ >= emitInterval) {

            // これないと止まる
            smokeEmitTimer_ -= emitInterval;

            // プレイヤーのワールド座標（親子付け込み）
            Vector3 playerPos = player_->GetWorldTranslation();

            // プレイヤーより少し下 & 後ろに出す
            Vector3 spawnPos = {
                playerPos.x,
                playerPos.y - 0.3f,
                playerPos.z - 1.5f
            };

            // ランダムな広がり
            static std::mt19937 rng{ std::random_device{}() };
            std::uniform_real_distribution<float> distX(-0.05f, 0.05f);
            std::uniform_real_distribution<float> distY(-0.05f, 0.05f);

            Vector3 vel = {
                distX(rng),          // 横にフワフワ
                distY(rng),          // 上下にフワフワ
                -0.6f                // 後ろ(Z-)へ流れる
            };

            // 0.6秒で消える、スケール 0.18 → 0.0 に縮む
            auto smoke = std::make_unique<Smoke>();
            smoke->Initialize(
                smokeModel_,
                spawnPos,
                vel,
                0.2f,   // lifeTime
                0.18f,  // startScale
                0.0f    // endScale
            );

            // ムーブしてvectorに追加
            engineSmokes_.push_back(std::move(smoke));
        }

        // 既存パーティクルの更新
        for (auto& s : engineSmokes_) {
            s->Update(dt);
        }

        // 寿命が尽きたものを削除
        engineSmokes_.erase(
            std::remove_if(
                engineSmokes_.begin(), engineSmokes_.end(),
                [](const std::unique_ptr<Smoke>& p) {
                    return p->IsFinished();
                }
            ),
            engineSmokes_.end()
        );
    }

    // レールカメラ更新
    if (isRailCameraActive_) {
        railCamera_->Update();

        camera_.matView = railCamera_->GetCamera()->matView;
        camera_.matProjection = railCamera_->GetCamera()->matProjection;
        camera_.TransferMatrix();
    }
    else {
        camera_.UpdateMatrix();
    }

    // スピードライン更新
    if (isRailCameraActive_) {
        const auto& camWT = railCamera_->GetWorldTransform();
        speedLine_.Update(dt, camWT.translation_);
    }

    // ===== クリア/失敗 判定 =====
    if (result_ == GameResult::None) {
        // 1. 失敗判定（プレイヤー爆発が終わった）
        if (player_->IsExplosionFinished()) {
            result_ = GameResult::Fail;
            isEnd_ = true;
        }
        // 2. クリア判定（スコア条件）
        else if (uiManager_.GetScore()->GetScore() >= 1000) {
            result_ = GameResult::Clear;
            clearScore_ = uiManager_.GetScore()->GetScore();

            // ★ ここでクリア演出スタート
            isClearAnimating_ = true;
            clearAnimTimer_ = 0.0f;
            // ※ isEnd_ はまだ立てない
        }
    }

    // ===== クリア演出（自機ブースト） =====
    if (result_ == GameResult::Clear && isClearAnimating_) {
        clearAnimTimer_ += dt;

        // ★ 自機のワールド変換を直接操作
        auto& wt = player_->GetWorldTransform();

        // 前方に加速（値はあとで調整してOK）
        float boostSpeed = 1.0f;     // 1秒で50ユニット進むイメージ
        wt.translation_.z += boostSpeed * dt;

        // ちょっと上に上がる＆少し機体を傾けると「飛び去る感」が出る
        wt.translation_.y += 5.0f * dt;
        wt.rotation_.x -= 0.5f * dt;  // 手前に傾ける or お好みで

        // 一定時間後に縮小していく
        if (clearAnimTimer_ > 0.5f) {
            float shrinkT = clearAnimTimer_ - 0.5f; // 0.0 からスタート
            float scale = 1.0f - shrinkT * 1.0f;    // 1秒で0まで
            if (scale < 0.0f) {
                scale = 0.0f;
            }
            wt.scale_ = { scale, scale, scale };
        }

        wt.UpdateMatrix();

        // 1.5秒くらい経ったらシーン終了
        if (clearAnimTimer_ >= 2.0f) {
            isEnd_ = true;
            isClearAnimating_ = false;
        }
    }
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
    if (!countDown_.IsInputLocked()&& result_ == GameResult::None) {
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

IScene* GameScene::NextScene() const {
    // 結果に応じて遷移先を切り替える
    if (result_ == GameResult::Clear) {
        return new ClearScene(clearScore_);  // クリア時
    }
    else {
        return new FinishScene();  // 失敗時（Fail or None）
    }
}
