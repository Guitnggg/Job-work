#include "GameScene.h"

#include "FinishScene.h"

using namespace KamataEngine;

GameScene::GameScene() {}

GameScene::~GameScene() {
    delete model_;
    delete skydome_;
    delete player_;
}

void GameScene::Initialize()
{
    // 各初期化処理
    dxCommon_ = DirectXCommon::GetInstance();  // DirectX
    input_ = Input::GetInstance();             // 入力
    audio_ = Audio::GetInstance();             // サウンド

    worldTransform_.Initialize();              // ワールド変換データ
    camera_.Initialize();                      // カメラ（ビューポート）

    // モデルの生成
    model_ = Model::Create();

    // 天球
    skydome_ = new Skydome();
    skydome_->Initialize(&camera_);

    // Player
    player_ = new Player();
    player_->Initialize(&camera_);

    // 入力を受け付けるようにする
    input_ = Input::GetInstance();
}

void GameScene::Update() {

    skydome_->Update();

    player_->Update();









    

    //if (input_->PushKey(DIK_SPACE)) {  // シーン変遷の条件を書く
    //    isEnd_ = true;
    //}
}

void GameScene::Draw() {
    // 3Dオブジェクト描画前処理
    Model::PreDraw();

    skydome_->Draw();

    player_->Draw();

    // 3Dオブジェクト描画後処理
    Model::PostDraw();
}

IScene* GameScene::NextScene() const {
    return new FinishScene();
}
