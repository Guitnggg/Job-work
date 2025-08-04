#include "GameScene.h"

#include "FinishScene.h"

using namespace KamataEngine;

GameScene::GameScene() {}

GameScene::~GameScene() {
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

}

void GameScene::Update() {
    // 入力を受け付けるようにする
    input_ = Input::GetInstance();

    if (input_->PushKey(DIK_SPACE)) {  // シーン変遷の条件を書く
        isEnd_ = true;
    }
}

void GameScene::Draw() {

}

IScene* GameScene::NextScene() const {
    return new FinishScene();
}
