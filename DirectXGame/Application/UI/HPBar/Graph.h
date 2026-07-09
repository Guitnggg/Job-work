#pragma once

#include <memory>

#include "base/TextureManager.h"
#include <2d/Sprite.h>

/// <summary>
/// グラフ表示クラス 背景バー（赤）＋前景バー（緑）
/// </summary>
class Graph {
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    Graph();

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~Graph() = default;

    /// <summary>
    /// 初期化処理
    /// </summary>
    void Initialize();

    /// <summary>
    /// 表示する割合（0.0～1.0）を設定
    /// </summary>
    /// <param name="rate">表示比率</param>
    void SetValue(float rate);

    /// <summary>
    /// 表示割合を取得する
    /// </summary>
    float GetValue() const { return value_; }

    /// <summary>
    /// 更新処理
    /// </summary>
    void Update();

    /// <summary>
    /// 描画処理
    /// </summary>
    void Draw();

private:
    std::unique_ptr<KamataEngine::Sprite> backBar_ = nullptr;  // 裏のグラフ（赤）
    std::unique_ptr<KamataEngine::Sprite> frontBar_ = nullptr; // 表のグラフ（緑）

    uint32_t texHandle_;

    float value_ = 1.0f; // 現在の表示比率(0.0～1.0)

    static constexpr float kBarWidth = 200.0f;
    static constexpr float kBarHeight = 10.0f;
    static constexpr float kBarPosX = 24.0f;
    static constexpr float kBarPosY = 64.0f;
};
