#pragma once

#include <2d/Sprite.h>
#include "base/TextureManager.h"

class Graph {
public:

    /// <summary>
    /// コンストラクタ
    /// </summary>
    Graph();

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~Graph();

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
    KamataEngine::Sprite* backBar_ = nullptr;   // 裏のグラフ（赤）
    KamataEngine::Sprite* frontBar_ = nullptr;  // 表のグラフ（緑）

    float value_;  // 現在の表示比率(0.0～1.0)
};

