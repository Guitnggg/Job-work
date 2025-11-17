#include "GameScene.h"
#include "FinishScene.h"

using namespace KamataEngine;

GameScene::GameScene() {}

GameScene::~GameScene() {
    delete railCamera_;
    delete skydome_;
    delete player_;
    delete graph_;
    delete score_;

    delete worldTransform_;
    delete model_;
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

    // 2Dグラフ
    graph_ = new Graph();
    graph_->Initialize();

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

    auto* audio = KamataEngine::Audio::GetInstance();
    countDown_.SetAudio(
        audio->LoadWave("./Resources/SE/CountBeep.wav"),
        audio->LoadWave("./Resources/SE/Start.wav")
    );

    // 弾・敵
    bulletManager_.Initialize();
    enemyManager_.Initialize();
    enemyManager_.LoadEnemySCV("Resources/levels/stage1.json");

    // スコア
    score_ = new Score();
    score_->Initialize();

    // 開始
    countDown_.Start();

    isEnd_ = false;
}

void GameScene::Update() {
    const float dt = 1.0f / 60.0f;

    // 3カウント
    countDown_.Update(dt);

    // 天球
    skydome_->Update();

    // プレイヤー
    if (!countDown_.IsInputLocked()) {
        player_->Update();
    }

    // 弾（入力＋更新）
    bulletManager_.Update(input_, player_, countDown_);

    // 敵スポーン & 更新
    if (!countDown_.IsInputLocked()) {
        const Vector3 playerPos = player_->GetWorldTranslation();
        enemyManager_.Update(dt, playerPos);

        // 当たり判定
        CollisionManager::ResolvePlayerEnemyCollisions(
            player_, enemyManager_.GetEnemies(), countDown_);

        CollisionManager::ResolveBulletEnemyCollisions(
            bulletManager_.GetBullets(), enemyManager_.GetEnemies(), countDown_);

        // 死んだ敵の削除
        enemyManager_.RemoveDeadEnemies();

        // スコア
        score_->Update();
    }

    // HPバー
    float hpRate = static_cast<float>(player_->GetHP()) / 100.0f;
    graph_->SetValue(hpRate);
    graph_->Update();

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

    // 爆発演出が終わったらシーン終了
    if (player_->IsExplosionFinished()) {
        isEnd_ = true;
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

    // 敵
    enemyManager_.Draw(&camera_);

    // 弾
    bulletManager_.Draw(&camera_);

    // プレイヤー
    if (!countDown_.IsInputLocked()) {
        player_->Draw(&camera_);
    }

    Model::PostDraw();
#pragma endregion

#pragma region 前景スプライト描画
    Sprite::PreDraw(commandList);

    countDown_.Draw();
    graph_->Draw();
    score_->Draw();

    Sprite::PostDraw();
#pragma endregion
}

IScene* GameScene::NextScene() const {
    return new FinishScene();
}
