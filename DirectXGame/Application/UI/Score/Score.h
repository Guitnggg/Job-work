#pragma once

#include <2d/Sprite.h>
#include <base/TextureManager.h>
#include <base/DirectXCommon.h>

/// <summary>
/// スコア表示クラス
/// 1つの数字スプライトを横に並べ、整数値を桁ごとに描画する仕組み
/// </summary>
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

    /// <summary>
    /// スコアを加算する
    /// </summary>
	void Add(int value);

    /// <summary>
    /// 現在のスコアを取得する
    /// </summary>
    int GetScore()const { return score_; }

    /// <summary>
    /// スプライトの表示位置を設定する
    /// </summary>
    void SetPosition(float x, float y);

private:
    uint32_t textureHandle_ = 0;
    KamataEngine::Vector2 size_ = { 32.0f, 64.0f };  // 1文字のサイズ

    float startX_ = 0.0f;  // 表示位置（x座標）
    float startY_ = 5.0f;  // 表示位置（y座標）

    int score_ = 0;
	float displayedScore_ = 0.0f; // 表示中のスコア

    float scorePopTimer_ = 0.0f;

    static const int kDigitCount = 5;                  // 最大桁数（99999まで）
    KamataEngine::Sprite* digitSprites_[kDigitCount];  // 各桁ごとのスプライト
};

