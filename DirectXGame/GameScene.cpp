#include "GameScene.h"

#include "FinishScene.h"

using namespace KamataEngine;

GameScene::GameScene() {}

GameScene::~GameScene() {
    delete model_;
    delete skydome_;
    delete player_;
    delete railCamera_;
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

    // レールカメラ
    railCamera_ = new RailCamera();
    railCamera_->Initialize();

    // 天球
    skydome_ = new Skydome();
    skydome_->Initialize(&camera_);

    // Player
    player_ = new Player();
    player_->SetParent(&railCamera_->GetWorldTransform());
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
    // コマンドリストの取得
    ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

#pragma region 背景スプライト描画
    // 背景スプライト描画前処理
    Sprite::PreDraw(commandList);



    // スプライト描画後処理
    Sprite::PostDraw();

    // 深度バッファクリア
    dxCommon_->ClearDepthBuffer();
#pragma endregion

#pragma region 3Dオブジェクト描画
    // 3Dオブジェクト描画前処理
    Model::PreDraw();

    // 天球
    skydome_->Draw();

    // Player
    player_->Draw();

    // 3Dオブジェクト描画後処理
    Model::PostDraw();
#pragma endregion

#pragma region 前景スプライト描画
    // 前景スプライト描画前処理
    Sprite::PreDraw(commandList);


    // スプライト描画後処理
    Sprite::PostDraw();
#pragma endregion
}

IScene* GameScene::NextScene() const {
    return new FinishScene();
}
