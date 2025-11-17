#include "GameScene.h"

#include "FinishScene.h"

#include <fstream>
#include <string>
#include <json.hpp>
using json = nlohmann::json;

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

    /// 2Dグラフ ///
    graph_ = new Graph();
    graph_->Initialize();
    timer_ = 1.0f;

    bullets_.clear();
    fireCooldownFrames_ = 0;

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

    // ========== スコア ==========
    score_ = new Score();
    score_->Initialize();

    LoadEnemySCV("Resources/levels/stage1.json");

    enemySpawnTimer_ = 0.0f;   // 時間計測用

    // 開始
    countDown_.Start();
}

void GameScene::Update() {
    // ========== 3カウント制御 ==========
    const float dt = 1.0f / 60.0f;
    countDown_.Update(dt);

    // ========== 天球更新 ==========
    skydome_->Update();

    // ========== プレイヤー更新 ==========
    if (!countDown_.IsInputLocked()) {
        player_->Update();
    } 

    // ========== 射撃 ==========
    if (!countDown_.IsInputLocked()) {
        if (fireCooldownFrames_ > 0) { --fireCooldownFrames_; }

        if (input_->PushKey(DIK_SPACE) && fireCooldownFrames_ == 0) {
            fireCooldownFrames_ = kFireCooldownMax_;
            auto b = std::make_unique<Bullet>();
            b->Initialize();
            // プレイヤーのワールド位置を取得して弾の初期位置に使う
            // （GetWorldTranslation は CharactorBase で提供）
            const Vector3 muzzle = player_->GetWorldTranslation();
            b->FireFrom(muzzle, { 0.0f, 0.0f, 1.0f }); // +Z方向へ
            bullets_.push_back(std::move(b));
        }
    }

    // ========== 敵スポーン制御 ==========
    if (!countDown_.IsInputLocked()) {
        // タイマー更新
        enemySpawnTimer_ += dt;

        // ★ JSON( SCV ) の値に従ってスポーン
        SpawnEnemiesBySCV();

        // 追尾：プレイヤーのワールド座標を毎フレーム渡す
        const Vector3 playerPos = player_->GetWorldTranslation();
        for (auto& e : enemies_) {
            if (auto* s = dynamic_cast<SeekerEnemy*>(e.get())) {
                s->SetTarget(playerPos);
            }
            e->Update();
        }

        // プレイヤーとの当たり判定
        ResolvePlayerEnemyCollisions();

        // 弾と敵の当たり判定
        ResolveBulletEnemyCollisions();

        // 死亡した敵を消す
        enemies_.erase(
            std::remove_if(enemies_.begin(), enemies_.end(),
                [](const std::unique_ptr<CharactorBase>& e) {
                    if (auto* s = dynamic_cast<SeekerEnemy*>(e.get())) { return s->IsDead(); }
                    return false;
                }),
            enemies_.end());

        /// スコア ///
        score_->Update();
    }

    /// 2Dグラフ ///
    float hpRate = static_cast<float>(player_->GetHP()) / 100.0f;
    graph_->SetValue(hpRate);
    graph_->Update();

    // ========== 弾の更新 & 後始末 ==========
    for (auto& b : bullets_) { if (b) { b->Update(); } }
    bullets_.erase(
        std::remove_if(bullets_.begin(), bullets_.end(),
            [](const std::unique_ptr<Bullet>& b) { return !b || b->IsDead(); }),
        bullets_.end());

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
    for (auto& enemy : enemies_) {
        enemy->Draw(&camera_);
    }

    for (auto& b : bullets_) { b->Draw(&camera_); }

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

    /// 2Dグラフ ///
    graph_->Draw();

    /// スコア ///
    score_->Draw();

    // スプライト描画後処理
    Sprite::PostDraw();
#pragma endregion
}

IScene* GameScene::NextScene() const {
    return new FinishScene();
}

void GameScene::ResolvePlayerEnemyCollisions() {
    if (!player_ || countDown_.IsInputLocked() || player_->IsExplosionFinished()) { return; }

    Collider* pc = player_->GetCollider().get();
    if (!pc) { return; }

    const auto p = pc->GetTranslate();
    const float pr = pc->GetRadius();

    for (auto& e : enemies_) {
        if (!e) { continue; }
        Collider* ec = e->GetCollider().get();
        if (!ec) { continue; }

        const auto q = ec->GetTranslate();
        const float er = ec->GetRadius();

        const float dx = p.x - q.x, dy = p.y - q.y, dz = p.z - q.z;
        const float dist2 = dx * dx + dy * dy + dz * dz;
        const float r = pr + er;

        if (dist2 <= r * r) {
            player_->OnCollision(e.get());
            e->OnCollision(player_);
        }
    }
}

void GameScene::ResolveBulletEnemyCollisions() {
    if (countDown_.IsInputLocked()) { return; }
    // 弾×敵：単純な球同士判定
    for (auto& b : bullets_) {
        if (!b || b->IsDead()) { continue; }
        Collider* bc = b->GetCollider().get();

        if (!bc) { continue; }
        const auto bp = bc->GetTranslate();
        const float br = bc->GetRadius();

        for (auto& e : enemies_) {
            if (!e) { continue; }
            // 既に死んだ敵は飛ばす
            if (auto* s = dynamic_cast<SeekerEnemy*>(e.get())) {
                if (s->IsDead()) { continue; }
            }
            Collider* ec = e->GetCollider().get();
            if (!ec) { continue; }
            const auto ep = ec->GetTranslate();
            const float er = ec->GetRadius();

            const float dx = bp.x - ep.x, dy = bp.y - ep.y, dz = bp.z - ep.z;
            const float dist2 = dx * dx + dy * dy + dz * dz;
            const float rr = br + er;

            if (dist2 <= rr * rr) {
                // 相互通知：敵は死に、弾は消える
                e->OnCollision(b.get());
                b->OnCollision(e.get());

                // 1発で1体想定なので次の弾へ
                break;
            }
        }
    }
}

void GameScene::LoadEnemySCV(const std::string& path)
{
    enemySpawnList_.clear();

    std::ifstream ifs(path);
    if (!ifs) { return; }

    json root;
    ifs >> root;

    if (root.contains("randomAreas")) {
        // 固定シードにしておくと毎回同じ配置になる
        static std::mt19937_64 rng{ 123456789 };

        for (auto& r : root["randomAreas"]) {
            int   count = r.value("count", 0);
            float timeMin = r.value("timeMin", 0.0f);
            float timeMax = r.value("timeMax", timeMin);

            auto  centerNode = r["posCenter"];
            Vector3 center{
                centerNode[0].get<float>(),
                centerNode[1].get<float>(),
                centerNode[2].get<float>()
            };

            auto rangeNode = r["posRange"];
            Vector3 range{
                rangeNode[0].get<float>(),
                rangeNode[1].get<float>(),
                rangeNode[2].get<float>()
            };

            float baseSpeed = r.value("speed", 0.2f);
            float speedRange = r.value("speedRange", 0.0f);
            float turnRate = r.value("turnRate", 0.15f);
            int   hp = r.value("hp", 1);
            float radius = r.value("radius", 1.0f);
            float lifeTime = r.value("lifeTime", 30.0f);

            std::uniform_real_distribution<float> timeDist(timeMin, timeMax);
            std::uniform_real_distribution<float> dx(-range.x, range.x);
            std::uniform_real_distribution<float> dy(-range.y, range.y);
            std::uniform_real_distribution<float> dz(-range.z, range.z);
            std::uniform_real_distribution<float> speedDist(
                baseSpeed - speedRange,
                baseSpeed + speedRange
            );

            for (int i = 0; i < count; ++i) {
                EnemySpawnData d;

                d.time = timeDist(rng);
                d.pos = {
                    center.x + dx(rng),
                    center.y + dy(rng),
                    center.z + dz(rng)
                };
                d.speed = speedDist(rng);
                d.turnRate = turnRate;
                d.hp = hp;
                d.radius = radius;
                d.lifeTime = lifeTime;

                enemySpawnList_.push_back(d);
            }
        }
    }

    // 出現時間でソート
    std::sort(enemySpawnList_.begin(), enemySpawnList_.end(),
        [](const EnemySpawnData& a, const EnemySpawnData& b) {
            return a.time < b.time;
        });
}

void GameScene::SpawnEnemiesBySCV()
{
    const float t = enemySpawnTimer_;

    while (!enemySpawnList_.empty()) {
        const auto& d = enemySpawnList_.front();

        if (t < d.time) break;

        // プレイヤーの現在位置を基準にする
        const Vector3 playerPos = player_->GetWorldTranslation();

        // JSONのposを「プレイヤーからのオフセット」として使う
        Vector3 spawnPos{
            playerPos.x + d.pos.x,   // 左右オフセット
            playerPos.y + d.pos.y,   // 上下オフセット
            playerPos.z + d.pos.z    // 前後オフセット（レールの進行方向）
        };

        // 敵生成
        auto s = std::make_unique<SeekerEnemy>();

        s->SetInitialPosition(spawnPos);
        s->SetSpeed(d.speed);
        s->SetTurnRate(d.turnRate);
        s->SetInitialHP(d.hp);
        s->SetColliderRadius(d.radius);
        s->SetLifeTime(d.lifeTime);

        s->Initialize();

        enemies_.push_back(std::move(s));
        enemySpawnList_.erase(enemySpawnList_.begin());
    }
}