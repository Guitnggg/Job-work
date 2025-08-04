#include "FinishScene.h"

#include "TitleScene.h"

using namespace KamataEngine;

FinishScene::FinishScene() {}

FinishScene::~FinishScene() {
    delete BackgroundSprite_;
    delete FinishSprite_;
    delete ReturnSprite_;
}

void FinishScene::Initialize() {
    dxCommon_ = DirectXCommon::GetInstance();

    BackgroundTextureHandle_ = TextureManager::Load("./Resources/finish/Wood.png");
    BackgroundSprite_ = Sprite::Create(BackgroundTextureHandle_, { 0.0f,0.0f });

    FinishTextureHandle_ = TextureManager::Load("./Resources/finish/End.png");
    FinishSprite_ = Sprite::Create(FinishTextureHandle_, { 150.0f, 200.0f });

    ReturnTextureHandle_ = TextureManager::Load("./Resources/finish/Return.png");
    ReturnSprite_ = Sprite::Create(ReturnTextureHandle_, { 150.0f, 550.0f });
}

void FinishScene::Update() {
    // 入力を受け付けるようにする
    input_ = Input::GetInstance();

    /// Start点滅 ///
    blinkTimer_ += 1.0f / 60.0f;  // 点滅タイマー更新

    if (blinkTimer_ >= blinkInterval_) {
        blinkTimer_ -= blinkInterval_;
    }

    // アルファをサイン波で変化させる
    float alpha = 0.5f + 0.5f * sinf(blinkTimer_ / blinkInterval_ * 2.0f * 3.14159265f);
    ReturnSprite_->SetColor({ 1.0f, 1.0f, 1.0f, alpha });

    /// シーン変遷 ///
    if (input_->PushKey(DIK_SPACE)) {  // シーン変遷の条件を書く
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

    BackgroundSprite_->Draw();
    FinishSprite_->Draw();
    ReturnSprite_->Draw();

    // スプライト描画後処理
    Sprite::PostDraw();

    // 深度バッファクリア
    dxCommon_->ClearDepthBuffer();
#pragma endregion
}

IScene* FinishScene::NextScene() const {
    return new TitleScene();
}
