#include "IntroductionScene.h"

#include "TitleScene.h"
#include "GameScene.h"

using namespace KamataEngine;

IntroductionScene::IntroductionScene() {}

IntroductionScene::~IntroductionScene() {
    delete returnTitleSprite_;
    delete introSprite_;
    delete asteroidModel_;
    for (auto asteroid : asteroids_) {
        delete asteroid;
    }
}

void IntroductionScene::Initialize() {
    dxCommon_ = DirectXCommon::GetInstance();

    // シーン変遷の初期化
    nextScene_ = SceneName::None;

    // 各種テクスチャ
    returnTitleTextureHandle_ = TextureManager::Load("./Resources/introduction/Esc-export.png");
    returnTitleSprite_ = Sprite::Create(returnTitleTextureHandle_, { 20.0f,20.0f });

    introTextureHandle_ = TextureManager::Load("./Resources/Introduction/setumei.png");
    introSprite_ = Sprite::Create(introTextureHandle_, { 0.0f,0.0f });

    // 各種サウンド
    changeSEHandle_ = Audio::GetInstance()->LoadWave("./Resources/SE/SceneChange.wav");

    // モデルの生成
    model_ = Model::Create();

    // カメラ
    camera_.Initialize();

    // 天球
    skydome_ = new Skydome();
    skydome_->Initialize(&camera_);

    // 小惑星
    asteroidModel_ = Model::CreateFromOBJ("Asteroid", true);
    asteroids_.reserve(asteroidCount_);
    for (int i = 0; i < asteroidCount_; i++) {
        SpawnAsteroid();
    }
}

void IntroductionScene::Update() {
    // 入力を受け付けるようにする
    input_ = Input::GetInstance();

    // 天球更新
    skydome_->Update();

    // 小惑星出現タイマー更新
    const float dt = 1.0f / 60.0f;
    spawnTimer_ += dt;

    // 小惑星更新
    for (auto* asteroid : asteroids_) {
        asteroid->Update();

        // 
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

    // １つ前のシーンへ
    if (input_->PushKey(DIK_ESCAPE)) {
        Audio::GetInstance()->PlayWave(changeSEHandle_);
        nextScene_ = SceneName::Title;
        isEnd_ = true;
    }

    // 次のシーンへ
    if (input_->PushKey(DIK_SPACE)) {
        Audio::GetInstance()->PlayWave(changeSEHandle_);
        nextScene_ = SceneName::InGame;
        isEnd_ = true;
    }
}

void IntroductionScene::Draw() {
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

    // 天球描画
    skydome_->Draw();

    // 小惑星描画
    for (auto asteroid : asteroids_) {
        asteroid->Draw(camera_);
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

    returnTitleSprite_->Draw();

    introSprite_->Draw();

    // スプライト描画後処理
    Sprite::PostDraw();
#pragma endregion
}

IScene* IntroductionScene::NextScene() const {
    switch (nextScene_) {
    case SceneName::Title:
        return new TitleScene();
        break;

    case SceneName::InGame:
        return new GameScene();
        break;

    default:
        return nullptr;
        break;
    }
}

Asteroid* IntroductionScene::SpawnAsteroid() {
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

float IntroductionScene::Rand(float min, float max) {
    std::uniform_real_distribution<float>dist(min, max);
    return dist(mt_);
}

TransitionHint IntroductionScene::GetTransitionHint(SceneName to) const {
    TransitionHint h{};
    if (to == SceneName::InGame) {
        h.style = TransitionStyle::WhiteFlash;
        h.flashTime = 0.35f;   // 既定0.16fより長め（好みに合わせて調整）
        // h.fadeSpeed = 0.03f; // 必要なら黒フェード速度も上書き可能
    }
    return h;
}