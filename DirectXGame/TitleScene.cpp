#include "TitleScene.h"

#include "GameScene.h"

using namespace KamataEngine;

TitleScene::TitleScene() {}

TitleScene::~TitleScene() {
    delete sprite_;
}

void TitleScene::Initialize() {
    dxCommon_ = DirectXCommon::GetInstance();

    textureHandle_ = TextureManager::Load("./Resources/title/title.png");
    sprite_ = Sprite::Create(textureHandle_, { 0.0f,0.0f });
}

void TitleScene::Update() {

    // 入力を受け付けるようにする
    input_ = Input::GetInstance();

    if (input_->PushKey(DIK_SPACE)) {  // シーン変遷の条件を書く
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

    sprite_->Draw();

    // スプライト描画後処理
    Sprite::PostDraw();
    // 深度バッファクリア
    dxCommon_->ClearDepthBuffer();
#pragma endregion
}

IScene* TitleScene::NextScene() const {
    return new GameScene();
}
