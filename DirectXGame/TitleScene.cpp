#include "TitleScene.h"

#include "IntroductionScene.h"

using namespace KamataEngine;

TitleScene::TitleScene() {}

TitleScene::~TitleScene() {
    delete TitleSprite_;
    delete StartSprite_;
    delete asteroidModel_;
    for (auto asteroid : asteroids_) {
        delete asteroid;
    }
    delete skydome_;
}

void TitleScene::Initialize() {
    // 各初期化処理
    dxCommon_ = DirectXCommon::GetInstance();

    // 入力を受け付けるようにする
    worldTransform_ = new WorldTransform();
    worldTransform_->Initialize();

    // カメラ
    camera_.Initialize();

    // 入力を受け付けるようにする
    input_ = Input::GetInstance();

    // 各種テクスチャ
    TitleTextureHandle_ = TextureManager::Load("./Resources/title/GameTitle.png");
    TitleSprite_ = Sprite::Create(TitleTextureHandle_, titlePosition_);

    StartTextureHandle_ = TextureManager::Load("./Resources/title/Start.png");
    StartSprite_ = Sprite::Create(StartTextureHandle_, { 150.0f,550.0f });
    StartSprite_->SetColor({ 1.0f,1.0f,1.0f,0.0f });  // 最初は透明

    // 各種サウンド
    changeSEHandle_ = Audio::GetInstance()->LoadWave("./Resources/SE/SceneChange.wav");

    // 小惑星生成
    asteroidModel_ = Model::CreateFromOBJ("Asteroid", true);
    asteroids_.reserve(asteroidCount_);
    for(int i = 0; i < asteroidCount_; i++) {
        SpawnAsteroid();
    }

    // 天球
    skydome_ = new Skydome();
    skydome_->Initialize(&camera_);
}

void TitleScene::Update() {    
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

    // Title落下用
    if (titlePosition_.y < titleTargetPosition_.y) {
        titlePosition_.y += titleFallSpeed_;
        if (titlePosition_.y > titleTargetPosition_.y) {
            titlePosition_.y = titleTargetPosition_.y; // 超えたら固定
            isTitleFallFinished_ = true;

            // 点滅の初期化
            blinkTimer_ = 0.0f;
        }
        TitleSprite_->SetPosition(titlePosition_);
    }

    // Start点滅
    if (isTitleFallFinished_) {
        blinkTimer_ += 1.0f / 60.0f;
        if (blinkTimer_ >= blinkInterval_) {
            blinkTimer_ -= blinkInterval_;
        }
        const float alpha = 0.5f + 0.5f * sinf(blinkTimer_ / blinkInterval_ * 2.0f * 3.14159265f);
        StartSprite_->SetColor({ 1.0f, 1.0f, 1.0f, alpha });
    }
    else {
        // 完了前は常に透明のまま
        StartSprite_->SetColor({ 1.0f, 1.0f, 1.0f, 0.0f });
    }

    // シーン遷移（落下完了後のみ）
    if (isTitleFallFinished_ && input_->PushKey(DIK_SPACE)) {
        Audio::GetInstance()->PlayWave(changeSEHandle_);
        isEnd_ = true;
    }
}

void TitleScene::Draw() {
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
    // 前景スプライト描画前処理
    Sprite::PreDraw(commandList);

    /// <summary>
    /// ここに前景スプライトの描画処理を追加できる
    /// </summary>

    TitleSprite_->Draw();
    StartSprite_->Draw();

    // スプライト描画後処理
    Sprite::PostDraw();
#pragma endregion

}

IScene* TitleScene::NextScene() const {
    return new IntroductionScene();
}

Asteroid* TitleScene::SpawnAsteroid() {
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

float TitleScene::Rand(float min, float max){
    std::uniform_real_distribution<float> dist(min, max);
    return dist(mt_);
}
