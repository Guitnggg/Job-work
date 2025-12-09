#include "Score.h"

using namespace KamataEngine;

Score::~Score() {
    for (int i = 0; i < kDigitCount; i++) {
        delete sprite_[i];
    }
}

void Score::Initialize() {
    textureHandle_ = TextureManager::Load("./Resources/InGame/number.png");

    // 画面右上に表示するためにx座標を計算
    float screenWidth = static_cast<float>(KamataEngine::DirectXCommon::GetInstance()->GetBackBufferWidth());
    startX = screenWidth - (size_.x * kDigitCount) - 10.0f;

    // 各桁のスプライトを作成
    for (int i = 0; i < kDigitCount; ++i) {
        sprite_[i] = Sprite::Create(textureHandle_, { startX + size_.x * i, startY });
        sprite_[i]->SetSize(size_);
    }

    score_ = 0;
}

void Score::Update() {
    int32_t number = score_;  // 表示対象のスコア値
    int32_t digit = 10000;    // 一番上の桁からスタートする

    for (int i = 0; i < kDigitCount; ++i) {
        int nowNumber = number / digit;  // 現在の桁の数字
        number = number % digit;         // 残りの数字に更新
        digit /= 10;                     // 次の桁へ

        // 数字の部分に対応するテクスチャ範囲を設定
        sprite_[i]->SetTextureRect({ size_.x * nowNumber, 0 }, size_);
    }
}

void Score::Draw() {
    for (int i = 0; i < kDigitCount; ++i) {
        sprite_[i]->Draw();
    }
}

void Score::SetPosition(float x, float y){
    // 描画開始位置を更新、桁ごとに再配置
    startX = x;
    startY = y;

    for (int i = 0; i < kDigitCount; ++i) {
        sprite_[i]->SetPosition({ startX + size_.x * i,startY });
    }
}
