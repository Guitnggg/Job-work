#include "Score.h"

#include "Application/Utility/GameTime.h"

#include <algorithm>

namespace {
constexpr float kRightMargin = 10.0f;
constexpr float kScoreFollowRate = 0.22f;
constexpr float kScorePopDuration = 0.18f;
constexpr float kScorePopScale = 0.35f;
}

void Score::Initialize() {
    textureHandle_ = KamataEngine::TextureManager::Load("./Resources/InGame/number.png");

    // 画面右上に表示するためにx座標を計算
    float screenWidth = static_cast<float>(KamataEngine::DirectXCommon::GetInstance()->GetBackBufferWidth());
    startX_ = screenWidth - (size_.x * kDigitCount) - kRightMargin;

    // 各桁のスプライトを作成
    for (int i = 0; i < kDigitCount; ++i) {
        digitSprites_[i].reset(KamataEngine::Sprite::Create(textureHandle_, { startX_ + size_.x * i, startY_ }));
        digitSprites_[i]->SetSize(size_);
    }

    score_ = 0;
    displayedScore_ = 0.0f;
    scorePopTimer_ = 0.0f;
}

void Score::Update() {
    displayedScore_ += (static_cast<float>(score_) - displayedScore_) * kScoreFollowRate;
    if (scorePopTimer_ > 0.0f) {
        scorePopTimer_ = (std::max)(0.0f, scorePopTimer_ - GameTime::kDeltaTime);
        const float pulse = 1.0f + (scorePopTimer_ / kScorePopDuration) * kScorePopScale;
        for (int i = 0; i < kDigitCount; ++i) {
            digitSprites_[i]->SetSize({ size_.x * pulse, size_.y * pulse });
        }
    }
    else {
        for (int i = 0; i < kDigitCount; ++i) {
            digitSprites_[i]->SetSize(size_);
        }
    }

    int32_t number = static_cast<int32_t>(displayedScore_ + 0.5f); // 表示対象のスコア値
    int32_t digit = 10000;                                         // 一番上の桁からスタートする

    for (int i = 0; i < kDigitCount; ++i) {
        int nowNumber = number / digit; // 現在の桁の数字
        number = number % digit;        // 残りの数字に更新
        digit /= 10;                    // 次の桁へ

        // 数字の部分に対応するテクスチャ範囲を設定
        digitSprites_[i]->SetTextureRect({ size_.x * nowNumber, 0 }, size_);
    }
}

void Score::Draw() {
    for (int i = 0; i < kDigitCount; ++i) {
        digitSprites_[i]->Draw();
    }
}

void Score::Add(int value) {
    score_ += value;
    scorePopTimer_ = kScorePopDuration;
}

void Score::SetPosition(float x, float y) {
    // 描画開始位置を更新、桁ごとに再配置
    startX_ = x;
    startY_ = y;

    for (int i = 0; i < kDigitCount; ++i) {
        digitSprites_[i]->SetPosition({ startX_ + size_.x * i, startY_ });
    }
}
