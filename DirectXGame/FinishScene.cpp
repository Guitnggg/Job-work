#include "FinishScene.h"

#include "TitleScene.h"

using namespace KamataEngine;

FinishScene::FinishScene() {}

FinishScene::~FinishScene() {
    delete FinishSprite_;
    delete ReturnSprite_;
}

void FinishScene::Initialize() {
    // 各種初期化処理
    dxCommon_ = DirectXCommon::GetInstance();

    // ワールド変換データ
    worldTransform_ = new WorldTransform();
    worldTransform_->Initialize();

    // カメラ
    camera_.Initialize();

    // 入力を受け付けるようにする
    input_ = Input::GetInstance();

    // 各種テクスチャ
    FinishTextureHandle_ = TextureManager::Load("./Resources/finish/End.png");
    FinishSprite_ = Sprite::Create(FinishTextureHandle_, { 150.0f, 200.0f });

    ReturnTextureHandle_ = TextureManager::Load("./Resources/finish/Return.png");
    ReturnSprite_ = Sprite::Create(ReturnTextureHandle_, { 120.0f, 550.0f });

    // 各種サウンド
    changeSEHandle_ = Audio::GetInstance()->LoadWave("./Resources/SE/SceneChange.wav");

    // 小惑星生成
    asteroidModel_ = Model::CreateFromOBJ("Asteroid", true);
    asteroids_.reserve(asteroidCount_);
    for (int i = 0; i < asteroidCount_; i++) {
        SpawnAsteroid();
    }

    // 天球
    skydome_ = new Skydome();
    skydome_->Initialize(&camera_);
}

void FinishScene::Update() {
    // 天球更新
    skydome_->Update();

    // 小惑星出現タイマー更新
    const float dt = 1.0f / 60.0f;
    spawnTimer_ += dt;

    // 小惑星更新
    for (auto* asteroid : asteroids_) {
        asteroid->Update();

        // 画面から見えなくなったらリスポーン
        if (asteroid->GetZ() < recycleZ_ && spawnTimer_ >= spawnInterval_) {
            spawnTimer_ = 0.0f;

            Vector3 pos = {
                Rand(-25.0f,25.0f),
                Rand(-15.0f,15.0f),
                Rand(spawnZMin_,spawnZMax_)
            };
            Vector3 velocity = { 0.0f,0.0f,Rand(-0.3f,-0.1f) };
            Vector3 rotate = { Rand(0.01f,0.03f),Rand(0.01f,0.03f),Rand(0.01f,0.03f) };

            asteroid->Respawn(pos, velocity, rotate);
        }
    }

    blinkTimer_ += 1.0f / 60.0f;  // 点滅タイマー更新

    if (blinkTimer_ >= blinkInterval_) {
        blinkTimer_ -= blinkInterval_;
    }

    // アルファをサイン波で変化させる
    float alpha = 0.5f + 0.5f * sinf(blinkTimer_ / blinkInterval_ * 2.0f * 3.14159265f);
    ReturnSprite_->SetColor({ 1.0f, 1.0f, 1.0f, alpha });

    /// シーン変遷 ///
    if (input_->PushKey(DIK_SPACE)) {  // シーン変遷の条件を書く
        Audio::GetInstance()->PauseWave(changeSEHandle_);
        isEnd_ = true;
    }
}

void FinishScene::Draw() {
    // コマンドリストの取得
    ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

#pragma region 背景スプライト描画
    // 背景スプライト描画前処理
    Sprite::PreDraw(commandList);

    /// <summary>
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

    // 小惑星描画
    for (auto asteroid : asteroids_) {
        asteroid->Draw(camera_);
    }

    // 天球描画
    skydome_->Draw();

    // 3Dオブジェクト描画後処理
    Model::PostDraw();

#pragma endregion

#pragma region 前景スプライト描画
    // 背景スプライト描画前処理
    Sprite::PreDraw(commandList);

    /// <summary>
    /// ここに背景スプライトの描画処理を追加できる
    /// </summary>

    FinishSprite_->Draw();
    ReturnSprite_->Draw();

    // スプライト描画後処理
    Sprite::PostDraw();
#pragma endregion
}

IScene* FinishScene::NextScene() const {
    return new TitleScene();
}

Asteroid* FinishScene::SpawnAsteroid() {
    // 位置、速度、回転をランダムに決定
    Vector3 pos = {
        Rand(-25.0f,25.0f),
        Rand(-15.0f,15.0f),
        Rand(spawnZMin_,spawnZMax_)
    };
    Vector3 velocity = { 0.0f,0.0f,Rand(-0.3f,-0.1f) };
    Vector3 rotate = { Rand(0.01f,0.03f),Rand(0.01f,0.03f),Rand(0.01f,0.03f) };

    // インスタンス生成
    Asteroid* asteroid = new Asteroid();
    asteroid->Initialize(asteroidModel_, pos, velocity, rotate);

    // 配列に登録
    asteroids_.push_back(asteroid);
    return asteroid;
}

float FinishScene::Rand(float min, float max) {
    std::uniform_real_distribution<float>dist(min, max);
    return dist(mt_);
}