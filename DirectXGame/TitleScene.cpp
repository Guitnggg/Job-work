#include "TitleScene.h"

#include "IntroductionScene.h"

using namespace KamataEngine;

TitleScene::TitleScene() {}

TitleScene::~TitleScene() {
    delete BackgroundSprite_;
    delete TitleSprite_;
    delete StartSprite_;
    delete fadeSprite_;
}

void TitleScene::Initialize() {
    dxCommon_ = DirectXCommon::GetInstance();

    // 各種テクスチャ
    BackgroundTextureHandle_ = TextureManager::Load("./Resources/title/Wood.png");
    BackgroundSprite_ = Sprite::Create(BackgroundTextureHandle_, { 0.0f,0.0f });

    TitleTextureHandle_ = TextureManager::Load("./Resources/title/GameTitle.png");
    TitleSprite_ = Sprite::Create(TitleTextureHandle_, titlePosition_);

    StartTextureHandle_ = TextureManager::Load("./Resources/title/Start.png");
    StartSprite_ = Sprite::Create(StartTextureHandle_, { 150.0f,550.0f });

    // FadeIn
    uint32_t whiteHandle_ = TextureManager::Load("./Resources/white1x1.png");
    fadeSprite_ = Sprite::Create(whiteHandle_, { 0.0f, 0.0f });
    fadeSprite_->SetSize({
        (float)dxCommon_->GetBackBufferWidth(),
        (float)dxCommon_->GetBackBufferHeight()
    });
    fadeSprite_->SetColor({ 0.0f, 0.0f, 0.0f, fadeAlpha_ });
}

void TitleScene::Update() {

    // 入力を受け付けるようにする
    input_ = Input::GetInstance();

    if (isFadingIn_) {
        fadeAlpha_ -= fadeSpeed_;
        if (fadeAlpha_ <= 0.0f) {
            fadeAlpha_ = 0.0f;
            isFadingIn_ = false;
        }
        fadeSprite_->SetColor({ 0.0f, 0.0f, 0.0f, fadeAlpha_ });
    }
    else {
        // Title落下用
        if (titlePosition_.y < titleTargetPosition_.y) {
            titlePosition_.y += titleFallSpeed_;
            if (titlePosition_.y > titleTargetPosition_.y) {
                titlePosition_.y = titleTargetPosition_.y; // 超えたら固定
                isTitleFallFinished_ = true;
            }
            TitleSprite_->SetPosition(titlePosition_);
        }

        // Start点滅
        blinkTimer_ += 1.0f / 60.0f;  // 点滅タイマー更新

        if (blinkTimer_ >= blinkInterval_) {
            blinkTimer_ -= blinkInterval_;
        }

        // アルファをサイン波で変化させる
        float alpha = 0.5f + 0.5f * sinf(blinkTimer_ / blinkInterval_ * 2.0f * 3.14159265f);
        StartSprite_->SetColor({ 1.0f,1.0f,1.0f,alpha });

        // シーン変遷
        if (isTitleFallFinished_ && input_->PushKey(DIK_SPACE)) {  // シーン変遷の条件を書く
            isEnd_ = true;
        }
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

    BackgroundSprite_->Draw();
    TitleSprite_->Draw();
    StartSprite_->Draw();
    
    if (fadeSprite_) {
        fadeSprite_->Draw();
    }

    // スプライト描画後処理
    Sprite::PostDraw();
    // 深度バッファクリア
    dxCommon_->ClearDepthBuffer();
#pragma endregion
}

IScene* TitleScene::NextScene() const {
    return new IntroductionScene();
}
