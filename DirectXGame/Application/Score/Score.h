#pragma once

#include <2d/Sprite.h>
#include <base/TextureManager.h>
#include <base/DirectXCommon.h>

class Score {
public:

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~Score();

    /// <summary>
    /// 初期化処理
    /// </summary>
    void Initialize();

    /// <summary>
    /// 更新処理
    /// </summary>
    void Update();

    /// <summary>
    /// 描画処理
    /// </summary>
    void Draw();

private:
    uint32_t textureHandle_ = 0;
    KamataEngine::Vector2 size_ = { 32.0f, 64.0f };  // 1文字のサイズ

    float startX = 0.0f;   // 表示位置（x座標）
    float startY = 5.0f;  // 表示位置（y座標）

    int score_ = 0;

    static const int kDigitCount = 5;            // 最大桁数（99999まで）
    KamataEngine::Sprite* sprite_[kDigitCount];  // 各桁ごとのスプライト
};

