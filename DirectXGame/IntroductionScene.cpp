#include "IntroductionScene.h"

#include "TitleScene.h"
#include "GameScene.h"

using namespace KamataEngine;

IntroductionScene::IntroductionScene() {}

IntroductionScene::~IntroductionScene() {}

void IntroductionScene::Initialize() {
    dxCommon_ = DirectXCommon::GetInstance();

    // シーン変遷の初期化
    nextScene_ = SceneName::None;

    // 各種サウンド
    changeSEHandle_ = Audio::GetInstance()->LoadWave("./Resources/SE/SceneChange.wav");

    // モデルの生成
    model_ = Model::Create();

    // カメラ
    camera_.Initialize();

    // 天球
    skydome_ = new Skydome();
    skydome_->Initialize(&camera_);
}

void IntroductionScene::Update() {

    // 入力を受け付けるようにする
    input_ = Input::GetInstance();

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
