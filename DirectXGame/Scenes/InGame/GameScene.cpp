#include "GameScene.h"

#include "FinishScene.h"

using namespace KamataEngine;

GameScene::GameScene() {}

GameScene::~GameScene() {
    delete railCamera_;
    delete skydome_;
    delete player_;
}

void GameScene::Initialize() {
    // 各初期化処理
    dxCommon_ = DirectXCommon::GetInstance();  // DirectX
    input_ = Input::GetInstance();             // 入力
    audio_ = Audio::GetInstance();             // サウンド

    worldTransform_ = new WorldTransform();
    worldTransform_->Initialize();              // ワールド変換データ
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
    count1TextureHandle_ = TextureManager::Load("./Resources/InGame/1.png");
    count2TextureHandle_ = TextureManager::Load("./Resources/InGame/2.png");
    count3TextureHandle_ = TextureManager::Load("./Resources/InGame/3.png");
    startTextureHandle_ = TextureManager::Load("./Reosurces/InGame/Go.png");

    // アンカー中央、画面中央へ
    auto makeCenterd = [](uint32_t tex) {
        return Sprite::Create(tex, { 640.0f,360.0f }, { 1,1,1,0 }, { 0.5f,0.5f }, false, false);
        };

}

void GameScene::Update() {
    // 天球更新
    skydome_->Update();

    // プレイヤー更新
    player_->Update();



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

    if (input_->PushKey(DIK_SPACE)) {  // シーン変遷の条件を書く
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

    // プレイヤー描画
    player_->Draw();

    // 3Dオブジェクト描画後処理
    Model::PostDraw();
#pragma endregion

#pragma region 前景スプライト描画
    // 前景スプライト描画前処理
    Sprite::PreDraw(commandList);

    /// <summary>
    /// ここに前景スプライトの描画処理を追加できる
    /// </summary>

    count1Sprite_->Draw();

    // スプライト描画後処理
    Sprite::PostDraw();
#pragma endregion
}

IScene* GameScene::NextScene() const {
    return new FinishScene();
}

float GameScene::EaseOutBack(float t, float s) const {
    t -= 1.0f;
    return t * t * ((s + 1.0f) * t + s) + 1.0f;
}

float GameScene::CurrentPhaseDuration() const {
    switch (startPhase_) {
    case StartPhase::ReadyDelay: return readyDelay_;
    case StartPhase::Count3:     return countUnit_;
    case StartPhase::Count2:     return countUnit_;
    case StartPhase::Count1:     return countUnit_;
    case StartPhase::Go:         return goDuration_;
    case StartPhase::Done:       return 0.0f;
    }
    return 0.0f;
}

KamataEngine::Sprite* GameScene::CurrentPhaseSprite() const {
    switch (startPhase_) {
    case StartPhase::Count3: return count3Sprite_;
    case StartPhase::Count2: return count2Sprite_;
    case StartPhase::Count1: return count1Sprite_;
    case StartPhase::Go:     return startSprite_;
    default:                 return nullptr;
    }
}

float GameScene::CurrentPhaseAlpha(float t01) const
{
    // 0→1→0 の三角波（前半フェードイン、後半フェードアウト）
    if (t01 < 0.5f) {
        return t01 / 0.5f;
    }
    else {
        return 1.0f - (t01 - 0.5f) / 0.5f;
    }
}

float GameScene::CurrentPhaseScale(float t01) const {
    // easeOutBack で 1.20 → 1.00
    float e = EaseOutBack(std::clamp(t01, 0.0f, 1.0f), backS_);
    return countScaleStart_ + (countScaleEnd_ - countScaleStart_) * e;
}

void GameScene::AdvancePhase() {
    // 次のフェーズへ
    phaseTimer_ = 0.0f;

    switch (startPhase_) {
    case StartPhase::ReadyDelay:
        startPhase_ = StartPhase::Count3;
        audio_->PlayWave(seBeepHandle_);  // 3 の頭で鳴らす
        break;

    case StartPhase::Count3:
        startPhase_ = StartPhase::Count2;
        audio_->PlayWave(seBeepHandle_);
        break;

    case StartPhase::Count2:
        startPhase_ = StartPhase::Count1;
        audio_->PlayWave(seBeepHandle_);
        break;

    case StartPhase::Count1:
        startPhase_ = StartPhase::Go;
        goPlayed_ = false;  // Go の開始で SE を鳴らす準備
        // 入力解禁タイミングは Go の開始 or 終了で好みが分かれる
        // 今回は開始時に解禁（Update内で処理）
        break;

    case StartPhase::Go:
        startPhase_ = StartPhase::Done;
        // Go!! の終了で数字は消える。以降は通常プレイ。
        // HUDの個別起動をここで連鎖させてもOK
        break;

    case StartPhase::Done:
    default:
        break;
    }
}
