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

    // ===== クリア/失敗 判定 =====
    if (result_ == GameResult::None) {
        // 1. 失敗判定（プレイヤー爆発が終わった）
        if (player_->IsExplosionFinished()) {
            result_ = GameResult::Fail;
            isEnd_ = true;
        }
        // 2. クリア判定（スコア条件）
        else if (uiManager_.GetScore()->GetScore() >= 200) {
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
