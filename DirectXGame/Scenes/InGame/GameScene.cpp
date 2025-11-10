#include "GameScene.h"

#include "FinishScene.h"

using namespace KamataEngine;

GameScene::GameScene() {}

GameScene::~GameScene() {
    delete railCamera_;
    delete skydome_;
    delete player_;

    delete worldTransform_;
    delete model_;
}

void GameScene::Initialize() {
    // 各初期化処理
    dxCommon_ = DirectXCommon::GetInstance();  // DirectX
    input_ = Input::GetInstance();             // 入力
    audio_ = Audio::GetInstance();             // サウンド

    worldTransform_ = new WorldTransform();
    worldTransform_->Initialize();             // ワールド変換データ
    camera_.Initialize();                      // カメラ（ビューポート）

    // モデルの生成
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

    // ========== 3カウント用 ==========
    countDown_.InitializeFromPaths(
        "./Resources/InGame/3.png",
        "./Resources/InGame/2.png",
        "./Resources/InGame/1.png",
        "./Resources/InGame/GO.png"
    );

    // 調整
    countDown_.SetTimings(0.1f, 0.5f, 0.4f);
    countDown_.SetScaleRange(1.2f, 1.0f);
    countDown_.SetBackOvershoot(1.7f);

    // SE
    auto* audio = KamataEngine::Audio::GetInstance();
    countDown_.SetAudio(
        audio->LoadWave("./Resources/SE/CountBeep.wav"),
        audio->LoadWave("./Resources/SE/Start.wav")
    );

    // ========== 敵スポーン用 ==========
    enemies_.clear();
    enemySpawnTimer_ = 0.0f;

    // 開始
    countDown_.Start();
}

void GameScene::Update() {
    // 天球更新
    skydome_->Update();

    // プレイヤー更新
    if (!countDown_.IsInputLocked()) {
        player_->Update();
    }

    // ========== レールカメラ更新 ==========
    if (isRailCameraActive_) {
        railCamera_->Update();

        camera_.matView = railCamera_->GetCamera()->matView;
        camera_.matProjection = railCamera_->GetCamera()->matProjection;
        camera_.TransferMatrix();
    }
    else {
        camera_.UpdateMatrix();
    }

    // ========== 3カウント制御 ==========
    const float dt = 1.0f / 60.0f;
    countDown_.Update(dt);

    // ========== 敵スポーン制御 ==========
    if (!countDown_.IsInputLocked()) {
        // タイマー更新
        enemySpawnTimer_ += dt;

        // スポーン判定
        if (enemySpawnTimer_ >= kEnemySpawnInterval_) {
            enemySpawnTimer_ = 0.0f;

            // 敵生成
            auto enemy = std::make_unique<SeekerEnemy>();

            // 出現位置をちょっとランダムに（必要なら固定でもOK）
            static std::mt19937_64 rng{ 1234567 };
            std::uniform_real_distribution<float> rx(-8.0f, 8.0f);
            std::uniform_real_distribution<float> ry(-3.0f, 3.0f);

            enemy->SetInitialPosition({ rx(rng), ry(rng), 100.0f });
            enemy->SetSpeed(0.9f);
            enemy->SetTurnRate(0.12f);
            enemy->SetColliderRadius(1.2f);
            enemy->SetInitialHP(3);
            enemy->Initialize();

            enemies_.push_back(std::move(enemy));
        }

        // 追尾：プレイヤーのワールド座標を毎フレーム渡す
        const Vector3 playerPos = player_->GetWorldTranslation(); // ★ プレイヤー座標取得（既存API）
        for (auto& enemy : enemies_) {
            enemy->SetTarget(playerPos);
            enemy->Update();
        }

        // 死亡した敵を消す
        enemies_.erase(
            std::remove_if(enemies_.begin(), enemies_.end(),
                [](const std::unique_ptr<SeekerEnemy>& e) { return e->IsDead(); }),
            enemies_.end());
    }

    // ========== シーン変遷条件 ==========
    if (input_->TriggerKey(DIK_SPACE)) {
        player_->Kill();
    }

    // 爆発演出が終わったらシーン終了
    if (player_->IsExplosionFinished()) {
        isEnd_ = true;
    }
}

void GameScene::Draw() {
    // コマンドリストの取得
    ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

#pragma region 背景スプライト描画
    // 背景スプライト描画前処理
    Sprite::PreDraw(commandList);

    ///<summary>
    /// ここに背景スプライトの描画処理を追加できる
    /// </summary>


    // スプライト描画後処理
    Sprite::PostDraw();

    // 深度バッファクリア
    dxCommon_->ClearDepthBuffer();
#pragma endregion 

#pragma region 3Dオブジェクト描画
    // 3Dオブジェクト描画前処理
    Model::PreDraw();

    /// <summary>
    /// ここに3Dオブジェクトの描画処理を追加できる
    /// </summary>

    // 天球描画
    skydome_->Draw();

    // 敵描画
    /*for(auto& enemy : enemies_) {
        enemy->Draw(&camera_);
    }*/

    // プレイヤー描画
    if (!countDown_.IsInputLocked()) {
        player_->Draw(&camera_);
    }

    // 3Dオブジェクト描画後処理
    Model::PostDraw();
#pragma endregion

#pragma region 前景スプライト描画
    // 前景スプライト描画前処理
    Sprite::PreDraw(commandList);

    /// <summary>
    /// ここに前景スプライトの描画処理を追加できる
    /// </summary>

    countDown_.Draw();

    // スプライト描画後処理
    Sprite::PostDraw();
#pragma endregion
}

IScene* GameScene::NextScene() const {
    return new FinishScene();
}