#include "ClearScene.h"

#include "Scenes/Title/TitleScene.h"

using namespace KamataEngine;

ClearScene::ClearScene(int finalScore) :finalScore_(finalScore) {}

ClearScene::~ClearScene() {
    delete skydome_;
    delete worldTransform_;

    for (auto* a : asteroids_) {
        delete a;
    }
    asteroids_.clear();

    delete clearTextSprite_;
}

void ClearScene::Initialize() {
    // 各種初期化処理
    dxCommon_ = DirectXCommon::GetInstance();

    // ワールド変換データ
    worldTransform_ = new WorldTransform();
    worldTransform_->Initialize();

    // カメラ
    camera_.Initialize();

    // 入力を受け付けるようにする
    input_ = Input::GetInstance();

    // 各種サウンド
    changeSEHandle_ = Audio::GetInstance()->LoadWave("./Resources/SE/SceneChange.wav");
    pointSEHandle_ = Audio::GetInstance()->LoadWave("./Resources/SE/point.wav");

    // 小惑星生成
    asteroidModel_ = Model::CreateFromOBJ("Asteroid", true);
    asteroids_.reserve(asteroidCount_);
    for (int i = 0; i < asteroidCount_; i++) {
        SpawnAsteroid();
    }

    // 天球
    skydome_ = new Skydome();
    skydome_->Initialize(&camera_);

    // ★ スコアUI（0からカウントアップしていく）
    scoreUI_.Initialize();
    displayedScore_ = 0;

    // ★ GAME CLEAR!! テキスト
    uint32_t clearTex = TextureManager::Load("./Resources/Clear/GameClear.png");

    // ※ テクスチャパスは好きなものに変えてOK
    clearTextSprite_ = Sprite::Create(clearTex, { 640.0f, 200.0f });
    clearTextSprite_->SetAnchorPoint({ 0.5f, 0.5f });

    // 最初は見えないサイズ＆透明
    clearTextSprite_->SetSize({ 0.0f, 0.0f });
    clearTextSprite_->SetColor({ 1.0f, 1.0f, 1.0f, 0.0f });

    float centerX = 1280.0f * 0.5f - (32.0f * 5 * 0.5f);
    float centerY = 720.0f * 0.6f;
    scoreUI_.SetPosition(centerX, centerY);

    // 演出開始フェーズ
    phase_ = ClearPhase::CameraMove;
    phaseTimer_ = 0.0f;
}

void ClearScene::Update() {
    // 天球更新
    skydome_->Update();

    const float dt = 1.0f / 60.0f;

    // 小惑星出現タイマー更新
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

    // ===== クリア演出フェーズ制御 =====
    phaseTimer_ += dt;

    switch (phase_) {
    case ClearPhase::CameraMove:
        // 1.5秒ほど背景だけ流して“余韻”
        if (phaseTimer_ >= 1.5f) {
            phase_ = ClearPhase::TitlePop;
            phaseTimer_ = 0.0f;
        }
        break;

    case ClearPhase::TitlePop: {
        // GAME CLEAR!! がバウンスしながら出てくる
        float duration = 0.8f;
        float t = (std::min)(phaseTimer_ / duration, 1.0f);

        float over = 1.2f; // 一瞬大きく
        float scale;
        if (t < 0.6f) {
            scale = over * (t / 0.6f); // 0 → 1.2
        }
        else {
            scale = over + (1.0f - over) * ((t - 0.6f) / 0.4f); // 1.2 → 1.0
        }

        KamataEngine::Vector2 size = {
            clearTextBaseSize_.x * scale,
            clearTextBaseSize_.y * scale
        };
        clearTextSprite_->SetSize(size);
        clearTextSprite_->SetColor({ 1.0f, 1.0f, 1.0f, t }); // αも0→1

        if (t >= 1.0f) {
            phase_ = ClearPhase::ResultCount;
            phaseTimer_ = 0.0f;
        }
    } break;

    case ClearPhase::ResultCount: {
        //Audio::GetInstance()->PlayWave(pointSEHandle_);
        // スコアを0→finalScore_までカウントアップ
        const float speed = 500.0f; // 1秒で約500点増える感じ
        int target = static_cast<int>(displayedScore_ + speed * dt);
        if (target > finalScore_) {
            //Audio::GetInstance()->StopWave(pointSEHandle_);
            target = finalScore_;
        }
        int add = target - displayedScore_;
        if (add > 0) {
            scoreUI_.Add(add);
            displayedScore_ = target;

        }

        // カウントアップが終わったら 0.5秒待って入力待ちへ
        if (displayedScore_ >= finalScore_ && phaseTimer_ >= 0.5f) {
            phase_ = ClearPhase::WaitInput;
            phaseTimer_ = 0.0f;
        }
    } break;

    case ClearPhase::WaitInput:
        // 入力待ち
        break;
    }

    // スコアスプライト更新
    scoreUI_.Update();

    /// シーン変遷 ///
    if (phase_ == ClearPhase::WaitInput && input_->PushKey(DIK_SPACE)) {
        Audio::GetInstance()->PlayWave(changeSEHandle_);
        isEnd_ = true;
    }
}

void ClearScene::Draw() {
    // コマンドリストの取得
    ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

#pragma region 背景スプライト描画
    Sprite::PreDraw(commandList);

    Sprite::PostDraw();
    dxCommon_->ClearDepthBuffer();
#pragma endregion

#pragma region 3Dオブジェクト描画
    Model::PreDraw();

    // 小惑星描画
    for (auto asteroid : asteroids_) {
        asteroid->Draw(camera_);
    }

    // 天球描画
    skydome_->Draw();

    Model::PostDraw();
#pragma endregion

#pragma region 前景スプライト描画
    Sprite::PreDraw(commandList);

    // GAME CLEAR!! テキスト
    if (clearTextSprite_) {
        clearTextSprite_->Draw();
    }

    // スコア
    scoreUI_.Draw();

    Sprite::PostDraw();
#pragma endregion
}

IScene* ClearScene::NextScene() const {
    return new TitleScene();
}

Asteroid* ClearScene::SpawnAsteroid() {
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

float ClearScene::Rand(float min, float max) {
    std::uniform_real_distribution<float>dist(min, max);
    return dist(mt_);
}